///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2025 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.rdm.ViewTypes;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import org.junit.Before;
import org.junit.Test;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import static org.junit.Assert.assertEquals;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

public class ReactorWatchlistItemHandlerJunit {
    private final ReactorChannel reactorChannel = mock(ReactorChannel.class);
    private final ConsumerRole consumerRole = mock(ConsumerRole.class);
    private final Watchlist watchlist = mock(Watchlist.class);
    private final WlDirectoryHandler directoryHandler = mock(WlDirectoryHandler.class);
    private final WlService wlService = mock(WlService.class);
    private final ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
    private final ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
    private final ConsumerWatchlistOptions consumerWatchlistOptions = new ConsumerWatchlistOptions();
    private final Service.ServiceState serviceState = new Service.ServiceState();
    private final Service.ServiceInfo serviceInfo = new Service.ServiceInfo();
    private final Service rdmService = DirectoryMsgFactory.createService();
    private final List<Long> capabilitiesList = new ArrayList<>();

    @Before
    public void init() {
        // Prepare mocks & stabs
        consumerWatchlistOptions.obeyOpenWindow(false);

        serviceState.serviceState(1);

        capabilitiesList.add((long) DomainTypes.MARKET_PRICE);
        serviceInfo.capabilitiesList(capabilitiesList);

        rdmService.applyHasState();
        rdmService.applyHasInfo();
        rdmService.state(serviceState);
        rdmService.info(serviceInfo);

        when(reactorChannel.majorVersion()).thenReturn(14);
        when(reactorChannel.minorVersion()).thenReturn(1);
        when(reactorChannel.state()).thenReturn(ReactorChannel.State.UP);

        when(consumerRole.watchlistOptions()).thenReturn(consumerWatchlistOptions);

        when(watchlist.role()).thenReturn(consumerRole);
        when(watchlist.reactorChannel()).thenReturn(reactorChannel);
        when(watchlist.directoryHandler()).thenReturn(directoryHandler);
        when(watchlist.watchlistOptions()).thenReturn(consumerWatchlistOptions);

        when(directoryHandler.service(1)).thenReturn(wlService);

        when(wlService.rdmService()).thenReturn(rdmService);
    }

    @Test
    public void handleRequest_TwoSnapshotRequestsWithSameView_BothAddedIntoUserRequestList() {
        WlItemHandler itemHandler = new  WlItemHandler(watchlist);

        // Prepare view
        List<Integer> viewFieldList = new ArrayList<>();
        prepareView(viewFieldList, 22, 25);

        // Create first request message
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        prepareRequestMsg(false, viewFieldList, requestMsg);

        WlRequest wlRequest = ReactorFactory.createWlRequest();
        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(0, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());

        wlRequest.stream()._requestPending = true;
        wlRequest.stream()._refreshState = WlStream.RefreshStates.REFRESH_VIEW_PENDING;

        // Create second request message
        prepareRequestMsg(false, viewFieldList, requestMsg);

        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(0, wlRequest.stream().waitingRequestList().size());
        assertEquals(2, wlRequest.stream().userRequestList().size());
    }

