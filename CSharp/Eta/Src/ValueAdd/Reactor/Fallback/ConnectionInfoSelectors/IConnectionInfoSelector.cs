/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor.Fallback.ConnectionInfoSelectors
{
    /// <summary>
    /// Used in <see cref="ReactorChannel"/> to switch between different transport connections accoding to implementation algorithm,
    /// e.g. &quot;Round-robin&quot; or &quot;Preferred Host&quot;.
    /// </summary>
    internal interface IConnectionInfoSelector
    {
        /// <summary>
        /// Current connection in list.
        /// </summary>
        ReactorConnectInfo Current { get; }

        /// <summary>
        /// Next connection in list according to implemented switching algorithm.
        /// </summary>
        ReactorConnectInfo Next { get; }

        /// <summary>
        /// Switches to next connection in list.
        /// </summary>
        /// <param name="reconnectAttempt">This is used to indicate reconnect attempt count when the value is not -1.</param>
        /// <returns>Returns corresponding <see cref="ReactorConnectInfo"/> which is equal to previous <see cref="Next"/> property.</returns>
        ReactorConnectInfo SwitchToNext(int reconnectAttempt = -1);

        /// <summary>
        /// Connection switching algorithm dependent value.
        /// It's always <c>false</c> for algorithms that treat connections equally (e.g. &quot;Round-robin&quot;).
        /// </summary>
        bool ShouldSwitchPrematurely { get; }
    }
}
