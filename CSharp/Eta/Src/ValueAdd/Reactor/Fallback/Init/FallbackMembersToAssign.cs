/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor.Fallback.ConnectionInfoSelectors;
using LSEG.Eta.ValueAdd.Reactor.Fallback.Timers;

namespace LSEG.Eta.ValueAdd.Reactor.Fallback.Init
{
    /// <summary>
    /// Responsible for <see cref="ReactorChannel"/> members values creation basing on <see cref="ReactorPreferredHostOptions"/> passed.
    /// Also holds those members to be assigned later in <see cref="ReactorWorker"/>.
    /// For example, when <see cref="ReactorChannel.IOCtl(ReactorChannelIOCtlCode, object, out ReactorErrorInfo?)"/> is called during PH fallback and assignment needs to be postponed until its completion.
    /// </summary>
    internal class FallbackMembersToAssign
    {
        public IFallbackTimer? FallbackTimer { get; private set; }
        public IConnectionInfoSelector? ConnectionInfoSelector { get; private set; }
        public ReactorPreferredHostOptions? PreferredHostOptions { get; private set; }

        public FallbackMembersToAssign(ReactorPreferredHostOptions preferredHostOptions, IReadOnlyList<ReactorConnectInfo> connectionList)
        {
            if (preferredHostOptions.EnablePreferredHostOptions)
            {
                ConnectionInfoSelector = new PreferredHostConnectionInfoSelector(connectionList, preferredHostOptions.ConnectionListIndex);

                if (!string.IsNullOrEmpty(preferredHostOptions.DetectionTimeSchedule))
                {
                    FallbackTimer = new CronFallbackTimer(preferredHostOptions.DetectionTimeSchedule);
                }
                else if (preferredHostOptions.DetectionTimeInterval > 0)
                {
                    FallbackTimer = new IntervalFallbackTimer(preferredHostOptions.DetectionTimeInterval);
                }
                else
                {
                    FallbackTimer = null;
                }
            }
            else
            {
                ConnectionInfoSelector = new RoundRobinConnectionInfoSelector(connectionList);
                FallbackTimer = null;
            }
            // prevent application to change PreferredHostOptions during those options being applied asynchronously
            PreferredHostOptions = new ReactorPreferredHostOptions(preferredHostOptions);
        }
    }
}
