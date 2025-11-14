/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.Tests.ValueAddTest.Fallback.ConnectionInfoSelectors
{
    /// <summary>
    /// Assigns indices for objects from passed sequence, then allow to retrieve index using reference to object itself.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public class ObjectIndexMap<T>
        where T : class
    {
        private readonly ConditionalWeakTable<T, ObjectIndex> m_WeakTable;

        public ObjectIndexMap(IEnumerable<T> seq)
        {
            m_WeakTable = new();
            var i = 0;
            foreach (var item in seq)
            {
                m_WeakTable.Add(item, new ObjectIndex(i));
                i++;
            }
        }

        /// <summary>
        /// Retrieves index of object in initial sequence passed to this index map constructor.
        /// </summary>
        /// <param name="obj"></param>
        /// <returns>Returns index of <paramref name="obj"/> in initial sequence or -1 if it's absent there</returns>
        public int IndexOf(T obj) =>
            m_WeakTable.TryGetValue(obj, out var objectIndex)
                ? objectIndex.Value
                : -1;

        /// <summary>
        /// Retrieves index of object in initial sequence passed to this index map constructor.
        /// </summary>
        /// <param name="obj"></param>
        /// <returns>Returns index of <paramref name="obj"/> in initial sequence or -1 if it's absent there</returns>
        public int this[T obj] => IndexOf(obj);

        private record ObjectIndex(int Value);
    }

    /// <summary>
    /// Helper class for constructor type parameter inference of <see cref="ObjectIndexMap<T>"/>.
    /// </summary>
    public static class ObjectIndexMap
    {
        public static ObjectIndexMap<T> Of<T>(IEnumerable<T> seq)
            where T : class
            => new(seq);
    }
}