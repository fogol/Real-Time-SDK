/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Map;

import com.refinitiv.ema.access.OmmBaseImpl.OmmImplState;
import com.refinitiv.ema.access.OmmCommonImpl.ImplementationType;
import com.refinitiv.ema.access.OmmLoggerClient.Severity;
import com.refinitiv.ema.access.OmmState.DataState;
import com.refinitiv.ema.access.OmmState.StatusCode;
import com.refinitiv.ema.access.OmmState.StreamState;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginAttribFlags;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginSupportFeaturesFlags;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelEventTypes;



class BaseSession<T>
{
	// Login section
	protected OmmBaseImpl<T>			_ommBaseImpl;
	protected ActiveConfig				_activeConfig;
	
	protected List<BaseSessionChannelInfo<T>> 	_sessionChannelList;
	protected List<BaseSessionChannelInfo<T>>  	_tempActionSessionChannelList;
	protected int                   	_numOfLoginOk = 0;
	protected int						_numOfLoginClose = 0;
	protected LoginRefresh 				_loginRefresh; // login refresh aggregation for ConsumerSession
	
	protected Buffer 					_rsslEncBuffer;
	
	protected boolean 					_sendInitialLoginRefresh;
	
	protected boolean					_enableSingleOpen = false;
	
	
	protected int 						_state = OmmBaseImpl.OmmImplState.NOT_INITIALIZED; /* This is used to handle the current state of OmmConsumer */
	
	protected State						_rsslState; /* This is used to set the state of login status message for the login stream */

	com.refinitiv.eta.codec.StatusMsg   _rsslStatusMsg; /* This is used to set the state of login status message for the login stream */
	
	protected StatusMsgImpl				_statusMsgImpl; /* This is used to set login status message for the login stream */
	
	protected OmmEventImpl<T>			_eventImpl;
	
	private int _currentRsslDataState;

	BaseSession(OmmBaseImpl<T> baseImpl)
	{
		_ommBaseImpl = baseImpl;
		_activeConfig = _ommBaseImpl.activeConfig();
		_sessionChannelList = new ArrayList<BaseSessionChannelInfo<T>>();
		
		_tempActionSessionChannelList = new ArrayList<BaseSessionChannelInfo<T>>();
		
		_loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();
		_loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
			
		/* Sets the current state of OmmBaseImpl */
		_state = baseImpl._state;
		
		_rsslState = CodecFactory.createState();
		_rsslStatusMsg = (com.refinitiv.eta.codec.StatusMsg)CodecFactory.createMsg();
		_statusMsgImpl = new StatusMsgImpl(_ommBaseImpl.objManager());
		
		_eventImpl = new OmmEventImpl<T>(baseImpl);
		
		_currentRsslDataState = DataStates.SUSPECT;
	}
	
	
	void ommImplState(int state)
	{
		_state = state;
	}
	
	int ommImplState()
	{
		return _state;
	}
	
	 OmmBaseImpl<T> ommBaseImpl()
	 {
		 return _ommBaseImpl;
	 }
	 
	void addSessionChannelInfo(BaseSessionChannelInfo<T> sessionChannelInfo)
	{
		if(!_sessionChannelList.contains(sessionChannelInfo))
		{
			_sessionChannelList.add(sessionChannelInfo);
		}
	}
	
