/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.consumer;

import java.net.InetAddress;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.shared.provider.SymbolListItems;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceItem;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceRefresh;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceUpdate;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.InstrumentNameTypes;
import com.refinitiv.eta.shared.rdm.symbollist.SymbolListItem;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.reactor.*;

/*
 * This is the post handler for the ETA Value Add consumer application.
 * It provides methods for sending on stream and off stream post messages.
 */
class PostHandler
{
	private final int TRANSPORT_BUFFER_SIZE_MSG_POST = 5000;

    private int nextPostId;
    private int nextSeqNum;
    private double itemData;

    private final int POST_MESSAGE_FREQUENCY = 5; // in seconds
    private long nextPostTime;
    private boolean postWithMsg;
    private boolean shouldOffstreamPost;
    private boolean shouldOnstreamPost;
    private boolean shouldSymbolListPost;
    private boolean hasInputPrincipalIdentitity;
    private String publisherId;
    private String publisherAddress;
    private boolean postRefresh;
    private Buffer postNestedMsgPayLoad;
    private final MarketPriceRefresh marketPriceRefresh = new MarketPriceRefresh();
    private final MarketPriceUpdate marketPriceUpdate = new MarketPriceUpdate();
    private final RefreshMsg slRefreshMsg = (RefreshMsg)CodecFactory.createMsg();
    private final UpdateMsg slUpdateMsg = (UpdateMsg)CodecFactory.createMsg();
    private int streamId;
    private final Buffer postItemName;
    private int serviceId;
    private DataDictionary dictionary;
    private final Map slMap = CodecFactory.createMap();
    private final MapEntry slMapEntry = CodecFactory.createMapEntry();
    private final Buffer slBuffer = CodecFactory.createBuffer();
    private final List<SymbolListItem> symbolListItemList = new ArrayList<>();

    private final PostMsg postMsg = (PostMsg)CodecFactory.createMsg();
    private final FieldList fList = CodecFactory.createFieldList();
    private final FieldEntry fEntry = CodecFactory.createFieldEntry();
    private final Real tempReal = CodecFactory.createReal();
    private final EncodeIterator encIter = CodecFactory.createEncodeIterator();
    private final EncodeIterator postMsgEncIter = CodecFactory.createEncodeIterator();
    private final StatusMsg statusMsg = (StatusMsg)CodecFactory.createMsg();
    private boolean offstreamPostSent = false;
    private boolean symbolListPostSent = false;
    private boolean isUpdateAdd = true;

    private final ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

    PostHandler()
    {
        postWithMsg = true;
        shouldOffstreamPost = false;
        shouldOnstreamPost = false;
        shouldSymbolListPost = false;
        hasInputPrincipalIdentitity = false;
        nextPostId = 1;
        nextSeqNum = 1;
        itemData = 12.00;
        postItemName = CodecFactory.createBuffer();
        postRefresh = true;
        initSymbolListItemList();
    }

    /**
     * Initializes the symbol list item fields.
     */
    private void initSymbolListItemList()
    {
        SymbolListItem item = new SymbolListItem();
        item.isInUse = true;
        item.itemName.data("SPY.DG");
        symbolListItemList.add(item);
        item = new SymbolListItem();
        item.isInUse = true;
        item.itemName.data("AMZN.O");
        symbolListItemList.add(item);
    }

    /* Enables offstream posting mode */
    void enableOffstreamPost(boolean shouldOffstreamPost)
    {
        this.shouldOffstreamPost = shouldOffstreamPost;
    }

    /* Enables onstream posting mode */
    void enableOnstreamPost(boolean shouldOnstreamPost)
    {
        this.shouldOnstreamPost = shouldOnstreamPost;
    }

    /* Enables symbol list posting mode */
    void enableSymbolListPost(boolean shouldSymbolListPost)
    {
        this.shouldSymbolListPost = shouldSymbolListPost;
    }

    /*
     * Returns whether or not offstream posting mode is enabled.
     */
    boolean enableOffstreamPost()
    {
        return shouldOffstreamPost;
    }

    /*
     * Returns whether or not onstream posting mode is enabled.
     */
    boolean enableOnstreamPost()
    {
        return this.shouldOnstreamPost;
    }

    /*
     * Returns whether or not symbol list posting mode is enabled.
     */
    boolean enableSymbolListPost()
    {
        return this.shouldSymbolListPost;
    }

