/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024 LSEG. All rights reserved.                   --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor.Fallback.ConnectionInfoSelectors
{
    internal class RoundRobinConnectionInfoSelector : BaseConnectionInfoSelector, IConnectionInfoSelector
    {
        public RoundRobinConnectionInfoSelector(IReadOnlyList<ReactorConnectInfo> connectInfos)
            : base(connectInfos)
        { }

        public bool ShouldSwitchPrematurely => false;

        public ReactorConnectInfo Next => m_ConnectInfos[GetNextConnectInfosIndex(m_CurrentIndex)];

        public ReactorConnectInfo SwitchToNext(int reconnectAttempt = -1)
        {
            m_CurrentIndex = GetNextConnectInfosIndex(m_CurrentIndex);

            return Current;
        }

        public override string ToString()
        {
            return $"{nameof(RoundRobinConnectionInfoSelector)}{{index: {m_CurrentIndex}}}";
        }
    }
}
