/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Example.VACommon;

namespace LSEG.Eta.ValueAdd.WatchlistConsumer;

/// <summary>
/// Contains information associated with each open channel in the Watchlist Consumer.
/// </summary>
internal class ChannelInfo
{
    public ReactorConnectOptions ConnectOptions { get; set; } = new();
    public ReactorConnectInfo ConnectInfo { get; set; } = new();
    public ConsumerRole ConsumerRole { get; set; } = new();
    public PostHandler PostHandler { get; set; } = new();
    public DataDictionary Dictionary { get; set; } = new();
    public int FieldDictionaryStreamId { get; set; }
    public int EnumDictionaryStreamId { get; set; }
    public bool ShouldOffStreamPost { get; set; }
    public bool ShouldOnStreamPost { get; set; }
    public bool ShouldEnableEncrypted { get; set; }
    public EncryptionProtocolFlags EncryptionProtocol { get; set; }
    public Buffer PostItemName { get; set; } = new();
    public DecodeIterator DIter { get; set; } = new();
    public Msg ResponseMsg { get; set; } = new();
    public LoginRefresh LoginRefresh { get; set; } = new();
    public bool HasServiceInfo { get; set; }
    public Service ServiceInfo { get; set; } = new();
    public ReactorChannel? ReactorChannel { get; set; }
    internal bool isChannelClosed;

    /// <summary>
    /// Represented by epoch time in milliseconds.
    /// </summary>
    public System.DateTime LoginReissueTime { get; set; }

    public ConnectionArg ConnectionArg { get; set; } = new();
    public bool CanSendLoginReissue { get; set; }

    public ChannelInfo()
    {
        ConnectOptions.ConnectionList.Add(ConnectInfo);
    }
}

internal static class ChannelInfoExtension
{
    public static IEnumerable<ChannelInfo> GetOpen(this IEnumerable<ChannelInfo> channelInfos) => channelInfos.Where(i => !i.isChannelClosed);
}