	void removeSessionChannelInfo(BaseSessionChannelInfo<T> sessionChannelInfo)
	{
		if(_sessionChannelList.contains(sessionChannelInfo))
		{
			_sessionChannelList.remove(sessionChannelInfo);
		}
	}
	
	
	public void handleLoginReqTimeout()
	{
		if (_activeConfig.loginRequestTimeOut == 0)
		{
			while (ommImplState() < OmmImplState.LOGIN_STREAM_OPEN_OK && ommImplState() != OmmImplState.RSSLCHANNEL_UP_STREAM_NOT_OPEN
					&& ommImplState() != OmmImplState.RSSLCHANNEL_CLOSED)
				_ommBaseImpl.rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread);
			
			/* Throws OmmInvalidUsageException when EMA receives login reject from the data source. */
			if(ommImplState() == OmmImplState.RSSLCHANNEL_UP_STREAM_NOT_OPEN)
			{
				throw _ommBaseImpl.ommIUExcept().message(_ommBaseImpl._loginCallbackClient.loginFailureMessage(), OmmInvalidUsageException.ErrorCode.LOGIN_REQUEST_REJECTED);
			}
			
			if(numOfLoginOk() > 0)
			{
				BaseSessionChannelInfo<T> sessionChannelInfo = aggregateLoginResponse();
				
				_ommBaseImpl._loginCallbackClient.processRefreshMsg(null, sessionChannelInfo.reactorChannel(), loginRefresh());
				
				sendInitialLoginRefresh(true);
				
				checkLoginResponseAndCloseReactorChannel();
								
				return;
			}
			else
			{
				StringBuilder strBuilder = _ommBaseImpl.strBuilder().append("login failed (timed out after waiting ").append(_activeConfig.loginRequestTimeOut).append(" milliseconds) for ");
				int count = _activeConfig.configSessionChannelSet.size();
				for(ConsumerSessionChannelConfig  config : _activeConfig.configSessionChannelSet)
				{
					if(--count > 0)
						strBuilder.append(config.name + ", ");
					else
						strBuilder.append(config.name);
				}	
				

				String excepText = strBuilder.toString();
	
				if (_ommBaseImpl.loggerClient().isErrorEnabled())
					_ommBaseImpl.loggerClient().error(_ommBaseImpl.formatLogMessage(_activeConfig.instanceName, excepText, Severity.ERROR));
	
				throw _ommBaseImpl.ommIUExcept().message(excepText, OmmInvalidUsageException.ErrorCode.LOGIN_REQUEST_TIME_OUT);
			}
		}
		else
		{
			_ommBaseImpl.resetEventTimeout();
			TimeoutEvent timeoutEvent = _ommBaseImpl.addTimeoutEvent(_activeConfig.loginRequestTimeOut * 1000, _ommBaseImpl);
	
			while (!_ommBaseImpl.eventTimeout() && (ommImplState() < OmmImplState.LOGIN_STREAM_OPEN_OK) && (ommImplState() != OmmImplState.RSSLCHANNEL_UP_STREAM_NOT_OPEN)
					&& ommImplState() != OmmImplState.RSSLCHANNEL_CLOSED)
			{
				_ommBaseImpl.rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread);
			}
	
