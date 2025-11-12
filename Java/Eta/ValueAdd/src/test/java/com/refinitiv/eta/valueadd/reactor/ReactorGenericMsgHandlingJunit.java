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
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg;
import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.*;

public class ReactorGenericMsgHandlingJunit {
    private final ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
    private final ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
    private final ConsumerRole reactorRole = mock(ConsumerRole.class);
    private final Reactor reactor = ReactorFactory.createReactor(reactorOptions, errorInfo);
    private final Reactor reactorSpy = spy(reactor);
    private final ReactorChannel reactorChannel = mock(ReactorChannel.class);
    private final TransportBuffer transportBuffer = mock(TransportBuffer.class);

    private final DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
    private final EncodeIterator encodeIter = CodecFactory.createEncodeIterator();

    private static final int LOGIN_STREAM_ID = 1;
    private static final int DIRECTORY_STREAM_ID = 2;

    @Before
    public void init() {
        decodeIter.clear();
        encodeIter.clear();

        // Prepare mocks & stabs
        when(reactorRole.type()).thenReturn(ReactorRoleTypes.CONSUMER);
        when(reactorRole.rttEnabled()).thenReturn(Boolean.TRUE);

        when(reactorChannel.role()).thenReturn(reactorRole);

        doReturn(ReactorCallbackReturnCodes.SUCCESS).when(reactorSpy)
                .sendAndHandleLoginMsgCallback(anyString(), any(ReactorChannel.class),
                any(TransportBuffer.class), any(Msg.class), any(LoginMsg.class),
                any(ReactorErrorInfo.class));

        doReturn(ReactorCallbackReturnCodes.SUCCESS).when(reactorSpy)
                .sendAndHandleDefaultMsgCallback(anyString(), any(ReactorChannel.class),
                any(TransportBuffer.class), any(Msg.class), any(ReactorErrorInfo.class));
    }

    @Test
    public void processChannelMessage_HandleCCSMessageInLoginDomain_SendItToLoginMsgCallback() {

        Msg msg = TestUtil.createLoginCCSMessage(encodeIter, decodeIter, LOGIN_STREAM_ID);

        int ret = reactorSpy.processChannelMessage(reactorChannel, decodeIter, msg, transportBuffer, errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        verify(reactorSpy).sendAndHandleLoginMsgCallback(eq("Reactor.processLoginMessage"),
                        any(ReactorChannel.class), any(TransportBuffer.class), any(Msg.class),
                        any(LoginMsg.class), any(ReactorErrorInfo.class));
    }

    @Test
    public void processChannelMessage_HandleRTTMessageWithTicksInLoginDomain_SendItToLoginMsgCallback() {

        Msg msg = TestUtil.createLoginRTTMessage(encodeIter, decodeIter, LOGIN_STREAM_ID, true);

        int ret = reactorSpy.processChannelMessage(reactorChannel, decodeIter, msg, transportBuffer, errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        verify(reactorSpy).sendAndHandleLoginMsgCallback(eq("Reactor.processLoginMessage"),
                any(ReactorChannel.class), any(TransportBuffer.class), any(Msg.class),
                any(LoginMsg.class), any(ReactorErrorInfo.class));
    }

    @Test
    public void processChannelMessage_HandleRTTMessageWithoutTicksInLoginDomain_SendItToDefaultMsgCallback() {

        Msg msg =  TestUtil.createLoginRTTMessage(encodeIter, decodeIter, LOGIN_STREAM_ID,false);

        int ret = reactorSpy.processChannelMessage(reactorChannel, decodeIter, msg, transportBuffer, errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        verify(reactorSpy).sendAndHandleDefaultMsgCallback(eq("Reactor.processLoginMessage"),
                any(ReactorChannel.class), any(TransportBuffer.class), any(Msg.class),
                any(ReactorErrorInfo.class));
    }

    @Test
    public void processChannelMessage_HandleGenericMessageInLoginDomain_SendItToDefaultMsgCallback() {

        Msg msg = TestUtil.createGenericMessage(DomainTypes.LOGIN, LOGIN_STREAM_ID);

        int ret = reactorSpy.processChannelMessage(reactorChannel, decodeIter, msg, transportBuffer, errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        verify(reactorSpy).sendAndHandleDefaultMsgCallback(eq("Reactor.processLoginMessage"),
                any(ReactorChannel.class), any(TransportBuffer.class), any(Msg.class),
                any(ReactorErrorInfo.class));
    }

    @Test
    public void processChannelMessage_HandleDirectoryCSMessageInDirectoryDomain_SendItToDirectoryMsgCallback() {
        Msg msg =  TestUtil.createDirectoryCSMessage(encodeIter, decodeIter, DIRECTORY_STREAM_ID);

        int ret = reactorSpy.processChannelMessage(reactorChannel, decodeIter, msg, transportBuffer, errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        verify(reactorSpy).sendAndHandleDirectoryMsgCallback(eq("Reactor.processDirectoryMessage"),
                any(ReactorChannel.class), any(TransportBuffer.class), any(Msg.class),
                any(DirectoryMsg.class), any(ReactorErrorInfo.class));

    }

    @Test
    public void processChannelMessage_HandleGenericMessageInDirectoryDomain_SendItToDefaultMsgCallback() {
        Msg msg = TestUtil.createGenericMessage(DomainTypes.SOURCE, DIRECTORY_STREAM_ID);

        int ret = reactorSpy.processChannelMessage(reactorChannel, decodeIter, msg, transportBuffer, errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        verify(reactorSpy).sendAndHandleDefaultMsgCallback(eq("Reactor.processDirectoryMessage"),
                any(ReactorChannel.class), any(TransportBuffer.class), any(Msg.class),
                any(ReactorErrorInfo.class));
    }
}
