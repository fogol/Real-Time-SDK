/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import com.refinitiv.eta.codec.Codec;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Set;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

public class TransportTestRunner {

    public enum RunningState
    {
        INACTIVE, INITIALIZING, RUNNING, TERMINATED
    };

    public enum MessageContentType
    {
        UNIFORM, SEQUENCE, RANDOM, MIXED_RANDOM_START, MIXED_UNIFORM_START
    };

    public static int PRINT_NUM_BYTES = 80;

    public interface DataHandler
    {
        void handleMessage(TransportBuffer buffer);
        int receivedCount();
        void setExpectedMessage(ByteBuffer b);
        ArrayList<Boolean> comparisonResults();
    };

    public static class PackedTestArgs implements DataHandler
    {
        // set default test args
        int runTime = 30;
        int guaranteedOutputBuffers = 7000;
        boolean globalLocking = true;
        boolean writeLocking = true;
        boolean blocking = false;
        int compressionType = Ripc.CompressionTypes.NONE;
        int compressionLevel = 6;
        int packedMessageSize = 100;
        int packedCount = 1;
        MessageContentType messageContent = MessageContentType.UNIFORM;
        boolean printReceivedData = false;
        boolean debug = false;
        int expectedTotalBytes = -1;
        int expectedUncompressedBytes = -1;

        private static int portNumber = 15100;
        String PORT_NUMBER;

        private PackedTestArgs(){}

        public static PackedTestArgs getInstance()
        {
            PackedTestArgs args = new PackedTestArgs();
            args.PORT_NUMBER = Integer.toString(portNumber++);
            System.out.println ("PACKED PORT NUMBER:" + args.PORT_NUMBER);
            return args;
        }


        // DataHandler support
        int _receivedCount = 0;
        ArrayList<ByteBuffer> _expectedMessages = new ArrayList<ByteBuffer>(20);
        ArrayList<Boolean> _compareResults = new ArrayList<Boolean>(20);

        public String toString()
        {
            return String.format("runTime=%-4d\nguaranteedOutputBuffers=%-5d\nglobalLocking=%-5s\twriteLocking=%-5s\tblocking=%-5s\ncompressionType=%-1d\tcompressionLevel=%-1d\tdataType=%s",
                    runTime, guaranteedOutputBuffers, globalLocking, writeLocking, blocking, compressionType, compressionLevel, messageContent);
        }

        @Override
        public void handleMessage(TransportBuffer buffer)
        {
            ++_receivedCount;
            if (debug)
                System.out.println("[Packed] receivedCount=" + _receivedCount + " len=" + buffer.length() + " pos=" + buffer.data().position());

            int receivedPos = buffer.data().position();
            if (printReceivedData)
            {
                System.out.print("RECEIVE: ");
                int num = buffer.length();
                if (num > PRINT_NUM_BYTES)
                    num = PRINT_NUM_BYTES;
                for (int n = 0; n < num; n++)
                {
                    System.out.print(String.format("%02X ", buffer.data().get()));
                }
                System.out.println();
            }

            ByteBuffer receivedMsg = buffer.data();

            ByteBuffer expected = _expectedMessages.get(_receivedCount - 1);

            if (printReceivedData)
            {
                System.out.print("EXPECT:  ");
                expected.position(0);
                int num = expected.limit() - expected.position();
                if (num > PRINT_NUM_BYTES)
                    num = PRINT_NUM_BYTES;
                for (int n = 0; n < num; n++)
                {
                    System.out.print(String.format("%02X ", expected.get()));
                }
                System.out.println();
                receivedMsg.position(receivedPos);
            }

            expected.position(0);
            if (expected.compareTo(receivedMsg) == 0)
            {
                if(printReceivedData)
                    System.out.println("\tTRUE");
                _compareResults.add(Boolean.TRUE);
            }
            else
            {
                if(printReceivedData)
                    System.out.println("\tFALSE");
                _compareResults.add(Boolean.FALSE);
            }

        }

        @Override
        public int receivedCount()
        {
            return _receivedCount;
        }

        @Override
        public void setExpectedMessage(ByteBuffer b)
        {
            _expectedMessages.add(b);
        }

        @Override
        public ArrayList<Boolean> comparisonResults()
        {
            return _compareResults;
        }

    }

    public static class TestArgs implements DataHandler
    {
        // set default test args
        int runTime = 30;
        int guaranteedOutputBuffers = 7000;
        boolean globalLocking = true;
        boolean writeLocking = true;
        boolean blocking = false;
        int compressionType = Ripc.CompressionTypes.NONE;
        int compressionLevel = 6;
        int messageSizes[];
        MessageContentType messageContent = MessageContentType.UNIFORM;
        boolean printReceivedData = false;
        boolean debug = false;
        int expectedTotalBytes = -1;
        int expectedUncompressedBytes = -1;
        boolean encrypted = false;
        String[] securityProtocolVersions = {"1.2", "1.3"};
        String serverSecurityProvider = "SunJSSE";
        String clientSecurityProvider = "SunJSSE";

        private static int portNumber = 15200;
        String PORT_NUMBER;

        private TestArgs(){}

        public static TestArgs getInstance()
        {
            TestArgs args = new TestArgs();
            args.PORT_NUMBER = Integer.toString(portNumber++);
            System.out.println ("PORT NUMBER:" + args.PORT_NUMBER);
            return args;
        }


        // DataHandler support
        int _receivedCount = 0;
        ArrayList<ByteBuffer> _expectedMessages = new ArrayList<ByteBuffer>(20);
        ArrayList<Boolean> _compareResults = new ArrayList<Boolean>(20);

        public String toString()
        {
            return String.format("runTime=%-4d\nguaranteedOutputBuffers=%-5d\nglobalLocking=%-5s\twriteLocking=%-5s\tblocking=%-5s\ncompressionType=%-1d\tcompressionLevel=%-1d\tdataType=%s",
                    runTime, guaranteedOutputBuffers, globalLocking, writeLocking, blocking, compressionType, compressionLevel, messageContent);
        }