			if (_ommBaseImpl.eventTimeout())
			{
				if(numOfLoginOk() > 0 && isInitialLoginRefreshSent() == false)
				{
					BaseSessionChannelInfo<T> sessionChannelInfo = aggregateLoginResponse();
					
					_ommBaseImpl._loginCallbackClient.processRefreshMsg(null, sessionChannelInfo.reactorChannel(), loginRefresh());
				
					sendInitialLoginRefresh(true);
					
					checkLoginResponseAndCloseReactorChannel();
				
					return;
				}
				
				StringBuilder strBuilder = _ommBaseImpl.strBuilder().append("login failed (timed out after waiting ").append(_activeConfig.loginRequestTimeOut).append(" milliseconds) for ");
				int count = _activeConfig.configSessionChannelSet.size();
				for(ConsumerSessionChannelConfig  config : _activeConfig.configSessionChannelSet)
				{
					if(--count > 0)
						strBuilder.append(config.name + ", ");
					else
						strBuilder.append(config.name);
				}	
				

				String excepText = strBuilder.toString();
	
				if (_ommBaseImpl.loggerClient().isErrorEnabled())
					_ommBaseImpl.loggerClient().error(_ommBaseImpl.formatLogMessage(_activeConfig.instanceName, excepText, Severity.ERROR));
	
				throw _ommBaseImpl.ommIUExcept().message(excepText, OmmInvalidUsageException.ErrorCode.LOGIN_REQUEST_TIME_OUT);
			}
			else if (ommImplState() == OmmImplState.RSSLCHANNEL_CLOSED) /* Throws OmmInvalidUsageException when all session channels are down. */
			{
				timeoutEvent.cancel();
				
				StringBuilder strBuilder = _ommBaseImpl.strBuilder().append("login failed (timed out after waiting ").append(_activeConfig.loginRequestTimeOut).append(" milliseconds) for ");
				int count = _activeConfig.configSessionChannelSet.size();
				for(ConsumerSessionChannelConfig  config : _activeConfig.configSessionChannelSet)
				{
					if(--count > 0)
						strBuilder.append(config.name + ", ");
					else
						strBuilder.append(config.name);
				}
				
				String excepText = strBuilder.toString();
				
				if (_ommBaseImpl.loggerClient().isErrorEnabled())
					_ommBaseImpl.loggerClient().error(_ommBaseImpl.formatLogMessage(_activeConfig.instanceName, excepText, Severity.ERROR));
				
				throw _ommBaseImpl.ommIUExcept().message(excepText, OmmInvalidUsageException.ErrorCode.LOGIN_REQUEST_TIME_OUT);
			}
			else if (ommImplState() == OmmImplState.RSSLCHANNEL_UP_STREAM_NOT_OPEN) /* Throws OmmInvalidUsageException when EMA receives login reject from the data source. */
			{
				timeoutEvent.cancel();
				throw _ommBaseImpl.ommIUExcept().message(_ommBaseImpl._loginCallbackClient.loginFailureMessage(), OmmInvalidUsageException.ErrorCode.LOGIN_REQUEST_REJECTED);
			}
			else
			{
				timeoutEvent.cancel();
				
				/* This is used to notify login refresh after receiving source directory response */
				if(numOfLoginOk() > 0 && isInitialLoginRefreshSent() == false)
				{
					BaseSessionChannelInfo<T> sessionChannelInfo = aggregateLoginResponse();
					
					sendInitialLoginRefresh(true);
					
					_ommBaseImpl._loginCallbackClient.processRefreshMsg(null, sessionChannelInfo.reactorChannel(), loginRefresh());
					
					checkLoginResponseAndCloseReactorChannel();
				
					return;
				}
			}
		}
	}
	
	public List<BaseSessionChannelInfo<T>> sessionChannelList()
	{
		return _sessionChannelList;
	}
	
	void increaseNumOfLoginOk()
	{
		_numOfLoginOk++;
	}
	
	int numOfLoginOk()
	{
		return _numOfLoginOk;
	}
	
	void increaseNumOfLoginClose()
	{
		_numOfLoginClose++;
	}
	
	void sendInitialLoginRefresh(boolean vaue)
	{
		_sendInitialLoginRefresh = vaue;
	}
	
	boolean isInitialLoginRefreshSent()
	{
		return _sendInitialLoginRefresh;
	}
	
	protected List<BaseSessionChannelInfo<T>>  activeSessionChannelList()
	{
		_tempActionSessionChannelList.clear();
		
		BaseSessionChannelInfo<T> sessionChannelInfo;
		for(int index = 0; index < _sessionChannelList.size(); index++)
		{
			sessionChannelInfo = _sessionChannelList.get(index);
			
			if(sessionChannelInfo.reactorChannel() != null)
			{
				if(sessionChannelInfo.reactorChannel().state() == ReactorChannel.State.UP || 
						sessionChannelInfo.reactorChannel().state() == ReactorChannel.State.READY)
				{
					_tempActionSessionChannelList.add(sessionChannelInfo);
				}
			}
		}
		
		return _tempActionSessionChannelList;
	}

	public BaseSessionChannelInfo<T> aggregateLoginResponse()
	{
		List<BaseSessionChannelInfo<T>> activeChannelList = activeSessionChannelList();
		
		if(activeChannelList.size() == 0)
			return null;
		
		_loginRefresh.clear();
		BaseSessionChannelInfo<T> firstLoginResponse = null;
		
		
		int attribFlags = 0;
		int featuresFlags = 0;
		
		for(int index = 0; index < activeChannelList.size(); index++)
		{
			BaseSessionChannelInfo<T> sessionChannel = activeChannelList.get(index);
			
			if(sessionChannel.loginRefresh().flags() > 0)
			{
				if(firstLoginResponse == null)
				{
					attribFlags = sessionChannel.loginRefresh().attrib().flags();
					featuresFlags = sessionChannel.loginRefresh().features().flags();
					
					firstLoginResponse = sessionChannel;
				}
				else
				{
					attribFlags &= sessionChannel.loginRefresh().attrib().flags();
					featuresFlags &= sessionChannel.loginRefresh().features().flags();
				}
			}
		}
		
		
		
		/* Ensure that at least one channel receives the login refresh message from the provider */
		if(firstLoginResponse != null)
		{
			/* Copy the first login refresh message to _loginRefresh */
			firstLoginResponse.loginRefresh().copy(_loginRefresh);
			
			/* Aggregate login response attributes and features from all session channels, starting at index 1(since we've already copied the first login refresh */
			for(int index = 1; index < activeChannelList.size(); index++)
			{
				BaseSessionChannelInfo<T> sessionChannel = activeChannelList.get(index);
				
				if( (attribFlags & LoginAttribFlags.HAS_ALLOW_SUSPECT_DATA) != 0)
				{
					long allowSuspectData = _loginRefresh.attrib().allowSuspectData();
					allowSuspectData &= sessionChannel.loginRefresh().attrib().allowSuspectData();
					
					_loginRefresh.attrib().allowSuspectData(allowSuspectData);
				}
				
				if( (attribFlags & LoginAttribFlags.HAS_PROVIDE_PERM_EXPR) != 0)
				{
					long providePermExpr = _loginRefresh.attrib().providePermissionExpressions();
					providePermExpr &= sessionChannel.loginRefresh().attrib().providePermissionExpressions();
					
					_loginRefresh.attrib().providePermissionExpressions(providePermExpr);
				}
				
				if( (attribFlags & LoginAttribFlags.HAS_PROVIDE_PERM_PROFILE) != 0)
				{
					long providePermProfile = _loginRefresh.attrib().providePermissionProfile();
					providePermProfile &= sessionChannel.loginRefresh().attrib().providePermissionProfile();
					
					_loginRefresh.attrib().providePermissionProfile(providePermProfile);
				}
				
	
				if( (attribFlags & LoginAttribFlags.HAS_CONSUMER_SUPPORT_RTT) != 0)
				{
					long supportRTT = _loginRefresh.attrib().supportRTTMonitoring();
					supportRTT &= sessionChannel.loginRefresh().attrib().supportRTTMonitoring();
				
					_loginRefresh.attrib().supportRTTMonitoring(supportRTT);
				}
				
				if((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REQUESTS) != 0)
				{
					long batchRequests = _loginRefresh.features().supportBatchRequests();
					batchRequests &= sessionChannel.loginRefresh().features().supportBatchRequests();
					
					_loginRefresh.features().supportBatchRequests(batchRequests);
				}
				
				if((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_POST) != 0)
				{
					long supportPost = _loginRefresh.features().supportOMMPost();
					supportPost &= sessionChannel.loginRefresh().features().supportOMMPost();
					
					_loginRefresh.features().supportOMMPost(supportPost);
				}
				
				if((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_OPT_PAUSE) != 0)
				{
					long supportOPTPause = _loginRefresh.features().supportOptimizedPauseResume();
					supportOPTPause &= sessionChannel.loginRefresh().features().supportOptimizedPauseResume();
					
					_loginRefresh.features().supportOptimizedPauseResume(supportOPTPause);
				}
				
				if((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_VIEW) != 0)
				{
					long supportView = _loginRefresh.features().supportViewRequests();
					supportView &= sessionChannel.loginRefresh().features().supportViewRequests();
					
					_loginRefresh.features().supportViewRequests(supportView);
				}
				
				if((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REISSUES) != 0)
				{
					long batchReissue = _loginRefresh.features().supportBatchReissues();
					batchReissue &= sessionChannel.loginRefresh().features().supportBatchReissues();
					
					_loginRefresh.features().supportBatchReissues(batchReissue);
				}
				
				if((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_CLOSES) != 0)
				{
					long batchClose = _loginRefresh.features().supportBatchCloses();
					batchClose &= sessionChannel.loginRefresh().features().supportBatchCloses();
					
					_loginRefresh.features().supportBatchCloses(batchClose);
				}
				
				if((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_ENH_SL) != 0)
				{
					long supportEHN_SL = _loginRefresh.features().supportEnhancedSymbolList();
					supportEHN_SL &= sessionChannel.loginRefresh().features().supportEnhancedSymbolList();
				
					_loginRefresh.features().supportEnhancedSymbolList(supportEHN_SL);
				}
				
				// This is for NiProv session functionality.  If one login supports it, report that the login supports it in aggregate.
				if((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD) != 0)
				{
					long supportPROV_DDL = _loginRefresh.features().supportProviderDictionaryDownload();
					supportPROV_DDL |= sessionChannel.loginRefresh().features().supportProviderDictionaryDownload();
				
					_loginRefresh.features().supportProviderDictionaryDownload(supportPROV_DDL);
				}
			}
			
			/* Makes sure that only supported attributes and features are provided to users */
			attribFlags &= (LoginAttribFlags.HAS_ALLOW_SUSPECT_DATA | LoginAttribFlags.HAS_PROVIDE_PERM_EXPR | LoginAttribFlags.HAS_PROVIDE_PERM_PROFILE | LoginAttribFlags.HAS_CONSUMER_SUPPORT_RTT);
			featuresFlags &= (LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REQUESTS | LoginSupportFeaturesFlags.HAS_SUPPORT_POST | LoginSupportFeaturesFlags.HAS_SUPPORT_OPT_PAUSE |
					LoginSupportFeaturesFlags.HAS_SUPPORT_VIEW| LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REISSUES | LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_CLOSES | LoginSupportFeaturesFlags.HAS_SUPPORT_ENH_SL | LoginSupportFeaturesFlags.HAS_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD);
			
			/* Always enables the single open and allow suspect data flags */
			if(_ommBaseImpl.implType() == ImplementationType.CONSUMER)
			{
				attribFlags |= (LoginAttribFlags.HAS_SINGLE_OPEN | LoginAttribFlags.HAS_ALLOW_SUSPECT_DATA);
				_loginRefresh.attrib().flags(attribFlags);
				_loginRefresh.features().flags(featuresFlags);
			}
			
			/* Always enable the single open feature */
			if(_ommBaseImpl.implType() == ImplementationType.CONSUMER && _enableSingleOpen)	
			{
				/* Overrides the single open as it is handled by EMA */
				_loginRefresh.attrib().allowSuspectData(1);
				_loginRefresh.attrib().singleOpen(1);
			}
		}
		
		return firstLoginResponse;
	}
	
	public LoginRefresh loginRefresh()
	{
		return _loginRefresh;
	}
		
	
	Buffer encodedBuffer()
	{
		return _rsslEncBuffer;
	}
	
	public void enableSingleOpen(boolean enableSingleOpen)
	{
		_enableSingleOpen = enableSingleOpen;
		
	}

	/* This is used to check and close a ReactorChannel which doesn't provide a login response in time for the handleLoginReqTimeout() method */
	public void checkLoginResponseAndCloseReactorChannel()
	{
		/* Closes channels if it doesn't receive a login response */
		if( (_numOfLoginOk + _numOfLoginClose) <  _sessionChannelList.size() )
		{
			List<BaseSessionChannelInfo<T>> removeChannelList = new ArrayList<BaseSessionChannelInfo<T>>();
			
			for(int i = 0; i < _sessionChannelList.size(); i++)
			{
				BaseSessionChannelInfo<T> sessionChannelInfo = _sessionChannelList.get(i);
				
				if(!sessionChannelInfo.receivedLoginRefresh())
				{
					removeChannelList.add(sessionChannelInfo);
				}
			}
			
			for (int index = removeChannelList.size() -1; index >= 0; index--)
			{
				_ommBaseImpl.closeSessionChannel(removeChannelList.get(index));
			}
		}
	}
	
	public boolean checkAllSessionChannelHasState(int state)
	{
		int result = state;
		for(int i = 0; i < _sessionChannelList.size(); i++)
		{
			BaseSessionChannelInfo<T> sessionChannelInfo = _sessionChannelList.get(i);
			
			result &= sessionChannelInfo.state();
		}
		
		return result == state ? true: false;
	}
	
	public boolean checkAtLeastOneSessionChannelHasState(int state)
	{
		for(int i = 0; i < _sessionChannelList.size(); i++)
		{
			BaseSessionChannelInfo<T> sessionChannelInfo = _sessionChannelList.get(i);
			
			if( (sessionChannelInfo.state() & state) == state)
				return true;
		}
	
		return false;
	}
	
	void populateStatusMsg()
	{		
		_rsslStatusMsg.clear();
		_rsslStatusMsg.msgClass(MsgClasses.STATUS);
		_rsslStatusMsg.streamId(1);
		_rsslStatusMsg.domainType(DomainTypes.LOGIN);
		_rsslStatusMsg.applyHasMsgKey();
		MsgKey msgKey = _rsslStatusMsg.msgKey();
		msgKey.clear();
		
		if(_loginRefresh.checkHasUserNameType())
		{
			msgKey.applyHasNameType();
			msgKey.nameType(_loginRefresh.userNameType());
		}
		
		if(_loginRefresh.checkHasUserName())
		{
			msgKey.applyHasName();
			msgKey.name(_loginRefresh.userName());
		}
	}
	
	void handleLoginStreamForChannelDown(List<LoginItem<T>>	loginItemList, ReactorChannel reactorChannel, int sessionListSize)
	{
		/* Update the aggregated login response as this session channel is not ready. */
		aggregateLoginResponse();
		
		if (loginItemList == null)
			return;
		
		/* Checks whether this is the last channel being closed */
		boolean closedStream = sessionListSize == 1 ? true : false;
		
		populateStatusMsg();
		
		if(closedStream)
		{
			_rsslState.streamState(StreamState.CLOSED);
			
			_ommBaseImpl.ommImplState(OmmImplState.RSSLCHANNEL_CLOSED);
		}
		else
			_rsslState.streamState(StreamState.OPEN);
		
		
		if(_sendInitialLoginRefresh)
		{
			if(checkConnectionsIsDown())
			{
				_rsslState.dataState(DataState.SUSPECT);
			}
			else
			{
				if(checkUserStillLogin())
					_rsslState.dataState(DataState.OK);
				else
					_rsslState.dataState(DataState.SUSPECT);
			}
			
		}
		else
		{
			_rsslState.dataState(DataState.SUSPECT);
		}

		_currentRsslDataState = _rsslState.dataState();

		_rsslState.code(StateCodes.NONE);
		_rsslState.text().data("session channel closed");
		_rsslStatusMsg.state(_rsslState);
		_rsslStatusMsg.applyHasState();
		
		_statusMsgImpl.decode(_rsslStatusMsg, reactorChannel.majorVersion(), reactorChannel.minorVersion(), null);
		
		for (int idx = 0; idx < loginItemList.size(); ++idx)
		{
			_eventImpl._item = loginItemList.get(idx);
			_eventImpl._channel = reactorChannel;
			
			if(_ommBaseImpl.implType() == OmmCommonImpl.ImplementationType.CONSUMER)
			{
				((OmmConsumerClient)_eventImpl._item.client()).onAllMsg(_statusMsgImpl, _eventImpl);
				((OmmConsumerClient) _eventImpl._item.client()).onStatusMsg(_statusMsgImpl, _eventImpl);
			}
			else
			{
				((OmmProviderClient)_eventImpl._item.client()).onAllMsg(_statusMsgImpl, _eventImpl);
				((OmmProviderClient) _eventImpl._item.client()).onStatusMsg(_statusMsgImpl, _eventImpl);
			}
		}
	}
	
	public boolean checkConnectionsIsDown()
	{
		for(BaseSessionChannelInfo<T> entry : _sessionChannelList)
		{
			if(entry.reactorChannel().state() == ReactorChannel.State.UP ||
					entry.reactorChannel().state() ==  ReactorChannel.State.READY)
	        {
	            return false;
	        }
	    }

		return true;
	}
		
	public boolean checkUserStillLogin()
	{
		for(BaseSessionChannelInfo<T> entry : _sessionChannelList)
		{
			if(entry.loginRefresh().state().streamState() == StreamState.OPEN &&
					entry.loginRefresh().state().dataState() == DataState.OK)
	        {
	            return true;
	        }
	    }

		return false;
	}
	
	public void processChannelEvent(BaseSessionChannelInfo<T> sessionChannelInfo, ReactorChannelEvent event)
	{
		List<LoginItem<T>>	loginItemList = _ommBaseImpl.loginCallbackClient().loginItemList();
		
		_rsslState.clear();
		
		switch ( event.eventType() )
		{
		case ReactorChannelEventTypes.CHANNEL_UP:
			
			if(_sendInitialLoginRefresh)
				return;
			
			sessionChannelInfo.sendChannelUp = true;
			
			if (loginItemList == null)
				return;
			
			populateStatusMsg();
			
			_rsslState.streamState(StreamState.OPEN);
			_rsslState.dataState(DataState.SUSPECT);
			_rsslState.code(StateCodes.NONE);
			_rsslState.text().data("session channel up");
			_rsslStatusMsg.state(_rsslState);
			_rsslStatusMsg.applyHasState();

			_currentRsslDataState = _rsslState.dataState();

			_statusMsgImpl.decode(_rsslStatusMsg, event.reactorChannel().majorVersion(), event.reactorChannel().minorVersion(), null);
			
			for (int idx = 0; idx < loginItemList.size(); ++idx)
			{
				_eventImpl._item = loginItemList.get(idx);
				_eventImpl._channel = event.reactorChannel();
				
				if(_ommBaseImpl.implType() == OmmCommonImpl.ImplementationType.CONSUMER)
				{
					((OmmConsumerClient)_eventImpl._item.client()).onAllMsg(_statusMsgImpl, _eventImpl);
					((OmmConsumerClient) _eventImpl._item.client()).onStatusMsg(_statusMsgImpl, _eventImpl);
				}
				else
				{
					((OmmProviderClient)_eventImpl._item.client()).onAllMsg(_statusMsgImpl, _eventImpl);
					((OmmProviderClient) _eventImpl._item.client()).onStatusMsg(_statusMsgImpl, _eventImpl);
				}
			}
			
			break;
		
		case ReactorChannelEventTypes.CHANNEL_READY:
			
			if(sessionChannelInfo.sendChannelUp || loginItemList == null)
				return;
			
			populateStatusMsg();
			
			_rsslState.streamState(StreamState.OPEN);
			
			if(_sendInitialLoginRefresh == true)
				_rsslState.dataState(DataState.OK);
			else
				_rsslState.dataState(DataState.SUSPECT);

			_currentRsslDataState = _rsslState.dataState();

			_rsslState.code(StateCodes.NONE);
			_rsslState.text().data("session channel up");
			_rsslStatusMsg.state(_rsslState);
			_rsslStatusMsg.applyHasState();
			
			_statusMsgImpl.decode(_rsslStatusMsg, event.reactorChannel().majorVersion(), event.reactorChannel().minorVersion(), null);
			
			for (int idx = 0; idx < loginItemList.size(); ++idx)
			{
				_eventImpl._item = loginItemList.get(idx);
				_eventImpl._channel = event.reactorChannel();
				
				if(_ommBaseImpl.implType() == OmmCommonImpl.ImplementationType.CONSUMER)
				{
					((OmmConsumerClient)_eventImpl._item.client()).onAllMsg(_statusMsgImpl, _eventImpl);
					((OmmConsumerClient) _eventImpl._item.client()).onStatusMsg(_statusMsgImpl, _eventImpl);
				}
				else
				{
					((OmmProviderClient)_eventImpl._item.client()).onAllMsg(_statusMsgImpl, _eventImpl);
					((OmmProviderClient) _eventImpl._item.client()).onStatusMsg(_statusMsgImpl, _eventImpl);
				}
			}
			
			sessionChannelInfo.sendChannelUp = true;
			
			break;
			
		  case ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING:
			  
			sessionChannelInfo.sendChannelUp = false;
			
			if (loginItemList == null)
				return;
			
			populateStatusMsg();
			
			_rsslState.streamState(StreamState.OPEN);
			
			/* Update the aggregated login response as this session channel is not ready. */
			aggregateLoginResponse();
			
			
			if(_sendInitialLoginRefresh)
			{
				if(checkConnectionsIsDown())
				{
					_rsslState.dataState(DataState.SUSPECT);
				}
				else
				{
					if(checkUserStillLogin())
					{
						_rsslState.dataState(DataState.OK);
					}
					else
					{
						_rsslState.dataState(DataState.SUSPECT);
					}
				}
			}
			else
			{
				_rsslState.dataState(DataState.SUSPECT);
			}

			_currentRsslDataState = _rsslState.dataState();

			_rsslState.code(StateCodes.NONE);
			_rsslState.text().data("session channel down reconnecting");
			_rsslStatusMsg.state(_rsslState);
			_rsslStatusMsg.applyHasState();
			
			_statusMsgImpl.decode(_rsslStatusMsg, event.reactorChannel().majorVersion(), event.reactorChannel().minorVersion(), null);
			
			for (int idx = 0; idx < loginItemList.size(); ++idx)
			{
				_eventImpl._item = loginItemList.get(idx);
				_eventImpl._channel = event.reactorChannel();
				
				if(_ommBaseImpl.implType() == OmmCommonImpl.ImplementationType.CONSUMER)
				{
					((OmmConsumerClient)_eventImpl._item.client()).onAllMsg(_statusMsgImpl, _eventImpl);
					((OmmConsumerClient) _eventImpl._item.client()).onStatusMsg(_statusMsgImpl, _eventImpl);
				}
				else
				{
					((OmmProviderClient)_eventImpl._item.client()).onAllMsg(_statusMsgImpl, _eventImpl);
					((OmmProviderClient) _eventImpl._item.client()).onStatusMsg(_statusMsgImpl, _eventImpl);
				}
			}
			
			break;
		case ReactorChannelEventTypes.CHANNEL_DOWN:
			
			sessionChannelInfo.sendChannelUp = false;
			
			handleLoginStreamForChannelDown(loginItemList, event.reactorChannel(), _sessionChannelList.size());
			
			break;
		case ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK:
			
			if (loginItemList == null)
				return;

			populateStatusMsg();

			_rsslState.streamState(StreamState.OPEN);
			_rsslState.dataState(_currentRsslDataState);

			_rsslState.code(OmmState.StatusCode.PREFERRED_HOST_START_FALLBACK);
			_rsslState.text().data("preferred host starting fallback");
			_rsslStatusMsg.state(_rsslState);
			_rsslStatusMsg.applyHasState();

			_statusMsgImpl.decode(_rsslStatusMsg, event.reactorChannel().majorVersion(), event.reactorChannel().minorVersion(), null);

			for (int idx = 0; idx < loginItemList.size(); ++idx)
			{
				_eventImpl._item = loginItemList.get(idx);
				_eventImpl._channel = event.reactorChannel();

				if(_ommBaseImpl.implType() == OmmCommonImpl.ImplementationType.CONSUMER)
				{
					((OmmConsumerClient)_eventImpl._item.client()).onAllMsg(_statusMsgImpl, _eventImpl);
					((OmmConsumerClient) _eventImpl._item.client()).onStatusMsg(_statusMsgImpl, _eventImpl);
				}
				else
				{
					((OmmProviderClient)_eventImpl._item.client()).onAllMsg(_statusMsgImpl, _eventImpl);
					((OmmProviderClient) _eventImpl._item.client()).onStatusMsg(_statusMsgImpl, _eventImpl);
				}
			}

			break;
		case ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE:
			
			if (loginItemList == null)
				return;

			populateStatusMsg();

			_rsslState.streamState(StreamState.OPEN);
			_rsslState.dataState(_currentRsslDataState);

			_rsslState.code(OmmState.StatusCode.PREFERRED_HOST_COMPLETE);
			_rsslState.text().data("preferred host complete");
			_rsslStatusMsg.state(_rsslState);
			_rsslStatusMsg.applyHasState();

			_statusMsgImpl.decode(_rsslStatusMsg, event.reactorChannel().majorVersion(), event.reactorChannel().minorVersion(), null);

			for (int idx = 0; idx < loginItemList.size(); ++idx)
			{
				_eventImpl._item = loginItemList.get(idx);
				_eventImpl._channel = event.reactorChannel();

				if(_ommBaseImpl.implType() == OmmCommonImpl.ImplementationType.CONSUMER)
				{
					((OmmConsumerClient)_eventImpl._item.client()).onAllMsg(_statusMsgImpl, _eventImpl);
					((OmmConsumerClient) _eventImpl._item.client()).onStatusMsg(_statusMsgImpl, _eventImpl);
				}
				else
				{
					((OmmProviderClient)_eventImpl._item.client()).onAllMsg(_statusMsgImpl, _eventImpl);
					((OmmProviderClient) _eventImpl._item.client()).onStatusMsg(_statusMsgImpl, _eventImpl);
				}
			}
			break;
		case ReactorChannelEventTypes.PREFERRED_HOST_NO_FALLBACK:
			
			if (loginItemList == null)
				return;

			populateStatusMsg();

			_rsslState.streamState(StreamState.OPEN);
			_rsslState.dataState(_currentRsslDataState);

			_rsslState.code(OmmState.StatusCode.PREFERRED_HOST_NO_FALLBACK);
			_rsslState.text().data("preferred no fallback");
			_rsslStatusMsg.state(_rsslState);
			_rsslStatusMsg.applyHasState();

			_statusMsgImpl.decode(_rsslStatusMsg, event.reactorChannel().majorVersion(), event.reactorChannel().minorVersion(), null);

			for (int idx = 0; idx < loginItemList.size(); ++idx)
			{
				_eventImpl._item = loginItemList.get(idx);
				_eventImpl._channel = event.reactorChannel();

				if(_ommBaseImpl.implType() == OmmCommonImpl.ImplementationType.CONSUMER)
				{
					((OmmConsumerClient)_eventImpl._item.client()).onAllMsg(_statusMsgImpl, _eventImpl);
					((OmmConsumerClient) _eventImpl._item.client()).onStatusMsg(_statusMsgImpl, _eventImpl);
				}
				else
				{
					((OmmProviderClient)_eventImpl._item.client()).onAllMsg(_statusMsgImpl, _eventImpl);
					((OmmProviderClient) _eventImpl._item.client()).onStatusMsg(_statusMsgImpl, _eventImpl);
				}
			}
			break;
		default:
			break;
		}
	}
	

	public boolean hasActiveChannel()
	{
		for(BaseSessionChannelInfo<T> sessionChannelInfo : _sessionChannelList)
		{
			if(sessionChannelInfo.reactorChannel() != null)
			{
				ReactorChannel.State state  = sessionChannelInfo.reactorChannel().state();
				if (state == ReactorChannel.State.READY || state == ReactorChannel.State.UP)
				{
					return true;
				}
			}
		}
		
		return false;
	}
	
	public void currentLoginDataState(int dataState)
	{
		_currentRsslDataState = dataState;	
	}
}
