/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Concurrent;

namespace LSEG.Ema.PerfTools.ConsPerf;

public class Pool<T>
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