        @Override
        public void handleMessage(TransportBuffer buffer)
        {
            ++_receivedCount;
            if (debug)
                System.out.println("receivedCount=" + _receivedCount + " len=" + buffer.length() + " pos=" + buffer.data().position());

            int receivedPos = buffer.data().position();
            if (printReceivedData)
            {
                System.out.print("RECEIVE: ");
                int num = buffer.length();
                if (num > PRINT_NUM_BYTES)
                    num = PRINT_NUM_BYTES;
                for (int n = 0; n < num; n++)
                {
                    System.out.print(String.format("%02X ", buffer.data().get()));
                }
                System.out.println();
            }

            ByteBuffer receivedMsg = buffer.data();

            ByteBuffer expected = _expectedMessages.get(_receivedCount - 1);

            if (printReceivedData)
            {
                System.out.print("EXPECT:  ");
                expected.position(0);
                int num = expected.limit() - expected.position();
                if (num > PRINT_NUM_BYTES)
                    num = PRINT_NUM_BYTES;
                for (int n = 0; n < num; n++)
                {
                    System.out.print(String.format("%02X ", expected.get()));
                }
                System.out.println();
                receivedMsg.position(receivedPos);
            }

            expected.position(0);
            if (expected.compareTo(receivedMsg) == 0)
            {
                if(printReceivedData)
                    System.out.println("\tTRUE");
                _compareResults.add(Boolean.TRUE);
            }
            else
            {
                if(printReceivedData)
                    System.out.println("\tFALSE");
                _compareResults.add(Boolean.FALSE);
            }
        }

        @Override
        public int receivedCount()
        {
            return _receivedCount;
        }

        @Override
        public void setExpectedMessage(ByteBuffer b)
        {
            _expectedMessages.add(b);
        }

        @Override
        public ArrayList<Boolean> comparisonResults()
        {
            return _compareResults;
        }

    }

    /**
     * This server will accept connects and process messages until instructed to
     * terminate. It will keep track of the following statistics:
     * <ul>
     * <li>writeCount per priority.
     * <li>messageCount per priority (verify if this is different from
     * writeCount).
     * <li>bytesWritten per priority.
     * <li>uncompressedBytesWritten per priority.
     * </ul>
     */
    public static class EtajServer implements Runnable
    {
        boolean DEBUG = false;

        BindOptions _bindOptions;
        AcceptOptions _acceptOptions;
        Server _server;
        Selector _selector;
        String _errorMsg;
        boolean _globalLocking;
        DataHandler _dataHandler;

        // statistics
        long _messageCount = 0;
        long _compressedBytesRead = 0;
        long _bytesRead = 0;

        volatile boolean _running = true;
        RunningState _runningState = RunningState.INACTIVE;

        final Error _error = TransportFactory.createError();
        final ReadArgs _readArgs = TransportFactory.createReadArgs();
        final InProgInfo _inProgInfo = TransportFactory.createInProgInfo();
        final static int TIMEOUTMS = 1000; // 100 milliseconds

        /**
         * A blocking EtajServer is not supported for junits.
         *
         * @param bindOptions
         * @param acceptOptions
         * @param globalLocking
         */
        public EtajServer(BindOptions bindOptions, AcceptOptions acceptOptions,
                          boolean globalLocking, DataHandler dataHandler)
        {
            assert (bindOptions != null);
            assert (acceptOptions != null);

            _bindOptions = bindOptions;
            _acceptOptions = acceptOptions;
            _globalLocking = globalLocking;
            _dataHandler = dataHandler;
        }

        /**
         * Check if the server is running.
         *
         * @return true if running.
         */
        public RunningState state()
        {
            return _runningState;
        }

        /**
         * Instructs the server to terminate.
         */
        public void terminate()
        {
            if (DEBUG)
                System.out.println("EtajServer: terminate() entered.");

            _running = false;
        }

        public long messageCount()
        {
            return _messageCount;
        }

        public long compressedBytesRead()
        {
            return _compressedBytesRead;
        }

        public long bytesRead()
        {
            return _bytesRead;
        }

        /**
         * Returns the error message that caused the server to abort.
         *
         * @return String containing error message.
         */
        public String errorMsg()
        {
            return _errorMsg;
        }

        /*
         * Sets up the Server by calling bind, and if non-blocking registers for
         * OP_ACCEPT.
         */
        private boolean setupServer()
        {
            if (DEBUG)
                System.out.println("EtajServer: setupServer() entered");

            if (_bindOptions.channelsBlocking())
            {
                _errorMsg = "Blocking EtajServer is not supported in junits.";
                return false;
            }

            if (DEBUG)
                System.out.println("EtajServer: setupServer() binding");

            _server = Transport.bind(_bindOptions, _error);
            if (_server == null)
            {
                _errorMsg = "errorCode=" + _error.errorId() + " errorText=" + _error.text();
                return false;
            }

            if (DEBUG)
                System.out.println("EtajServer: setupServer() opening selector");

            try
            {
                _selector = Selector.open();
                _server.selectableChannel().register(_selector, SelectionKey.OP_ACCEPT);
            }
            catch (Exception e)
            {
                _errorMsg = e.toString();
                return false;
            }

            if (DEBUG)
                System.out.println("EtajServer: setupServer() completed successfully");

            return true;
        }

        private void closeSockets()
        {
            if (DEBUG)
                System.out.println("EtajServer: closeSockets() entered");

            if (_selector == null)
                return;

            if (_server != null)
            {
                _server.close(_error);
                _server = null;
            }

            for (SelectionKey key : _selector.keys())
            {
                if (key.attachment() != null)
                    ((Channel)key.attachment()).close(_error);
                key.cancel();
            }

            try
            {
                _selector.close();
            }
            catch (IOException e)
            {
            }

            _selector = null;
        }

