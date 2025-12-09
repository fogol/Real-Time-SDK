/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using FakeItEasy;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.Tests.ValueAddTest;
using System;
using System.Linq;
using Xunit;

namespace LSEG.Eta.Tests.ValueAddTest.ReactorChannelPreferredHostTests
{
    public class IOCtlTests
    {
        [Fact]
        public void Should_return_error_when_Reactor_is_null()
        {
            // Arrange
            var reactorChannel = new ReactorChannel();
            // Act
            var retCode = reactorChannel.IOCtl(ReactorChannelIOCtlCode.PREFERRED_HOST_OPTIONS, GetEnabledPreferredHostOptions(), out var errorInfo);
            // Assert
            Assert.Equal(ReactorReturnCode.FAILURE, retCode);
            Assert.NotNull(errorInfo);
            Assert.StartsWith("Reactor cannot be null", errorInfo.Error?.Text);
        }

        [Fact]
        public void Should_return_error_when_Reactor_is_shutting_down()
        {
            // Arrange
            var reactorChannel = new ReactorChannel();
            var consumer = new TestReactor();
            reactorChannel.Reactor = consumer.Reactor;

            var shutdownRetCode = consumer.Reactor.Shutdown(out var shutdownErrorInfo);
            Assert.Equal(ReactorReturnCode.SUCCESS, shutdownRetCode);
            Assert.Null(shutdownErrorInfo);

            // Act
            var retCode = reactorChannel.IOCtl(ReactorChannelIOCtlCode.PREFERRED_HOST_OPTIONS, GetEnabledPreferredHostOptions(), out var errorInfo);

            // Assert
            Assert.Equal(ReactorReturnCode.SHUTDOWN, retCode);
            Assert.NotNull(errorInfo);
            Assert.StartsWith("Reactor is shutdown", errorInfo.Error?.Text);
        }

        [Fact]
        public void Should_return_error_when_value_is_null()
        {
            // Arrange
            var reactorChannel = new ReactorChannel();
            var consumer = new TestReactor();
            reactorChannel.Reactor = consumer.Reactor;
            // Act
            var retCode = reactorChannel.IOCtl(ReactorChannelIOCtlCode.PREFERRED_HOST_OPTIONS, null, out var errorInfo);
            // Assert
            Assert.Equal(ReactorReturnCode.PARAMETER_INVALID, retCode);
            Assert.NotNull(errorInfo);
            Assert.StartsWith("value cannot be null", errorInfo.Error?.Text);
        }

        [Fact]
        public void Should_return_error_when_transport_channel_is_null()
        {
            // Arrange
            var reactorChannel = new ReactorChannel();
            var consumer = new TestReactor();
            reactorChannel.Reactor = consumer.Reactor;
            reactorChannel.Channel = null;
            // Act
            var retCode = reactorChannel.IOCtl(ReactorChannelIOCtlCode.PREFERRED_HOST_OPTIONS, GetEnabledPreferredHostOptions(), out var errorInfo);
            // Assert
            Assert.Equal(ReactorReturnCode.SHUTDOWN, retCode);
            Assert.NotNull(errorInfo);
            Assert.StartsWith("Channel is shutdown", errorInfo.Error?.Text);
        }

        [Fact]
        public void Should_return_error_when_ioctl_code_is_unsupported()
        {
            // Arrange
            var reactorChannel = new ReactorChannel();
            var consumer = new TestReactor();
            reactorChannel.Reactor = consumer.Reactor;
            reactorChannel.Channel = A.Fake<IChannel>();
            // assigning a value that is out of range of enum
            var ioctlCode = (ReactorChannelIOCtlCode)Enum.GetValues<ReactorChannelIOCtlCode>().Cast<int>().Max() + 1;
            // Act
            var retCode = reactorChannel.IOCtl(ioctlCode, GetEnabledPreferredHostOptions(), out var errorInfo);
            // Assert
            Assert.Equal(ReactorReturnCode.PARAMETER_INVALID, retCode);
            Assert.NotNull(errorInfo);
            Assert.StartsWith("Unsupported IOCtl code", errorInfo.Error?.Text);
        }

        private static ReactorPreferredHostOptions GetEnabledPreferredHostOptions() =>
            new()
            {
                EnablePreferredHostOptions = true,
            };
    }
}
