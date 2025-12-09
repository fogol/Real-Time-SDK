/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import com.refinitiv.ema.access.OmmNiProviderImpl.StreamInfo;
import com.refinitiv.ema.access.OmmNiProviderImpl.StreamType;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel.State;

class NiProvSessionTransportBuffer {
	public TransportBuffer transportBuffer = null;
	public NiProviderSessionChannelInfo<OmmProviderClient> niProvSessionChannel = null;
	public Channel rsslChannel = null;
	
	public NiProvSessionTransportBuffer()
	{
		clear();
	}
	
	public void releaseBuffer(ReactorErrorInfo errorInfo)
	{
		// If the rsslChannel doesn't match, this means that it the old underlying ETA channel has been closed so the transport buffer has been cleaned up already.
		if(transportBuffer != null && niProvSessionChannel != null && niProvSessionChannel.reactorChannel() != null && niProvSessionChannel.reactorChannel().channel() == rsslChannel)
		{
			niProvSessionChannel.reactorChannel().releaseBuffer(transportBuffer, errorInfo);
		}
		
		transportBuffer = null;
		rsslChannel = null;
	}
	
	// This assumes that buffer has already been released
	public void clear()
	{
		transportBuffer = null;
		niProvSessionChannel = null;
		rsslChannel = null;
	}
}

class PackedMsgImpl implements PackedMsg {
	private TransportBuffer transportBuffer;
	private ReactorChannel reactorChannel;
	private int maxSize;
	private int remainingSize;
	private int packedMsgCount;
	private long clientHandle;
	private long itemHandle;
	private OmmIProviderImpl iProviderImpl;
	private OmmNiProviderImpl niProviderImpl;
	private OmmInvalidUsageExceptionImpl _ommIUExcept;
	private ReactorErrorInfo errorInfo;
	
	private Buffer encBuffer;
	private List<NiProvSessionTransportBuffer> rwfSessionBufferList = null;
	EncodeIterator encIter;
	
	boolean initBufferCalled = false;

	final static int DEFAULT_MAX_SIZE = 6000;
	
	public PackedMsgImpl(OmmProvider provider) {
		errorInfo = ReactorFactory.createReactorErrorInfo();
		
		if (provider instanceof OmmIProviderImpl)
		{
			iProviderImpl = (OmmIProviderImpl)provider;
		}
		else if (provider instanceof OmmNiProviderImpl)
		{
			niProviderImpl = (OmmNiProviderImpl)provider;
		}
		maxSize = DEFAULT_MAX_SIZE;
	}

	// Retrieves the reactor channel and gets transportBuffer for niProvider applications
	public PackedMsg initBuffer() {
		clear();
		maxSize = DEFAULT_MAX_SIZE;
		remainingSize = maxSize;

		if (iProviderImpl != null)
		{
			String temp = "This method is used for Non-Interactive Provider only. Setting a client handle with initBuffer(long clientHandle) is required when using an Interactive Provider." ;
			throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_USAGE);
		}
		else if (niProviderImpl != null)
		{
			if(niProviderImpl.niProviderSession() == null)
			{
				this.reactorChannel = niProviderImpl._channelCallbackClient.channelList().get(0).rsslReactorChannel();
			}
			else
			{
				if(rwfSessionBufferList == null)
					rwfSessionBufferList = new ArrayList<NiProvSessionTransportBuffer>();
				
				rwfSessionBufferList.clear();
				
				if(encBuffer == null)
					encBuffer = CodecFactory.createBuffer();
			}
			
		}
		
