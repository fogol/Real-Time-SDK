/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using FakeItEasy;
using LSEG.Eta.Common;
using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.ValueAdd.Reactor.Fallback.ConnectionInfoSelectors;
using System.Collections.Generic;
using System.Linq;
using Xunit;

namespace LSEG.Eta.Tests.ValueAddTest.ReactorChannelPreferredHostTests
{
    public class FallbackPreferredHostTests
    {
        private readonly IReactor m_ReactorMock;

        public FallbackPreferredHostTests()
        {
            m_ReactorMock = A.Fake<IReactor>();
            A.CallTo(() => m_ReactorMock.ReactorLock).Returns(A.Fake<Locker>());
        }

        [Theory]
        [MemberData(nameof(GetDisabledPreferredHostOptions))]
        public void Should_return_error_when_preferred_host_disabled(ReactorConnectOptions connectOptions)
        {
            // Arrange
            var expectedRetCode = ReactorReturnCode.INVALID_USAGE;
            var reactorChannel = new ReactorChannel(connectOptions, A.Fake<IConnectionInfoSelector>(), m_ReactorMock)
            {
                Role = new ConsumerRole()
            };
            // Act
            var retCode = reactorChannel.FallbackPreferredHost(out var errorInfo);
            // Assert
            Assert.Equal(expectedRetCode, retCode);
            Assert.NotNull(errorInfo);
            Assert.StartsWith("Preferred host feature is not enabled for the specified ReactorChannel", errorInfo.Error.Text);
            Assert.Equal("ReactorChannel.FallbackPreferredHost", errorInfo.Location);
            Assert.Equal(expectedRetCode, errorInfo.Code);
        }

        public static IEnumerable<object[]> GetDisabledPreferredHostOptions()
        {
            return new ReactorPreferredHostOptions[] {
                new(){EnablePreferredHostOptions = false},
                new(){EnablePreferredHostOptions = false, DetectionTimeInterval = 10},
                new(){EnablePreferredHostOptions = false, DetectionTimeSchedule = "* * * * *"},
            }
            .Select(x =>
            {
                var connectOpts = new ReactorConnectOptions();
                x.Copy(connectOpts.PreferredHostOptions);
                return connectOpts;
            })
            .Append(null)
            .ToXunitMemberData();
        }

        [Theory]
        [MemberData(nameof(GetEnabledPreferredHostOptions))]
        public void Should_immediately_send_PREFERRED_HOST_NO_FALLBACK_and_return_success_when_already_on_preferred_host(ReactorConnectOptions connectOptions)
        {
            // Arrange
            var expectedRetCode = ReactorReturnCode.SUCCESS;
            var connectionInfoSelectorMock = A.Fake<IConnectionInfoSelector>(opt => opt.Implements<IConnectionInfoSelectorPreservedState>());
            var reactorChannel = new ReactorChannel(connectOptions, connectionInfoSelectorMock, m_ReactorMock)
            {
                Role = new ConsumerRole()
            };

            A.CallTo(() => connectionInfoSelectorMock.ShouldSwitchPrematurely).Returns(false);

            // Act
            var retCode = reactorChannel.FallbackPreferredHost(out var errorInfo);

            // Assert
            Assert.Equal(expectedRetCode, retCode);
            Assert.Null(errorInfo);
            A.CallTo(() => m_ReactorMock.SendPreferredHostNoFallback(reactorChannel)).MustHaveHappenedOnceExactly();
        }

        [Theory]
        [MemberData(nameof(GetEnabledPreferredHostOptions))]
        public void Should_send_PREFERRED_HOST_START_FALLBACK_to_ReactorWorker_when_not_on_preferred_host(ReactorConnectOptions connectOptions)
        {
            // Arrange
            var expectedRetCode = ReactorReturnCode.SUCCESS;
            var connectionInfoSelectorMock = A.Fake<IConnectionInfoSelector>(opt => opt.Implements<IConnectionInfoSelectorPreservedState>());
            var reactorChannel = new ReactorChannel(connectOptions, connectionInfoSelectorMock, m_ReactorMock)
            {
                Role = new ConsumerRole()
            };

            A.CallTo(() => connectionInfoSelectorMock.ShouldSwitchPrematurely).Returns(true);

            // Act
            var retCode = reactorChannel.FallbackPreferredHost(out var errorInfo);

            // Assert
            Assert.Equal(expectedRetCode, retCode);
            Assert.Null(errorInfo);
            A.CallTo(() => m_ReactorMock.SendWorkerImplEvent(ReactorEventImpl.ImplType.PREFERRED_HOST_START_FALLBACK, reactorChannel, null))
                .MustHaveHappenedOnceExactly();
        }

        public static IEnumerable<object[]> GetEnabledPreferredHostOptions()
        {
            return new ReactorPreferredHostOptions[] {
                new(){EnablePreferredHostOptions = true},
                new(){EnablePreferredHostOptions = true, DetectionTimeInterval = 10},
                new(){EnablePreferredHostOptions = true, DetectionTimeSchedule = "* * * * *"},
            }
            .Select(x =>
            {
                var connectOpts = new ReactorConnectOptions();
                x.Copy(connectOpts.PreferredHostOptions);
                return connectOpts;
            })
            .ToXunitMemberData();
        }
    }
}
