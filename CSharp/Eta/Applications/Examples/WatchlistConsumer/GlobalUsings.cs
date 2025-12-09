/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

global using System.Net;
global using System.Net.Sockets;

global using LSEG.Eta.Rdm;
global using LSEG.Eta.Codec;
global using LSEG.Eta.Common;
global using LSEG.Eta.Transports;
global using LSEG.Eta.ValueAdd.Rdm;
global using LSEG.Eta.Example.Common;
global using LSEG.Eta.ValueAdd.Reactor;

global using Array = LSEG.Eta.Codec.Array;
global using Buffer = LSEG.Eta.Codec.Buffer;

global using static LSEG.Eta.Rdm.Dictionary;
global using static LSEG.Eta.ValueAdd.Rdm.LoginMsgType;