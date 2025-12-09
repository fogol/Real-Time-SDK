/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Ema.Access.Tests.Utils
{
    public static class ComplexTypeExtensions
    {
        public static T Tap<T>(this T self, Action<T> action)
            where T : ComplexType
        {
            action(self);
            return self;
        }
    }
}