    @Test
    public void handleRequest_TwoSnapshotRequestsWithDifferentViews_FirstAddedIntoUserRequestListSecondAddedIntoWaitingRequestList() {
        WlItemHandler itemHandler = new  WlItemHandler(watchlist);

        // Prepare first view
        List<Integer> viewFieldList = new ArrayList<>();
        prepareView(viewFieldList, 22, 25);

        // Create first request message
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        prepareRequestMsg(false, viewFieldList, requestMsg);

        WlRequest wlRequest = ReactorFactory.createWlRequest();
        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(0, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());

        wlRequest.stream()._requestPending = true;
        wlRequest.stream()._refreshState = WlStream.RefreshStates.REFRESH_VIEW_PENDING;

        // Prepare second view
        prepareView(viewFieldList, 0, 6);

        // Create second request message
        prepareRequestMsg(false, viewFieldList, requestMsg);

        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REQUEST, wlRequest.state());
        assertEquals(1, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());
    }

    @Test
    public void handleRequest_FirstStreamingRequestSecondSnapshotRequestWithSameView_BothAddedIntoUserRequestList() {
        WlItemHandler itemHandler = new  WlItemHandler(watchlist);

        // Prepare view
        List<Integer> viewFieldList = new ArrayList<>();
        prepareView(viewFieldList, 22, 25);

        // Create first request message
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        prepareRequestMsg(true, viewFieldList, requestMsg);

        WlRequest wlRequest = ReactorFactory.createWlRequest();
        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(0, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());

        wlRequest.stream()._requestPending = true;
        wlRequest.stream()._refreshState = WlStream.RefreshStates.REFRESH_VIEW_PENDING;

        // Create second request message
        prepareRequestMsg(false, viewFieldList, requestMsg);

        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(0, wlRequest.stream().waitingRequestList().size());
        assertEquals(2, wlRequest.stream().userRequestList().size());
    }

    @Test
    public void handleRequest_FirstStreamingRequestSecondSnapshotRequestWithDifferentViews_FirstAddedIntoUserRequestListSecondAddedIntoWaitingRequestList() {
        WlItemHandler itemHandler = new  WlItemHandler(watchlist);

        // Prepare first view
        List<Integer> viewFieldList = new ArrayList<>();
        prepareView(viewFieldList, 22, 25);

        // Create first request message
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        prepareRequestMsg(true, viewFieldList, requestMsg);

        WlRequest wlRequest = ReactorFactory.createWlRequest();
        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(0, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());

        wlRequest.stream()._requestPending = true;
        wlRequest.stream()._refreshState = WlStream.RefreshStates.REFRESH_VIEW_PENDING;

        // Prepare second view
        prepareView(viewFieldList, 0, 6);

        // Create second request message
        prepareRequestMsg(false, viewFieldList, requestMsg);

        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REQUEST, wlRequest.state());
        assertEquals(1, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());
    }

    @Test
    public void handleRequest_FirstSnapshotRequestSecondStreamingRequestWithSameView_FirstAddedIntoUserRequestListSecondAddedIntoWaitingRequestList() {
        WlItemHandler itemHandler = new  WlItemHandler(watchlist);

        // Prepare view
        List<Integer> viewFieldList = new ArrayList<>();
        prepareView(viewFieldList, 22, 25);

        // Create first request message
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        prepareRequestMsg(false, viewFieldList, requestMsg);

        WlRequest wlRequest = ReactorFactory.createWlRequest();
        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(0, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());

        wlRequest.stream()._requestPending = true;
        wlRequest.stream()._refreshState = WlStream.RefreshStates.REFRESH_VIEW_PENDING;

        // Create second request message
        prepareRequestMsg(true, viewFieldList, requestMsg);

        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REQUEST, wlRequest.state());
        assertEquals(1, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());
    }

    @Test
    public void handleRequest_FirstSnapshotRequestSecondStreamingRequestWithDifferentViews_FirstAddedIntoUserRequestListSecondAddedIntoWaitingRequestList() {
        WlItemHandler itemHandler = new  WlItemHandler(watchlist);

        // Prepare first view
        List<Integer> viewFieldList = new ArrayList<>();
        prepareView(viewFieldList, 22, 25);

        // Create first request message
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        prepareRequestMsg(false, viewFieldList, requestMsg);

        WlRequest wlRequest = ReactorFactory.createWlRequest();
        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(0, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());

        wlRequest.stream()._requestPending = true;
        wlRequest.stream()._refreshState = WlStream.RefreshStates.REFRESH_VIEW_PENDING;

        // Prepare second view
        prepareView(viewFieldList, 0, 6);

        // Create second request message
        prepareRequestMsg(true, viewFieldList, requestMsg);

        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REQUEST, wlRequest.state());
        assertEquals(1, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());
    }

    @Test
    public void handleRequest_TwoStreamingRequestsWithSameView_FirstAddedIntoUserRequestListSecondAddedIntoWaitingRequestList() {
        WlItemHandler itemHandler = new  WlItemHandler(watchlist);

        // Prepare view
        List<Integer> viewFieldList = new ArrayList<>();
        prepareView(viewFieldList, 22, 25);

        // Create first request message
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        prepareRequestMsg(true, viewFieldList, requestMsg);

        WlRequest wlRequest = ReactorFactory.createWlRequest();
        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(0, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());

        wlRequest.stream()._requestPending = true;
        wlRequest.stream()._refreshState = WlStream.RefreshStates.REFRESH_VIEW_PENDING;

        // Create second request message
        prepareRequestMsg(true, viewFieldList, requestMsg);

        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REQUEST, wlRequest.state());
        assertEquals(1, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());
    }

    @Test
    public void handleRequest_TwoStreamingRequestsWithDifferentViews_FirstAddedIntoUserRequestListSecondAddedIntoWaitingRequestList() {
        WlItemHandler itemHandler = new  WlItemHandler(watchlist);

        // Prepare first view
        List<Integer> viewFieldList = new ArrayList<>();
        prepareView(viewFieldList, 22, 25);

        // Create first request message
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        prepareRequestMsg(true, viewFieldList, requestMsg);

        WlRequest wlRequest = ReactorFactory.createWlRequest();
        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(0, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());

        wlRequest.stream()._requestPending = true;
        wlRequest.stream()._refreshState = WlStream.RefreshStates.REFRESH_VIEW_PENDING;

        // Prepare second view
        prepareView(viewFieldList, 0, 6);

        // Create second request message
        prepareRequestMsg(true, viewFieldList, requestMsg);

        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REQUEST, wlRequest.state());
        assertEquals(1, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());
    }

    private void prepareView(List<Integer> viewFieldList, int... values) {
        if(viewFieldList == null) {
            return;
        }

        viewFieldList.clear();
        for(int value : values) {
            viewFieldList.add(value);
        }
    }

    private void prepareRequestMsg(boolean isStreaming, List<Integer> viewFieldList, RequestMsg requestMsg) {
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        if(isStreaming) {
            requestMsg.applyStreaming();
        }
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().applyHasServiceId();
        requestMsg.applyHasView();
        requestMsg.msgKey().name().data("TRI.N");
        requestMsg.msgKey().serviceId(1);
        encodeViewFieldIdList(reactorChannel, viewFieldList, requestMsg);
    }

   private static void encodeViewFieldIdList(ReactorChannel rc, List<Integer> fieldIdList, RequestMsg msg)
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(1024));
        Int tempInt = CodecFactory.createInt();
        UInt tempUInt = CodecFactory.createUInt();
        Array viewArray = CodecFactory.createArray();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        encodeIter.setBufferAndRWFVersion(buf, rc.majorVersion(), rc.minorVersion());
        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

        elementList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(encodeIter, null, 0));

        elementEntry.clear();
        elementEntry.name(ElementNames.VIEW_TYPE);
        elementEntry.dataType(DataTypes.UINT);

        tempUInt.value(ViewTypes.FIELD_ID_LIST);

        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(encodeIter, tempUInt));

        elementEntry.clear();
        elementEntry.name(ElementNames.VIEW_DATA);
        elementEntry.dataType(DataTypes.ARRAY);
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encodeInit(encodeIter, 0));
        viewArray.primitiveType(DataTypes.INT);
        viewArray.itemLength(2);

        assertEquals(CodecReturnCodes.SUCCESS, viewArray.encodeInit(encodeIter));

        for (Integer viewField : fieldIdList)
        {
            arrayEntry.clear();
            tempInt.value(viewField);
            assertEquals(CodecReturnCodes.SUCCESS, arrayEntry.encode(encodeIter, tempInt));
        }
        assertEquals(CodecReturnCodes.SUCCESS, viewArray.encodeComplete(encodeIter, true));
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encodeComplete(encodeIter, true));
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(encodeIter, true));

        msg.containerType(DataTypes.ELEMENT_LIST);
        msg.encodedDataBody(buf);
    }
}
