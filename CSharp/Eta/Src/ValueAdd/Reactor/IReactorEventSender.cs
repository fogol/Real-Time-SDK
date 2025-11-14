/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Allows mocking of <see cref="Reactor"/> methods related to sending internal events.
    /// </summary>
    internal interface IReactorEventSender
    {
        internal void SendPreferredHostComplete(ReactorChannel reactorChannel);

        internal void SendPreferredHostNoFallback(ReactorChannel reactorChannel);

        internal ReactorReturnCode SendWorkerImplEvent(ReactorEventImpl.ImplType eventType, ReactorChannel? reactorChannel, object? additionalPayload = null);
    }
}