		getBuffer();
		initBufferCalled = true;
		return this;
	}
	
	// Retrieves the reactor channel and gets transportBuffer for niProvider applications
	public PackedMsg initBuffer(int maxSize) {
		clear();
		this.maxSize = maxSize;
		remainingSize = maxSize;
		
		if (iProviderImpl != null)
		{
			String temp = "This method is used for Non-Interactive Provider only. Setting a client handle with initBuffer(long clientHandle, int maxSize) is required when using an Interactive Provider." ;
			throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_USAGE);
		}
		else if (niProviderImpl != null)
		{
			if(niProviderImpl.niProviderSession() == null)
			{
				this.reactorChannel = niProviderImpl._channelCallbackClient.channelList().get(0).rsslReactorChannel();
			}
			else
			{
				if(rwfSessionBufferList == null)
					rwfSessionBufferList = new ArrayList<NiProvSessionTransportBuffer>();
				
				rwfSessionBufferList.clear();
				
				if(encBuffer == null)
					encBuffer = CodecFactory.createBuffer();
			}
		}
		
		getBuffer();
		initBufferCalled = true;
		return this;
	}
	
	// Sets the client handle associated with this PackedMsg for IProvider to retrieve the channel and transportBuffer
	public PackedMsg initBuffer(long clientHandle) {
		clear();
		ClientSession clientSession;
		this.clientHandle = clientHandle;
		maxSize = DEFAULT_MAX_SIZE;
		remainingSize = maxSize;

		if (iProviderImpl != null)
		{
			LongObject handleObject = new LongObject();
			handleObject.value(clientHandle);
			clientSession = iProviderImpl.serverChannelHandler().getClientSession(handleObject);

			this.reactorChannel = clientSession.channel();
		}
		else if (niProviderImpl != null)
		{
			String temp = "This method is used for Interactive Provider only. Using initBuffer() is required when using a Non-Interactive Provider, as it does not use a client handle." ;
			throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_USAGE);
		}

		getBuffer();
		initBufferCalled = true;
		return this;
	}
	
	// Sets the client handle associated with this PackedMsg for IProvider to retrieve the channel and transportBuffer
	public PackedMsg initBuffer(long clientHandle, int maxSize) {
		clear();
		ClientSession clientSession;
		this.clientHandle = clientHandle;
		this.maxSize = maxSize;
		remainingSize = maxSize;

		if (iProviderImpl != null)
		{
			LongObject handleObject = new LongObject();
			handleObject.value(clientHandle);
			clientSession = iProviderImpl.serverChannelHandler().getClientSession(handleObject);

			this.reactorChannel = clientSession.channel();
		}
		else if (niProviderImpl != null)
		{
			String temp = "This method is used for Interactive Provider only. Using initBuffer(int maxSize) is required when using a Non-Interactive Provider, as it does not use a client handle." ;
			throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_USAGE);
		}
		
		getBuffer();
		initBufferCalled = true;
		return this;
	}
	
	void getBuffer()
	{
		if (errorInfo == null)
			errorInfo = ReactorFactory.createReactorErrorInfo();
		
		// If iProviderImpl is null, niProviderImpl should not be null
		if(iProviderImpl != null || niProviderImpl.niProviderSession() == null )
		{
			if (reactorChannel.channel() == null)
			{
				String temp = "Failed to retrieve transport buffer. No active channel exists." ;
				throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.NO_ACTIVE_CHANNEL);
			}
			transportBuffer = reactorChannel.getBuffer(maxSize, true, errorInfo);
			if (transportBuffer == null)
			{
				String temp = "Failed to retrieve transport buffer from channel." ;
				throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.NO_BUFFERS);
			}
		}
		else
		{
			// NiProvider session
			for(int index = 0; index < niProviderImpl.niProviderSession().sessionChannelList().size(); ++index)
			{
				NiProviderSessionChannelInfo<OmmProviderClient> channelInfo = (NiProviderSessionChannelInfo<OmmProviderClient>)niProviderImpl.niProviderSession().sessionChannelList().get(index);
				
				// Only get a buffer if the state is READY.
				if(channelInfo.reactorChannel().state() == State.READY)
				{
					TransportBuffer tempBuffer = channelInfo.reactorChannel().getBuffer(maxSize, true, errorInfo);
					if (tempBuffer == null)
					{
						while(rwfSessionBufferList.size() != 0)
						{
							NiProvSessionTransportBuffer tmpBuffer = rwfSessionBufferList.remove(0);
							
							tmpBuffer.releaseBuffer(errorInfo);
							tmpBuffer.clear();
						}
						
						String temp = "Failed to retrieve transport buffer from channel: ";
						throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.NO_BUFFERS);
					}
					
					
					NiProvSessionTransportBuffer packedSessionBuffer = new NiProvSessionTransportBuffer();
					packedSessionBuffer.rsslChannel = channelInfo.reactorChannel().channel();
					packedSessionBuffer.niProvSessionChannel = channelInfo;
					packedSessionBuffer.transportBuffer = tempBuffer;
					
					
					rwfSessionBufferList.add(packedSessionBuffer);
				}
				
				// Setup and allocate the encode buffer.  Re-allocate if we need a larger bytebuffer.
				if(encBuffer.capacity() < maxSize)
				{
					encBuffer.clear();
					
					encBuffer.data(ByteBuffer.allocate(maxSize));
				}
				else
				{
					// Just reset the buffer length
					encBuffer.data().clear();
				}
				
			}
		}
	}
	
	public PackedMsg addMsg(Msg msg, long handle) {
		// If this reactorChannel has no channel set, our connection is not established anymore
		// For a NiProvider session, this will be handled below.
		if ((reactorChannel != null && reactorChannel.channel() == null))
		{
			String temp = "addMsg() failed because connection is not established." ;
			throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.NO_ACTIVE_CHANNEL);
		}
		
		if (handle == 0)
		{
			String temp = "Item handle must be set when calling addMsg()." ;
			throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}
		
		if (initBufferCalled == false )
		{
			String temp = "addMsg() failed because initBuffer() was not called." ;
			throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}
		
		itemHandle = handle;
		
		int retCode = 0;
		MsgImpl msgImpl = (MsgImpl)msg;
		
		if (encIter == null)
			encIter = CodecFactory.createEncodeIterator();
		else
			encIter.clear();
		
		// Set StreamId of the message
		ItemInfo itemInfo = null;
		boolean niProviderStreamAdded = false;	// Check if stream was added for NiProvider
		StreamInfo niProviderStream = null;
		
		if (iProviderImpl != null)
		{
			iProviderImpl.userLock().lock();
						
			itemInfo = iProviderImpl.getItemInfo(handle);
			if (itemInfo == null)	// Stream is down
			{
				releaseBuffer();
				String temp = "No item info exists for this handle." ;
				throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}
			iProviderImpl.userLock().unlock();
			
			if (msgImpl == null || msgImpl._rsslMsg == null)
			{
				releaseBuffer();
				String temp = "Incoming message to pack was null." ;
				throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}
			msgImpl._rsslMsg.streamId((int)itemInfo.streamId().value());
			if (msgImpl.hasServiceName())
			{
				if ((iProviderImpl.directoryServiceStore().serviceId(msgImpl.serviceName())) != null)
				{
					msgImpl.msgServiceId(iProviderImpl.directoryServiceStore().serviceId(msgImpl.serviceName()).value());
				}
				else
				{
					releaseBuffer();
					String temp = "Attempt to add " + DataType.asString(Utilities.toEmaMsgClass[msgImpl._rsslMsg.msgClass()]) + " with service name of " + msgImpl.serviceName() +
							" that was not included in the SourceDirectory. Dropping this " + DataType.asString(Utilities.toEmaMsgClass[msgImpl._rsslMsg.msgClass()]) + ".";
					throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				}
			}
			else if (msgImpl.hasServiceId())
			{
				if ( iProviderImpl.directoryServiceStore().serviceName(msgImpl.serviceId()) == null )
				{
					releaseBuffer();
					String temp = "Attempt to add " + DataType.asString(Utilities.toEmaMsgClass[msgImpl._rsslMsg.msgClass()]) + " with service id of " + msgImpl.serviceId() +
							" that was not included in the SourceDirectory. Dropping this " + DataType.asString(Utilities.toEmaMsgClass[msgImpl._rsslMsg.msgClass()]) + ".";
					throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				}
			}
		}
		else if (niProviderImpl != null)
		{
			StreamInfo streamInfo = niProviderImpl.getStreamInfo(handle);
			
			if (streamInfo != null)
			{
				 msgImpl._rsslMsg.streamId(niProviderImpl.getStreamInfo(handle).streamId());
			}
			else
			{
				streamInfo = (StreamInfo)niProviderImpl._objManager._streamInfoPool.poll();
		    	if (streamInfo == null)
		    	{
		    		streamInfo = niProviderImpl.new StreamInfo(StreamType.PROVIDING, niProviderImpl.nextProviderStreamId());
		    		streamInfo.handle(handle);
		    		niProviderImpl._objManager._streamInfoPool.updatePool(streamInfo);
		    	}
		    	else
		    	{
		    		streamInfo.clear();
		    		streamInfo.set(StreamType.PROVIDING, niProviderImpl.nextProviderStreamId());
		    		streamInfo.handle(handle);
		    	}
		    	
		    	msgImpl._rsslMsg.streamId(streamInfo.streamId());
				niProviderImpl._handleToStreamInfo.put(streamInfo.handle(), streamInfo);
				niProviderStreamAdded = true;
				niProviderStream = streamInfo;
			}
			
			if (msgImpl.hasServiceName())
			{
				if ((niProviderImpl.directoryServiceStore().serviceId(msgImpl.serviceName())) != null)
				{
					msgImpl.msgServiceId(niProviderImpl.directoryServiceStore().serviceId(msgImpl.serviceName()).value());
				}
				else
				{
					releaseBuffer();
					String temp = "Attempt to add " + DataType.asString(Utilities.toEmaMsgClass[msgImpl._rsslMsg.msgClass()]) + " with service name of " + msgImpl.serviceName() +
							" that was not included in the SourceDirectory. Dropping this " + DataType.asString(Utilities.toEmaMsgClass[msgImpl._rsslMsg.msgClass()]) + ".";
					throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				}
			}
			else if (msgImpl.hasServiceId())
			{		
				if ( niProviderImpl.directoryServiceStore().serviceName(msgImpl.serviceId()) == null )
				{
					releaseBuffer();
					String temp = "Attempt to add " + DataType.asString(Utilities.toEmaMsgClass[msgImpl._rsslMsg.msgClass()]) + " with service id of " + msgImpl.serviceId() +
							" that was not included in the SourceDirectory. Dropping this " + DataType.asString(Utilities.toEmaMsgClass[msgImpl._rsslMsg.msgClass()]) + ".";
					throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				}
			}
		}
		
		if(iProviderImpl != null || niProviderImpl.niProviderSession() == null)
		{

			encIter.setBufferAndRWFVersion(transportBuffer, reactorChannel.majorVersion(), reactorChannel.minorVersion());

			retCode = msgImpl._rsslMsg.encode(encIter);
			if (retCode < CodecReturnCodes.SUCCESS)
			{
				if (niProviderStreamAdded && niProviderStream != null)
				{
					niProviderImpl._handleToStreamInfo.remove(niProviderStream.handle());
					niProviderStream.returnToPool();
					niProviderStream = null;
				}
				
				if (retCode == CodecReturnCodes.BUFFER_TOO_SMALL)
				{
					String temp = "Not enough space remaining in this PackedMsg buffer after encoding message." ;
					throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.BUFFER_TOO_SMALL);
				}
				else
				{
					releaseBuffer();
					String temp = "Failed to encode message during addMsg()." ;
					throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.FAILURE);
				}
			}
		
			// This will handle both RWF and JSON.
			if ((retCode = reactorChannel.packBuffer(transportBuffer, errorInfo)) < ReactorReturnCodes.SUCCESS) 
			{
				if (niProviderStreamAdded && niProviderStream != null)
				{
					niProviderImpl._handleToStreamInfo.remove(niProviderStream.handle());
					niProviderStream.returnToPool();
					niProviderStream = null;
				}
				
				String temp;
				switch(errorInfo.code())
				{
					case CodecReturnCodes.BUFFER_TOO_SMALL:
						temp = "Not enough space remaining in this PackedMsg buffer." ;
						throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.BUFFER_TOO_SMALL);
					case ReactorReturnCodes.FAILURE:
						releaseBuffer();
						temp = "Failed to pack buffer during addMsg().";
						throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.FAILURE);
					case ReactorReturnCodes.SHUTDOWN:
						releaseBuffer();
						temp = "Failed to pack buffer during addMsg().";
						throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.CHANNEL_ERROR);
					default: 
						releaseBuffer();
						temp = "Failed to pack buffer during addMsg().";
						throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.FAILURE);
				}
			}
			
			remainingSize = retCode;	// Set remainingSize to our return from packBuffer
			packedMsgCount++;
		}
		else
		{
			int packedMessages = 0;
			boolean encodedData = false;
			encBuffer.data().clear();
			
			// Go through JSON first, because JSON messages will generally be larger than OMMRWF, so if there is a BUFFER_TOO_SMALL scenario, we should hit it
			// with JSON before RWF.
			boolean jsonRemainingSizeSet = false;
			int bufferListSize = rwfSessionBufferList.size();
			int index = 0;
			
			for(int count = 0; count < bufferListSize; ++count)
			{
				NiProvSessionTransportBuffer packedBuffer = rwfSessionBufferList.get(index);
				
				// If the original ETA Channel for the pack does not match the current reactor channel, remove the channel from the list and continue. 
				if(packedBuffer.rsslChannel != packedBuffer.niProvSessionChannel.reactorChannel().channel())
				{
					rwfSessionBufferList.remove(index);
					packedBuffer.clear();
					continue;
				}
				
				if(encodedData == false)
				{
					// NiProvider session
					encIter.setBufferAndRWFVersion(encBuffer, packedBuffer.niProvSessionChannel.reactorChannel().majorVersion(), packedBuffer.niProvSessionChannel.reactorChannel().minorVersion());
					
					retCode = msgImpl._rsslMsg.encode(encIter);
					if (retCode < CodecReturnCodes.SUCCESS)
					{
						if (niProviderStreamAdded && niProviderStream != null)
						{
							niProviderImpl._handleToStreamInfo.remove(niProviderStream.handle());
							niProviderStream.returnToPool();
							niProviderStream = null;
						}
						
						if (retCode == CodecReturnCodes.BUFFER_TOO_SMALL)
						{
							String temp = "Not enough space remaining in this PackedMsg buffer." ;
							throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.BUFFER_TOO_SMALL);
						}
						else
						{
							releaseBuffer();
							String temp = "Failed to encode message during addMsg()." ;
							throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.FAILURE);
						}
					}
					
					// Check to see if this can fit in the previous remaining size.  If not, return an error.
					if(encBuffer.length() > remainingSize)
					{
						String temp = "Not enough space remaining in this PackedMsg buffer." ;
						throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.BUFFER_TOO_SMALL);
					}
					
					encodedData = true;
				}
				
				encBuffer.data().flip();
				
				packedBuffer.transportBuffer.data().put(encBuffer.data());
				
				// This will handle both RWF and JSON.
				if ((retCode = packedBuffer.niProvSessionChannel.reactorChannel().packBuffer(packedBuffer.transportBuffer, errorInfo)) < ReactorReturnCodes.SUCCESS) 
				{
					
					if (niProviderStreamAdded && niProviderStream != null)
					{
						niProviderImpl._handleToStreamInfo.remove(niProviderStream.handle());
						niProviderStream.returnToPool();
						niProviderStream = null;
					}
					
					String temp;
					switch(errorInfo.code())
					{
						case CodecReturnCodes.BUFFER_TOO_SMALL:
							temp = "Not enough space remaining in this PackedMsg buffer." ;
							throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.BUFFER_TOO_SMALL);
						case ReactorReturnCodes.FAILURE:
							// This means that there was a channel failure between the initial check and here, so just cleanup the packed buffer and continue.
							if(packedBuffer.niProvSessionChannel.reactorChannel().state() != State.READY)
							{
								rwfSessionBufferList.remove(index);
								packedBuffer.clear();
								continue;
							}
							releaseBuffer();
							temp = "Failed to pack buffer during addMsg().";
							throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.FAILURE);
						case ReactorReturnCodes.SHUTDOWN:
							releaseBuffer();
							temp = "Failed to pack buffer during addMsg().";
							throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.CHANNEL_ERROR);
						default: 
							releaseBuffer();
							temp = "Failed to pack buffer during addMsg().";
							throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.FAILURE);
					}
				}
				else
				{
					packedMessages++;
					if(jsonRemainingSizeSet == false)
						remainingSize = retCode;	// Set remainingSize to our return from packBuffer.  JSON should always be a smaller buffer after encoding, so in the case of a mixed JSON and RWF connection set, we will defer to the remaining JSON buffer
				}
				
				
				index++;
			}
			
			if(packedMessages == 0)
			{
				if (niProviderStreamAdded && niProviderStream != null)
				{
					niProviderImpl._handleToStreamInfo.remove(niProviderStream.handle());
					niProviderStream.returnToPool();
					niProviderStream = null;
				}
				
				String temp = "addMsg() failed because connection is not established." ;
				throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.NO_ACTIVE_CHANNEL);
			}
			
			packedMsgCount++;
		}
		
		return this;
	}

	public int remainingSize() {
		if (transportBuffer == null)
			return -1;
		return remainingSize;
	}

	public int packedMsgCount() {
		return packedMsgCount;
	}

	public int maxSize() {
		return maxSize;
	}

	public PackedMsg clear() {
		initBufferCalled = false;
		remainingSize = 0;
		packedMsgCount = 0;
		releaseBuffer();
		
		if(rwfSessionBufferList != null)
			rwfSessionBufferList.clear();
		return this;
	}
	
	public TransportBuffer getTransportBuffer()
	{
		return transportBuffer;
	}
	
	public void setTransportBuffer(TransportBuffer transportBuffer)
	{
		this.transportBuffer = transportBuffer;
	}
	
	public void setCalledInitbuffer(boolean calledInit)
	{
		initBufferCalled = calledInit;
	}
	
	OmmInvalidUsageExceptionImpl ommIUExcept()
	{
		if (_ommIUExcept == null)
			_ommIUExcept = new OmmInvalidUsageExceptionImpl();
		
		return _ommIUExcept;
	}
	
	long getClientHandle()
	{
		return clientHandle;
	}
	
	long getItemHandle()
	{
		return itemHandle;
	}
	
	void releaseBuffer()
	{
		if (transportBuffer != null)
		{
			reactorChannel.releaseBuffer(transportBuffer, errorInfo);
			transportBuffer = null;
		}
		
		NiProvSessionTransportBuffer packedBuffer;
		
		if(rwfSessionBufferList != null)
		{
			int bufferListSize = rwfSessionBufferList.size();
			
			for(int count = 0; count < bufferListSize; ++count)
			{
				packedBuffer = rwfSessionBufferList.get(count);
				packedBuffer.releaseBuffer(errorInfo);
				packedBuffer.clear();
			}
		}
		
		initBufferCalled = false;
		
	}
	
	List<NiProvSessionTransportBuffer> getRwfBufferList()
	{
		return rwfSessionBufferList;
	}
}
