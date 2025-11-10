/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2021,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.eta.valueadd.common.LimitedVaPool;

import java.util.concurrent.locks.ReentrantLock;

class ServerPool
{
	private boolean _intialized = false;
	private LimitedVaPool _clientSessionPool = new LimitedVaPool(true);
	private LimitedVaPool _itemInfoPool = new LimitedVaPool(true);
	private static long CLIENT_HANDLE = 0;
	private static long ITEM_HANDLE = 0;

	static ReentrantLock _clientHandleLock = new java.util.concurrent.locks.ReentrantLock();
	static ReentrantLock _itemLock = new java.util.concurrent.locks.ReentrantLock();
	
	void initialize(OmmServerBaseImpl ommServerBaseImpl,
						   int clientSession,
						   int itemInfo,
						   int clientSessionPoolLimit,
						   int itemInfoPoolLimit)
	{
		if ( _intialized )
			return;

		setItemInfoPoolLimit(itemInfoPoolLimit);
		setClientSessionPoolLimit(clientSessionPoolLimit);

		if (clientSessionPoolLimit > 0) clientSession = Math.min(clientSession, clientSessionPoolLimit);

		for (int index = 0; index < clientSession ; ++index)
		{
			_clientSessionPool.add(new ClientSession(ommServerBaseImpl));
		}

		int numberOfItemInfo = clientSession * itemInfo;
		if (itemInfoPoolLimit > 0) numberOfItemInfo = Math.min(numberOfItemInfo, itemInfoPoolLimit);
		
		for (int index = 0; index < numberOfItemInfo ; ++index)
		{
			_itemInfoPool.add(new ItemInfo());
		}
		
		_intialized = true;
	}

	void setClientSessionPoolLimit(int limit)
	{
		if (limit > 0 || limit == -1) _clientSessionPool.setLimit(limit);
	}

	void setItemInfoPoolLimit(int limit)
	{
		if (limit > 0 || limit == -1) _itemInfoPool.setLimit(limit);
	}
	
	ClientSession getClientSession(OmmServerBaseImpl ommServerBaseImpl)
	{
		ClientSession clientSession = (ClientSession)_clientSessionPool.poll();
		if( clientSession == null )
		{
			clientSession = new ClientSession(ommServerBaseImpl);
			_clientSessionPool.updatePool(clientSession);
		}
		else
		{
			clientSession.clear();
			clientSession.setOmmServerBaseImpl(ommServerBaseImpl);
		}

		return clientSession;
	}
	
	ItemInfo getItemInfo()
	{
		ItemInfo itemInfo = (ItemInfo)_itemInfoPool.poll();
		
		if( itemInfo == null )
		{
			itemInfo = new ItemInfo();
			_itemInfoPool.updatePool(itemInfo);
		}
		else
		{
			itemInfo.clear();
		}

		return itemInfo;
	}
	
	static long getClientHandle()
	{
		_clientHandleLock.lock();
		++CLIENT_HANDLE;
		_clientHandleLock.unlock();
		
		return CLIENT_HANDLE;
	}
	
	static long getItemHandle()
	{
		_itemLock.lock();
		++ITEM_HANDLE;
		_itemLock.unlock();
		
		return ITEM_HANDLE;
	}
}
