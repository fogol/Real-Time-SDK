using System.Collections.Concurrent;

using LSEG.Ema.Access;

namespace LSEG.Ema.PerfTools.ConsPerf;

public class MessageQueueThread
{
    private volatile bool _exit;
    private volatile bool _exitAck;
    private readonly ConcurrentQueue<MsgWrapper> _messageQueue;
    private readonly Pool<MsgWrapper<RefreshMsg>> _refreshMsgPool;
    private readonly Pool<MsgWrapper<UpdateMsg>> _updateMsgPool;
    private readonly Pool<MsgWrapper<StatusMsg>> _statusMsgPool;
    private readonly IMsgProcessor _msgProcessor;

    public MessageQueueThread(
        ConcurrentQueue<MsgWrapper> messageQueue,
        Pool<MsgWrapper<RefreshMsg>> refreshMsgPool,
        Pool<MsgWrapper<UpdateMsg>> updateMsgPool,
        Pool<MsgWrapper<StatusMsg>> statusMsgPool,
        IMsgProcessor msgProcessor)
    {
        _messageQueue = messageQueue;
        _refreshMsgPool = refreshMsgPool;
        _updateMsgPool = updateMsgPool;
        _statusMsgPool = statusMsgPool;
        _msgProcessor = msgProcessor;
    }

    public void Shutdown()
    {
        _exit = true;
        var endTime = DateTime.Now.AddSeconds(5);
        while (!_exitAck && DateTime.Now < endTime)
        {
            Thread.Sleep(TimeSpan.FromMilliseconds(500));
        }
    }

    public void Run()
    {
        while (!_exit)
        {
            if (_messageQueue.TryDequeue(out var messageWrapper))
            {
                switch (messageWrapper.BaseMsg.DataType)
                {
                    case DataType.DataTypes.REFRESH_MSG:
                        var refreshMsgWrapper = (MsgWrapper<RefreshMsg>)messageWrapper;

                        _msgProcessor.ProcessRefreshMsg(refreshMsgWrapper.Msg, refreshMsgWrapper.Handle, refreshMsgWrapper.Closure);

                        _refreshMsgPool.Return(refreshMsgWrapper);

                        break;
                    case DataType.DataTypes.UPDATE_MSG:
                        var updateMsgWrapper = (MsgWrapper<UpdateMsg>)messageWrapper;

                        _msgProcessor.ProcessUpdateMsg(updateMsgWrapper.Msg);

                        _updateMsgPool.Return(updateMsgWrapper);

                        break;
                    case DataType.DataTypes.STATUS_MSG:
                        var statusMsgWrapper = (MsgWrapper<StatusMsg>)messageWrapper;

                        _msgProcessor.ProcessStatusMsg(statusMsgWrapper.Msg, statusMsgWrapper.Closure);

                        _statusMsgPool.Return(statusMsgWrapper);

                        break;
                }
            }
            else
            {
                Thread.Sleep(TimeSpan.FromMilliseconds(1));
            }
        }
        Console.WriteLine($"MessageQueueThread {Thread.CurrentThread.ManagedThreadId} exited.");
        _exitAck = true;
    }
}