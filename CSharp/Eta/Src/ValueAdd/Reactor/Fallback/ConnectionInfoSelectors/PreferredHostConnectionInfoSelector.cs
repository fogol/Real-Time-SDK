/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024 LSEG. All rights reserved.                   --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor.Fallback.ConnectionInfoSelectors
{
    internal class PreferredHostConnectionInfoSelector : BaseConnectionInfoSelector, IConnectionInfoSelector
    {
        private readonly int m_PreferredIndex;
        private int m_CurrentNonPreferredIndex;

        public PreferredHostConnectionInfoSelector(IReadOnlyList<ReactorConnectInfo> connectInfos, int preferredConnectionIndex)
            : base(connectInfos)
        {
            if (preferredConnectionIndex < 0)
                throw new ArgumentOutOfRangeException(nameof(preferredConnectionIndex), $"Preferred connection index cannot be negative ({preferredConnectionIndex}).");
            if (preferredConnectionIndex > connectInfos.Count - 1)
                throw new ArgumentOutOfRangeException(nameof(preferredConnectionIndex),
                    $"Preferred connection index ({preferredConnectionIndex}) cannot exceed maximum connection infos index ({connectInfos.Count - 1}).");

            m_PreferredIndex = preferredConnectionIndex;
            m_CurrentIndex = m_PreferredIndex;
            m_CurrentNonPreferredIndex = m_PreferredIndex != 0
                ? 0
                : GetNextConnectInfosIndex(m_PreferredIndex);
        }

        public bool ShouldSwitchPrematurely => m_CurrentIndex != m_PreferredIndex;

        public ReactorConnectInfo Next => m_ConnectInfos[GetNextOrPreferredIndex().currentIndex];

        public ReactorConnectInfo SwitchToNext(int reconnectAttempt = -1)
        {
            /* Retry with the PH first if the current index is the PH for the first reconnect attempt */
            if(reconnectAttempt == 1 && m_CurrentIndex == m_PreferredIndex)
            {
                return Current;
            }

            (m_CurrentIndex, m_CurrentNonPreferredIndex) = GetNextOrPreferredIndex();

            return Current;
        }

        public override string ToString()
        {
            return $"{nameof(PreferredHostConnectionInfoSelector)}{{index: {m_CurrentIndex}, preferred index: {m_PreferredIndex}}}";
        }

        private (int currentIndex, int currentNonPreferredIndex) GetNextOrPreferredIndex()
        {
            var result = (currentIndex: m_CurrentIndex, currentNonPreferredIndex: m_CurrentNonPreferredIndex);
            if (result.currentIndex != m_PreferredIndex)
            {
                result.currentIndex = m_PreferredIndex;
            }
            else
            {
                result.currentIndex = m_CurrentNonPreferredIndex;
                result.currentNonPreferredIndex = GetNextConnectInfosIndex(m_CurrentNonPreferredIndex);
                if (result.currentNonPreferredIndex == m_PreferredIndex)
                {
                    result.currentNonPreferredIndex = GetNextConnectInfosIndex(result.currentNonPreferredIndex);
                }
            }

            return result;
        }
    }
}