        private boolean verifyChannelInfoCompressionType(Channel channel)
        {
            ChannelInfo channelInfo = TransportFactory.createChannelInfo();
            if (channel.info(channelInfo, _error) != TransportReturnCodes.SUCCESS)
            {
                _errorMsg = "verifyChannelInfoCompressionType() channel.info() failed. error="
                        + _error.text();
                return false;
            }

            if (channelInfo.compressionType() != _bindOptions.compressionType())
            {
                _errorMsg = "verifyChannelInfoCompressionType() channelInfo.compressionType("
                        + channelInfo.compressionType()
                        + ") did not match bindOptions.compressionType("
                        + _bindOptions.compressionType() + ")";
                return false;
            }

            return true;
        }

        private boolean initializeChannel(Channel channel)
        {
            if (DEBUG)
                System.out.println("EtajServer: initializeChannel() entered");

            _inProgInfo.clear();
            int ret = channel.init(_inProgInfo, _error);
            if (ret == TransportReturnCodes.SUCCESS)
            {
                if (DEBUG)
                    System.out.println("EtajServer: initializeChannel() SUCCESS - verifying compressionType is "
                            + _bindOptions.compressionType());

                // SUCCESS, channel is ready, first verify compression Type
                // is what the server specified in BindOptions.
                return verifyChannelInfoCompressionType(channel);
            }
            else if (ret == TransportReturnCodes.CHAN_INIT_IN_PROGRESS)
            {
                return true;
            }
            else
            {
                _errorMsg = "initializeChannel failed, TransportReturnCode=" + ret;
                return false;
            }
        }

        private boolean processRead(Channel channel)
        {
            _readArgs.clear();
            do
            {
                TransportBuffer msgBuf = channel.read(_readArgs, _error);
                if (msgBuf != null)
                {
                    if (DEBUG)
                        System.out.println("Message read: " + msgBuf.length());
                    _dataHandler.handleMessage(msgBuf);
                }
                else
                {
                    if (_readArgs.readRetVal() == TransportReturnCodes.READ_PING)
                    {
                        // Note that we are not tracking client pings.
                        return true;
                    }
                    else if (_readArgs.readRetVal() == TransportReturnCodes.READ_WOULD_BLOCK)
                    {
                        System.out.println("Channel READ_WOULD_BLOCK");
                        return true;
                    }
                }
            }
            while (_readArgs.readRetVal() > TransportReturnCodes.SUCCESS);

            if (_readArgs.readRetVal() != TransportReturnCodes.SUCCESS && _running)
            {
                // ignore this error if we are terminating, which would mean that the client terminated as expected.
                _errorMsg = "processRead(): channel.read() returned " + _readArgs.readRetVal()
                        + ", " + _error.text();
                return false;
            }
            else
            {
                return true;
            }
        }

        private boolean processSelector()
        {
            Set<SelectionKey> keySet = null;

            try
            {
                if (_selector.select(TIMEOUTMS) > 0)
                {
                    keySet = _selector.selectedKeys();
                    if (keySet != null)
                    {
                        Iterator<SelectionKey> iter = keySet.iterator();
                        while (iter.hasNext())
                        {
                            SelectionKey key = iter.next();
                            iter.remove();

                            if (key.isAcceptable())
                            {
                                if (DEBUG)
                                    System.out
                                            .println("EtajServer: processSelector() accepting a connection");

                                Channel channel = _server.accept(_acceptOptions, _error);
                                if (channel != null)
                                {
                                    channel.selectableChannel().register(_selector, SelectionKey.OP_READ,
                                            channel);
                                }
                                else
                                {
                                    _errorMsg = "server.accept() failed to return a valid Channel, error="
                                            + _error.text();
                                    return false;
                                }
                            }

                            if (key.isReadable())
                            {
                                Channel channel = (Channel)key.attachment();
                                if (DEBUG)
                                    System.out
                                            .println("EtajServer: processSelector() channel is readable, channelState="
                                                    + channel.state());

                                if (channel.state() == ChannelState.ACTIVE)
                                {
                                    if (!processRead(channel))
                                        return false;
                                }
                                else if (channel.state() == ChannelState.INITIALIZING)
                                {
                                    if (!initializeChannel(channel))
                                        return false;
                                }
                                else
                                {
                                    _errorMsg = "channel state (" + channel.state()
                                            + ") is no longer ACTIVE, aborting.";
                                    return false;
                                }

                            }
                        }
                    }
                }

            }
            catch (IOException e)
            {
                assertTrue("server failure during select, error=" + e.toString(), false);
            }

            return true;
        }

        @Override
        public void run()
        {
            _runningState = RunningState.INITIALIZING;

            if (DEBUG)
                System.out.println("EtajServer: run() entered");

            // Initialize Transport
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(_globalLocking);
            if (Transport.initialize(initArgs, _error) != TransportReturnCodes.SUCCESS)
            {
                _errorMsg = "errorCode=" + _error.errorId() + " errorText=" + _error.text();
                _running = false;
            }
            else if (!setupServer())
            {
                _running = false;
            }
            else
            {
                _runningState = RunningState.RUNNING;
            }

            while (_running)
            {
                if (!processSelector())
                    _running = false;
            }

            closeSockets();

            if (_errorMsg != null)
                System.out.println("EtajServer: run(): error occurred, " + _errorMsg);

            if (Transport.uninitialize() != TransportReturnCodes.SUCCESS && _errorMsg == null)
            {
                _errorMsg = "Transport.uninitialize() failed.";
                return;
            }

            _runningState = RunningState.TERMINATED;
            if (DEBUG)
                System.out.println("EtajServer: run() completed");
        }

    } // end of EtajServer class

    /**
     * This client take a channel and send messages until instructed to
     * terminate. It will keep track of the following statistics:
     * <ul>
     * <li>writeCount per priority.
     * <li>messageCount per priority (verify if this is different from
     * writeCount).
     * <li>bytesWritten per priority.
     * <li>uncompressedBytesWritten per priority.
     * </ul>
     */
    static class EtajClient implements Runnable
    {
        boolean DEBUG = false;

        Channel _channel;

        String _errorMsg;
        int _id;
        int _priority;
        boolean _globalLocking;