    /* 
     * Set user provided publisher id and publisher address
     */
    void setPublisherInfo(String publisherId, String publisherAddress)
    {
    	this.hasInputPrincipalIdentitity = true;
    	this.publisherId = publisherId;
    	this.publisherAddress = publisherAddress;
    }
    
    
    /* increases the value of the data being posted */
    private double nextItemData()
    {
        itemData += 0.01;
        return itemData;
    }

    /*
     * Initializes timer for Post messages to be sent This method simply gets
     * the current time and sets up the first time that a post message should be
     * sent based off of that and the post interval.
     */
    void initPostHandler()
    {
        nextPostTime = System.currentTimeMillis() + POST_MESSAGE_FREQUENCY * 1000;
    }

    /*
     * Uses the current time and the nextPostTime to determine whether a postMsg
     * should be sent. If a post message should be sent, the time is calculated
     * for the next post after this one. Additionally, the postWithMsg flag is
     * toggled so that posting alternates between posting with a full message as
     * payload or data as payload.
     */
    int handlePosts(ReactorChannel chnl, ReactorErrorInfo errorInfo)
    {
        long currentTime = System.currentTimeMillis();

        if (currentTime >= nextPostTime)
        {
            if (shouldOnstreamPost)
            {
                int ret = sendOnstreamPostMsg(chnl, postWithMsg, errorInfo);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }
            if (shouldSymbolListPost)
            {
                int ret = sendSymbolListPostMsg(chnl, errorInfo);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }
            if (shouldOffstreamPost)
            {
                int ret = sendOffstreamPostMsg(chnl, errorInfo);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }

            nextPostTime = currentTime + POST_MESSAGE_FREQUENCY * 1000;

            /* iterate between post with msg and post with data */
            postWithMsg = !postWithMsg;
            if (errorInfo != null)
                System.out.println(errorInfo.error().text());
        }

        return CodecReturnCodes.SUCCESS;
    }

