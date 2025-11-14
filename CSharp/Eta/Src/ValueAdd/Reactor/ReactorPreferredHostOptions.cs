/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024 LSEG. All rights reserved.                   --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Configuration options for specifying a preferred host to switch over when the connection is lost
    /// or specified detection time.
    /// </summary>
    /// <seealso cref="ReactorConnectOptions"/>
    public sealed class ReactorPreferredHostOptions
    {
        /// <summary>
        /// Gets or sets whether to enable the preferred host options.
        /// </summary>
        public bool EnablePreferredHostOptions { get; set; } = false;

        /// <summary>
        /// Gets or sets cron time schedule to switch over to a preferred host. Optional.
        /// <para>
        /// <see cref="DetectionTimeInterval"/> is used instead if this property is empty.
        /// </para>
        /// </summary>
        public string DetectionTimeSchedule { get; set; } = string.Empty;

        /// <summary>
        /// Gets or sets time interval in second unit to switch over to a preferred host. Optional.
        /// </summary>
        public uint DetectionTimeInterval { get; set; } = 0;

        /// <summary>
        /// Gets or sets an index in <see cref="ReactorConnectOptions.ConnectionList"/> to set as preferred host.
        /// </summary>
        public int ConnectionListIndex { get; set; } = 0;

        /// <summary>
        /// Constructs instance of <see cref="ReactorPreferredHostOptions"/> with default settings.
        /// </summary>
        public ReactorPreferredHostOptions()
        { }

        /// <summary>
        /// Constructs instance of <see cref="ReactorPreferredHostOptions"/> with settings copied from <paramref name="sourceOpts"/>.
        /// </summary>
        /// <param name="sourceOpts"></param>
        public ReactorPreferredHostOptions(ReactorPreferredHostOptions sourceOpts)
            : this()
        {
            sourceOpts.Copy(this);
        }

        /// <summary>
        /// Constructs instance of <see cref="ReactorPreferredHostOptions"/> with settings copied from <paramref name="sourceInfo"/>.
        /// </summary>
        /// <param name="sourceInfo"></param>
        public ReactorPreferredHostOptions(ReactorPreferredHostInfo sourceInfo)
            : this()
        {
            if (sourceInfo == null)
                return;

            EnablePreferredHostOptions = sourceInfo.IsPreferredHostEnabled;
            ConnectionListIndex = sourceInfo.ConnectionListIndex;
            DetectionTimeInterval = sourceInfo.DetectionTimeInterval;
            DetectionTimeSchedule = sourceInfo.DetectionTimeSchedule;
        }

        /// <summary>
        /// Clears this object to default.
        /// </summary>
        public void Clear()
        {
            EnablePreferredHostOptions = false;
            DetectionTimeSchedule = string.Empty;
            DetectionTimeInterval = 0;
            ConnectionListIndex = 0;
        }

        /// <summary>
        /// This method will perform a deep copy into the passed in parameter's
        /// members from the Object calling this method.
        /// </summary>
        /// <param name="destOpts">the value getting populated with the values of the calling Object</param>
        /// <returns><see cref="ReactorReturnCode.SUCCESS"/> on success,
        /// <see cref="ReactorReturnCode.FAILURE"/> if destOpts is null.
        /// </returns>
        public ReactorReturnCode Copy(ReactorPreferredHostOptions destOpts)
        {
            if (destOpts == null)
                return ReactorReturnCode.FAILURE;

            destOpts.EnablePreferredHostOptions = EnablePreferredHostOptions;
            destOpts.DetectionTimeSchedule = DetectionTimeSchedule;
            destOpts.DetectionTimeInterval = DetectionTimeInterval;
            destOpts.ConnectionListIndex = ConnectionListIndex;

            return ReactorReturnCode.SUCCESS;
        }

        /// <inheritdoc/>
        public override string ToString()
        {
            return $"{nameof(ReactorPreferredHostOptions)}{{ " +
                $"EnablePreferredHostOptions={EnablePreferredHostOptions}, " +
                $"DetectionTimeSchedule=\"{DetectionTimeSchedule}\", " +
                $"DetectionTimeInterval={DetectionTimeInterval}, " +
                $"ConnectionListIndex={ConnectionListIndex}" +
                " }";
        }
    }
}
