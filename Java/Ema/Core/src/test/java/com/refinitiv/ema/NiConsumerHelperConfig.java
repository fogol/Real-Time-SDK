///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2025 LSEG. All rights reserved.                   --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema;

import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;

public class NiConsumerHelperConfig {
	
	public boolean acceptLogin = true;
	public boolean acceptConnection = true;
	public LoginRefresh loginRefresh = null;
	public String port = "14003";
	public boolean acceptJson = true;
	
	public NiConsumerHelperConfig()
	{
		loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();
		loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
		loginRefresh.state().dataState(DataStates.OK);
		loginRefresh.state().streamState(StreamStates.OPEN);
		loginRefresh.state().code(StateCodes.NONE);
		loginRefresh.applyHasFeatures();
		loginRefresh.applySolicited();
		loginRefresh.applyHasAttrib();
		loginRefresh.attrib().applyHasApplicationName();
		loginRefresh.attrib().applicationName().data("NiConsumer");
		loginRefresh.attrib().applyHasApplicationId();
		loginRefresh.attrib().applicationId().data("100");
		loginRefresh.features().applyHasSupportProviderDictionaryDownload();
		loginRefresh.features().supportProviderDictionaryDownload(1);
		
	}
	
	public void copy(NiConsumerHelperConfig destConfig)
	{
		destConfig.acceptLogin = acceptLogin;
		destConfig.acceptConnection = acceptConnection;
		destConfig.port = port;
		destConfig.acceptJson = acceptJson;
		loginRefresh.copy(destConfig.loginRefresh);		
	}
	

}
