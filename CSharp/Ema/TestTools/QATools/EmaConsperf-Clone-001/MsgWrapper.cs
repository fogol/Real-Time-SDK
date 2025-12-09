/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;

namespace LSEG.Ema.PerfTools.ConsPerf;

public abstract class MsgWrapper
{
    public MsgWrapper(Msg msg)
    {
        BaseMsg = msg;
    }

    public Msg BaseMsg { get; }
    public long Handle { get; set; }
    public object? Closure { get; set; }
}

public class MsgWrapper<T> : MsgWrapper
    where T : Msg
{
    public MsgWrapper(T msg)
        : base(msg)
    {
    }

    public T Msg => (T)BaseMsg;
}