        // statistics
        long _messageCount = 0;
        long _uncompressedBytesWritten = 0;
        long _bytesWritten = 0;

        volatile boolean _running = true;
        RunningState _runningState = RunningState.INACTIVE;

        final Error _error = TransportFactory.createError();
        final ReadArgs _readArgs = TransportFactory.createReadArgs();
        final InProgInfo _inProgInfo = TransportFactory.createInProgInfo();
        final WriteArgs _writeArgs = TransportFactory.createWriteArgs();
        final static int TIMEOUTMS = 1000; // 100 milliseconds

        java.util.Random _gen;

        /**
         * .
         */
        public EtajClient(int id, int priority,
                          Channel channel, boolean globalLocking)
        {
            assert (channel != null);

            _id = id;

            if (priority <= WritePriorities.LOW)
                _priority = priority;
            else
                _priority = WritePriorities.LOW;

            _channel = channel;
            _globalLocking = globalLocking;

            _gen = new java.util.Random(2289374);

            if (DEBUG)
                System.out.println("EtajClient id=" + _id + ": created. priority=" + _priority);
        }

        /**
         * Return server is running.
         *
         * @return true if running.
         */
        public RunningState state()
        {
            return _runningState;
        }

        /**
         * Instructs the server to terminate.
         */
        public void terminate()
        {
            if (DEBUG)
                System.out.println("EtajClient id=" + _id + ": terminate() entered.");

            _running = false;
        }

        public long messageCount()
        {
            return _messageCount;
        }

        public long uncompressedBytesWritten()
        {
            return _uncompressedBytesWritten;
        }

        public long bytesWritten()
        {
            return _bytesWritten;
        }

        /**
         * Returns the error message that caused the server to abort.
         *
         * @return String containing error message.
         */
        public String errorMsg()
        {
            return _errorMsg;
        }

        // dataBuf: user data to put in each of the messages
        // packCount: number of packed messages to send
        private int writePackedMessages(PackedTestArgs args)
        {
            int retVal = 0;
            TransportBuffer msgBuf = null;
            int userBytes = 0;
            final Error error = TransportFactory.createError();

            int payloadSize = (args.packedMessageSize + RsslSocketChannel.RIPC_PACKED_HDR_SIZE) * args.packedCount;
            // Packed buffer
            msgBuf = _channel.getBuffer(payloadSize, true, _error);

            int availableRemaining = payloadSize - msgBuf.length();
            if (args.debug)
                System.out.println("[Packing] start payloadSize=" + payloadSize + " availableRemaining=" + availableRemaining);
            for (int p = 1;
                 p <= args.packedCount;
                 p++)
            {
                if (args.debug)
                    System.out.println("[Packing] #" + p + " availableRemaining=" + availableRemaining);

                // populate buffer with payload
                ByteBuffer dataBuf = ByteBuffer.allocate(args.packedMessageSize);
                dataBuf.clear();
                populateMessage(dataBuf, dataBuf.capacity(), args.messageContent);

                args.setExpectedMessage(dataBuf); // for comparison on the receive side
                dataBuf.position(0);
                for( int n = 0; n < dataBuf.limit(); n++ )
                {
                    msgBuf.data().put(dataBuf.get());
                }
                userBytes += dataBuf.limit();

                // pack
                availableRemaining = _channel.packBuffer(msgBuf, error);
            }
            if (args.debug)
                System.out.println("[Packing] availableRemaining at end of packing:" + availableRemaining);

            _writeArgs.clear();

            do
            {
                if (retVal != 0)
                    System.out.println("writePackedMessages(): last retVal=" + retVal
                            + " msgBufPos=" + msgBuf.data().position()
                            + " msgBufLimit=" + msgBuf.data().limit());
                retVal = _channel.write(msgBuf, _writeArgs, _error);
                if(!_running)
                {
                    System.out.println("EtajClient id=" + _id + ": writePackedMessages(): write() retVal=" + retVal + " while run time expired.");
                    try
                    {
                        if (DEBUG)
                            Thread.sleep(250);
                        else
                            Thread.sleep(1);
                    }
                    catch (InterruptedException e)
                    {
                    }
                }
            }
            while (retVal == TransportReturnCodes.WRITE_CALL_AGAIN);

            if (retVal >= TransportReturnCodes.SUCCESS)
            {
                ++_messageCount;
                _bytesWritten += _writeArgs.bytesWritten();
                _uncompressedBytesWritten += _writeArgs.uncompressedBytesWritten();

                if (DEBUG)
                    System.out.println("EtajClient id=" + _id + ": writePackedMessages(): _bytesWritten="
                            + _bytesWritten + " _uncompressedBytesWritten="
                            + _uncompressedBytesWritten
                            + " userBytes=" + userBytes);

                retVal = _channel.flush(_error);

                if (retVal < TransportReturnCodes.SUCCESS)
                {
                    _errorMsg = "writeMessage(): flush failed, retVal=" + retVal + " error="
                            + _error.text();

                    System.out.println("EtajClient id=" + _id + ": writeMessage(): error="
                            + _errorMsg);
                    return retVal;
                }
            }
            else
            {
                _errorMsg = "writeMessage(): write failed, retVal=" + retVal + " error="
                        + _error.text();

                System.out.println("EtajClient id=" + _id + ": writeMessage(): error="
                        + _errorMsg);
                return retVal;
            }

            return retVal;
        }

