/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;

using LSEG.Eta.Transports;

namespace LSEG.Eta.Example.VACommon
{
    public class ConnectionArgsParser : IArgsParser
    {
        public List<ConnectionArg> ConnectionList { get; private set; } = new List<ConnectionArg>();

        /// <inheritdoc/>
        public bool IsStart(string[] args, int argOffset)
        {
            if ("-c".Equals(args[argOffset])
                || "-tcp".Equals(args[argOffset]))
            {
                return true;
            }

            return false;
        }

        /// <inheritdoc/>
        public int Parse(string[] args, int argOffset)
        {
            int offset = 0;

            if ("-c".Equals(args[argOffset]) ||
                "-tcp".Equals(args[argOffset]))
            {
                offset = ParseSocketConnectionArgs(args, argOffset);
            }
            else
            {
                offset = IArgsParser.ERROR_RETURN_CODE;
            }

            return offset;
        }

        /// <summary>
        /// parses TCP socket connection arguments
        /// </summary>
        /// <param name="args"></param>
        /// <param name="argOffset"></param>
        /// <returns></returns>
        private int ParseSocketConnectionArgs(string[] args, int argOffset)
        {
            int retCode = IArgsParser.ERROR_RETURN_CODE;
            ConnectionArg? connectionArg;

            HostArgsParser hostArgsParser = new();
            if ((args.Length - 1) >= argOffset + 3
                && hostArgsParser.IsStart(args, argOffset + 1))
            {
                retCode = hostArgsParser.Parse(args, argOffset + 1);
                if (retCode == IArgsParser.ERROR_RETURN_CODE)
                {
                    return retCode;
                }

                connectionArg = new ConnectionArg
                {
                    ConnectionType = ConnectionType.SOCKET,
                    HostList = hostArgsParser.HostList,
                };
                ConnectionList.Add(connectionArg);
            }
            else
            {
                return retCode;
            }

            // parse service name
            if (!args[retCode].StartsWith("-") && !args[retCode].Contains(':'))
            {
                connectionArg.Service = args[retCode];
                retCode++;
            }

            // parse item arguments for this connection
            ItemArgsParser itemArgsParser = new();
            if (itemArgsParser.IsStart(args, retCode))
            {
                retCode = itemArgsParser.Parse(args, retCode);
                connectionArg.ItemList = itemArgsParser.ItemList;
            }
            else if (!args[retCode].Equals("-tsServiceName")
                     && !args[retCode].Equals("-tunnel")
                     && !args[retCode].Equals("-tsAuth")
                     && !args[retCode].Equals("-tsDomain"))
            {
                retCode = IArgsParser.ERROR_RETURN_CODE;
            }

            return retCode;
        }
    }
}
