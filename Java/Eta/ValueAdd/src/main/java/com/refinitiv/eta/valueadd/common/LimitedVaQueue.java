/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.common;

public class LimitedVaQueue extends VaQueue implements ValueAddQueue
{
    int _limit = -1;

    @Override
    public void add(VaNode node)
    {
        if (checkPoolLimit(_limit))
        {
            super.add(node);
        }
    }

    public void setLimit(int newLimit)
    {
        _limit = newLimit;
        if (newLimit > 0 && _size > newLimit)
        {
            int diff = _size - newLimit;
            VaNode node = _head;
            if (diff > newLimit)
            {
                for (int i = 0; i < newLimit - 1; i++) node = node.next();
                _tail = node;
                node.next(null);
            }
            else
            {
                for (int i = 0; i < diff - 1; i++) node = node.next();
                _head = node.next();
                node.next(null);
            }
            _size = newLimit;
        }
    }

    private boolean checkPoolLimit(int limit)
    {
        return limit < 0 || _size < limit;
    }
}
