/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.ValueAdd.Reactor.Fallback.ConnectionInfoSelectors;
using LSEG.Eta.ValueAdd.Reactor.Fallback.Init;
using LSEG.Eta.ValueAdd.Reactor.Fallback.Timers;
using System.Collections.Generic;
using Xunit;

namespace LSEG.Eta.Tests.ValueAddTest.Fallback
{
    public class FallbackMembersToAssignTests
    {
        [Theory]
        [MemberData(nameof(GetDisabledPreferredHostOptions))]
        public void Should_create_roundrobin_connection_selector_when_preferred_host_options_disabled(ReactorPreferredHostOptions phOpts)
        {
            // Act
            var membersToAssign = new FallbackMembersToAssign(phOpts, CreateSimpleConnectionList());
            // Assert
            Assert.IsType<RoundRobinConnectionInfoSelector>(membersToAssign.ConnectionInfoSelector);
            Assert.Null(membersToAssign.FallbackTimer);
        }

        [Fact]
        public void Should_create_preferred_host_connection_selector_when_preferred_host_options_enabled()
        {
            // Arrange
            var phOpts = new ReactorPreferredHostOptions { EnablePreferredHostOptions = true };
            // Act
            var membersToAssign = new FallbackMembersToAssign(phOpts, CreateSimpleConnectionList());
            // Assert
            Assert.IsType<PreferredHostConnectionInfoSelector>(membersToAssign.ConnectionInfoSelector);
            Assert.Null(membersToAssign.FallbackTimer);
        }

        [Fact]
        public void Should_create_interval_fallback_timer_when_DetectionTimeInterval_specified()
        {
            // Arrange
            var phOpts = new ReactorPreferredHostOptions { EnablePreferredHostOptions = true, DetectionTimeInterval = 500 };
            // Act
            var membersToAssign = new FallbackMembersToAssign(phOpts, CreateSimpleConnectionList());
            // Assert
            Assert.IsType<IntervalFallbackTimer>(membersToAssign.FallbackTimer);
            Assert.IsType<PreferredHostConnectionInfoSelector>(membersToAssign.ConnectionInfoSelector);
        }

        [Fact]
        public void Should_create_CRON_fallback_timer_when_DetectionTimeSchedule_specified()
        {
            // Arrange
            var phOpts = new ReactorPreferredHostOptions { EnablePreferredHostOptions = true, DetectionTimeSchedule = "* * * * *" };
            // Act
            var membersToAssign = new FallbackMembersToAssign(phOpts, CreateSimpleConnectionList());
            // Assert
            Assert.IsType<CronFallbackTimer>(membersToAssign.FallbackTimer);
            Assert.IsType<PreferredHostConnectionInfoSelector>(membersToAssign.ConnectionInfoSelector);
        }

        [Fact]
        public void Should_create_CRON_fallback_timer_when_both_DetectionTimes_specified()
        {
            // Arrange
            var phOpts = new ReactorPreferredHostOptions
            {
                EnablePreferredHostOptions = true,
                DetectionTimeSchedule = "* * * * *",
                DetectionTimeInterval = 500,
            };
            // Act
            var membersToAssign = new FallbackMembersToAssign(phOpts, CreateSimpleConnectionList());
            // Assert
            Assert.IsType<CronFallbackTimer>(membersToAssign.FallbackTimer);
            Assert.IsType<PreferredHostConnectionInfoSelector>(membersToAssign.ConnectionInfoSelector);
        }

        public static IEnumerable<object[]> GetDisabledPreferredHostOptions()
        {
            return new[] {
                new[] { new ReactorPreferredHostOptions { EnablePreferredHostOptions = false, } },
                new[] { new ReactorPreferredHostOptions { EnablePreferredHostOptions = false, DetectionTimeInterval = 10, } },
                new[] { new ReactorPreferredHostOptions { EnablePreferredHostOptions = false, DetectionTimeSchedule = "* * * * *", } },
                new[] { new ReactorPreferredHostOptions { EnablePreferredHostOptions = false, DetectionTimeInterval = 10, DetectionTimeSchedule = "* * * * *", } },
            };
        }

        private IReadOnlyList<ReactorConnectInfo> CreateSimpleConnectionList() =>
            new List<ReactorConnectInfo>
            {
                new(),
                new(),
                new(),
            };
    }
}
