/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor.Fallback.ConnectionInfoSelectors
{
    internal abstract class BaseConnectionInfoSelector : IConnectionInfoSelectorPreservedState
    {
        protected readonly IReadOnlyList<ReactorConnectInfo> m_ConnectInfos;
        protected int m_CurrentIndex;

        protected BaseConnectionInfoSelector(IReadOnlyList<ReactorConnectInfo> connectInfos)
        {
            if (connectInfos == null)
                throw new ArgumentNullException(nameof(connectInfos));
            if (connectInfos.Count == 0)
                throw new ArgumentException("Connection infos list cannot be empty", nameof(connectInfos));

            m_ConnectInfos = connectInfos;
        }

        public ReactorConnectInfo Current => m_ConnectInfos[m_CurrentIndex];

        public int CurrentIndex { get => m_CurrentIndex; set => m_CurrentIndex = value; }

        protected int GetNextConnectInfosIndex(int value) => (value + 1) % m_ConnectInfos.Count;
    }
}
