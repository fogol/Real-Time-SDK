/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.Directory;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryClose;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.*;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.*;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import static org.junit.Assert.fail;

/* Utility functions for use with Reactor Junit tests. */
public class TestUtil
{
    /** Copies a ReactorErrorInfo object. */
    public static void copyErrorInfo(ReactorErrorInfo srcErrorInfo, ReactorErrorInfo destErrorInfo)
    {
        destErrorInfo.location(srcErrorInfo.location());
        destErrorInfo.error().channel(srcErrorInfo.error().channel());
        destErrorInfo.error().errorId(srcErrorInfo.error().errorId());
        destErrorInfo.error().sysError(srcErrorInfo.error().sysError());
        if (srcErrorInfo.error().text() != null)
            destErrorInfo.error().text(srcErrorInfo.error().text());
        destErrorInfo.code(srcErrorInfo.code());
    }

    /** Copy ReactorMsgEvent parts. */
    public static void copyMsgEvent(ReactorMsgEvent otherEvent, ReactorMsgEvent event)
    {
        if (otherEvent.msg() != null)
        {
            event.msg(CodecFactory.createMsg());
            otherEvent.msg().copy(event.msg(), CopyMsgFlags.ALL_FLAGS);
        }
        
        /* Copy transport buffer if present. */
        if (otherEvent.transportBuffer() != null)
            event.transportBuffer(new CopiedTransportBuffer(otherEvent.transportBuffer())); 
        
        if (otherEvent.streamInfo() != null)
        {
            event.streamInfo().userSpecObject(otherEvent.streamInfo().userSpecObject());

            if (otherEvent.streamInfo().serviceName() != null)
                event.streamInfo().serviceName(otherEvent.streamInfo().serviceName());
        }

        event.reactorChannel(otherEvent.reactorChannel());
        TestUtil.copyErrorInfo(otherEvent.errorInfo(), event.errorInfo());
    }

    /** Copies a LoginMsg. Used for both RDMLoginMsgEvent and TunnelStreamAuthInfo. */
    public static void copyLoginMsg(LoginMsg srcMsg, LoginMsg destMsg)
    {
        switch(srcMsg.rdmMsgType())
        {
            case REQUEST: 
                destMsg.rdmMsgType(LoginMsgType.REQUEST);
                ((LoginRequest)srcMsg).copy((LoginRequest)destMsg);
                break;
            case CLOSE:
                destMsg.rdmMsgType(LoginMsgType.CLOSE);
                ((LoginClose)srcMsg).copy((LoginClose)destMsg);
                break;
            case REFRESH:
                destMsg.rdmMsgType(LoginMsgType.REFRESH);
                ((LoginRefresh)srcMsg).copy((LoginRefresh)destMsg);
                break;
            case STATUS: 
                destMsg.rdmMsgType(LoginMsgType.STATUS);
                ((LoginStatus)srcMsg).copy((LoginStatus)destMsg);
                break;
            case CONSUMER_CONNECTION_STATUS:
                destMsg.rdmMsgType(LoginMsgType.CONSUMER_CONNECTION_STATUS);
                ((LoginConsumerConnectionStatus)srcMsg).copy((LoginConsumerConnectionStatus)destMsg); 
                break;
            case RTT:
                destMsg.rdmMsgType(LoginMsgType.RTT);
                ((LoginRTT) srcMsg).copy((LoginRTT) destMsg);
                break;
            default: 
                fail("Unknown LoginMsgType."); 
                break;
        }
    }

    /** Copies a DirectoryMsg. */
    public static void copyDirectoryMsg(DirectoryMsg srcMsg, DirectoryMsg destMsg)
    {
        switch(srcMsg.rdmMsgType())
        {
            case REQUEST:
                destMsg.rdmMsgType(DirectoryMsgType.REQUEST);
                ((DirectoryRequest)srcMsg).copy((DirectoryRequest)destMsg);
                break;
            case CLOSE:
                destMsg.rdmMsgType(DirectoryMsgType.CLOSE);
                ((DirectoryClose)srcMsg).copy((DirectoryClose)destMsg);
                break;
            case REFRESH:
                destMsg.rdmMsgType(DirectoryMsgType.REFRESH);
                ((DirectoryRefresh)srcMsg).copy((DirectoryRefresh)destMsg);
                break;
            case UPDATE:
                destMsg.rdmMsgType(DirectoryMsgType.UPDATE);
                ((DirectoryUpdate)srcMsg).copy((DirectoryUpdate)destMsg);
                break;
            case STATUS:
                destMsg.rdmMsgType(DirectoryMsgType.STATUS);
                ((DirectoryStatus)srcMsg).copy((DirectoryStatus)destMsg);
                break;
            case CONSUMER_STATUS: 
                destMsg.rdmMsgType(DirectoryMsgType.CONSUMER_STATUS);
                ((DirectoryConsumerStatus)srcMsg).copy((DirectoryConsumerStatus)destMsg);
                break;
            default: 
                fail("Unknown DirectoryMsgType.");
                break;
        }
    }