        ////////////////////////////////////////////
        // returns TransportReturnCodes
        private int writeMessage(ByteBuffer dataBuf)
        {
            int retVal;
            TransportBuffer msgBuf = null;

            while (msgBuf == null)
            {
                // Not packed 
                msgBuf = _channel.getBuffer(dataBuf.limit(), false, _error);
                if (msgBuf == null)
                {
                    System.out.println("EtajClient id=" + _id
                            + ": writeMessage(): no msgBufs available, errorId=" + _error.errorId()
                            + " error=" + _error.text() + ", attempting to flush.");

                    retVal = _channel.flush(_error);
                    if (DEBUG)
                        System.out.println("EtajClient id=" + _id
                                + ": writeMessage(): flush() returned retVal=" + retVal);

                    if (retVal < TransportReturnCodes.SUCCESS)
                    {
                        _errorMsg = "writeMessage(): no msgBufs available to write and flush failed, retVal="
                                + retVal + " error=" + _error.text();

                        System.out.println("EtajClient id=" + _id + ": writeMessage(): error="
                                + _errorMsg);
                        return retVal;
                    }

                    try
                    {
                        if (DEBUG)
                            Thread.sleep(250);
                        else
                            Thread.sleep(1);
                    }
                    catch (InterruptedException e)
                    {
                    }

                    if (!_running)
                    {
                        _errorMsg = "writeMessage(): no msgBufs available to write and run time expired";
                        return TransportReturnCodes.FAILURE;
                    }
                }
            }

            // populate msgBuf with data.
            dataBuf.position(0);
            for( int n = 0; n < dataBuf.limit(); n++ )
            {
                msgBuf.data().put(dataBuf.get());
            }

            // write
            _writeArgs.clear();
            _writeArgs.priority(_priority);

            int origLen=msgBuf.length(), origPos=msgBuf.data().position(), origLimit=msgBuf.data().limit();
            retVal = 0;

            do
            {
                if (retVal !=0)
                    System.out.println("writeMessage(): last retVal=" + retVal + " origLen="
                            + origLen + " origPos=" + origPos + " origLimit="
                            + origLimit + " msgBufPos=" + msgBuf.data().position()
                            + " msgBufLimit=" + msgBuf.data().limit());
                retVal = _channel.write(msgBuf, _writeArgs, _error);
                if(!_running)
                {
                    System.out.println("EtajClient id=" + _id + ": writeMessage(): write() retVal=" + retVal + " while run time expired.");
                    try
                    {
                        if (DEBUG)
                            Thread.sleep(250);
                        else
                            Thread.sleep(1);
                    }
                    catch (InterruptedException e)
                    {
                    }
                }
            }
            while (retVal == TransportReturnCodes.WRITE_CALL_AGAIN);

            if (retVal >= TransportReturnCodes.SUCCESS)
            {
                ++_messageCount;
                _bytesWritten += _writeArgs.bytesWritten();
                _uncompressedBytesWritten += _writeArgs.uncompressedBytesWritten();

                if (DEBUG)
                    System.out.println("EtajClient id=" + _id + ": writeMessage(): _bytesWritten="
                            + _bytesWritten + " _uncompressedBytesWritten="
                            + _uncompressedBytesWritten
                            + " userBytes=" + dataBuf.limit());

                retVal = _channel.flush(_error);

                if (retVal < TransportReturnCodes.SUCCESS)
                {
                    _errorMsg = "writeMessage(): flush failed, retVal=" + retVal + " error="
                            + _error.text();

                    System.out.println("EtajClient id=" + _id + ": writeMessage(): error="
                            + _errorMsg);
                    return retVal;
                }
            }
            else
            {
                _errorMsg = "writeMessage(): write failed, retVal=" + retVal + " error="
                        + _error.text();

                System.out.println("EtajClient id=" + _id + ": writeMessage(): error="
                        + _errorMsg);
                return retVal;
            }

            return 0;
        }

        private void populateMessage(ByteBuffer buf, int messageSize, MessageContentType type)
        {
            switch (type)
            {
                case RANDOM:

                    for (int i=0; i < messageSize; i++)
                    {
                        buf.put((byte)(_gen.nextInt() % 255));
                    }
                    break;

                case UNIFORM:
                default:
                    for (int idx = 0; idx < messageSize; idx++)
                    {
                        buf.put((byte)1);
                    }
                    break;

                case SEQUENCE:
                    for (int idx = 0; idx < messageSize; idx++)
                    {
                        buf.put((byte)(idx % 255));
                    }
                    break;

                case MIXED_RANDOM_START:
                case MIXED_UNIFORM_START:
                    boolean random = false;
                    if (type == MessageContentType.MIXED_RANDOM_START)
                        random = true;
                    for (int idx = 1; idx <= messageSize; idx++)
                    {
                        if (random)
                        {
                            buf.put((byte)(_gen.nextInt() % 255));
                        }
                        else
                        {
                            buf.put((byte)7);
                        }
                        // approximately the amount of data for a fragment
                        if (idx % 6137 == 0)
                            random = !random;
                    }
                    break;

            }

            buf.position(0);
            buf.limit(messageSize);
        }

        @Override
        public void run()
        {
            _runningState = RunningState.INITIALIZING;

            if (DEBUG)
                System.out.println("EtajClient id=" + _id + ": run() running");

            // Initialize Transport
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(_globalLocking);
            if (Transport.initialize(initArgs, _error) != TransportReturnCodes.SUCCESS)
            {
                _errorMsg = "run(): errorCode=" + _error.errorId() + " errorText=" + _error.text();
                _running = false;
            }

            // get channel info for maxFragmentSize.
            ChannelInfo channelInfo = TransportFactory.createChannelInfo();
            if (_channel.info(channelInfo, _error) != TransportReturnCodes.SUCCESS)
            {
                _errorMsg = "run(): channel.info() failed. errorCode=" + _error.errorId()
                        + " errorText=" + _error.text();
                _running = false;
            }

            _runningState = RunningState.RUNNING;

            while (_running); // DRT for now not doing anything in client thread

            if (Transport.uninitialize() != TransportReturnCodes.SUCCESS && _errorMsg == null)
            {
                _errorMsg = "EtajClient id=" + _id + ": Transport.uninitialize() failed.";
            }

            _runningState = RunningState.TERMINATED;
            System.out.println("EtajClient id=" + _id + ": run() complete. messageCount=" + _messageCount);

        }
    } // end of EtajClient class

