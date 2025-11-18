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
