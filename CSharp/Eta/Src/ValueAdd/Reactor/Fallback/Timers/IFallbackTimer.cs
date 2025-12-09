/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor.Fallback.Timers
{
    /// <summary>
    /// Timer to be used to count interval between attempts to fallback to preferred host.
    /// It just receives current time via <see cref="Reset(long)"/> method and updates <see cref="TriggerTimeMs"/> property accordingly.
    /// It's up to caller side what to do with updated <see cref="TriggerTimeMs"/> value.
    /// </summary>
    internal interface IFallbackTimer
    {
        /// <summary>
        /// Moment of time when timer should be triggered in milliseconds.
        /// Value is updated on <see cref="Reset(long)"/> method call.
        /// When timer isn't started (i.e. no <see cref="Reset(long)"/> call was made) returns <see cref="long.MaxValue"/>.
        /// </summary>
        long TriggerTimeMs { get; }

        /// <summary>
        /// Starts "countdown" to new trigger. 
        /// </summary>
        /// <param name="nowMs">Moment of time that is treated like current time. It's supposed to be value from <see cref="ReactorUtil.GetCurrentTimeMilliSecond"/>.</param>
        void Reset(long nowMs);
    }
}