    public static BindOptions defaultBindOptions(String portNumber)
    {
        BindOptions bindOptions = TransportFactory.createBindOptions();
        bindOptions.majorVersion(Codec.majorVersion());
        bindOptions.minorVersion(Codec.minorVersion());
        bindOptions.protocolType(Codec.protocolType());
        bindOptions.connectionType(ConnectionTypes.SOCKET);
        bindOptions.serviceName(portNumber);
        bindOptions.serverToClientPings(false);
        return bindOptions;
    }

    public static BindOptions encryptedBindOptions(String portNumber, String[] securityProtocolVersions, String serverSecurityProvider)
    {
        BindOptions bindOptions = TransportFactory.createBindOptions();
        bindOptions.connectionType(ConnectionTypes.ENCRYPTED);
        initEncryptionOptions(bindOptions.encryptionOptions(), securityProtocolVersions, serverSecurityProvider);
        bindOptions.serviceName(portNumber);
        bindOptions.sysRecvBufSize(64 * 1024);

        return bindOptions;
    }

    public static AcceptOptions defaultAcceptOptions()
    {
        AcceptOptions acceptOptions = TransportFactory.createAcceptOptions();
        return acceptOptions;
    }

    public static ConnectOptions defaultConnectOptions(String portNumber)
    {
        ConnectOptions connectOptions = TransportFactory.createConnectOptions();
        connectOptions.majorVersion(Codec.majorVersion());
        connectOptions.minorVersion(Codec.minorVersion());
        connectOptions.protocolType(Codec.protocolType());
        connectOptions.connectionType(ConnectionTypes.SOCKET);
        connectOptions.unifiedNetworkInfo().address("localhost");
        connectOptions.unifiedNetworkInfo().serviceName(portNumber);
        return connectOptions;
    }

    public static ConnectOptions encryptedConnectOptions(String portNumber, String clientSecurityProvider)
    {
        ConnectOptions connectOptions = TransportFactory.createConnectOptions();

        connectOptions.connectionType(ConnectionTypes.ENCRYPTED);
        connectOptions.encryptionOptions().connectionType(ConnectionTypes.SOCKET);
        initEncryptionOptions(connectOptions.encryptionOptions(), new String[] {"1.3", "1.2"}, clientSecurityProvider);
        connectOptions.tunnelingInfo().tunnelingType("None");
        connectOptions.unifiedNetworkInfo().address("localhost");
        connectOptions.unifiedNetworkInfo().serviceName(portNumber);
        connectOptions.majorVersion(Codec.majorVersion());
        connectOptions.minorVersion(Codec.minorVersion());
        connectOptions.protocolType(Codec.protocolType());

        return connectOptions;
    }

    private static void initEncryptionOptions(EncryptionOptions encryptionOptions, String[] securityProtocolVersions, String clientSecurityProvider)
    {
        encryptionOptions.KeystoreFile(CryptoHelperTest.VALID_CERTIFICATE);
        encryptionOptions.KeystorePasswd(CryptoHelperTest.KEYSTORE_PASSWORD);
        encryptionOptions.SecurityProtocolVersions(securityProtocolVersions);
        encryptionOptions.SecurityProvider(clientSecurityProvider);
    }

    private static void initEncryptionOptions(ServerEncryptionOptions encryptionOptions, String[] securityProtocolVersions, String serverSecurityProvider)
    {
        encryptionOptions.keystoreFile(CryptoHelperTest.VALID_CERTIFICATE);
        encryptionOptions.keystorePasswd(CryptoHelperTest.KEYSTORE_PASSWORD);
        encryptionOptions.securityProtocolVersions(securityProtocolVersions);
        encryptionOptions.securityProvider(serverSecurityProvider);
    }

    /**
     * Start and initialize the client channels.
     *
     * @param blocking
     * @param compressionType
     * @return
     */
    public static Channel startClientChannel(int guaranteedOutputBuffers,
                                             boolean blocking,
                                             boolean writeLocking,
                                             int compressionType,
                                             String portNumber,
                                             boolean encrypted,
                                             String clientSecurityProvider)
    {
        System.out.println("startClientChannel(): entered");

        Channel channel;
        ConnectOptions connectOptions = encrypted ? encryptedConnectOptions(portNumber, clientSecurityProvider) : defaultConnectOptions(portNumber);
        connectOptions.blocking(blocking);
        connectOptions.compressionType(compressionType);
        connectOptions.channelWriteLocking(writeLocking);
        connectOptions.guaranteedOutputBuffers(guaranteedOutputBuffers);

        Error error = TransportFactory.createError();
        InProgInfo inProgInfo = TransportFactory.createInProgInfo();

        if ((channel = Transport.connect(connectOptions, error)) == null)
        {
            System.out.println("startClientChannel(): Transport.connect() for channel "
                    +  "failed, errorId=" + error.errorId() + " error=" + error.text());
            return null;
        }
        System.out.println("startClientChannel(): Channel is INITIALIZING");


        // loop until all connections are ACTIVE.
        long timeout = System.currentTimeMillis() + 20000; // 20 second timeout
        boolean initializing = false;

        do
        {
            initializing = false;
            int retVal;
            if (channel.state() != ChannelState.INITIALIZING)
            {
                continue;
            }

            if ((retVal = channel.init(inProgInfo, error)) < TransportReturnCodes.SUCCESS)
            {
                System.out.println("startClientChannel(): channel.init() "
                        + " failed, errorId=" + error.errorId() + " error=" + error.text());
                return null;
            }
            else if (retVal == TransportReturnCodes.CHAN_INIT_IN_PROGRESS)
            {
                initializing = true;
            }
            else
            {
                System.out.println("startClientChannel(): Channel is ACTIVE");
            }
        }
        while (initializing && System.currentTimeMillis() < timeout);

        if (!initializing)
        {
            System.out.println("startClientChannel() initialized");
            return channel;
        }
        else
        {
            System.out.println("startClientChannel(): failed to initialize channel");
            return null;
        }
    }

