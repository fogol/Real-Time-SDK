/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Transports;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace LSEG.Eta.Tests
{
    internal class CustomMockChannel : MockChannel
    {
        public WriteActions[] ActionsPattern { get; set; } = new WriteActions[1];

        public int ActionIndex = 0;

        public void CreateNetworkBuffer(int size)
        {
            m_networkBuffer = new ByteBuffer(size);
        }

        public void CheckNetworkBuffer(Action<ByteBuffer> check)
        {
            check.Invoke(m_networkBuffer);
        }

        public override int Send(IList<ArraySegment<byte>> buffers, out Error error)
        {
            error = null;
            int byteWritten = 0;

            foreach (ArraySegment<byte> buffer in buffers)
            {
                if (ActionsPattern[ActionIndex % ActionsPattern.Length] == WriteActions.NORMAL)
                {
                    int maxLength = GetMaxWrite(buffer.Count);

                    m_networkBuffer.Put(buffer.Array, buffer.Offset, maxLength);

                    byteWritten += maxLength;

                    ActionIndex++;
                    if (maxLength != buffer.Count)
                        break; // Break if reaches the maximum write size
                }
                else if (ActionsPattern[ActionIndex % ActionsPattern.Length] == WriteActions.WOULD_BLOCK)
                {
                    error = new Error
                    {
                        ErrorId = TransportReturnCode.WRITE_FLUSH_FAILED,
                        SysError = (int)SocketError.WouldBlock,
                        Text = "An operation on a nonblocking socket cannot be completed immediately"
                    };

                    if (byteWritten == 0) byteWritten = -1;

                    ActionIndex++;
                    break;
                }
                else if (ActionsPattern[ActionIndex % ActionsPattern.Length] == WriteActions.ERROR)
                {
                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        SysError = (int)SocketError.ConnectionReset,
                        Text = "The connection was reset by the remote peer"
                    };

                    byteWritten = -1;

                    ActionIndex++;
                    break;
                }
            }         

            return byteWritten;
        }
    }
}
