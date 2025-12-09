///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2025 LSEG. All rights reserved.                   --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema;

import com.refinitiv.ema.access.ChannelInformation.ProtocolType;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.CopyMsgFlags;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.json.converter.ConversionResults;
import com.refinitiv.eta.json.converter.ConverterFactory;
import com.refinitiv.eta.json.converter.DecodeJsonMsgOptions;
import com.refinitiv.eta.json.converter.GetJsonMsgOptions;
import com.refinitiv.eta.json.converter.JsonConverter;
import com.refinitiv.eta.json.converter.JsonConverterBuilder;
import com.refinitiv.eta.json.converter.JsonConverterError;
import com.refinitiv.eta.json.converter.JsonConverterProperties;
import com.refinitiv.eta.json.converter.JsonMsg;
import com.refinitiv.eta.json.converter.JsonMsgClasses;
import com.refinitiv.eta.json.converter.JsonProtocol;
import com.refinitiv.eta.json.converter.ParseJsonOptions;
import com.refinitiv.eta.json.converter.RWFToJsonOptions;
import com.refinitiv.eta.json.converter.ServiceNameIdConverter;
import com.refinitiv.eta.rdm.Dictionary;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.*;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;

import java.io.IOException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.ArrayDeque;
import java.util.Iterator;
import java.util.Set;
import java.util.concurrent.locks.ReentrantLock;

/* This assumes that Transport.initialize() has already been called.
 * This class implements an ETA-based Non-Interactive Consumer/ADH Simulator that will do the following:
 * 1. Create a listening socket
 * 2. Upon an incoming connection, accept the channel, and send whatever configured Login Refresh is set
 * 3. Whenever a message comes in, decode it and cache it into a locked queue
 * 4. Disconnect whenever triggered by the test.
 * 
 */

public class NiConsumerHelper implements Runnable, ServiceNameIdConverter {
	
	public Server server = null;
	public Channel channel = null;
	public Thread thread;
	boolean initialized = false;
	
	boolean shutdownThread = false;
	
	public NiConsumerHelperConfig config = new NiConsumerHelperConfig();
	
	// ETA constructs for sending data, accepting, etc.
	private Selector _selector;
	private AcceptOptions _acceptOpts = TransportFactory.createAcceptOptions();
	private InProgInfo _inProgInfo = TransportFactory.createInProgInfo();
	private ReadArgs _readArgs = TransportFactory.createReadArgs();
	private WriteArgs _writeArgs = TransportFactory.createWriteArgs();
	
	private DecodeIterator _decodeIter = CodecFactory.createDecodeIterator();
	private DecodeIterator _jsonDecodeIter = CodecFactory.createDecodeIterator();
	private EncodeIterator _encodeIter = CodecFactory.createEncodeIterator();
	
	private Msg _msg = CodecFactory.createMsg();
	private Error _error = TransportFactory.createError();
	
	private ReentrantLock _userLock = new java.util.concurrent.locks.ReentrantLock();

	private ArrayDeque<Msg> _messageQueue = new ArrayDeque<Msg>();
	
	private LoginRequest _loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
	
	private boolean _closeChannel;
	
	DataDictionary dict = CodecFactory.createDataDictionary();
	
	private String dictionaryLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmNiProviderTests/TestRDMFieldDictionary";
	
    private JsonConverter converter = null;
    private DecodeJsonMsgOptions decodeJsonMsgOptions = ConverterFactory.createDecodeJsonMsgOptions();
    private RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
    private ConversionResults conversionResults = ConverterFactory.createConversionResults();
    private ParseJsonOptions parseJsonOptions = ConverterFactory.createParseJsonOptions();
    private JsonMsg jsonMsg = ConverterFactory.createJsonMsg();
    private GetJsonMsgOptions getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();
    private JsonConverterError converterError = ConverterFactory.createJsonConverterError();
    
    //   {\"Type\":\"Pong\"}
    private static final byte[] WS_JSON_PONG_MESSAGE = new byte[]{
            0X7B, 0X22, 0X54, 0X79, 0X70, 0X65, 0X22, 0X3A, 0X22, 0X50, 0X6F, 0X6E, 0X67, 0X22, 0X7D
    };
    
