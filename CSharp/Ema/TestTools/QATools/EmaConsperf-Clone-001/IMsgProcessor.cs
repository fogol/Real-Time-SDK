/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;

namespace LSEG.Ema.PerfTools.ConsPerf;

public interface IMsgProcessor
{
    void ProcessRefreshMsg(RefreshMsg msg, long handle, object? closure);
    void ProcessStatusMsg(StatusMsg msg, object? closure);
    void ProcessUpdateMsg(UpdateMsg msg);
}

