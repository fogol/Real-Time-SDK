/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// This class represents preferred host information returned by the <see cref="ReactorChannel.Info(ReactorChannelInfo, out ReactorErrorInfo?)"/> call
    /// </summary>
    public sealed class ReactorPreferredHostInfo
    {
        /// <summary>
        /// This is used to check whether the preferred host feature is configured for this channel.
        /// </summary>
        public bool IsPreferredHostEnabled { get; internal set; } = false;

        /// <summary>
        /// Gets cron time schedule to switch over to a preferred host.
        /// </summary>
        public string DetectionTimeSchedule { get; internal set; } = string.Empty;

        /// <summary>
        /// Gets time interval in second unit to switch over to a preferred host.
        /// </summary>
        public uint DetectionTimeInterval { get; internal set; } = 0;

        /// <summary>
        /// Gets an index in <see cref="ReactorConnectOptions.ConnectionList"/> to set as preferred host.
        /// </summary>
        public int ConnectionListIndex { get; internal set; } = 0;

        /// <summary>
        /// Gets the remaining detection time in seconds to perform fallback to preferred host.
        /// </summary>
        public uint RemainingDetectionTime { get; internal set; } = 0;

        /// <summary>
        /// Clears this object to default.
        /// </summary>
        public void Clear()
        {
            IsPreferredHostEnabled = false;
            DetectionTimeSchedule = string.Empty;
            DetectionTimeInterval = 0;
            ConnectionListIndex = 0;
            RemainingDetectionTime = 0;
        }

        internal void AssignFrom(ReactorPreferredHostOptions options)
        {
            IsPreferredHostEnabled = options.EnablePreferredHostOptions;
            DetectionTimeSchedule = options.DetectionTimeSchedule;
            DetectionTimeInterval = options.DetectionTimeInterval;
            ConnectionListIndex = options.ConnectionListIndex;
        }
    }
}
