/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.niprovider.series500.ex510_MP_Session_FileCfg;

import java.util.ArrayList;
import java.util.List;

import com.refinitiv.ema.access.ChannelInformation;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmInvalidUsageException;
import com.refinitiv.ema.access.OmmNiProviderConfig;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmProviderClient;
import com.refinitiv.ema.access.OmmProviderEvent;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.PostMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;

class AppClient implements OmmProviderClient
{
	List<ChannelInformation> channelInList = new ArrayList<ChannelInformation>();
	
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent event)
	{
		//APIQA
		if (refreshMsg.domainType() == 1)
		{
			System.out.println(refreshMsg + "\nevent session info (refresh)\n");
			printSessionInfo(event);
		}
		else
		{
			System.out.println( refreshMsg + "\nevent channel info (refresh)\n" + event.channelInformation() );
		}
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmProviderEvent event) 
	{
		//APIQA
		if (updateMsg.domainType() == 1)
		{
			System.out.println(updateMsg + "\nevent session info (update)\n");
			printSessionInfo(event);
		}
		else
		{
			System.out.println( updateMsg + "\nevent channel info (update)\n" + event.channelInformation() );
		}
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event) 
	{
		//APIQA
		if (statusMsg.domainType() == 1)
		{
			System.out.println(statusMsg + "\nevent session info (status)\n");
			printSessionInfo(event);
		}
		else
		{
			System.out.println( statusMsg + "\nevent channel info (status)\n" + event.channelInformation() );
		}
	}
	
	void printSessionInfo(OmmProviderEvent event)
	{
		event.sessionChannelInfo(channelInList);
		
		for(ChannelInformation channelInfo : channelInList) 
		{
			System.out.println(channelInfo);
		}
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent providerEvent) {}
	public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent) {}
	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onAllMsg(Msg msg, OmmProviderEvent providerEvent) {}
}

public class NiProvider {
	//API QA
	static void printHelp()
	{

	    System.out.println("\nOptions:\n" + "  -?\tShows this usage\n" 
	    		+ "  if the application will attempt to make an http or encrypted \n "
	    		+ "           connection, ChannelType must need to be set to ChannelType::RSSL_HTTP \n"
	    		+ "            or ChannelType::RSSL_ENCRYPTED in EMA configuration file.\n"
	    		+ "  -ph Proxy host name.\n"
	    		+ "  -pp Proxy port number.\n"
	    		+ "  -plogin User name on proxy server.\n"
	    		+ "  -ppasswd Password on proxy server.\n" 
	    		+ "  -pdomain Proxy Domain.\n"
	    		+ "  -krbfile Proxy KRB file.\n" 
	    		+ "  -keyfile keystore file for encryption.\n"
	    		+ "  -keypasswd keystore password for encryption.\n"
	    		+ "  -spTLSv1.2 Enable TLS 1.2 security protocol. Default enables both TLS 1.2 and TLS 1.3 (optional). \n"
	    		+ "  -spTLSv1.3 Enable TLS 1.3 security protocol. Default enables both TLS 1.2 and TLS 1.3 (optional). \n"
				+ "  -securityProvider Specify security provider for encrypted connection that is going to be used \n"
				+ "\t(SunJSSE and Conscrypt options are currently supported).\n"
	    		+ "\n");
	}
	static boolean readCommandlineArgs(String[] args, OmmNiProviderConfig config)
	{
		    try
		    {
		        int argsCount = 0;
		        boolean tls12 = false;
		        boolean tls13 = false;

		        while (argsCount < args.length)
		        {
		            if (0 == args[argsCount].compareTo("-?"))
		            {
		                printHelp();
		                return false;
		            }
	    			else if ("-keyfile".equals(args[argsCount]))
	    			{
	    				config.tunnelingKeyStoreFile(argsCount < (args.length-1) ? args[++argsCount] : null);
	    				++argsCount;				
	    			}	
	    			else if ("-keypasswd".equals(args[argsCount]))
	    			{
	    				config.tunnelingKeyStorePasswd(argsCount < (args.length-1) ? args[++argsCount] : null);
	    				++argsCount;				
	    			}
					else if ("-securityProvider".equals(args[argsCount]))
					{
						config.tunnelingSecurityProvider(argsCount < (args.length-1) ? args[++argsCount] : null);
						++argsCount;
					}
	    			else if ("-ph".equals(args[argsCount]))
	    			{
	    				config.tunnelingProxyHostName(args[++argsCount]);
	    				++argsCount;				
	    			}	
	    			else if ("-pp".equals(args[argsCount]))
	    			{
	    				config.tunnelingProxyPort(argsCount < (args.length-1) ? args[++argsCount] : null);
	    				++argsCount;				
	    			}			
	    			else if ("-plogin".equals(args[argsCount]))
	    			{
	    				config.tunnelingCredentialUserName(argsCount < (args.length-1) ? args[++argsCount] : null);
	    				++argsCount;				
	    			}	
	    			else if ("-ppasswd".equals(args[argsCount]))
	    			{
	    				config.tunnelingCredentialPasswd(argsCount < (args.length-1) ? args[++argsCount] : null);
	    				++argsCount;				
	    			}			
	    			else if ("-pdomain".equals(args[argsCount]))
	    			{
	    				config.tunnelingCredentialDomain(argsCount < (args.length-1) ? args[++argsCount] : null);
	    				++argsCount;				
	    			}	
	    			else if ("-krbfile".equals(args[argsCount]))
	    			{
	    				config.tunnelingCredentialKRB5ConfigFile(argsCount < (args.length-1) ? args[++argsCount] : null);
	    				++argsCount;				
	    			}	
	    			else if ("-spTLSv1.2".equals(args[argsCount]))	   
	    			{
	    				tls12 = true;
	    				++argsCount;
	    			}
	    			else if ("-spTLSv1.3".equals(args[argsCount]))
	    			{
	    				tls13 = true;
	    				++argsCount;
	    			}
	    			else // unrecognized command line argument
	    			{
	    				printHelp();
	    				return false;
	    			}			
	    		}
		        
		        // Set security protocol versions of TLS based on configured values, with default having TLS 1.2 and 1.3 enabled
		        if ((tls12 && tls13) || (!tls12 && !tls13))
		        {
		        	config.tunnelingSecurityProtocol("TLS");
		        	config.tunnelingSecurityProtocolVersions(new String[] {"1.2", "1.3"});
		        }
		        else if (tls12)
		        {
		        	config.tunnelingSecurityProtocol("TLS");
		        	config.tunnelingSecurityProtocolVersions(new String[]{"1.2"});
		        }
		        else if (tls13)
		        {
		        	config.tunnelingSecurityProtocol("TLS");
		        	config.tunnelingSecurityProtocolVersions(new String[]{"1.3"});
		        }
	        }
	        catch (Exception e)
	        {
	        	printHelp();
	            return false;
	        }
			
			return true;
	}

