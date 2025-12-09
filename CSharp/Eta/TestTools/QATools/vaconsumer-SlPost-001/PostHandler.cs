/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net;
using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;


namespace LSEG.Eta.ValueAdd.Consumer
{
    /// <summary>
    /// This is the post handler for the ETA Value Add consumer application.
    /// </summary>
    ///
    /// It provides methods for sending on stream and off stream post messages.
    /// This is the post handler for the ETA Value Add consumer application.
    /// It provides methods for sending on stream and off stream post messages.
    ///
    internal class PostHandler
    {
        public bool EnableOnstreamPost { get; internal set; }

        public bool EnableOffstreamPost { get; internal set; }

        public bool EnableSymbolListPost { get; internal set; } = false;

        /// <summary>
        /// Stream Id for posting data.
        /// </summary>
        public int StreamId { get; internal set; }

        /// <summary>
        /// Stream Id for posting Symbol List data.
        /// </summary>
        public int SlStreamId { get; internal set; }

        /// <summary>
        /// Stream Id for posting Symbol List data.
        /// </summary>
        public int OffStreamId { get; internal set; }

        /// <summary>
        /// Service id for posting message.
        /// </summary>
        public int ServiceId { get; internal set; }

        /// <summary>
        /// Data dictionary used for encoding posting data.
        /// </summary>
        internal DataDictionary Dictionary { get; set; } = new();

        /// <summary>
        /// Item name for posting message.
        /// </summary>
        public Codec.Buffer PostItemName { get; internal set; } = new Eta.Codec.Buffer();

        /// <summary>
        /// Item name for posting message.
        /// </summary>
        public Codec.Buffer SlPostItemName { get; internal set; } = new Eta.Codec.Buffer();

        /// <summary>
        /// Item name for off-stream posting message.
        /// </summary>
        public Codec.Buffer OffPostItemName { get; internal set; } = new Eta.Codec.Buffer();

        private const int TRANSPORT_BUFFER_SIZE_MSG_POST = 5000;
        private const int TRANSPORT_BUFFER_SIZE_DATA_POST = 5000;

        private int m_NextPostId;
        private int m_NextSeqNum;
        private double m_ItemData;

        private readonly TimeSpan POST_MESSAGE_FREQUENCY = TimeSpan.FromSeconds(5);

        private System.DateTime m_NextPostTime = System.DateTime.MinValue;
        private bool m_PostWithMsg;
        private bool m_HasInputPrincipalIdentitity;
        private string m_PublisherId = string.Empty;
        private string m_PublisherAddress = string.Empty;
        private bool m_PostRefresh;
        private bool m_SlPostRefresh;
        private MarketPriceRefresh m_MarketPriceRefresh = new();
        private MarketPriceUpdate m_MarketPriceUpdate = new();
        private Eta.Codec.Buffer postNestedMsgPayLoad = new Eta.Codec.Buffer();

        private IPostMsg m_PostMsg = (IPostMsg)new Msg();
        private FieldList fList = new();
        private FieldEntry fEntry = new();
        private Codec.Real tempReal = new();
        private EncodeIterator m_EncodeIterator = new();
        private EncodeIterator m_PostMessageEncodeIterator = new();
        private IStatusMsg m_StatusMsg = (IStatusMsg)new Msg();
        private bool m_OffstreamPostSent = false;

        private ReactorSubmitOptions m_SubmitOptions = new();

        private IRefreshMsg slRefreshMsg = (IRefreshMsg)new Msg();
        private IUpdateMsg slUpdateMsg = (IUpdateMsg)new Msg();

        private Map slMap = new Map();
        private MapEntry slMapEntry = new MapEntry();
        private Eta.Codec.Buffer slBuffer = new Eta.Codec.Buffer();
        private List<SymbolListItem> symbolListItemList = new List<SymbolListItem>();

        private bool m_IsUpdateAdd = true;


        public PostHandler()
        {
            m_PostWithMsg = true;
            m_HasInputPrincipalIdentitity = false;
            m_NextPostId = 1;
            m_NextSeqNum = 1;
            m_ItemData = 12.00;
            m_PostRefresh = true;
            m_SlPostRefresh = true;
            InitSymbolListItemList();
        }

