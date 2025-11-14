/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2024 LSEG. All rights reserved.                   --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;

namespace LSEG.Eta.ValueAdd.WatchlistConsumer;

public partial class WatchlistConsumerConfig
{
    internal class Json
    {
        public PreferredHost? PreferredHost { get; set; } = default!;

        public ICollection<Connection> ConnectionList { get; set; } = new System.Collections.ObjectModel.Collection<Connection>();
    }

    internal class PreferredHost
    {
        public bool EnablePH { get; set; } = default!;

        public int ConnectListIndex { get; set; } = 0;

        public int? DetectionTimeInterval { get; set; } = default!;

        /// <summary>Cron expression</summary>
        public string? DetectionTimeSchedule { get; set; } = "";
    }

    internal class Connection
    {
        public string? Host { get; set; } = "localhost";

        /// <summary>Port number must be an integer between 0 and 65535</summary>
        public int? Port { get; set; } = 14002;

        public bool? SessionMgnt { get; set; } = false;

        [System.ComponentModel.DataAnnotations.Range(0, int.MaxValue)]
        public int? ConnType { get; set; } = 0;

        public int? EncryptedConnType { get; set; } = default!;

        public EncryptionProtocolFlags? EncryptionProtocolFlags { get; set; } = default!;

        public string? InterfaceName { get; set; } = default!;

        public string? Location { get; set; } = default!;

        public string? ProxyHost { get; set; } = default!;

        public string? ProxyPort { get; set; } = default!;

        public string? ProxyUserName { get; set; } = default!;

        public string? ProxyPassword { get; set; } = default!;
    }

    [Flags]
    internal enum EncryptionProtocolFlags
    {
        TLSv1_2 = 0x4,
        TLSv1_3 = 0x8,
        All = TLSv1_2 | TLSv1_3
    }
}