    /** Copies a DictionaryMsg. */
    public static void copyDictionaryMsg(DictionaryMsg srcMsg, DictionaryMsg destMsg)
    {
        switch(srcMsg.rdmMsgType())
        {
            case REQUEST:
                destMsg.rdmMsgType(DictionaryMsgType.REQUEST);
                ((DictionaryRequest)srcMsg).copy((DictionaryRequest)destMsg);
                break;
            case CLOSE:
                destMsg.rdmMsgType(DictionaryMsgType.CLOSE);
                ((DictionaryClose)srcMsg).copy((DictionaryClose)destMsg);
                break;
            case REFRESH:
                destMsg.rdmMsgType(DictionaryMsgType.REFRESH);
                ((DictionaryRefresh)srcMsg).copy((DictionaryRefresh)destMsg);
                break;
            case STATUS:
                destMsg.rdmMsgType(DictionaryMsgType.STATUS);
                ((DictionaryStatus)srcMsg).copy((DictionaryStatus)destMsg);
                break;
            default:
                fail("Unknown DictionaryMsgType.");
                break;
        }
    }

    static Msg createLoginCCSMessage(EncodeIterator encodeIter, DecodeIterator decodeIter, int streamId) {
        Buffer genericBuffer = CodecFactory.createBuffer();
        genericBuffer.data(ByteBuffer.allocate(512));

        decodeIter.clear();
        encodeIter.clear();
        encodeIter.setBufferAndRWFVersion(genericBuffer, Codec.majorVersion(), Codec.minorVersion());

        Msg msg = CodecFactory.createMsg();
        GenericMsg genericMsg = (GenericMsg)msg;
        genericMsg.msgClass(MsgClasses.GENERIC);
        genericMsg.domainType(DomainTypes.LOGIN);
        genericMsg.streamId(streamId);
        genericMsg.containerType(DataTypes.MAP);
        genericMsg.flags(GenericMsgFlags.HAS_MSG_KEY);
        genericMsg.msgKey().flags(MsgKeyFlags.HAS_NAME);
        genericMsg.msgKey().name(ElementNames.CONS_CONN_STATUS);
        genericMsg.encodeInit(encodeIter, 0);

        Map map = CodecFactory.createMap();
        MapEntry mapEntry = CodecFactory.createMapEntry();

        map.flags(MapFlags.NONE);
        map.keyPrimitiveType(DataTypes.ASCII_STRING);
        map.containerType(DataTypes.ELEMENT_LIST);
        map.encodeInit(encodeIter, 0, 0);

        mapEntry.clear();
        mapEntry.flags(MapEntryFlags.NONE);
        mapEntry.action(MapEntryActions.DELETE);
        mapEntry.encode(encodeIter, ElementNames.WARMSTANDBY_INFO);

        map.encodeComplete(encodeIter, true);
        genericMsg.encodeComplete(encodeIter, true);

        decodeIter.setBufferAndRWFVersion(genericBuffer, Codec.majorVersion(), Codec.minorVersion());
        msg.decode(decodeIter);

        return msg;
    }

