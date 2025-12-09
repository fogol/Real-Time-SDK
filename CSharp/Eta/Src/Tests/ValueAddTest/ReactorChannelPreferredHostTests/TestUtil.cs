/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.ValuedAdd.Tests;
using System;

namespace LSEG.Eta.Tests.ValueAddTest.ReactorChannelPreferredHostTests
{
    public static class TestUtil
    {
        public static void AssertInitialDataExchange(
            Consumer consumer,
            Provider provider,
            bool isSwitchToPreferredHost = false,
            bool hasChannelDownReconnecting = true)
        {
            consumer.TestReactor.Dispatch(-1);

            if (isSwitchToPreferredHost)
                consumer.TestReactor
                   .AssertReactorChannelEvent(ReactorChannelEventType.PREFERRED_HOST_STARTING_FALLBACK);

            if (hasChannelDownReconnecting)
                consumer.TestReactor
                    .AssertReactorChannelEvent(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING);

            consumer.TestReactor
                .AssertReactorChannelEvent(ReactorChannelEventType.CHANNEL_UP);

            if (isSwitchToPreferredHost)
                consumer.TestReactor
                   .AssertReactorChannelEvent(ReactorChannelEventType.PREFERRED_HOST_COMPLETE);

            // Provider receives channel-up/channel-ready
            provider.TestReactor
                .Dispatch(3)
                .AssertReactorChannelEvent(ReactorChannelEventType.CHANNEL_UP)
                .AssertReactorChannelEvent(ReactorChannelEventType.CHANNEL_READY);

            // Provider receives consumer login request
            provider.RespondToLoginRequest();

            consumer.TestReactor.Dispatch(1);
            // Save the stream ID used by each component to open the login stream (may be
            // different if the watchlist is enabled).
            consumer.DefaultSessionLoginStreamId = consumer.Role.RdmLoginRequest.StreamId;
            consumer.TestReactor.AssertLoginMsgEvent(LoginMsgType.REFRESH);

            // Provider receives consumer directory request
            provider.TestReactor.Dispatch(1);
            provider.RespondToDirectoryRequest();

            consumer.TestReactor
                .Dispatch(2)
                .AssertDirectoryMsgEvent(DirectoryMsgType.REFRESH);
            // Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled).
            consumer.DefaultSessionDirectoryStreamId = consumer.Role.RdmDirectoryRequest.StreamId;
            consumer.TestReactor
                .AssertReactorChannelEvent(ReactorChannelEventType.CHANNEL_READY);
        }

        public static string GetCronExpressionFromNow(double? minutes = null, double? hours = null)
        {
            var dt = DateTime.Now;

            if (minutes.HasValue)
                dt = dt.AddMinutes(minutes.Value);
            if (hours.HasValue)
                dt = dt.AddHours(hours.Value);

            return $"{dt:m H} * * *";
        }
    }
}
