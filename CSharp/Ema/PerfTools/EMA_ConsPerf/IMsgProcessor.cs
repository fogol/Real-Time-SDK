using LSEG.Ema.Access;

namespace LSEG.Ema.PerfTools.ConsPerf;

public interface IMsgProcessor
{
    void ProcessRefreshMsg(RefreshMsg msg, long handle, object? closure);
    void ProcessStatusMsg(StatusMsg msg, object? closure);
    void ProcessUpdateMsg(UpdateMsg msg);
}

