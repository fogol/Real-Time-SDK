/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor.Fallback.Timers
{
    internal class IntervalFallbackTimer : IFallbackTimer
    {
        private readonly uint m_DetectionTimeIntervalMs;

        public IntervalFallbackTimer(uint detectionTimeIntervalSec)
        {
            if (detectionTimeIntervalSec == 0)
                throw new ArgumentException("Detection time interval cannot be 0.", nameof(detectionTimeIntervalSec));
            m_DetectionTimeIntervalMs = detectionTimeIntervalSec * 1000;
        }

        public long TriggerTimeMs { get; private set; } = long.MaxValue;

        public void Reset(long nowMs)
        {
            TriggerTimeMs = nowMs + m_DetectionTimeIntervalMs;
        }
    }
}
