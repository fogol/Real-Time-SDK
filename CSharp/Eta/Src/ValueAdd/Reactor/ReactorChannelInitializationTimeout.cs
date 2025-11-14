/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Runtime.CompilerServices;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// ReactorChannel initialization timeout computation that can be used in <see cref="ReactorChannel"/> as well as in <see cref="Fallback.FallbackContext"/>.
    /// </summary>
    internal class ReactorChannelInitializationTimeout
    {
        private long m_InitializationTimeout;
        private long m_InitializationEndTimeMs;

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Timeout(long timeout)
        {
            m_InitializationTimeout = timeout;
            m_InitializationEndTimeMs = (timeout * 1000) + ReactorUtil.GetCurrentTimeMilliSecond();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public long Timeout()
        {
            return m_InitializationTimeout;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public long EndTimeMs()
        {
            return m_InitializationEndTimeMs;
        }
    }
}