        /**
        * Initializes the symbol list item fields.
        */
        private void InitSymbolListItemList()
        {
            SymbolListItem item = new SymbolListItem();
            item.IsInUse = true;
            item.ItemName.Data("TRI");
            symbolListItemList.Add(item);
            item = new SymbolListItem();
            item.IsInUse = true;
            item.ItemName.Data("RES-DS");
            symbolListItemList.Add(item);
        }

        /// <summary>
        /// Set user provided publisher id and publisher address
        /// </summary>
        /// <param name="publisherId"></param>
        /// <param name="publisherAddress"></param>
        public void SetPublisherInfo(string publisherId, string publisherAddress)
        {
            this.m_HasInputPrincipalIdentitity = true;
            this.m_PublisherId = publisherId;
            this.m_PublisherAddress = publisherAddress;
        }


        /// <summary>
        /// increases the value of the data being posted
        /// </summary>
        /// <returns></returns>
        private double NextItemData()
        {
            m_ItemData += 0.01;
            return m_ItemData;
        }


        /// <summary>
        /// Initializes timer for Post messages to be sent This method simply gets
        /// the current time and sets up the first time that a post message should be
        /// </summary>
        public void InitPostHandler()
        {
            m_NextPostTime = System.DateTime.Now + POST_MESSAGE_FREQUENCY;
        }


