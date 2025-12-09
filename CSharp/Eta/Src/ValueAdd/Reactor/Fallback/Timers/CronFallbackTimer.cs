/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using Cronos;

namespace LSEG.Eta.ValueAdd.Reactor.Fallback.Timers
{
    internal class CronFallbackTimer : IFallbackTimer
    {
        private readonly CronExpression m_DetectionTimeSchedule;

        public CronFallbackTimer(string detectionTimeSchedule)
        {
            if (string.IsNullOrEmpty(detectionTimeSchedule))
                throw new ArgumentException("Detection time schedule cannot be null or empty string.", nameof(detectionTimeSchedule));

            try
            {
                m_DetectionTimeSchedule = CronExpression.Parse(
                    detectionTimeSchedule,
                    CronExpressionHasSeconds(detectionTimeSchedule) ? CronFormat.IncludeSeconds : CronFormat.Standard);
            }
            catch (CronFormatException e)
            {
                throw new ArgumentException($"Invalid detection time schedule ({e.Message})", nameof(detectionTimeSchedule), e);
            }
        }

        public long TriggerTimeMs { get; private set; } = long.MaxValue;

        public void Reset(long nowMs)
        {
            var nowDTO = new DateTimeOffset(ReactorUtil.ConvertMilliSecondTimeToDateTime(nowMs));
            var triggerDTO = m_DetectionTimeSchedule.GetNextOccurrence(nowDTO, TimeZoneInfo.Local);
            TriggerTimeMs = triggerDTO.HasValue
                ? ReactorUtil.ConvertDateTimeToMilliSecondTime(triggerDTO.Value.LocalDateTime)
                : long.MaxValue;
        }

        private static bool CronExpressionHasSeconds(string cronExpression)
        {
            var numberOfParts = cronExpression.Split(' ', StringSplitOptions.RemoveEmptyEntries).Length;
            return numberOfParts == 6;
        }
    }
}
