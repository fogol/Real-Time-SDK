/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Linq;

namespace LSEG.Eta.Tests
{
    static class TestUtilities
    {
        public static bool CompareByteArray(byte[] first, int firstOffSet, byte[] second, int secondOffset, int length)
        {
            /* Checks whether both of them holds the same reference */
            if (first == second)
                return true;

            int index1 = firstOffSet;
            int index2 = secondOffset;

            for (; index1 < length; index1++, index2++)
            {
                if (first[index1] != second[index2])
                    return false;
            }

            return true;
        }

        public static IEnumerable<object[]> ToXunitMemberData<T>(this IEnumerable<T> data) =>
            data.Select(x => new object[] { x });
    }
}
