/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal class ReactorEventImpl : ReactorEvent
    {
        internal enum ImplType
        {
            INIT = 0,

            // sent from Reactor to Worker
            CHANNEL_INIT = 1,
            // sent from Worker to Reactor
            CHANNEL_UP = 2,
            // sent from either Reactor or Worker
            CHANNEL_DOWN = 3,
            // sent from Worker to Reactor
            CHANNEL_READY = 4,
            // sent from Reactor to Worker
            CHANNEL_CLOSE = 5,
            // sent from Worker to Reactor
            CHANNEL_CLOSE_ACK = 6,
            // sent from Worker to Reactor
            FLUSH_DONE = 7,

            WARNING = 8,

            // sent from Reactor to Worker
            FD_CHANGE = 9,
            // sent from Reactor to Worker
            FLUSH = 10,
            // sent from Reactor and possibly from Worker (in the case of an error)
            SHUTDOWN = 11,
            // sent from Worker to Reactor
            TUNNEL_STREAM_DISPATCH_TIMEOUT = 13,
            // sent from Reactor to itself to force a tunnel stream dispatch
            TUNNEL_STREAM_DISPATCH_NOW = 14,
            // sent from Reactor to itself to force a watchlist dispatch
            WATCHLIST_DISPATCH_NOW = 15,
            // sent from Worker to Reactor
            WATCHLIST_TIMEOUT = 17,

            // sent from Reactor to Woker
            TOKEN_MGNT = 18,

            // sent from Reactor to itself for dispatching to the application
            TOKEN_CREDENTIAL_RENEWAL = 19,

            // sent from ReactorChannel to Worker
            PREFERRED_HOST_OPTIONS_CHANGED = 20,
            // sent from Worker to Reactor
            PREFERRED_HOST_OPTIONS_APPLIED = 21,
            // sent from Reactor to Worker
            PREFERRED_HOST_START_FALLBACK = 22,
            // sent from Worker to Reactor
            PREFERRED_HOST_STARTING_FALLBACK = 23,
            // sent from Worker to Reactor
            PREFERRED_HOST_RECONNECT_COMPLETE = 24,
            // sent from Reactor to Worker
            PREFERRED_HOST_SWITCHOVER_COMPLETE = 25,
            // sent from Worker to Reactor
            PREFERRED_HOST_COMPLETE = 26,

            // The preferred host operation has determined that no fallback will be performed on this channel.
            PREFERRED_HOST_NO_FALLBACK = 27
        }

        internal ImplType EventImplType { get; set; }

        internal ReactorTokenSession? TokenSession {get; set; }

        internal object? AdditonalPayload { get; set; }

        public ReactorEventImpl()
        {
            Clear();
        }

        internal void Clear()
        {
            Type = ReactorEventType.REACTOR;
            EventImplType = ImplType.INIT;
            TokenSession = null;
            AdditonalPayload = null;
        }
    }
}