        /// <summary>
        /// Uses the current time and the nextPostTime to determine whether a postMsg
        /// should be sent.
        /// </summary>
        ///
        /// If a post message should be sent, the time is calculated for the next post
        /// after this one. Additionally, the postWithMsg flag is toggled so that posting
        /// alternates between posting with a full message as payload or data as payload.
        public ReactorReturnCode HandlePosts(ReactorChannel chnl, out ReactorErrorInfo? errorInfo)
        {
            System.DateTime currentTime = System.DateTime.Now;

            if (currentTime >= m_NextPostTime)
            {
                if (EnableOnstreamPost)
                {
                    ReactorReturnCode ret = SendOnstreamPostMsg(chnl, m_PostWithMsg, out errorInfo);
                    if (ret != ReactorReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
                if (EnableOffstreamPost)
                {
                    ReactorReturnCode ret = SendOffstreamPostMsg(chnl, m_PostWithMsg, out errorInfo);
                    if (ret != ReactorReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
                if (EnableSymbolListPost)
                {
                    ReactorReturnCode ret = SendSymbolListPostMsg(chnl, out errorInfo);
                    if (ret != ReactorReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }

                m_NextPostTime = currentTime + POST_MESSAGE_FREQUENCY;

                /* iterate between post with msg and post with data */
                m_PostWithMsg = !m_PostWithMsg;
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }


        /// <summary>
        /// Encodes and sends an on-stream post message.
        /// </summary>
        ///
        /// It will either send a post that contains a full message or a post that
        /// contains only data payload, based on the postWithMsg parameter. If true,
        /// post will contain a message.
        ///
        /// This method only sends one post message, however it is called
        /// periodically over a time increment by the handlePosts method
        private ReactorReturnCode SendOnstreamPostMsg(ReactorChannel chnl, bool postWithMsg, out ReactorErrorInfo? errorInfo)
        {
            if (StreamId == 0)
            {
                // no items available to post on
                Console.WriteLine("Currently no available market price streams to on-stream post to.  Will retry shortly.");
                errorInfo = null;
                return ReactorReturnCode.SUCCESS;
            }

            // get a buffer for the item request
            ITransportBuffer? msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_MSG_POST, false, out errorInfo);
            if (msgBuf == null)
                return ReactorReturnCode.FAILURE;

            CodecReturnCode ret;
            if (postWithMsg)
            {

                if ((ret = EncodePostWithMsg(chnl, msgBuf, (int)DomainType.MARKET_PRICE, StreamId, PostItemName, out errorInfo)) != CodecReturnCode.SUCCESS)
                {
                    return (ReactorReturnCode)ret;
                }
            }
            else
            {
                if ((ret = EncodePostWithData(chnl, msgBuf, (int)DomainType.MARKET_PRICE, StreamId, out errorInfo)) != CodecReturnCode.SUCCESS)
                {
                    return (ReactorReturnCode)ret;
                }
            }

            // send post message
            return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }


        /// Encodes and sends an off-stream post message.
        ///
        /// This method only sends one post message, however it is called
        /// periodically over a time increment by the handlePosts method
        private ReactorReturnCode SendOffstreamPostMsg(ReactorChannel chnl, bool postWithMsg, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;

            // get a buffer for the item request
            ITransportBuffer? msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_MSG_POST, false, out errorInfo);
            if (msgBuf == null)
                return ReactorReturnCode.FAILURE;

            CodecReturnCode ret;
            if (postWithMsg)
            {
                if ((ret = EncodePostWithMsg(chnl, msgBuf, (int)DomainType.MARKET_PRICE, OffStreamId, OffPostItemName, out errorInfo)) != CodecReturnCode.SUCCESS)
                {
                    return ReactorReturnCode.FAILURE;
                }
            }
            else
            {
                if ((ret = EncodePostWithData(chnl, msgBuf, (int)DomainType.MARKET_PRICE, OffStreamId, out errorInfo)) != CodecReturnCode.SUCCESS)
                {
                    return ReactorReturnCode.FAILURE;
                }
            }

            // send post message
            var submitRet = chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
            
            if (submitRet != ReactorReturnCode.SUCCESS)
                return ReactorReturnCode.FAILURE;

            m_OffstreamPostSent = true;
            return ReactorReturnCode.SUCCESS;
        }

        /**
        * Encodes and sends a Symbol List post message.
        *
        * This method only sends one post message, however it is called
        * periodically over a time increment by the handlePosts method
        */
        private ReactorReturnCode SendSymbolListPostMsg(ReactorChannel chnl, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            if (SlStreamId == 0)
            {
                // no items available to post on
                Console.WriteLine("Currently no available symbol list streams to on-stream post to.  Will retry shortly.");
                return ReactorReturnCode.SUCCESS;
            }

            // get a buffer for the item request
            ITransportBuffer? msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_MSG_POST, false, out errorInfo);
            if (msgBuf == null)
                return ReactorReturnCode.FAILURE;

            CodecReturnCode ret = EncodePostWithMsg(chnl, msgBuf, (int)DomainType.SYMBOL_LIST, SlStreamId, SlPostItemName, out errorInfo);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ReactorReturnCode.FAILURE;
            }

            return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }

        /// <summary>
        /// It encodes a PostMsg, populating the postUserInfo with the IPAddress and
        /// process ID of the machine running the application.
        /// </summary>
        ///
        /// The payload of the PostMsg is an UpdateMsg. The UpdateMsg contains a FieldList
        /// containing several fields. The same message encoding functionality used by the
        /// provider application is leveraged here to encode the contents.
        ///
        /// This method is internally used by sendPostMsg
        ///
        private CodecReturnCode EncodePostWithMsg(ReactorChannel chnl,
            ITransportBuffer msgBuf,
            int domainType, 
            int streamId,
            Eta.Codec.Buffer name, 
            out ReactorErrorInfo errorInfo)
        {
            errorInfo = null;

            // First encode message for payload
            m_PostMsg.Clear();

            // set-up message
            m_PostMsg.MsgClass = MsgClasses.POST;
            m_PostMsg.StreamId = streamId;
            m_PostMsg.DomainType = domainType;
            m_PostMsg.ContainerType = DataTypes.MSG;

            // Note: post message key not required for on-stream post
            m_PostMsg.ApplyPostComplete();
            m_PostMsg.ApplyAck();
            m_PostMsg.ApplyHasPostId();
            m_PostMsg.ApplyHasSeqNum();
            m_PostMsg.ApplyHasMsgKey();
            m_PostMsg.ApplyHasPostUserRights();
            m_PostMsg.PostId = m_NextPostId++;
            m_PostMsg.SeqNum = m_NextSeqNum++;

            // populate post user info
            if (m_HasInputPrincipalIdentitity)
            {
                m_PostMsg.PostUserInfo.UserAddrFromString(m_PublisherAddress);
                m_PostMsg.PostUserInfo.UserId = Int32.Parse(m_PublisherId);
            }
            else
            {
                try
                {
                    m_PostMsg.PostUserInfo.UserAddr = BitConverter.ToUInt32(Dns.GetHostAddresses(Dns.GetHostName())
                        .Where(ip => ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)!.FirstOrDefault()?.GetAddressBytes());
                }
                catch (Exception e)
                {
                    Console.WriteLine("Populating postUserInfo failed. Dns.GetHostAddresses(Dns.GetHostName()) exception: " + e.Message);
                    return CodecReturnCode.FAILURE;
                }
                m_PostMsg.PostUserInfo.UserId = Process.GetCurrentProcess().Id;
            }

            m_PostMsg.PostUserRights = PostUserRights.CREATE | PostUserRights.DELETE;

            m_PostMsg.MsgKey.ApplyHasNameType();
            m_PostMsg.MsgKey.ApplyHasName();
            m_PostMsg.MsgKey.ApplyHasServiceId();
            m_PostMsg.MsgKey.Name.Data(name.Data(), name.Position, name.Length);

            m_PostMsg.MsgKey.NameType = InstrumentNameTypes.RIC;
            m_PostMsg.MsgKey.ServiceId = ServiceId;

            // encode market price response into a buffer
            m_EncodeIterator.Clear();
            CodecReturnCode ret = m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"Encoder.SetBufferAndRWFVersion() failed:  <{ret}>");
                return ret;
            }

            ret = m_PostMsg.EncodeInit(m_EncodeIterator, 0);
            if (ret != CodecReturnCode.ENCODE_CONTAINER)
            {
                Console.WriteLine($"EncodeMsgInit() failed:  <{ret}>");
                return ret;
            }

            MarketPriceItem mpItemInfo = null;

            if (domainType == (int)DomainType.MARKET_PRICE)
            {
                mpItemInfo = new MarketPriceItem();
                mpItemInfo.InitFields();
                mpItemInfo.TRDPRC_1 = NextItemData();
                mpItemInfo.BID = NextItemData();
                mpItemInfo.ASK = NextItemData();
            }

            // get a buffer for nested market price refresh
            Codec.Buffer postNestedMsgPayLoad = new();
            postNestedMsgPayLoad.Data(new ByteBuffer(1024));

            // Although we are encoding RWF message, this code
            // encodes nested message into a separate buffer.
            // this is because MarketPrice.encode message is shared by all
            // applications, and it expects to encode the message into a stand alone
            // buffer.
            ret = m_EncodeIterator.EncodeNonRWFInit(postNestedMsgPayLoad);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeNonRWFDataTypeInit() failed:  <{ret.GetAsString()}>");
                return CodecReturnCode.FAILURE;
            }

