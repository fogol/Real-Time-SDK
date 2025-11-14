/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.ValueAdd.Reactor.Fallback.Timers;
using System;
using System.Collections.Generic;
using Xunit;
using DateTime = System.DateTime;

namespace LSEG.Eta.Tests.ValueAddTest.Fallback.Timers
{
    public class CronFallbackTimerTests
    {
        [Theory]
        [InlineData("")]
        [InlineData(null)]
        public void Ctor_should_throw_on_empty_or_null_schedule(string cronExpression)
        {
            var exception = Assert.Throws<ArgumentException>(() => new CronFallbackTimer(cronExpression));
            Assert.StartsWith("Detection time schedule cannot be null or empty string", exception.Message);
        }

        [Theory]
        [InlineData("* * * *")]
        [InlineData("/1 * * * *")]
        [InlineData("not a CRON schedule at all")]
        public void Ctor_should_throw_on_invalid_schedule(string cronExpression)
        {
            var exception = Assert.Throws<ArgumentException>(() => new CronFallbackTimer(cronExpression));
            Assert.StartsWith("Invalid detection time schedule", exception.Message);
        }

        [Theory]
        [InlineData("* * * * *")]
        [InlineData("*/10 * * * *")]
        public void TriggerTimeMs_should_return_undefined_time_initially(string cronExpression)
        {
            var timer = new CronFallbackTimer(cronExpression);
            Assert.Equal(long.MaxValue, timer.TriggerTimeMs);
        }

        [Theory]
        [MemberData(nameof(GetValidCronScheduleAndTriggerDateTime))]
        public void Reset_should_set_proper_TriggerTimeMs_value(string cronExpression, DateTime currentDateTime, DateTime triggerDateTime)
        {
            // Arrange
            var timer = new CronFallbackTimer(cronExpression);
            var currentTimeMs = ReactorUtil.ConvertDateTimeToMilliSecondTime(currentDateTime);
            // Act
            timer.Reset(currentTimeMs);
            // Assert
            Assert.Equal(ReactorUtil.ConvertDateTimeToMilliSecondTime(triggerDateTime), timer.TriggerTimeMs);
        }

        public static IEnumerable<object[]> GetValidCronScheduleAndTriggerDateTime()
        {
            // just basic examples in order to ensure that expression is properly passed to Cronos library
            return new[] {
                new object[] { "*/10 * * * *", new DateTime(2020, 04, 1, 9, 34, 15), new DateTime(2020, 04, 1, 9, 40, 0), },
                new object[] { "15 * * * *", new DateTime(2020, 04, 1, 9, 34, 15), new DateTime(2020, 04, 1, 10, 15, 0), },
                new object[] { "23 14 * * *", new DateTime(2020, 04, 1, 9, 34, 15), new DateTime(2020, 04, 1, 14, 23, 0), },
                new object[] { "4 8 * * *", new DateTime(2020, 04, 1, 9, 34, 15), new DateTime(2020, 04, 2, 8, 4, 0), },
                // seconds supported too
                new object[] { "*/10 * * * * *", new DateTime(2020, 04, 1, 9, 34, 15), new DateTime(2020, 04, 1, 9, 34, 20), },
            };
        }
    }
}