    /**
     * Wait for server state of RUNNING or TERMINATED.
     *
     * @param server
     * @return true if RunningState.RUNNING, false if RunningState.TERMINATED.
     */
    public static boolean waitForStateRunning(EtajServer server)
    {
        try
        {
            while (true)
            {
                if (server.state() == RunningState.RUNNING)
                    return true;
                else if (server.state() == RunningState.TERMINATED)
                    return false;
                else
                    Thread.sleep(100);
            }
        }
        catch (InterruptedException e)
        {
        }
        return false;
    }

    private static void terminateServerAndClients(Thread serverThread,
                                           EtajServer server,
                                           Thread clientThread,
                                           EtajClient etajClient,
                                           Channel channel)
    {
        System.out.println("terminateServerAndClients(): stopping clients");

        // instruct clients to stop
        etajClient.terminate();

        System.out.println("terminateServerAndClients(): waiting for clients to finish");

        // wait for all clients to finish
        long timeout = System.currentTimeMillis() + 12000000; // 20 second timeout
        boolean stillRunning;
        do
        {
            stillRunning = false;
            if (etajClient.state() == RunningState.RUNNING)
                stillRunning = true;

            try
            {
                Thread.sleep(1000);
            }
            catch (InterruptedException e)
            {
            }

            if (System.currentTimeMillis() > timeout)
            {
                System.out.println("terminateServerAndClients(): failed to stop clients after 10 seconds.");
                break;
            }
        }
        while (stillRunning);

        if (!stillRunning)
        {
            System.out.println("terminateServerAndClients(): flushing client channels.");
            Error error = TransportFactory.createError();
            int retVal;
            do
            {
                retVal = channel.flush(error);
                try
                {
                    Thread.sleep(1);
                }
                catch (InterruptedException e)
                {
                }
            }
            while (retVal > TransportReturnCodes.SUCCESS);

            if (retVal != TransportReturnCodes.SUCCESS)
                System.out.println("terminateServerAndClients(): channel.flush() failed. retVal="
                        + retVal + " errorId=" + error.errorId() + " error=" + error.text());
        }
        else
        {
            System.out.println("terminateServerAndClients(): skipping the flushing client channels, due to non-responsive client threads.");
        }


        System.out.println("terminateServerAndClients(): terminating server");
        server.terminate();

        System.out.println("terminateServerAndClients(): waiting for server to finish");
        // wait for server to terminate
        boolean serverStillRunning = false;
        do
        {
            serverStillRunning = false;
            if (server.state() == RunningState.RUNNING)
                serverStillRunning = true;

            try
            {
                Thread.sleep(100);
            }
            catch (InterruptedException e)
            {
            }
        }
        while (serverStillRunning);


        // join all client threads
        if (!stillRunning)
        {
            System.out.println("terminateServerAndClients(): joining client threads");
            try
            {
                clientThread.join();
            }
            catch (InterruptedException e)
            {
                e.printStackTrace();
            }
        }
        else
        {
            System.out.println("terminateServerAndClients(): skipping the joining of client threads, due to non-responsive client threads.");
        }

        try
        {
            serverThread.join();
        }
        catch (InterruptedException e)
        {
            e.printStackTrace();
        }

        System.out.println("terminateServerAndClients(): closing channels");

        // close all channels
        if (!stillRunning)
        {
            Error error = TransportFactory.createError();
            channel.close(error); // ignore return code since server will close the channels as well, just prior to this call.
        }
        else
        {
            System.out.println("terminateServerAndClients(): skipping the closing of client channels, due to non-responsive client threads.");
        }

        System.out.println("terminateServerAndClients(): completed");
    }

    public static void testRunner(String testName, TestArgs args)
    {
        testRunner(testName, args, null);
    }

    public static void testRunner(String testName, TestArgs args, java.util.function.BiConsumer<EtajServer, EtajClient> customChecks)
    {
        System.out.println("--------------------------------------------------------------------------------");
        System.out.println("Test: " + testName);
        System.out.println(args.toString());
        System.out.println("--------------------------------------------------------------------------------");
        long endTime = System.currentTimeMillis() + (args.runTime * 1000);

        assertTrue("Cannot have client blocking and globalLocking=true with server using same JVM, globalLocking will deadlock.", (args.blocking && args.globalLocking) == false);

        // BindOptions
        BindOptions bindOptions = args.encrypted
                ? encryptedBindOptions(args.PORT_NUMBER, args.securityProtocolVersions, args.serverSecurityProvider)
                : defaultBindOptions(args.PORT_NUMBER);
        bindOptions.compressionType(args.compressionType);
        if (args.compressionType > Ripc.CompressionTypes.NONE)
            bindOptions.compressionLevel(args.compressionLevel);

        // AcceptOptions
        AcceptOptions acceptOptions = defaultAcceptOptions();

        // create the server thread and start it
        EtajServer server = new EtajServer(bindOptions, acceptOptions, args.globalLocking, args);
        Thread serverThread = new Thread(server);
        serverThread.start();

        if (!waitForStateRunning(server))
        {
            assertTrue("server terminated while waiting for RUNNING state, error="
                    + server.errorMsg(), false);
        }
        else
        {
            System.out.println("Server bound");
            // start the channels that represent client session
            Channel clientChannel = startClientChannel(args.guaranteedOutputBuffers,
                    args.blocking,
                    args.writeLocking,
                    args.compressionType,
                    args.PORT_NUMBER, args.encrypted, args.clientSecurityProvider);
            assertNotNull("startClientChannel failed, check output", clientChannel);

            EtajClient etajClient = new EtajClient(1, // etajClientCount
                    0, // priority
                    clientChannel,
                    args.globalLocking);
            Thread clientThread = new Thread(etajClient);
            clientThread.start();

            int messageCount = 0;
            boolean testFailed = false;
            for (int testMessageSize : args.messageSizes)
            {
                ++messageCount;
                if (server.state() != RunningState.RUNNING)
                {
                    System.out.println("Terminating message loop (Server !RUNNING): at message "
                            + messageCount + " of " + args.messageSizes.length);
                    break;
                }

                ByteBuffer msgData = ByteBuffer.allocate(testMessageSize);
                args.setExpectedMessage(msgData);
                etajClient.populateMessage(msgData, msgData.capacity(), args.messageContent);
                int writeReturn = etajClient.writeMessage(msgData);
                if (writeReturn < TransportReturnCodes.SUCCESS)
                {
                    System.out.println("Terminating message loop (writeMessage return code "
                            + writeReturn + "): at message "
                            + messageCount + " of " + args.messageSizes.length);
                    testFailed = true;
                    break;
                }
                if (args.debug)
                    System.out.println("writeArgs.bytesWritten()=" + etajClient._writeArgs.bytesWritten()
                            + " writeArgs.uncompressedBytesWritten=" + etajClient._writeArgs.uncompressedBytesWritten());

                if (args.expectedTotalBytes != -1)
                {
                    assertTrue(args.expectedTotalBytes == etajClient._writeArgs.bytesWritten());
                }
                if (args.expectedUncompressedBytes != -1)
                {
                    assertTrue(args.expectedUncompressedBytes == etajClient._writeArgs.uncompressedBytesWritten());
                }

                try
                {
                    // wait for message to be received to compare with message sent
                    boolean messageTestComplete = false;
                    while (!messageTestComplete
                            && server.state() == RunningState.RUNNING
                            && System.currentTimeMillis() < endTime)
                    {
                        Thread.sleep(1000);

                        if (messageCount == args.receivedCount())
                        {
                            ArrayList<Boolean> results = args.comparisonResults();
                            if (results != null)
                            {
                                System.out.println(messageCount + " of " + args.messageSizes.length
                                        + ": message comparison " + results.get(messageCount-1));

                                // if (!results.get(messageCount-1).booleanValue())
                                //     testFailed = true;
                                messageTestComplete = true; // next
                                break;
                            }

                        }
                    }
                }
                catch (InterruptedException e)
                {
                }
            }

            if (customChecks != null)
            {
                customChecks.accept(server, etajClient);
            }

            // verify test made it through all messages before exit
            assertTrue(messageCount == args.messageSizes.length);
            assertTrue(!testFailed);

            terminateServerAndClients(serverThread, server, clientThread, etajClient, clientChannel);

        }

        // If a server failure occurred, fail the test.
        if (server.errorMsg() != null)
            assertTrue(server.errorMsg(), false);
    }