            m_PostMessageEncodeIterator.Clear();
            ret = m_PostMessageEncodeIterator.SetBufferAndRWFVersion(postNestedMsgPayLoad, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeIter.setBufferAndRWFVersion() failed:  <{ret}>");
                return CodecReturnCode.FAILURE;
            }

            if (domainType == (int)DomainType.MARKET_PRICE)
            {
                if (m_PostRefresh)
                {
                    m_MarketPriceRefresh.Clear();
                    m_MarketPriceRefresh.RefreshComplete = true;
                    m_MarketPriceRefresh.ClearCache = true;
                    m_MarketPriceRefresh.StreamId = streamId;
                    m_MarketPriceRefresh.ItemName.Data(name.Data(), name.Position, name.Length);
                    m_MarketPriceRefresh.State.StreamState(StreamStates.OPEN);
                    m_MarketPriceRefresh.State.DataState(DataStates.OK);
                    m_MarketPriceRefresh.State.Code(StateCodes.NONE);
                    m_MarketPriceRefresh.State.Text().Data("Item Refresh Completed");
                    m_MarketPriceRefresh.ServiceId = ServiceId;
                    m_MarketPriceRefresh.HasServiceId = true;
                    m_MarketPriceRefresh.MarketPriceItem = mpItemInfo;
                    m_MarketPriceRefresh.HasQos = true;
                    m_MarketPriceRefresh.Qos.IsDynamic = false;
                    m_MarketPriceRefresh.Qos.Rate(QosRates.TICK_BY_TICK);
                    m_MarketPriceRefresh.Qos.Timeliness(QosTimeliness.REALTIME);
                    m_MarketPriceRefresh.DataDictionary = Dictionary;

                    ret = m_MarketPriceRefresh.Encode(m_PostMessageEncodeIterator);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        Console.WriteLine($"EncodeMarketPriceRefresh() failed: " + ret.GetAsString());
                        return CodecReturnCode.FAILURE;
                    }

                    m_PostRefresh = false;
                }
                else
                {
                    m_MarketPriceUpdate.Clear();
                    m_MarketPriceUpdate.StreamId = streamId;
                    m_MarketPriceUpdate.ItemName.Data(name.Data(), name.Position, name.Length);
                    m_MarketPriceUpdate.MarketPriceItem = mpItemInfo;
                    m_MarketPriceUpdate.DataDictionary = Dictionary;

                    ret = m_MarketPriceUpdate.Encode(m_PostMessageEncodeIterator);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        Console.WriteLine($"EncodeMarketPriceUpdate() failed:  <{ret}>");
                        return CodecReturnCode.FAILURE;
                    }
                }
            }
            else if (domainType == (int)DomainType.SYMBOL_LIST)
            {
                if (m_SlPostRefresh)
                {
                    slRefreshMsg.Clear();
                    slRefreshMsg.MsgClass = MsgClasses.REFRESH;
                    slRefreshMsg.State.StreamState(StreamStates.OPEN);
                    slRefreshMsg.State.DataState(DataStates.OK);
                    slRefreshMsg.State.Code(StateCodes.NONE);
                    slRefreshMsg.Flags = RefreshMsgFlags.HAS_MSG_KEY | RefreshMsgFlags.SOLICITED | RefreshMsgFlags.REFRESH_COMPLETE | RefreshMsgFlags.HAS_QOS;
                    slRefreshMsg.State.Text().Data("Item Refresh Completed");
                    slRefreshMsg.MsgKey.Flags = MsgKeyFlags.HAS_SERVICE_ID | MsgKeyFlags.HAS_NAME;
                    slRefreshMsg.MsgKey.ServiceId = ServiceId;
                    slRefreshMsg.MsgKey.Name.Data(name.Data(), name.Position, name.Length);
                    slRefreshMsg.Qos.IsDynamic = false;
                    slRefreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);
                    slRefreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                    slRefreshMsg.DomainType = (int)DomainType.SYMBOL_LIST;
                    slRefreshMsg.ContainerType = DataTypes.MAP;
                    slRefreshMsg.StreamId = streamId;

                    ret = EncodeSymbolListMsg((Msg)slRefreshMsg, m_PostMessageEncodeIterator, SymbolListItems.SYMBOL_LIST_REFRESH, out errorInfo);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        Console.WriteLine("EncodeSymbolListMsg() failed: " + ret.GetAsString());
                        return CodecReturnCode.FAILURE;
                    }

                    m_SlPostRefresh = false;
                }
                else
                {
                    slUpdateMsg.Clear();
                    slUpdateMsg.MsgClass = MsgClasses.UPDATE;
                    slUpdateMsg.DomainType = (int)DomainType.SYMBOL_LIST;
                    slUpdateMsg.ContainerType = DataTypes.MAP;
                    slUpdateMsg.StreamId = streamId;

                    int responseType = m_IsUpdateAdd ? SymbolListItems.SYMBOL_LIST_UPDATE_ADD : SymbolListItems.SYMBOL_LIST_UPDATE_DELETE;
                    m_IsUpdateAdd = !m_IsUpdateAdd;
                    ret = EncodeSymbolListMsg((Msg)slUpdateMsg, m_PostMessageEncodeIterator, responseType, out errorInfo);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        Console.WriteLine("EncodeSymbolListMsg() failed:  " + ret.GetAsString());
                        return CodecReturnCode.FAILURE;
                    }
                }
            }


            ret = m_EncodeIterator.EncodeNonRWFComplete(postNestedMsgPayLoad, true);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeNonRWFDataTypeComplete() failed:  <{ret.GetAsString()}>");
                return CodecReturnCode.FAILURE;
            }

            // complete encode message
            if ((ret = m_PostMsg.EncodeComplete(m_EncodeIterator, true)) < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeMsgComplete() failed with return code: {ret}");
                return ret;
            }

            Console.WriteLine("\n\nSENDING POST WITH MESSAGE:\n" + "  streamId = " + m_PostMsg.StreamId
                + "\n  postId   = " + m_PostMsg.PostId + "\n  seqNum   = " + m_PostMsg.SeqNum);

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode EncodeSymbolListMsg(Msg msg, EncodeIterator encodeIter, int responseType, out ReactorErrorInfo errorInfo)
        {
            errorInfo = null;
            CodecReturnCode ret = msg.EncodeInit(encodeIter, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = "Msg.EncodeInit() failed with return code: " + ret.GetAsString();
                return ret;
            }

            /* encode map */
            slMap.Clear();
            slMap.Flags = 0;
            slMap.ContainerType = DataTypes.NO_DATA;
            slMap.KeyPrimitiveType = DataTypes.BUFFER;
            ret = slMap.EncodeInit(encodeIter, 0, 0);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = "map.EncodeInit() failed with return code: " + ret.GetAsString();
                return ret;
            }

            int i = 0;
            /* encode map entry */
            slMapEntry.Clear();
            slMapEntry.Flags = MapEntryFlags.NONE;
            slBuffer.Clear();
            switch (responseType)
            {
                // this is a refresh message, so begin encoding the entire symbol list
                case SymbolListItems.SYMBOL_LIST_REFRESH:
                    slBuffer.Data(symbolListItemList[i].ItemName.Data());
                    slMapEntry.Action = MapEntryActions.ADD;
                    break;
                // this is an update message adding a name
                case SymbolListItems.SYMBOL_LIST_UPDATE_ADD:
                    SymbolListItem updItem = new SymbolListItem();
                    updItem.ItemName.Data("IBM");
                    slBuffer.Data(updItem.ItemName.Data());
                    slMapEntry.Action = MapEntryActions.ADD;
                    break;

                // this is an update message deleting a name
                case SymbolListItems.SYMBOL_LIST_UPDATE_DELETE:
                    SymbolListItem delItem = new SymbolListItem();
                    delItem.ItemName.Data("IBM");
                    slBuffer.Data(delItem.ItemName.Data());
                    slMapEntry.Action = MapEntryActions.DELETE;
                    break;
                default:
                    errorInfo = new ReactorErrorInfo();
                    errorInfo.Error.Text = "Invalid SymbolListItems responseType: " + responseType;
                    return CodecReturnCode.FAILURE;
            }

            ret = slMapEntry.EncodeInit(encodeIter, slBuffer, 0);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = "MapEntry.EncodeInit() failed with return code: " + ret.GetAsString();
                return ret;
            }

            ret = slMapEntry.EncodeComplete(encodeIter, true);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = "MapEntry.EncodeComplete() failed with return code: " + ret.GetAsString();
                return ret;
            }

            /*
             * if this is a refresh message, finish encoding the entire symbol list
             * in the response
             */
            if (responseType == SymbolListItems.SYMBOL_LIST_REFRESH)
            {
                for (i = 1; i < symbolListItemList.Count; i++)
                {
                    if (symbolListItemList[i].IsInUse)
                    {
                        slBuffer.Data(symbolListItemList[i].ItemName.Data());
                        ret = slMapEntry.Encode(encodeIter, slBuffer);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            errorInfo = new ReactorErrorInfo();
                            errorInfo.Error.Text = "mapEntry.Encode() failed with return code: " + ret.GetAsString();
                            return ret;
                        }
                    }
                }
            }

            /* complete map */
            ret = slMap.EncodeComplete(encodeIter, true);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = "mapEntry.EncodeComplete() failed with return code: " + ret.GetAsString();
                return ret;
            }

            /* complete encode message */
            ret = msg.EncodeComplete(encodeIter, true);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = "msg.EncodeComplete() failed with return code: " + ret.GetAsString();
                return ret;
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// It encodes a PostMsg, populating the postUserInfo with the IPAddress and
        /// process ID of the machine running the application.
        /// </summary>
        ///
        /// The payload of the PostMsg is a FieldList containing several fields.
        ///
        /// This method is internally used by SendPostMsg
        ///
        private CodecReturnCode EncodePostWithData(ReactorChannel chnl, ITransportBuffer msgBuf, int domainType, int streamId, out ReactorErrorInfo errorInfo)
        {
            errorInfo = null;

            CodecReturnCode ret = 0;
            IDictionaryEntry dictionaryEntry;
            MarketPriceItem itemInfo = new MarketPriceItem();

            // clear encode iterator
            m_EncodeIterator.Clear();
            fList.Clear();
            fEntry.Clear();

            // set-up message
            m_PostMsg.MsgClass = MsgClasses.POST;
            m_PostMsg.StreamId = streamId;
            m_PostMsg.DomainType = domainType;
            m_PostMsg.ContainerType = DataTypes.FIELD_LIST;

            // Note: post message key not required for on-stream post
            m_PostMsg.ApplyPostComplete();
            m_PostMsg.ApplyAck(); // request ACK
            m_PostMsg.ApplyHasPostId();
            m_PostMsg.ApplyHasSeqNum();

            m_PostMsg.PostId = m_NextPostId++;
            m_PostMsg.PartNum = m_NextSeqNum;
            m_PostMsg.SeqNum = m_NextSeqNum++;

            // populate post user info
            if (m_HasInputPrincipalIdentitity)
            {
                m_PostMsg.PostUserInfo.UserAddrFromString(m_PublisherAddress);
                m_PostMsg.PostUserInfo.UserId = Int32.Parse(m_PublisherId);

            }
            else
            {
                try
                {
                    m_PostMsg.PostUserInfo.UserAddr = BitConverter.ToUInt32(Dns.GetHostAddresses(Dns.GetHostName())
                        .Where(ip => ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)!.FirstOrDefault()?.GetAddressBytes());
                }
                catch (Exception e)
                {
                    Console.WriteLine("Populating postUserInfo failed. Dns.GetHostAddresses(Dns.GetHostName()) exception: " + e.Message);
                    return CodecReturnCode.FAILURE;
                }
                m_PostMsg.PostUserInfo.UserId = Process.GetCurrentProcess().Id;
            }

            postNestedMsgPayLoad = new();
            postNestedMsgPayLoad.Data(new ByteBuffer(1024));

            // encode message
            m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);

            if ((ret = m_PostMsg.EncodeInit(m_EncodeIterator, 0)) < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeMsgInit() failed: <{ret.GetAsString()}>");
                return ret;
            }

            itemInfo.InitFields();
            itemInfo.TRDPRC_1 = NextItemData();
            itemInfo.BID = NextItemData();
            itemInfo.ASK = NextItemData();

            // encode field list
            fList.ApplyHasStandardData();
            if ((ret = fList.EncodeInit(m_EncodeIterator, null, 0)) < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeFieldListInit() failed: <{ret.GetAsString()}>");
                return ret;
            }

            // TRDPRC_1
            fEntry.Clear();
            dictionaryEntry = Dictionary.Entry(MarketPriceItem.TRDPRC_1_FID);
            if (dictionaryEntry != null)
            {
                fEntry.FieldId = MarketPriceItem.TRDPRC_1_FID;
                fEntry.DataType = dictionaryEntry.GetRwfType();
                tempReal.Clear();
                tempReal.Value(itemInfo.TRDPRC_1, RealHints.EXPONENT_2);
                if ((ret = fEntry.Encode(m_EncodeIterator, tempReal)) < CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine($"EncodeFieldEntry() failed: <{ret.GetAsString()}>");
                    return ret;
                }
            }
            // BID
            fEntry.Clear();
            dictionaryEntry = Dictionary.Entry(MarketPriceItem.BID_FID);
            if (dictionaryEntry != null)
            {
                fEntry.FieldId = MarketPriceItem.BID_FID;
                fEntry.DataType = dictionaryEntry.GetRwfType();
                tempReal.Clear();
                tempReal.Value(itemInfo.BID, RealHints.EXPONENT_2);
                if ((ret = fEntry.Encode(m_EncodeIterator, tempReal)) < CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine($"EncodeFieldEntry() failed: <{ret.GetAsString()}>");

                    return ret;
                }
            }
            // ASK
            fEntry.Clear();
            dictionaryEntry = Dictionary.Entry(MarketPriceItem.ASK_FID);
            if (dictionaryEntry != null)
            {
                fEntry.FieldId = MarketPriceItem.ASK_FID;
                fEntry.DataType = dictionaryEntry.GetRwfType();
                tempReal.Clear();
                tempReal.Value(itemInfo.ASK, RealHints.EXPONENT_2);
                if ((ret = fEntry.Encode(m_EncodeIterator, tempReal)) < CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine($"EncodeFieldEntry() failed: <{ret.GetAsString()}>");
                    return ret;
                }
            }

            // complete encode field list
            if ((ret = fList.EncodeComplete(m_EncodeIterator, true)) < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeFieldListComplete() failed: <{ret.GetAsString()}>");

                return ret;
            }

            m_PostMsg.EncodedDataBody = postNestedMsgPayLoad;

            Console.WriteLine("\n\nSENDING POST WITH DATA:\n" + "  streamId = " + m_PostMsg.StreamId
                + "\n  postId   = " + m_PostMsg.PostId + "\n  seqNum   = " + m_PostMsg.SeqNum);

            return CodecReturnCode.SUCCESS;
        }


        /// <summary>
        /// Encodes and sends an off-stream post close status message.
        /// </summary>
        public ReactorReturnCode CloseOffStreamPost(ReactorChannel chnl, out ReactorErrorInfo? errorInfo)
        {
            // first check if we have sent offstream posts
            if (!m_OffstreamPostSent)
            {
                errorInfo = null;
                return ReactorReturnCode.SUCCESS;
            }

            // get a buffer for the item request
            ITransportBuffer? msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_MSG_POST, false, out errorInfo);
            if (msgBuf == null)
                return ReactorReturnCode.FAILURE;

            m_PostMsg.Clear();

            // set-up post message
            m_PostMsg.MsgClass = MsgClasses.POST;
            m_PostMsg.StreamId = StreamId;
            m_PostMsg.DomainType = (int)DomainType.MARKET_PRICE;
            m_PostMsg.ContainerType = DataTypes.MSG;

            // Note: This example sends a status close when it shuts down.
            // So don't ask for an ACK (that we will never get)
            m_PostMsg.Flags = PostMsgFlags.POST_COMPLETE
                | PostMsgFlags.HAS_POST_ID | PostMsgFlags.HAS_SEQ_NUM
                | PostMsgFlags.HAS_POST_USER_RIGHTS | PostMsgFlags.HAS_MSG_KEY;

            m_PostMsg.PostId = m_NextPostId++;
            m_PostMsg.SeqNum = m_NextSeqNum++;
            m_PostMsg.PostUserRights = PostUserRights.CREATE | PostUserRights.DELETE;

            // set post item name
            m_PostMsg.MsgKey.ApplyHasNameType();
            m_PostMsg.MsgKey.ApplyHasName();
            m_PostMsg.MsgKey.ApplyHasServiceId();
            m_PostMsg.MsgKey.Name.Data(PostItemName.Data(), PostItemName.Position, PostItemName.Length);
            m_PostMsg.MsgKey.NameType = InstrumentNameTypes.RIC;
            m_PostMsg.MsgKey.ServiceId = ServiceId;

            // populate default post user info
            if (m_HasInputPrincipalIdentitity)
            {
                m_PostMsg.PostUserInfo.UserAddrFromString(m_PublisherAddress);
                m_PostMsg.PostUserInfo.UserId = Int32.Parse(m_PublisherId);
            }
            else
            {
                try
                {
                    m_PostMsg.PostUserInfo.UserAddr = BitConverter.ToUInt32(Dns.GetHostAddresses(Dns.GetHostName())
                        .Where(ip => ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)!.FirstOrDefault()?.GetAddressBytes());
                }
                catch (Exception e)
                {
                    Console.WriteLine("Populating postUserInfo failed. Dns.GetHostAddresses(Dns.GetHostName()) exception: " + e.Message);
                    return ReactorReturnCode.FAILURE;
                }
                m_PostMsg.PostUserInfo.UserId = Process.GetCurrentProcess().Id;
            }

            // set-up status message that will be nested in the post message
            m_StatusMsg.Flags = StatusMsgFlags.HAS_STATE;
            m_StatusMsg.MsgClass = MsgClasses.STATUS;
            m_StatusMsg.StreamId = StreamId;
            m_StatusMsg.DomainType = (int)DomainType.MARKET_PRICE;
            m_StatusMsg.ContainerType = DataTypes.NO_DATA;
            m_StatusMsg.State.StreamState(StreamStates.CLOSED);
            m_StatusMsg.State.DataState(DataStates.SUSPECT);

            // encode post message
            m_EncodeIterator.Clear();
            CodecReturnCode ret = m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"Encoder.SetBufferAndRWFVersion() failed:  <{ret}>");
                return ReactorReturnCode.FAILURE;
            }

            ret = m_PostMsg.EncodeInit(m_EncodeIterator, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"PostMsg.EncodeInit() failed:  <{ret}>");
                return ReactorReturnCode.FAILURE;
            }

            // encode nested status message
            ret = m_StatusMsg.EncodeInit(m_EncodeIterator, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"StatusMsg.EncodeInit() failed:  <{ret}>");
                return ReactorReturnCode.FAILURE;
            }

            // complete encode status message
            ret = m_StatusMsg.EncodeComplete(m_EncodeIterator, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"StatusMsg.EncodeComplete() failed:  <{ret}>");
                return ReactorReturnCode.FAILURE;
            }

            // complete encode post message
            ret = m_PostMsg.EncodeComplete(m_EncodeIterator, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"PostMsg.EncodeComplete() failed:  <{ret}>");
                return ReactorReturnCode.FAILURE;
            }

            // send post message
            return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }
    }
}
