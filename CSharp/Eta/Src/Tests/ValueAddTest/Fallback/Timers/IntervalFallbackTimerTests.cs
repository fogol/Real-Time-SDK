/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.ValueAdd.Reactor.Fallback.Timers;
using System;
using Xunit;

namespace LSEG.Eta.Tests.ValueAddTest.Fallback.Timers
{
    public class IntervalFallbackTimerTests
    {
        [Fact]
        public void Ctor_should_throw_on_zero_interval()
        {
            var exception = Assert.Throws<ArgumentException>(() => new IntervalFallbackTimer(0));
            Assert.StartsWith("Detection time interval cannot be 0", exception.Message);
        }

        [Theory]
        [InlineData(1)]
        [InlineData(2)]
        [InlineData(3)]
        [InlineData(10)]
        [InlineData(uint.MaxValue)]
        public void TriggerTimeMs_should_return_undefined_time_initially(uint intervalSec)
        {
            var timer = new IntervalFallbackTimer(intervalSec);
            Assert.Equal(long.MaxValue, timer.TriggerTimeMs);
        }

        [Theory]
        [InlineData(1)]
        [InlineData(2)]
        [InlineData(3)]
        [InlineData(10)]
        [InlineData(uint.MaxValue)]
        public void Reset_should_properly_update_TriggerTimeMs(uint intervalSec)
        {
            // Arrange
            var currentTimeMilliSec = ReactorUtil.GetCurrentTimeMilliSecond();
            var timer = new IntervalFallbackTimer(intervalSec);
            // Act
            timer.Reset(currentTimeMilliSec);
            // Assert
            Assert.Equal(currentTimeMilliSec, timer.TriggerTimeMs - intervalSec * 1000);
        }
    }
}