    /**
     * Encodes and sends an on-stream post message.
     * 
     * It will either send a post that contains a full message or a post that
     * contains only data payload, based on the postWithMsg parameter. If true,
     * post will contain a message.
     * 
     * This method only sends one post message, however it is called
     * periodically over a time increment by the handlePosts method
     */
    private int sendOnstreamPostMsg(ReactorChannel chnl, boolean postWithMsg, ReactorErrorInfo errorInfo)
    {
        if (streamId == 0)
        {
            // no items available to post on
            System.out.println("Currently no available market price streams to on-stream post to.  Will retry shortly.");
            return CodecReturnCodes.SUCCESS;
        }

        int ret;
        if (postWithMsg)
        {

            if ((ret = encodePostWithMsg(chnl, DomainTypes.MARKET_PRICE, errorInfo)) != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }
        else
        {
            if ((ret = encodePostWithData(chnl, DomainTypes.MARKET_PRICE, errorInfo)) != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }

        // send post message
        ret = chnl.submit(postMsg, submitOptions, errorInfo);
        if (ret != ReactorReturnCodes.SUCCESS)
            return CodecReturnCodes.FAILURE;

        return CodecReturnCodes.SUCCESS;
    }

    /**
     * Encodes and sends an off-stream post message.
     * 
     * This method only sends one post message, however it is called
     * periodically over a time increment by the handlePosts method
     */
    private int sendOffstreamPostMsg(ReactorChannel chnl, ReactorErrorInfo errorInfo)
    {
        int ret = encodePostWithMsg(chnl, DomainTypes.MARKET_PRICE, errorInfo);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        // send post message
        ret = chnl.submit(postMsg, submitOptions, errorInfo);
        if (ret != ReactorReturnCodes.SUCCESS)
            return CodecReturnCodes.FAILURE;

        offstreamPostSent = true;
        return CodecReturnCodes.SUCCESS;
    }

    /**
     * Encodes and sends a Symbol List post message.
     *
     * This method only sends one post message, however it is called
     * periodically over a time increment by the handlePosts method
     */
    private int sendSymbolListPostMsg(ReactorChannel chnl, ReactorErrorInfo errorInfo)
    {
        int ret = encodePostWithMsg(chnl, DomainTypes.SYMBOL_LIST, errorInfo);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        ret = chnl.submit(postMsg, submitOptions, errorInfo);
        if (ret != ReactorReturnCodes.SUCCESS)
            return CodecReturnCodes.FAILURE;

        symbolListPostSent = true;
        return CodecReturnCodes.SUCCESS;
    }

    /* This method is internally used by sendPostMsg */
    /*
     * It encodes a PostMsg, populating the postUserInfo with the IPAddress and
     * process ID of the machine running the application. The payload of the
     * PostMsg is an UpdateMsg. The UpdateMsg contains a FieldList containing
     * several fields. The same message encoding functionality used by the
     * provider application is leveraged here to encode the contents.
     */
    private int encodePostWithMsg(ReactorChannel chnl, int domain, ReactorErrorInfo errorInfo)
    {
        // First encode message for payload
        postMsg.clear();

        // set-up message
        postMsg.msgClass(MsgClasses.POST);
        postMsg.streamId(streamId);
        postMsg.domainType(domain);
        postMsg.containerType(DataTypes.MSG);

        // Note: post message key not required for on-stream post
        postMsg.applyPostComplete();
        postMsg.applyAck();
        postMsg.applyHasPostId();
        postMsg.applyHasSeqNum();
        postMsg.applyHasMsgKey();
        postMsg.applyHasPostUserRights();
        postMsg.postId(nextPostId++);
        postMsg.seqNum(nextSeqNum++);

       	// populate post user info        
        if (hasInputPrincipalIdentitity)
        {
        	postMsg.postUserInfo().userAddr(publisherAddress);	
        	postMsg.postUserInfo().userId(Integer.parseInt(publisherId));
        	
        }
        else
        {
        	try
        	{
        		postMsg.postUserInfo().userAddr(InetAddress.getLocalHost().getHostAddress());
        	}
        	catch (Exception e)
        	{
        		System.out.println("Populating postUserInfo failed. InetAddress.getLocalHost().getHostAddress exception: " + e.getLocalizedMessage());
        		return CodecReturnCodes.FAILURE;
        	}
            postMsg.postUserInfo().userId(Integer.parseInt(System.getProperty("pid", "1")));
        }
 
        postMsg.postUserRights(PostUserRights.CREATE | PostUserRights.DELETE);

        postMsg.msgKey().applyHasNameType();
        postMsg.msgKey().applyHasName();
        postMsg.msgKey().applyHasServiceId();
        postMsg.msgKey().name().data(postItemName.data(), postItemName.position(), postItemName.length());

        postMsg.msgKey().nameType(InstrumentNameTypes.RIC);
        postMsg.msgKey().serviceId(serviceId);

        // encode market price response into a buffer
        MarketPriceItem mpItemInfo = null;
        if (domain == DomainTypes.MARKET_PRICE)
        {
            mpItemInfo = new MarketPriceItem();
            mpItemInfo.initFields();
            mpItemInfo.TRDPRC_1 = nextItemData();
            mpItemInfo.BID = nextItemData();
            mpItemInfo.ASK = nextItemData();
        }

        // get a buffer for nested market price refresh
        postNestedMsgPayLoad = CodecFactory.createBuffer();
        postNestedMsgPayLoad.data(ByteBuffer.allocate(1024));

        postMsgEncIter.clear();
       int ret = postMsgEncIter.setBufferAndRWFVersion(postNestedMsgPayLoad, chnl.majorVersion(), chnl.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("EncodeIter.setBufferAndRWFVersion() failed:  <" + CodecReturnCodes.toString(ret));
            return CodecReturnCodes.FAILURE;
        }

        if (postRefresh)
        {
            if (domain == DomainTypes.MARKET_PRICE)
            {
                marketPriceRefresh.clear();
                marketPriceRefresh.applyRefreshComplete();
                marketPriceRefresh.applyClearCache();
                marketPriceRefresh.streamId(streamId);
                marketPriceRefresh.itemName().data(postItemName.data(), postItemName.position(), postItemName.length());
                marketPriceRefresh.state().streamState(StreamStates.OPEN);
                marketPriceRefresh.state().dataState(DataStates.OK);
                marketPriceRefresh.state().code(StateCodes.NONE);
                marketPriceRefresh.state().text().data("Item Refresh Completed");
                marketPriceRefresh.serviceId(serviceId);
                marketPriceRefresh.applyHasServiceId();
                marketPriceRefresh.marketPriceItem(mpItemInfo);
                marketPriceRefresh.applyHasQos();
                marketPriceRefresh.qos().dynamic(false);
                marketPriceRefresh.qos().rate(QosRates.TICK_BY_TICK);
                marketPriceRefresh.qos().timeliness(QosTimeliness.REALTIME);
                marketPriceRefresh.dictionary(dictionary);

                ret = marketPriceRefresh.encode(postMsgEncIter);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    System.out.println("encodeMarketPriceRefresh() failed:  <" + CodecReturnCodes.toString(ret));
                    return CodecReturnCodes.FAILURE;
                }
            }

            if (domain == DomainTypes.SYMBOL_LIST)
            {
                slRefreshMsg.clear();
                slRefreshMsg.msgClass(MsgClasses.REFRESH);
                slRefreshMsg.state().streamState(StreamStates.OPEN);
                slRefreshMsg.state().dataState(DataStates.OK);
                slRefreshMsg.state().code(StateCodes.NONE);
                slRefreshMsg.flags(RefreshMsgFlags.HAS_MSG_KEY | RefreshMsgFlags.SOLICITED | RefreshMsgFlags.REFRESH_COMPLETE | RefreshMsgFlags.HAS_QOS);
                slRefreshMsg.state().text().data("Item Refresh Completed");
                slRefreshMsg.msgKey().flags(MsgKeyFlags.HAS_SERVICE_ID | MsgKeyFlags.HAS_NAME);
                slRefreshMsg.msgKey().serviceId(serviceId);
                slRefreshMsg.msgKey().name().data(postItemName.data(), postItemName.position(), postItemName.length());
                slRefreshMsg.qos().dynamic(false);
                slRefreshMsg.qos().rate(QosRates.TICK_BY_TICK);
                slRefreshMsg.qos().timeliness(QosTimeliness.REALTIME);
                slRefreshMsg.domainType(DomainTypes.SYMBOL_LIST);
                slRefreshMsg.containerType(DataTypes.MAP);
                slRefreshMsg.streamId(streamId);

                ret = encodeSymbolListMsg(slRefreshMsg, postMsgEncIter, SymbolListItems.SYMBOL_LIST_REFRESH, errorInfo);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    System.out.println("encodeSymbolListMsg() failed:  <" + CodecReturnCodes.toString(ret));
                    return CodecReturnCodes.FAILURE;
                }
            }

            postRefresh = false;
        }
        else
        {
            if (domain == DomainTypes.MARKET_PRICE)
            {
                marketPriceUpdate.clear();
                marketPriceUpdate.streamId(streamId);
                marketPriceUpdate.itemName().data(postItemName.data(), postItemName.position(), postItemName.length());
                marketPriceUpdate.marketPriceItem(mpItemInfo);
                marketPriceUpdate.dictionary(dictionary);

                ret = marketPriceUpdate.encode(postMsgEncIter);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    System.out.println("encodeMarketPriceUpdate() failed:  <" + CodecReturnCodes.toString(ret));
                    return CodecReturnCodes.FAILURE;
                }
            }

            if (domain == DomainTypes.SYMBOL_LIST)
            {
                slUpdateMsg.clear();
                slUpdateMsg.msgClass(MsgClasses.UPDATE);
                slUpdateMsg.domainType(DomainTypes.SYMBOL_LIST);
                slUpdateMsg.containerType(DataTypes.MAP);
                slUpdateMsg.streamId(streamId);

                int responseType = isUpdateAdd ? SymbolListItems.SYMBOL_LIST_UPDATE_ADD : SymbolListItems.SYMBOL_LIST_UPDATE_DELETE;
                isUpdateAdd = !isUpdateAdd;
                ret = encodeSymbolListMsg(slUpdateMsg, postMsgEncIter, responseType, errorInfo);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    System.out.println("encodeSymbolListMsg() failed:  <" + CodecReturnCodes.toString(ret));
                    return CodecReturnCodes.FAILURE;
                }
            }
        }

        postMsg.encodedDataBody(postNestedMsgPayLoad);

        errorInfo.error().text("\n\nSENDING POST WITH MESSAGE:\n" + "  streamId = " + postMsg.streamId() + "\n  postId   = " + postMsg.postId() + "\n  seqNum   = " + postMsg.seqNum());

        return CodecReturnCodes.SUCCESS;
    }

    private int encodeSymbolListMsg(Msg msg, EncodeIterator encodeIter, int responseType, ReactorErrorInfo errorInfo)
    {
        int ret = msg.encodeInit(encodeIter, 0);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            errorInfo.error().text("Msg.encodeInit() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        /* encode map */
        slMap.clear();
        slMap.flags(0);
        slMap.containerType(DataTypes.NO_DATA);
        slMap.keyPrimitiveType(DataTypes.BUFFER);
        ret = slMap.encodeInit(encodeIter, 0, 0);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            errorInfo.error().text("map.encodeInit() failed with return code: " + ret);
            return ret;
        }

        int i = 0;
        /* encode map entry */
        slMapEntry.clear();
        slMapEntry.flags(MapEntryFlags.NONE);
        slBuffer.clear();
        switch (responseType)
        {
            // this is a refresh message, so begin encoding the entire symbol list
            case SymbolListItems.SYMBOL_LIST_REFRESH:
                slBuffer.data(symbolListItemList.get(i).itemName.data());
                slMapEntry.action(MapEntryActions.ADD);
                break;
            // this is an update message adding a name
            case SymbolListItems.SYMBOL_LIST_UPDATE_ADD:
                SymbolListItem updItem = new SymbolListItem();
                updItem.itemName.data("IBM.N");
                slBuffer.data(updItem.itemName.data());
                slMapEntry.action(MapEntryActions.ADD);
                break;

            // this is an update message deleting a name
            case SymbolListItems.SYMBOL_LIST_UPDATE_DELETE:
                SymbolListItem delItem = new SymbolListItem();
                delItem.itemName.data("IBM.N");
                slBuffer.data(delItem.itemName.data());
                slMapEntry.action(MapEntryActions.DELETE);
                break;
            default:
                errorInfo.error().text("Invalid SymbolListItems responseType: " + responseType);
                return CodecReturnCodes.FAILURE;
        }

        ret = slMapEntry.encodeInit(encodeIter, slBuffer, 0);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            errorInfo.error().text("MapEntry.encodeInit() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        ret = slMapEntry.encodeComplete(encodeIter, true);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            errorInfo.error().text("MapEntry.encodeComplete() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        /*
         * if this is a refresh message, finish encoding the entire symbol list
         * in the response
         */
        if (responseType == SymbolListItems.SYMBOL_LIST_REFRESH)
        {
            for (i = 1; i < symbolListItemList.size(); i++)
            {
                if (symbolListItemList.get(i).isInUse)
                {
                    slBuffer.data(symbolListItemList.get(i).itemName.data());
                    ret = slMapEntry.encode(encodeIter, slBuffer);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        errorInfo.error().text("mapEntry.encode() failed with return code: " + CodecReturnCodes.toString(ret));
                        return ret;
                    }
                }
            }
        }

        /* complete map */
        ret = slMap.encodeComplete(encodeIter, true);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            errorInfo.error().text("mapEntry.encodeComplete() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        /* complete encode message */
        ret = msg.encodeComplete(encodeIter, true);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            errorInfo.error().text("msg.encodeComplete() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    /* This method is internally used by sendPostMsg */
    /*
     * It encodes a PostMsg, populating the postUserInfo with the IPAddress and
     * process ID of the machine running the application. The payload of the
     * PostMsg is a FieldList containing several fields.
     */
    private int encodePostWithData(ReactorChannel chnl, int domain, ReactorErrorInfo errorInfo)
    {
        int ret;
        DictionaryEntry dictionaryEntry;
        MarketPriceItem itemInfo = new MarketPriceItem();

        // clear encode iterator
        encIter.clear();
        fList.clear();
        fEntry.clear();

        // set-up message
        postMsg.msgClass(MsgClasses.POST);
        postMsg.streamId(streamId);
        postMsg.domainType(domain);
        postMsg.containerType(DataTypes.FIELD_LIST);

        // Note: post message key not required for on-stream post
        postMsg.applyPostComplete();
        postMsg.applyAck(); // request ACK
        postMsg.applyHasPostId();
        postMsg.applyHasSeqNum();
        postMsg.postId(nextPostId++);
        postMsg.partNum(nextSeqNum);
        postMsg.seqNum(nextSeqNum++);

        // populate post user info
        if (hasInputPrincipalIdentitity)
        {
            postMsg.postUserInfo().userAddr(publisherAddress);
            postMsg.postUserInfo().userId(Integer.parseInt(publisherId));

        }
        else
        {
            try
            {
                postMsg.postUserInfo().userAddr(InetAddress.getLocalHost().getHostAddress());
            }
            catch (Exception e)
            {
                System.out.println("Populating postUserInfo failed. InetAddress.getLocalHost().getHostAddress exception: " + e.getLocalizedMessage());
                return CodecReturnCodes.FAILURE;
            }
            postMsg.postUserInfo().userId(Integer.parseInt(System.getProperty("pid", "1")));
        }

        postNestedMsgPayLoad = CodecFactory.createBuffer();
        postNestedMsgPayLoad.data(ByteBuffer.allocate(1024));
        // encode message
        encIter.setBufferAndRWFVersion(postNestedMsgPayLoad, chnl.majorVersion(), Codec.minorVersion());

        itemInfo.initFields();
        itemInfo.TRDPRC_1 = nextItemData();
        itemInfo.BID = nextItemData();
        itemInfo.ASK = nextItemData();

        // encode field list
        fList.applyHasStandardData();
        if ((ret = fList.encodeInit(encIter, null, 0)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("EncodeFieldListInit() failed: <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        // TRDPRC_1
        fEntry.clear();
        dictionaryEntry = dictionary.entry(MarketPriceItem.TRDPRC_1_FID);
        if (dictionaryEntry != null)
        {
            fEntry.fieldId(MarketPriceItem.TRDPRC_1_FID);
            fEntry.dataType(dictionaryEntry.rwfType());
            tempReal.clear();
            tempReal.value(itemInfo.TRDPRC_1, RealHints.EXPONENT_2);
            if ((ret = fEntry.encode(encIter, tempReal)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("EncodeFieldEntry() failed: <" + CodecReturnCodes.toString(ret) + ">");
                return ret;
            }
        }
        // BID
        fEntry.clear();
        dictionaryEntry = dictionary.entry(MarketPriceItem.BID_FID);
        if (dictionaryEntry != null)
        {
            fEntry.fieldId(MarketPriceItem.BID_FID);
            fEntry.dataType(dictionaryEntry.rwfType());
            tempReal.clear();
            tempReal.value(itemInfo.BID, RealHints.EXPONENT_2);
            if ((ret = fEntry.encode(encIter, tempReal)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("EncodeFieldEntry() failed: <" + CodecReturnCodes.toString(ret) + ">");

                return ret;
            }
        }
        // ASK
        fEntry.clear();
        dictionaryEntry = dictionary.entry(MarketPriceItem.ASK_FID);
        if (dictionaryEntry != null)
        {
            fEntry.fieldId(MarketPriceItem.ASK_FID);
            fEntry.dataType(dictionaryEntry.rwfType());
            tempReal.clear();
            tempReal.value(itemInfo.ASK, RealHints.EXPONENT_2);
            if ((ret = fEntry.encode(encIter, tempReal)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("EncodeFieldEntry() failed: <" + CodecReturnCodes.toString(ret) + ">");
                return ret;
            }
        }

        // complete encode field list
        if ((ret = fList.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("EncodeFieldListComplete() failed: <" + CodecReturnCodes.toString(ret) + ">");

            return ret;
        }

        errorInfo.error().text("\n\nSENDING POST WITH DATA:\n" + "  streamId = " + postMsg.streamId() + "\n  postId   = " + postMsg.postId() + "\n  seqNum   = " + postMsg.seqNum() + "\n");

        postMsg.encodedDataBody(postNestedMsgPayLoad);

        return CodecReturnCodes.SUCCESS;
    }

    /*
     * This function encodes and sends an off-stream post close status message.
     */
    int closeOffStreamPost(ReactorChannel chnl, int domain, ReactorErrorInfo errorInfo)
    {
        // first check if we have sent offstream posts
        if ((domain == DomainTypes.MARKET_PRICE && !offstreamPostSent) ||
                (domain == DomainTypes.SYMBOL_LIST && !symbolListPostSent))
            return CodecReturnCodes.SUCCESS;

        // get a buffer for the item request
        TransportBuffer msgBuf = chnl.getBuffer(TRANSPORT_BUFFER_SIZE_MSG_POST, false, errorInfo);
        if (msgBuf == null)
            return CodecReturnCodes.FAILURE;

        postMsg.clear();

        // set-up post message
        postMsg.msgClass(MsgClasses.POST);
        postMsg.streamId(streamId);
        postMsg.domainType(domain);
        postMsg.containerType(DataTypes.MSG);

        // Note: This example sends a status close when it shuts down.
        // So don't ask for an ACK (that we will never get)
        postMsg.flags(PostMsgFlags.POST_COMPLETE | PostMsgFlags.HAS_POST_ID | PostMsgFlags.HAS_SEQ_NUM | PostMsgFlags.HAS_POST_USER_RIGHTS | PostMsgFlags.HAS_MSG_KEY);

        postMsg.postId(nextPostId++);
        postMsg.seqNum(nextSeqNum++);
        postMsg.postUserRights(PostUserRights.CREATE | PostUserRights.DELETE);

        // set post item name
        postMsg.msgKey().applyHasNameType();
        postMsg.msgKey().applyHasName();
        postMsg.msgKey().applyHasServiceId();
        postMsg.msgKey().name().data(postItemName.data(), postItemName.position(), postItemName.length());
        postMsg.msgKey().nameType(InstrumentNameTypes.RIC);
        postMsg.msgKey().serviceId(serviceId);

       	// populate default post user info        
        if (hasInputPrincipalIdentitity)
        {
        	postMsg.postUserInfo().userAddr(publisherAddress);	
        	postMsg.postUserInfo().userId(Integer.parseInt(publisherId));        	
        }
        else
        {                            
        	try
        	{
        		postMsg.postUserInfo().userAddr(InetAddress.getLocalHost().getHostAddress());
        	}
        	catch (Exception e)
        	{
        		System.out.println("Populating postUserInfo failed. InetAddress.getLocalHost().getHostAddress exception: " + e.getLocalizedMessage());
        		return CodecReturnCodes.FAILURE;
        	}
        	postMsg.postUserInfo().userId(Integer.parseInt(System.getProperty("pid", "1")));
        }
        
        // set-up status message that will be nested in the post message
        statusMsg.flags(StatusMsgFlags.HAS_STATE);
        statusMsg.msgClass(MsgClasses.STATUS);
        statusMsg.streamId(streamId);
        statusMsg.domainType(domain);
        statusMsg.containerType(DataTypes.NO_DATA);
        statusMsg.state().streamState(StreamStates.CLOSED);
        statusMsg.state().dataState(DataStates.SUSPECT);

        // encode post message
        encIter.clear();
        int ret = encIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("Encoder.setBufferAndRWFVersion() failed:  <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        ret = postMsg.encodeInit(encIter, 0);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            System.out.println("PostMsg.encodeInit() failed:  <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        // encode nested status message
        ret = statusMsg.encodeInit(encIter, 0);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            System.out.println("StatusMsg.encodeInit() failed:  <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        // complete encode status message
        ret = statusMsg.encodeComplete(encIter, true);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            System.out.println("StatusMsg.encodeComplete() failed:  <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        // complete encode post message
        ret = postMsg.encodeComplete(encIter, true);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            System.out.println("PostMsg.encodeComplete() failed:  <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        // send post message
        return chnl.submit(msgBuf, submitOptions, errorInfo);
    }

    /*
     * Stream Id for posting data.
     */
    int streamId()
    {
        return streamId;
    }

    /*
     * Stream Id for posting data.
     */
    void streamId(int streamId)
    {
        this.streamId = streamId;
    }

    /*
     * Item name for posting message.
     */
    Buffer postItemName()
    {
        return postItemName;
    }

    /*
     * Service id for posting message.
     */
    int serviceId()
    {
        return serviceId;
    }

    /*
     * Service id for posting message.
     */
    void serviceId(int serviceId)
    {
        this.serviceId = serviceId;
    }

    /*
     * Data dictionary used for encoding posting data.
     */
    DataDictionary dictionary()
    {
        return dictionary;
    }

    /*
     * Data dictionary used for encoding posting data.
     */
    void dictionary(DataDictionary dictionary)
    {
        this.dictionary = dictionary;
    }
}