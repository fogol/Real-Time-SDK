/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Concurrent;
using System.Threading;
using System.Threading.Tasks;

using LSEG.Ema.Access;

namespace LSEG.Ema.Example.Traning.Consumer;

class Pool<T>
{
    private readonly ConcurrentQueue<T> _storage = new();
    private readonly Func<T> _ctor;
    private readonly int _limit;

    public Pool(Func<T> ctor, int initialSize = 0, int limit = int.MaxValue)
    {
        _ctor = ctor;
        _limit = limit;
        for (int i = 0; i < initialSize; i++)
        {
            _storage.Enqueue(_ctor());
        }
    }

    public T Get()
    {
        if (_storage.TryDequeue(out var result))
            return result;
        return _ctor();
    }

    public void Return(T value)
    {
        if (_storage.Count < _limit)
            _storage.Enqueue(value);
    }
}

class AppClient : IOmmConsumerClient
{
    private readonly ConcurrentQueue<Msg> _messageQueue;
    private readonly Pool<RefreshMsg> _refreshMsgPool;
    private readonly Pool<UpdateMsg> _updateMsgPool;
    private readonly Pool<StatusMsg> _statusMsgPool;

    private Task _executor;
    private CancellationTokenSource _exitCts;

    public AppClient(int updatePoolSize, int updateBufferSize)
    {
        _messageQueue = new();
        _refreshMsgPool = new(() => new RefreshMsg());
        _updateMsgPool = new(() => new UpdateMsg(updateBufferSize), initialSize: updatePoolSize, limit: updatePoolSize);
        _statusMsgPool = new(() => new StatusMsg());
        _exitCts = new CancellationTokenSource();

        _executor = Task.Run(RunAsync);
    }

    public async Task ExitAsync(TimeSpan timeout)
    {
        _exitCts.Cancel();
        await Task.WhenAny(_executor, Task.Delay(timeout));
    }

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
    {
        RefreshMsg copyRefreshMsg = _refreshMsgPool.Get();
        refreshMsg.Copy(copyRefreshMsg);
        _messageQueue.Enqueue(copyRefreshMsg);
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event)
    {
        UpdateMsg copyUpdateMsg = _updateMsgPool.Get();
        updateMsg.Copy(copyUpdateMsg);
        _messageQueue.Enqueue(copyUpdateMsg);
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event)
    {
        StatusMsg copyStatusMsg = _statusMsgPool.Get();
        statusMsg.Copy(copyStatusMsg);
        _messageQueue.Enqueue(copyStatusMsg);
    }

    private async Task RunAsync()
    {
        while (!_exitCts.IsCancellationRequested)
        {
            if (_messageQueue.TryDequeue(out var queuedMessage))
            {
                Console.WriteLine("Message Type: " + DataType.AsString(queuedMessage.DataType));
                Console.WriteLine("Item Name: " + (queuedMessage.HasName ? queuedMessage.Name() : "<not set>"));

                switch (queuedMessage)
                {
                    case RefreshMsg refreshMsg:
                        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));
                        Console.WriteLine("Item State: " + refreshMsg.State());

                        if (DataType.DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
                            Console.WriteLine(refreshMsg.Payload().FieldList());

                        _refreshMsgPool.Return(refreshMsg.Clear());

                        break;
                    case UpdateMsg updateMsg:
                        Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));

                        if (DataType.DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
                            Console.WriteLine(updateMsg.Payload().FieldList());

                        // Checks to limit the size of the pool
                        _updateMsgPool.Return(updateMsg.Clear());

                        break;
                    case StatusMsg statusMsg:
                        Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

                        if (statusMsg.HasState)
                            Console.WriteLine("Item State: " + statusMsg.State());

                        _statusMsgPool.Return(statusMsg.Clear());

                        break;
                }
            }
            else
            {
                await Task.Delay(TimeSpan.FromMilliseconds(1), _exitCts.Token); // Waits for a message from the message queue to avoid CPU peak.
            }
        }
    }
}

public class Consumer
{
    public static async Task Main(string[] args)
    {
        OmmConsumer? consumer = null;

        AppClient appClient = new(updatePoolSize: 300, updateBufferSize: 2048); // Specifies number of the UpdateMsg object in the pool for reuse

        try
        {
            consumer = new OmmConsumer(new OmmConsumerConfig().Host("localhost:14002").UserName("user"));

            consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Name("LSEG.L"), appClient);

            await Task.Delay(TimeSpan.FromMinutes(1));            // API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
        }
        catch (Exception e)
        {
            Console.WriteLine(e.Message);
        }
        finally
        {
            await appClient.ExitAsync(timeout: TimeSpan.FromSeconds(5));
            consumer?.Uninitialize();
        }
    }
}