    public int serviceNameToId(String serviceName, JsonConverterError error)
    {
    	if(serviceName == "TEST_NI_PUB")
    		return 11;
    	if(serviceName == "NI_PUB_1")
    		return 2;
    	if(serviceName == "NI_PUB_2")
    		return 3;
    	
    	return -1;
    }

    public String serviceIdToName(int id, JsonConverterError error)
    {
    	if(id == 11)
    		return "TEST_NI_PUB"; 
    	if(id == 2)
    		return "NI_PUB_1"; 
    	if(id == 3)
    		return "NI_PUB_2"; 
    	
    	return null;
    }


	/* Constructor.  This will bind the server, set up the selector, and this class' thread.
	 * 	 NOTE: Transport.initialize() is required to be called prior to running this. 
	 */
	public NiConsumerHelper(NiConsumerHelperConfig niConsConfig)
	{
		niConsConfig.copy(config);
		
		
		BindOptions bindOpts = TransportFactory.createBindOptions();
		bindOpts.guaranteedOutputBuffers(500);
		bindOpts.majorVersion(Codec.majorVersion());
		bindOpts.minorVersion(Codec.minorVersion());
		bindOpts.protocolType(Codec.protocolType());
		bindOpts.connectionType(ConnectionTypes.SOCKET);
		bindOpts.wSocketOpts().protocols("rssl.json.v2");
		bindOpts.serviceName(config.port);
		bindOpts.serverBlocking(false);
		
		if(dict.loadFieldDictionary(dictionaryLocation, _error) != CodecReturnCodes.SUCCESS)
		{
			throw new IllegalArgumentException("Unable to load dictionary. Error text: " + _error.text());
		}
		
		server = Transport.bind(bindOpts, _error);
		
		if(server == null)
		{
			throw new IllegalArgumentException("Bind failed. Error text: " + _error.text());
		}
		
		try
		{
		    _selector = Selector.open();
		    server.selectableChannel().register(_selector, SelectionKey.OP_ACCEPT, server);
		} catch (Exception exception)
        {
			throw new IllegalArgumentException("Selector failed. Error text: " + exception.getMessage());
        } 
		
		// Set direct write so we don't need to worry about 
		_writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
		
		thread = new Thread(this);
		thread.start();
	}
	
