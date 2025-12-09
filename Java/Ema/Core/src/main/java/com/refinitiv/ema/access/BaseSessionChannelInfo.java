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

import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;

class BaseSessionChannelInfo<T>
{	

	protected BaseSessionChannelConfig _sessionChannelConfig;
	
	protected List<ChannelInfo> _channelInfoList;
	
	protected BaseSession<T> _baseSession;
	
	protected boolean _receivedLoginRefresh;
	
	protected ReactorChannel _reactorChannel;
	
	protected LoginRefresh _loginRefresh; // Stores login refresh for this session channel
	
	protected OmmBaseImpl<T> _baseImpl;
	
	/* This is used to keep the state of this connection for administrative domains from OmmBaseImpl.OmmImplState
	   when initializing OmmConsumer instance */
	protected int								_state;

	public boolean sendChannelUp;
	
	private boolean _phOperationInProgress;
		
	BaseSessionChannelInfo(BaseSessionChannelConfig sessionChannelConfig, BaseSession<T> baseSession)
	{
		
		_sessionChannelConfig = sessionChannelConfig;
		
		_channelInfoList = new ArrayList<ChannelInfo>(sessionChannelConfig.configChannelSet.size());
		
		_baseSession = baseSession;
		
		_loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();
		_loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
		
		_baseImpl = baseSession.ommBaseImpl();
		
		_state = OmmBaseImpl.OmmImplState.REACTOR_INITIALIZED;
		
	}
	
	BaseSession<T> baseSession()
	{
		return _baseSession;
	}
	
	BaseSessionChannelConfig sessionChannelConfig()
	{
		return _sessionChannelConfig;
	}
	
	List<ChannelInfo> channelInfoList()
	{
		return _channelInfoList;
	}
	
	int state()
	{
		return _state;
	}
	
	void state(int state)
	{
		_state = state;
	}
	
	/* This is used to indicate whether the current session receives login refresh */
	void receivedLoginRefresh(boolean receivedLoginRefresh)
	{
		_receivedLoginRefresh = receivedLoginRefresh;
	}
	
	boolean receivedLoginRefresh()
	{
		return _receivedLoginRefresh;
	}

	void reactorChannel(ReactorChannel reactorChannel) 
	{
		_reactorChannel = reactorChannel;
	}
	
	ReactorChannel reactorChannel()
	{
		return _reactorChannel;
	}
	
	LoginRefresh loginRefresh()
	{
		return _loginRefresh;
	}
	
	// Stub method used by Consumer
	public void onChannelClose(ChannelInfo channelInfo)
	{
	}
	
	// Stub method used by Consumer
	public void close()
	{
	}
	boolean phOperationInProgress() { return _phOperationInProgress; }

	void phOperationInProgress(boolean value) { _phOperationInProgress = value; }
}
