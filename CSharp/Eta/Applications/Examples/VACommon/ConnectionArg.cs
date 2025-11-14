/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Linq;
using LSEG.Eta.Transports;

namespace LSEG.Eta.Example.VACommon
{

    /// <summary>
    /// Connection argument class for the Value Add consumer and non-interactive provider applications.
    /// </summary>
    public class ConnectionArg
    {
        /// <summary>
        /// Type of the connection.
        /// </summary>
        public ConnectionType ConnectionType { get; set; }

        /// <summary>
        /// Tls protocol.
        /// </summary>
        public EncryptionProtocolFlags EncryptionProtocolFlags { get; set; }

        /// <summary>
        /// Name of service to request items from on this connection.
        /// </summary>
        public string Service { get; set; }

        /* non-segmented connection */

        /// <summary>
        /// Hostname of provider to connect to.
        /// </summary>
        public string Hostname
        {
            get => GetHost().Hostname ?? string.Empty;
            set => GetHost().Hostname = value;
        }

        /// <summary>
        /// Port of provider to connect to.
        /// </summary>
        public string Port
        {
            get => GetHost().Port ?? string.Empty;
            set => GetHost().Port = value;
        }

        /// <summary>
        /// Hosts of provider to connect to.
        /// </summary>
        public List<HostArg> HostList { get; set; }

        /// <summary>
        /// Interface that the provider will be using.  This is optional
        /// </summary>
        public string? InterfaceName { get; set; }

        /// <summary>
        /// Item list for this connection.
        /// </summary>
        public List<ItemArg> ItemList { get; set; }

        /// <summary>
        /// Instantiates a new connection arg.
        /// </summary>
        ///
        /// <param name="connectionType">the connection type</param>
        /// <param name="service">the service</param>
        /// <param name="hostname">the hostname</param>
        /// <param name="port">the port</param>
        /// <param name="itemList">the item list</param>
        public ConnectionArg(ConnectionType connectionType, string service, string hostname, string port, List<ItemArg> itemList, EncryptionProtocolFlags encryptionProtocolFlags)
        {
            ConnectionType = connectionType;
            Service = service;
            HostList = new() { new HostArg { Hostname = hostname, Port = port } };
            ItemList = itemList;
            EncryptionProtocolFlags = encryptionProtocolFlags;
        }


        /// <summary>
        /// Instantiates a new connection arg.
        /// </summary>
        public ConnectionArg()
        {
            ItemList = new();
            Service = string.Empty;
            HostList = new();
            EncryptionProtocolFlags = EncryptionProtocolFlags.ENC_NONE;
        }

        private HostArg GetHost()
        {
            if (HostList == null)
            {
                HostList = new();
            }

            HostArg result;
            if (HostList.Count > 0)
            {
                result = HostList[0];
            }
            else
            {
                result = new HostArg();
                HostList.Add(result);
            }

            return result;
        }
    }
}