    public static void testRunnerPacked(String testName, PackedTestArgs args)
    {
        System.out.println("--------------------------------------------------------------------------------");
        System.out.println("Packed Test: " + testName);
        System.out.println(args.toString());
        System.out.println("--------------------------------------------------------------------------------");
        long endTime = System.currentTimeMillis() + (args.runTime * 1000);

        assertTrue("Cannot have client blocking and globalLocking=true with server using same JVM, globalLocking will deadlock.", (args.blocking && args.globalLocking) == false);

        // BindOptions
        BindOptions bindOptions = defaultBindOptions(args.PORT_NUMBER);
        bindOptions.compressionType(args.compressionType);
        if (args.compressionType > Ripc.CompressionTypes.NONE)
            bindOptions.compressionLevel(args.compressionLevel);

        // AcceptOptions
        AcceptOptions acceptOptions = defaultAcceptOptions();

        // create the server thread and start it
        EtajServer server = new EtajServer(bindOptions, acceptOptions, args.globalLocking, args);
        Thread serverThread = new Thread(server);
        serverThread.start();

        if (!waitForStateRunning(server))
        {
            assertTrue("server terminated while waiting for RUNNING state, error="
                    + server.errorMsg(), false);
        }
        else
        {
            // start the channels that represent client session
            Channel clientChannel = startClientChannel(args.guaranteedOutputBuffers,
                    args.blocking,
                    args.writeLocking,
                    args.compressionType,
                    args.PORT_NUMBER, false, null);
            assertNotNull("startClientChannel failed, check output", clientChannel);

            EtajClient etajClient = new EtajClient(1, // etajClientCount
                    0, // priority
                    clientChannel,
                    args.globalLocking);
            Thread clientThread = new Thread(etajClient);
            clientThread.start();

            if (server.state() != RunningState.RUNNING)
            {
                System.out.println("Terminating message loop (Server !RUNNING)");
                return;
            }

            System.out.println("[testRunnerPacked] sending " + args.packedCount
                    + " packed messages of size " + args.packedMessageSize);
            int writeReturn = etajClient.writePackedMessages(args);
            if (writeReturn < TransportReturnCodes.SUCCESS)
            {
                System.out.println("Terminating message loop writePackedMessages return code "
                        + writeReturn);

                return;
            }
            System.out.println("writeArgs.bytesWritten()=" + etajClient._writeArgs.bytesWritten()
                    + " writeArgs.uncompressedBytesWritten=" + etajClient._writeArgs.uncompressedBytesWritten());

            try
            {
                // wait for message to be received to compare with message sent
                boolean messageTestComplete = false;
                while (!messageTestComplete
                        && server.state() == RunningState.RUNNING
                        && System.currentTimeMillis() < endTime)
                {
                    Thread.sleep(100);

                    if (args.receivedCount() == args.packedCount)
                        messageTestComplete = true;
                }
            }
            catch (InterruptedException e)
            {
            }

            assertTrue(args.packedCount == args.receivedCount());

            ArrayList<Boolean> results = args.comparisonResults();
            if (results != null)
            {
                int count = 0;
                for (Boolean b : results)
                {
                    ++count;
                    if (!b.booleanValue())
                        System.out.println("[testRunnerPacked] message #" + count + " comparison failed");

                    assertTrue(b.booleanValue());
                }
                System.out.println("[testRunnerPacked] verified " + results.size()
                        + " of " + args.packedCount + " received messages");
            }

            terminateServerAndClients(serverThread, server, clientThread, etajClient, clientChannel);

        }

        // If a server failure occurred, fail the test.
        if (server.errorMsg() != null)
            assertTrue(server.errorMsg(), false);
    }
}
