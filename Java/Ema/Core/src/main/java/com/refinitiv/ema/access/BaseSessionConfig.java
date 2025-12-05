///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|              Copyright (C) 2024 LSEG. All rights reserved.                --
///*|-----------------------------------------------------------------------------


package com.refinitiv.ema.access;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.refinitiv.eta.valueadd.reactor.ReactorConnectOptions;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;

// This class represents the base channel config, containing the common configuration elements for a Session Channel
// used for both a Consumer Request Routing configuration and the NiProvider Session.
class BaseSessionChannelConfig {
	
	String					name;
	List<ChannelConfig>		configChannelSet;
	int reconnectAttemptLimit;
	int reconnectMinDelay;
	int reconnectMaxDelay;
	
	private ReactorConnectOptions 		_rsslReactorConnOptions = ReactorFactory.createReactorConnectOptions();
	
	BaseSessionChannelConfig(String name)
    {
        this.name = name;
        clear();
    }
	
	ReactorConnectOptions connectOptions()
	{
		return _rsslReactorConnOptions;
	}

    void clear()
    {
		 reconnectAttemptLimit = ActiveConfig.DEFAULT_RECONNECT_ATTEMPT_LIMIT;
		 reconnectMinDelay = ActiveConfig.DEFAULT_RECONNECT_MIN_DELAY;
		 reconnectMaxDelay = ActiveConfig.DEFAULT_RECONNECT_MAX_DELAY;
    	
    	if(configChannelSet != null)
    		configChannelSet.clear();
    	else
    		configChannelSet = new ArrayList<ChannelConfig>();	
    }

}
