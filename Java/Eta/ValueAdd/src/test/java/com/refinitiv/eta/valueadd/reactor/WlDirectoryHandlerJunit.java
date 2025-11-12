/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.*;
import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.*;

public class WlDirectoryHandlerJunit {
    private final Reactor reactor = mock(Reactor.class);
    private final Watchlist watchlist = mock(Watchlist.class);

    private final DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
    private final EncodeIterator encodeIter = CodecFactory.createEncodeIterator();

    private static final int DIRECTORY_STREAM_ID = 2;

    @Before
    public void init() {
        decodeIter.clear();
        encodeIter.clear();

        // Prepare mocks & stabs
        when(reactor.sendAndHandleDirectoryMsgCallback(anyString(), any(ReactorChannel.class),
                            any(TransportBuffer.class), any(Msg.class), any(DirectoryMsg.class),
                            any(WlRequest.class), any(ReactorErrorInfo.class)))
                .thenReturn(ReactorCallbackReturnCodes.SUCCESS);
        when(reactor.sendAndHandleDefaultMsgCallback(anyString(), any(ReactorChannel.class),
                any(TransportBuffer.class), any(Msg.class), any(WlRequest.class), any(ReactorErrorInfo.class)))
                .thenReturn(ReactorCallbackReturnCodes.SUCCESS);

        when(watchlist.reactor()).thenReturn(reactor);
    }

    @Test
    public void readGenericMsg_ReadGenericMessage_SendItToDefaultMsgCallback() {
        WlDirectoryHandler wlDirectoryHandler = new WlDirectoryHandler(watchlist);

        WlStream wlStream = new WlStream();
        wlStream.watchlist(watchlist);
        WlRequest wlRequest = ReactorFactory.createWlRequest();
        wlRequest.state(WlRequest.State.OPEN);
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        requestMsg.streamId(DIRECTORY_STREAM_ID);
        wlRequest._requestMsg = requestMsg;
        wlStream.userRequestList().add(wlRequest);

        ReactorErrorInfo errorInfo = new ReactorErrorInfo();

        Msg msg = TestUtil.createGenericMessage(DomainTypes.SOURCE, DIRECTORY_STREAM_ID);

        int ret = wlDirectoryHandler.readGenericMsg(wlStream, decodeIter, msg, errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        verify(reactor, times(1))
                .sendAndHandleDefaultMsgCallback(eq("WLDirectoryHandler.readGenericMsg"),
                nullable(ReactorChannel.class), nullable(TransportBuffer.class), any(Msg.class),
                any(WlRequest.class), any(ReactorErrorInfo.class));

    }

    @Test
    public void readGenericMsg_ReadDirectoryCSMessage_SendItToDirectoryMsgCallback() {
        WlDirectoryHandler wlDirectoryHandler = new WlDirectoryHandler(watchlist);

        WlStream wlStream = new WlStream();
        wlStream.watchlist(watchlist);
        WlRequest wlRequest = ReactorFactory.createWlRequest();
        wlRequest.state(WlRequest.State.OPEN);
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        requestMsg.streamId(DIRECTORY_STREAM_ID);
        wlRequest._requestMsg = requestMsg;
        wlStream.userRequestList().add(wlRequest);

        ReactorErrorInfo errorInfo = new ReactorErrorInfo();

        Msg msg = TestUtil.createDirectoryCSMessage(encodeIter, decodeIter, DIRECTORY_STREAM_ID);

        int ret = wlDirectoryHandler.readGenericMsg(wlStream, decodeIter, msg, errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        verify(reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WLDirectoryHandler.readGenericMsg"),
                        nullable(ReactorChannel.class), nullable(TransportBuffer.class), any(Msg.class),
                        any(DirectoryMsg.class), any(WlRequest.class), any(ReactorErrorInfo.class));

    }
}
