/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

import com.refinitiv.eta.valueadd.reactor.ReactorChannelEvent;

class ScheduleCloseNiProviderSessionChannel<T> implements TimeoutClient
{
	private NiProviderSession<T>			_niProviderSession;
	private NiProviderSessionChannelInfo<T>	_niProviderSessionChannelInfo;
	private ReentrantLock _userLock;
		
	ScheduleCloseNiProviderSessionChannel(NiProviderSession<T> niProviderSession, NiProviderSessionChannelInfo<T> channelInfo)
	{
		_niProviderSession = niProviderSession;
		_userLock = _niProviderSession.ommBaseImpl().userLock();
		_niProviderSessionChannelInfo = channelInfo;
	}

	@Override
	public void handleTimeoutEvent() 
	{	
		if(_niProviderSessionChannelInfo != null)
		{
			/* Keep the current size before SessionChannelInfo is removed from closeSessionChannel() */
			int size = _niProviderSession.sessionChannelList().size();
			
			/* Closes ReactorChannel and removes SessionChannelInfo */
			_niProviderSession.ommBaseImpl().closeSessionChannelOnly(_niProviderSessionChannelInfo);
			
			_niProviderSession.handleLoginStreamForChannelDown(_niProviderSession.ommBaseImpl().loginCallbackClient().loginItemList(), _niProviderSessionChannelInfo.reactorChannel(), size);
			
			_niProviderSession.ommBaseImpl()._channelCallbackClient.removeSessionChannel(_niProviderSessionChannelInfo);
			
			_niProviderSessionChannelInfo = null;
		}
		
		_niProviderSession = null;
	}

	@Override
	public ReentrantLock userLock() {
		return _userLock;
	}
}

class NiProviderSession<T> extends BaseSession<T>
{
	private static final String CLIENT_NAME = "NiProviderSession";
	private List<NiProvSessionTransportBuffer> _sessionTransportBufferList;
	
	boolean _clearedItemList;			// This is set to true when all items are cleared, either initially or when removeItems() is called due to the all channels going down
	
	NiProviderSession(OmmBaseImpl<T> baseImpl)
	{
		super(baseImpl);
		_clearedItemList = true;
		_sessionTransportBufferList = new ArrayList<NiProvSessionTransportBuffer>();
	}
	
	public boolean clearedItemList()
	{
		return _clearedItemList;
	}
	
	public void clearedItemList(boolean cleared)
	{
		_clearedItemList = cleared;
	}
	
	public void dispatch()
	{
		// noOp for now
	}
	
	public List<NiProvSessionTransportBuffer> sessionTransportBufferList()
	{
		return _sessionTransportBufferList;
	}
	
	@Override
	public void processChannelEvent(BaseSessionChannelInfo<T> sessionChannelInfo, ReactorChannelEvent event)
	{
		super.processChannelEvent(sessionChannelInfo, event);
	}
}