	/* Run method for the NiConsumer.  This is a simple one channel connection, and any additional channels will be automatically rejected
	 *  
	 */
	public void run() 
	{
		Set<SelectionKey> keySet = null;
		int ret;
		
		while(shutdownThread != true)
		{
			
			// Remove the selectablechannel and close the ETA channel
			if(_closeChannel && channel != null)
			{
				_selector.selectedKeys().remove(channel.selectableChannel().keyFor(_selector));
				channel.close(_error);
				channel = null;
				_closeChannel = false;
			}
			
			keySet = null;
			try
			{
				// Select for 100 miliseconds.
				int selRet = _selector.select(100); 
				if (selRet > 0)
				{
				    keySet = _selector.selectedKeys();
				}
				else if(selRet < 0 && channel != null)
				{
					// protection in case it breaks.
					_selector.selectedKeys().remove(channel.selectableChannel().keyFor(_selector));
				}
				else if(shutdownThread)
				{
					return;
				}
			}
			catch (IOException e)
			{
			    System.out.println("Selector error. Info: " + e.getMessage());
			    shutdownThread = true;
			    return;
			}
			
			if(keySet != null)
			{
				Iterator<SelectionKey> iter = keySet.iterator();
				
				while (iter.hasNext())
				{
					SelectionKey key = iter.next();
					iter.remove();
					if(!key.isValid())
					{
						key.cancel();
						continue;
					}
					if (key.isAcceptable())
					{
						_userLock.lock();
						// New connection, accept it if we don't currently have an active channel.
						_acceptOpts.channelReadLocking(true);
						_acceptOpts.channelWriteLocking(true);
						_acceptOpts.nakMount(false);
						
						if(channel != null)
						{
							System.out.println("New connection on channel for NiConsumer server that already has a channel, Rejecting.");
							_acceptOpts.nakMount(true);
							// Nakmount true means no channel.
							server.accept(_acceptOpts, _error);
						}
						else
						{
							channel = server.accept(_acceptOpts, _error);
							
							if(channel == null)
							{
								System.out.println("Accept error. Info: " + _error.text());
								shutdownInternal();
								return;
							}
							else
							{
								try
								{
									channel.selectableChannel().register(_selector, SelectionKey.OP_WRITE, channel);								
								} catch (Exception exception)
								{
									System.out.println("Selector exception. Info: " + exception.getMessage());
									shutdownInternal();
									shutdownThread = true;
									return;
								}
							}
						}
						_userLock.unlock();
					}
					else if (key.isReadable())
					{
						_userLock.lock();
						// Make sure that the channel is registered for reading
						try
						{
							channel.selectableChannel().register(_selector, SelectionKey.OP_READ, channel);								
						} catch (Exception exception)
						{
							System.out.println("Selector exception. Info: " + exception.getMessage());
							shutdownInternal();
							shutdownThread = true;
							_userLock.unlock();
							return;
						}
						
						// Initialize the channel
						if(channel.state() == ChannelState.INITIALIZING )
						{
							ret = channel.init(_inProgInfo, _error);
							
							switch(ret)
							{
								case  TransportReturnCodes.SUCCESS:
									// Channel is up, we just need to continue now.
									System.out.println("Channel for port " + config.port +" Is up");
									if(channel.protocolType() == ProtocolType.JSON)
									{
										if(converter == null)
										{
											JsonConverterBuilder converterBuilder = ConverterFactory.createJsonConverterBuilder();
											converter = converterBuilder.setDictionary(dict)
									                .setServiceConverter(this)
									                .setProperty(JsonConverterProperties.JSON_CPC_DEFAULT_SERVICE_ID, 2)
									                .setProperty(JsonConverterProperties.JSON_CPC_EXPAND_ENUM_FIELDS, false)
									                .setProperty(JsonConverterProperties.JSON_CPC_USE_DEFAULT_DYNAMIC_QOS, true)
									                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, true)
									                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_FIDS, true)
									                .build(converterError);
											
											if(converter == null)
											{
												System.out.println("JSON Converter could not be initialized.  Error info: " + converterError.getText());
												shutdownInternal();
												shutdownThread = true;
												return;
											}
										}
									}
									
									break;
								case TransportReturnCodes.CHAN_INIT_IN_PROGRESS:
									if (_inProgInfo.flags() == InProgFlags.SCKT_CHNL_CHANGE)
					                {
										SelectionKey oldKey = _inProgInfo.oldSelectableChannel().keyFor(_selector);
										oldKey.cancel();
										
										try {
											_inProgInfo.newSelectableChannel().register(_selector, SelectionKey.OP_READ, channel);
										} catch (ClosedChannelException exception) 
										{
											System.out.println("Selector error on FD change. Info: " + exception.getMessage());
											shutdownInternal();
											shutdownThread = true;
											_userLock.unlock();
											return;
										}
					                }
									break;
								default:
									System.out.println("channel init error. Info: " + _error.text());
									shutdownInternal();
									shutdownThread = true;
									_userLock.unlock();
									return;
							}
						}
						// Read from the channel
						else if(channel.state() == ChannelState.ACTIVE )
						{
							TransportBuffer readBuffer;
							
							do
							{
								readBuffer = channel.read(_readArgs, _error);
								if(readBuffer == null)
								{
									switch(_readArgs.readRetVal())
									{
										case TransportReturnCodes.READ_PING:
											// Send a ping in response... this is to avoid any timer issues
											if(channel.ping(_error) < TransportReturnCodes.SUCCESS)
											{
												System.out.println("Ping error. Info: " + _error.text());
												shutdownInternal();
												shutdownThread = true;
												_userLock.unlock();
												return;
											}
											break;
										case TransportReturnCodes.READ_FD_CHANGE:
											SelectionKey oldKey = channel.oldSelectableChannel().keyFor(_selector);
											oldKey.cancel();
											
											try {
												channel.selectableChannel().register(_selector, SelectionKey.OP_READ, channel);
											} catch (ClosedChannelException exception) 
											{
												System.out.println("Selector error on read FD change. Info: " + exception.getMessage());
												shutdownInternal();
												shutdownThread = true;
												_userLock.unlock();
												return;
											}
											break;
										default:
											// This may not be catastrophic for the test, so remove the channel from the selector and close the channel
											System.out.println("channel read error on port: " + config.port + ". Info: " + _error.text());
											channel.selectableChannel().keyFor(_selector).cancel();
											channel.close(_error);
											channel = null;
									}
								}
								else
								{
									if(channel.protocolType() == ProtocolType.JSON)
									{
										parseJsonOptions.clear();
								        parseJsonOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);
								        converterError.clear();
								        ret = converter.parseJsonBuffer(readBuffer, parseJsonOptions, converterError);

								        if (converterError.isFailed()) 
								        {
								        	System.out.println("Json parse error.  Text: " + converterError.getText());
											shutdownInternal();
											shutdownThread = true;
											_userLock.unlock();
											return;
								        }

								        do
								        {
									        jsonMsg.clear();
									        decodeJsonMsgOptions.clear();
									        decodeJsonMsgOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
									        ret = converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, converterError);
									        
									        if(ret == CodecReturnCodes.FAILURE)
									        {
									        	System.out.println("Json conversion error.  Text: " + converterError.getText());
												shutdownInternal();
												shutdownThread = true;
												_userLock.unlock();
												return;
									        }
									        
									        switch (jsonMsg.jsonMsgClass()) 
									        {
								                case JsonMsgClasses.RSSL_MESSAGE:
										        
											        if(readMsg((TransportBuffer)jsonMsg.rwfMsg().encodedMsgBuffer()) == -1)
											        {
											        	shutdownInternal();
														shutdownThread = true;
														_userLock.unlock();
														return;
											        }
											        break;
								                case JsonMsgClasses.PING: 
								                    final TransportBuffer pingBuff =
								                            channel.getBuffer(WS_JSON_PONG_MESSAGE.length, false, _error);
								                    pingBuff.data().put(WS_JSON_PONG_MESSAGE);
								                    
								                    // This should always go out because we're writing a fairly small packet here with direct write enabled
													ret = channel.write(pingBuff, _writeArgs, _error);
													if(ret != TransportReturnCodes.SUCCESS)
													{
														System.out.println("Failed to write a pong message. Error text: " + _error.text());
														shutdownInternal();
														shutdownThread = true;
														_userLock.unlock();
														return;
													}
													else if(ret > 0) 
													{
														channel.flush(_error);
													}
								                    break;
								                case JsonMsgClasses.PONG: {
								                	// Send a ping in response... this is to avoid any timer issues
													if(channel.ping(_error) < TransportReturnCodes.SUCCESS)
													{
														System.out.println("Ping error. Info: " + _error.text());
														shutdownInternal();
														shutdownThread = true;
														_userLock.unlock();
														return;
													}
								                    break;
								                }
								                default: 
								                {
								                	System.out.println("Received JSON error message: " + jsonMsg.jsonMsgData().toString());
													shutdownInternal();
													shutdownThread = true;
													_userLock.unlock();
													return;
								                }
									        }
								        } while(ret != CodecReturnCodes.END_OF_CONTAINER);
									}
									else if(readMsg(readBuffer) == -1)
									{
										shutdownInternal();
										shutdownThread = true;
										_userLock.unlock();
										return;
									}
								}
							}
							while(_readArgs.readRetVal() > TransportReturnCodes.SUCCESS);
						}
						_userLock.unlock();
					}
					else if (key.isWritable())
					{
						_userLock.lock();
						// This means initialization is complete, so register for reading.
						try
						{
							channel.selectableChannel().register(_selector, SelectionKey.OP_READ, channel);								
						} catch (Exception exception)
						{
							System.out.println("Selector exception. Info: " + exception.getMessage());
							shutdownInternal();
							shutdownThread = true;
							_userLock.unlock();
							return;
						}
						
						_userLock.unlock();
					}
					
				}
			}
		}
	}
	
	// Parse the incoming message, handle any logins and dictionary requests, and queue up the message into the message queue
	private int readMsg(TransportBuffer readBuffer)
	{
		
		int ret;
		_decodeIter.clear();
		
		_decodeIter.setBufferAndRWFVersion(readBuffer, channel.majorVersion(), channel.minorVersion());
		
		ret = _msg.decode(_decodeIter);
		if(ret != CodecReturnCodes.SUCCESS) 
		{
			System.out.println("RequestMsg.decode() failed with return code: " + CodecReturnCodes.toString(ret));
			return -1;
		}
				
		switch(_msg.domainType())
		{
			case DomainTypes.LOGIN:
				// This should always be either a close or a request
				switch(_msg.msgClass())
				{
				case MsgClasses.REQUEST:
					_loginRequest.clear();
					_loginRequest.rdmMsgType(LoginMsgType.REQUEST);
					// Decode and cache the request so we have it, and send configured refresh back
					ret = _loginRequest.decode(_decodeIter, _msg);
					if(ret != CodecReturnCodes.SUCCESS) 
					{
						System.out.println("RequestMsg.decode() failed with return code: " + CodecReturnCodes.toString(ret));
						return -1;
					}
					
					TransportBuffer writeBuff = channel.getBuffer(1500, false, _error);
					
					if(writeBuff == null)
					{
						System.out.println("Failed to get buffer for login response. Error text: " + _error.text());
						return -1;
					}
					
					_encodeIter.clear();
					_encodeIter.setBufferAndRWFVersion(writeBuff, channel.majorVersion(), channel.minorVersion());
					// Set the username to whatever was sent by the niprov.
					if(config.acceptLogin == true)
					{
						config.loginRefresh.streamId(_loginRequest.streamId());
						config.loginRefresh.userName().data(_loginRequest.userName().data());
						config.loginRefresh.encode(_encodeIter);
					}
					else
					{
						StatusMsg rejectStatus = (StatusMsg)CodecFactory.createMsg();
						rejectStatus.clear();
						rejectStatus.msgClass(MsgClasses.STATUS);
						rejectStatus.domainType(DomainTypes.LOGIN);
						rejectStatus.streamId(_loginRequest.streamId());
						rejectStatus.applyHasState();
						rejectStatus.state().streamState(StreamStates.CLOSED);
						rejectStatus.state().dataState(DataStates.SUSPECT);
						rejectStatus.state().code(StateCodes.NOT_ENTITLED);
						rejectStatus.encode(_encodeIter);
					}
					
					if(channel.protocolType() == ProtocolType.JSON)
			        {
			        	
				        jsonMsg.clear();
				        _jsonDecodeIter.clear();
				        _jsonDecodeIter.setBufferAndRWFVersion(writeBuff, channel.majorVersion(), channel.minorVersion());

				        final Msg rwfMsg = jsonMsg.rwfMsg();
				        ret = rwfMsg.decode(_jsonDecodeIter);
				        if (ret == CodecReturnCodes.SUCCESS) 
				        {
				            rwfToJsonOptions.clear();
				            rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
				            converter.convertRWFToJson(rwfMsg, rwfToJsonOptions, conversionResults, converterError);
				            if (converterError.isFailed()) {
				            	System.out.println("Failed to convert login response to Json response. Error text: " + _error.text());
								return -1;
				            }
				            
				            /* Releases the original buffer if success */
				            channel.releaseBuffer(writeBuff, _error);
				            
				            writeBuff = channel.getBuffer(1500, false, _error);
							
							if(writeBuff == null)
							{
								System.out.println("Failed to get buffer for login response after conversion. Error text: " + _error.text());
								return -1;
							}
							
							getJsonMsgOptions.clear();
							getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
							getJsonMsgOptions.isCloseMsg(false);
							
							if(converter.getJsonBuffer(writeBuff, getJsonMsgOptions, converterError) != CodecReturnCodes.SUCCESS)
							{
								System.out.println("Failed to get the JSON login for dictionary response after conversion. Error text: " + _error.text());
								return -1;
							}
				        }
			        }
					
					// This should always go out because we're writing a fairly small packet here with direct write enabled
					ret = channel.write(writeBuff, _writeArgs, _error);
					if(ret != TransportReturnCodes.SUCCESS)
					{
						System.out.println("Failed to write the login response. Error text: " + _error.text());
						return -1;
					}
					else if(ret > 0) 
					{
						channel.flush(_error);
					}
					
					System.out.println("Sent login response on port " + config.port);
					break;					
				case MsgClasses.CLOSE:
					System.out.println("Received close Login message, closing connection");
					channel.close(_error);
					channel = null;
					break;
				default:
					System.out.println("Received invalid Login message type: " + _msg.msgClass());
					channel.close(_error);
					channel = null;
					break;
				}
				break;
			case DomainTypes.DICTIONARY:
				switch(_msg.msgClass())
				{
					case MsgClasses.REQUEST:
						DictionaryRequest dictionaryRequest = (DictionaryRequest)DictionaryMsgFactory.createMsg();
						
						dictionaryRequest.rdmMsgType(DictionaryMsgType.REQUEST);
						dictionaryRequest.clear();
						
						// Decode the request so we can send the appropriate info back.
						ret = dictionaryRequest.decode(_decodeIter, _msg);
						if(ret != CodecReturnCodes.SUCCESS) 
						{
							System.out.println("RequestMsg.decode() failed with return code: " + CodecReturnCodes.toString(ret));
							return -1;
						}
						TransportBuffer writeBuff = channel.getBuffer(1500, false, _error);
						
						if(writeBuff == null)
						{
							System.out.println("Failed to get buffer for dictionary response. Error text: " + _error.text());
							return -1;
						}
						
						_encodeIter.clear();
						_encodeIter.setBufferAndRWFVersion(writeBuff, channel.majorVersion(), channel.minorVersion());
						
						
						DictionaryRefresh dictionaryRefresh = (DictionaryRefresh)DictionaryMsgFactory.createMsg();
						dictionaryRefresh.rdmMsgType(DictionaryMsgType.REFRESH);
						dictionaryRefresh.clear();
						dictionaryRefresh.dictionary(dict);
						dictionaryRefresh.streamId(dictionaryRequest.streamId());
						dictionaryRefresh.dictionaryType(Dictionary.Types.FIELD_DEFINITIONS);
						dictionaryRefresh.state().streamState(StreamStates.OPEN);
						dictionaryRefresh.state().dataState(DataStates.OK);
						dictionaryRefresh.state().code(StateCodes.NONE);
				        dictionaryRefresh.verbosity(dictionaryRequest.verbosity());
				        dictionaryRefresh.serviceId(dictionaryRequest.serviceId());
				        dictionaryRefresh.dictionaryName().data(dictionaryRequest.dictionaryName().data(), dictionaryRequest.dictionaryName().position(), dictionaryRequest.dictionaryName().length());
				        dictionaryRefresh.applySolicited();
				        // This is a tiny dictionary, so it should always be complete.
				        dictionaryRefresh.applyRefreshComplete();
				        
				        dictionaryRefresh.encode(_encodeIter);
					
				        if(channel.protocolType() == ProtocolType.JSON)
				        {
				        	
					        jsonMsg.clear();
					        _jsonDecodeIter.clear();
					        _jsonDecodeIter.setBufferAndRWFVersion(writeBuff, channel.majorVersion(), channel.minorVersion());
	
					        final Msg rwfMsg = jsonMsg.rwfMsg();
					        ret = rwfMsg.decode(_jsonDecodeIter);
					        if (ret == CodecReturnCodes.SUCCESS) 
					        {
					            rwfToJsonOptions.clear();
					            rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
					            converter.convertRWFToJson(rwfMsg, rwfToJsonOptions, conversionResults, converterError);
					            if (converterError.isFailed()) {
					            	System.out.println("Failed to convert dictionary Refresh to Json response. Error text: " + _error.text());
									return -1;
					            }
					            
					            /* Releases the original buffer if success */
					            channel.releaseBuffer(writeBuff, _error);
					            
					            writeBuff = channel.getBuffer(1500, false, _error);
								
								if(writeBuff == null)
								{
									System.out.println("Failed to get buffer for dictionary response after conversion. Error text: " + _error.text());
									return -1;
								}
								
								getJsonMsgOptions.clear();
								getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
								getJsonMsgOptions.isCloseMsg(false);
								
								if(converter.getJsonBuffer(writeBuff, getJsonMsgOptions, converterError) != CodecReturnCodes.SUCCESS)
								{
									System.out.println("Failed to get the JSON buffer for dictionary response after conversion. Error text: " + _error.text());
									return -1;
								}
					        }
				        }
						// This should always go out because we're writing a fairly small packet here with direct write enabled
						ret = channel.write(writeBuff, _writeArgs, _error);
						if(ret != TransportReturnCodes.SUCCESS)
						{
							System.out.println("Failed to write the Dictionary response. Error text: " + _error.text());
							return -1;
						}
						else if(ret > 0) 
						{
							channel.flush(_error);
						}
						break;
					case MsgClasses.CLOSE:
						// Do nothing, just print that we got the close
						System.out.println("Received Dictionary close for dictionary stream " + _msg.streamId() + " on port: " + config.port);
						break;
					default:
						System.out.println("Received invalid dictionary message type: " + _msg.msgClass());
						channel.close(_error);
						channel = null;
						break;
					
				}
				break;
			default:
				// All other types, including directories, will be processed by the main test thread.
				break;				
		}
		
		// Copy the message to the queue
		Msg newMsg = CodecFactory.createMsg();
		_msg.copy(newMsg, CopyMsgFlags.ALL_FLAGS);
		
		_messageQueue.addLast(newMsg);
		
		
		return 0;
	}
	
	public void shutdownInternal()
	{
		if(server != null)
		{
			server.close(_error);
			server = null;
		}
		
		if(channel != null)
		{
			channel.close(_error);
			channel = null;
		}
		
		if(_selector != null)
		{
			try {
				_selector.close();
			} catch (IOException exception) {
	            System.out.println("Selector close error. Info: " + exception.getMessage());
			}
			_selector = null;
		}
		
	}
	
	public void shutdown()
	{
		if(shutdownThread == false)	
		{
			shutdownThread = true;
			try {
				thread.join();
			} catch (InterruptedException exception) {
				// This hopefully means that the test has called shutdown in another place, so print out the error here and continue;
	            System.out.println("Join close error. Info: " + exception.getMessage());
			}
		}
		
		shutdownInternal();
		thread = null;
	}
	
	public void closeChannel()
	{
		_userLock.lock();
		_closeChannel = true;
		_userLock.unlock();
	}
	
	public void closeServer()
	{
		_userLock.lock();
		_closeChannel = true;
		_userLock.unlock();
	}

	
	public Msg getMessage()
	{
		Msg msg = null;
		
		_userLock.lock();
		if(_messageQueue.size() > 0)
		{
			msg = _messageQueue.removeFirst();
		}
		_userLock.unlock();
		
		return msg;
	}
	
	public int getMsgCount()
	{
		int count;
		_userLock.lock();
		count = _messageQueue.size();
		_userLock.unlock();
		
		return count;
	}
	
	public int sendBuffer(Buffer buffer)
	{
		if(channel != null)
		{
			int ret;
			TransportBuffer writeBuff = channel.getBuffer(buffer.length(), false, _error);
			if(writeBuff == null)
			{
	            System.out.println("Unable to get buffer. Error text: " + _error.text());
	            return -1;
			}
			
			buffer.copy(writeBuff.data());
			
			// This should always succeed because we're doing a direct write
			ret = channel.write(writeBuff, _writeArgs, _error);
			if(ret != TransportReturnCodes.SUCCESS)
			{
				System.out.println("Failed to write message. Error text: " + _error.text());
				return -1;
			}
			else if(ret > 0) 
			{
				channel.flush(_error);
			}
			
			return 0;
		}
		else
		{
			System.out.println("No active channel.");
            return -1;
		}
	}
}