	//END API QA

	public static void main(String[] args)
	{
		OmmProvider provider = null;
		AppClient appClient = new AppClient();
		try
		{
			OmmNiProviderConfig config = EmaFactory.createOmmNiProviderConfig();
			//API QA
			if (!readCommandlineArgs(args, config))
		                return;
			//END API QA
			
			provider = EmaFactory.createOmmProvider(config.providerName("Provider_Session").username("apiqa"), appClient);
			
			long ibmHandle = 5;
			long triHandle = 6;
			
			FieldList fieldList = EmaFactory.createFieldList();
			
			fieldList.add( EmaFactory.createFieldEntry().real(22, 14400, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 14700, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("TEST_NI_PUB").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), ibmHandle);
			
			fieldList.clear();
			
			fieldList.add( EmaFactory.createFieldEntry().real(22, 4100, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 4200, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 20,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 40, OmmReal.MagnitudeType.EXPONENT_0));
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("TEST_NI_PUB").name("TRI.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), triHandle);
			
			for( int i = 0; i < 60; i++ )
			{
				Thread.sleep(1000);
				try
				{
					fieldList.clear();
					fieldList.add(EmaFactory.createFieldEntry().real(22, 14400 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
					fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
					
					provider.submit( EmaFactory.createUpdateMsg().serviceName("TEST_NI_PUB").name("IBM.N").payload( fieldList ), ibmHandle );
					
					fieldList.clear();
					fieldList.add(EmaFactory.createFieldEntry().real(22, 4100 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
					fieldList.add(EmaFactory.createFieldEntry().real(30, 21 + i, OmmReal.MagnitudeType.EXPONENT_0));
					
					provider.submit( EmaFactory.createUpdateMsg().serviceName("TEST_NI_PUB").name("TRI.N").payload( fieldList ), triHandle );
				}
				catch(OmmInvalidUsageException excp) 
				{
					// There is no active channel currently associated with this session, so just continue until the channels are back up.
					if(excp.errorCode() == OmmInvalidUsageException.ErrorCode.NO_ACTIVE_CHANNEL)
					{
						continue;
					}
					else
					{
						System.out.println(excp.getMessage());
						break;
					}
				}
			}
		} 
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
		}
		finally 
		{
			if (provider != null) provider.uninitialize();
		}
	}
}