    static Msg createDirectoryCSMessage(EncodeIterator encodeIter, DecodeIterator decodeIter, int streamId) {
        Buffer genericBuffer = CodecFactory.createBuffer();
        genericBuffer.data(ByteBuffer.allocate(512));

        decodeIter.clear();
        encodeIter.clear();
        encodeIter.setBufferAndRWFVersion(genericBuffer, Codec.majorVersion(), Codec.minorVersion());

        Msg msg = CodecFactory.createMsg();
        GenericMsg genericMsg = (GenericMsg)msg;
        genericMsg.msgClass(MsgClasses.GENERIC);
        genericMsg.domainType(DomainTypes.SOURCE);
        genericMsg.streamId(streamId);
        genericMsg.containerType(DataTypes.MAP);
        genericMsg.flags(GenericMsgFlags.HAS_MSG_KEY);
        genericMsg.msgKey().flags(MsgKeyFlags.HAS_NAME);
        genericMsg.msgKey().name(ElementNames.CONS_STATUS);
        genericMsg.encodeInit(encodeIter, 0);

        ConsumerStatusService consumerStatusService = DirectoryMsgFactory.createConsumerStatusService();
        consumerStatusService.sourceMirroringMode(Directory.SourceMirroringMode.ACTIVE_WITH_STANDBY);
        consumerStatusService.serviceId(streamId);
        consumerStatusService.action(MapEntryActions.ADD);
        List<ConsumerStatusService> consumerStatusServiceList = new ArrayList<>();
        consumerStatusServiceList.add(consumerStatusService);

        Map map = CodecFactory.createMap();
        MapEntry mapEntry = CodecFactory.createMapEntry();
        UInt tempUInt = CodecFactory.createUInt();

        map.flags(MapFlags.NONE);
        map.keyPrimitiveType(DataTypes.UINT);
        map.containerType(DataTypes.ELEMENT_LIST);
        map.encodeInit(encodeIter, 0, 0);

        for(ConsumerStatusService serviceStatus : consumerStatusServiceList)
        {
            mapEntry.clear();
            mapEntry.flags(MapEntryFlags.NONE);
            mapEntry.action(serviceStatus.action());
            tempUInt.value(serviceStatus.serviceId());
            if (mapEntry.action() != MapEntryActions.DELETE)
            {
                mapEntry.encodeInit(encodeIter, tempUInt, 0);
                serviceStatus.encode(encodeIter);
                mapEntry.encodeComplete(encodeIter, true);
            }
            else
            {
                mapEntry.encode(encodeIter, tempUInt);
            }
        }

        map.encodeComplete(encodeIter, true);
        genericMsg.encodeComplete(encodeIter, true);

        decodeIter.setBufferAndRWFVersion(genericBuffer, Codec.majorVersion(), Codec.minorVersion());
        msg.decode(decodeIter);

        return msg;
    }

    static Msg createLoginRTTMessage(EncodeIterator encodeIter, DecodeIterator decodeIter,
                                     int streamId, boolean withTicks) {
        Buffer genericBuffer = CodecFactory.createBuffer();
        genericBuffer.data(ByteBuffer.allocate(512));

        decodeIter.clear();
        encodeIter.clear();
        encodeIter.setBufferAndRWFVersion(genericBuffer, Codec.majorVersion(), Codec.minorVersion());

        Msg msg = CodecFactory.createMsg();
        GenericMsg genericMsg = (GenericMsg)msg;
        genericMsg.msgClass(MsgClasses.GENERIC);
        genericMsg.domainType(DomainTypes.LOGIN);
        genericMsg.streamId(streamId);
        genericMsg.containerType(DataTypes.ELEMENT_LIST);
        genericMsg.flags(GenericMsgFlags.PROVIDER_DRIVEN);
        genericMsg.encodeInit(encodeIter, 0);

        ElementList elementList = CodecFactory.createElementList();

        elementList.flags(ElementListFlags.HAS_STANDARD_DATA);
        elementList.encodeInit(encodeIter, null, 0);

        if (withTicks)
        {
            ElementEntry elementEntry = CodecFactory.createElementEntry();
            UInt tmpUInt = CodecFactory.createUInt();

            elementEntry.dataType(DataTypes.UINT);
            elementEntry.name(ElementNames.TICKS);
            tmpUInt.value(1);
            elementEntry.encode(encodeIter, tmpUInt);
        }

        elementList.encodeComplete(encodeIter, true);
        genericMsg.encodeComplete(encodeIter, true);

        decodeIter.setBufferAndRWFVersion(genericBuffer, Codec.majorVersion(), Codec.minorVersion());
        msg.decode(decodeIter);

        return msg;
    }

    static Msg createGenericMessage(int domainTypes, int streamId) {
        Msg msg = CodecFactory.createMsg();
        GenericMsg genericMsg = (GenericMsg)msg;
        genericMsg.msgClass(MsgClasses.GENERIC);
        genericMsg.domainType(domainTypes);
        genericMsg.streamId(streamId);
        genericMsg.containerType(DataTypes.NO_DATA);
        genericMsg.applyHasMsgKey();
        genericMsg.msgKey().applyHasName();
        genericMsg.msgKey().name().data("genericMsgInGeneral");

        return msg;
    }
}
