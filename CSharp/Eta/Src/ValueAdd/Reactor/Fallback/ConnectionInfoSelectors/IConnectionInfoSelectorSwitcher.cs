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
    /// Allows to change <see cref="IConnectionInfoSelector"/> instance on fly in thread-safe manner.
    /// </summary>
    internal interface IConnectionInfoSelectorSwitcher : IDisposable
    {
        void SetConnectionInfoSelector(IConnectionInfoSelector connectionInfoSelector);
    }
}
