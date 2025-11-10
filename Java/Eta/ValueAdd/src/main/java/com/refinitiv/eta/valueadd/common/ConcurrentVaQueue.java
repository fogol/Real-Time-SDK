/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.common;

import java.util.concurrent.locks.ReentrantLock;

public class ConcurrentVaQueue implements ValueAddQueue
{
    ReentrantLock _lock = new ReentrantLock();
    ValueAddQueue _queue;

    public ConcurrentVaQueue()
    {
        _queue = new VaQueue();
    }

    public ConcurrentVaQueue(ValueAddQueue queue)
    {
        _queue = queue;
    }

    @Override
    public void add(VaNode node)
    {
        _lock.lock();
        try
        {
            _queue.add(node);
        }
        finally
        {
            _lock.unlock();
        }
    }

    @Override
    public VaNode poll()
    {
        _lock.lock();
        try
        {
            return _queue.poll();
        }
        finally
        {
            _lock.unlock();
        }
    }

    @Override
    public VaNode peek()
    {
        _lock.lock();
        try
        {
            return _queue.peek();
        }
        finally
        {
            _lock.unlock();
        }
    }

    @Override
    public boolean remove(VaNode node)
    {
        _lock.lock();
        try
        {
            return _queue.remove(node);
        }
        finally
        {
            _lock.unlock();
        }
    }

    @Override
    public int size()
    {
        _lock.lock();
        try
        {
            return _queue.size();
        }
        finally
        {
            _lock.unlock();
        }
    }

    @Override
    public void setLimit(int newLimit)
    {
        _lock.lock();
        try
        {
            _queue.setLimit(newLimit);
        }
        finally
        {
            _lock.unlock();
        }
    }
}
