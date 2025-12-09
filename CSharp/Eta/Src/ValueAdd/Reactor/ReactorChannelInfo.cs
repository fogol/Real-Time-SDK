/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Information returned by the <see cref="ReactorChannel.Info(ReactorChannelInfo, out ReactorErrorInfo?)"/> call.
    /// </summary>
    public sealed class ReactorChannelInfo
    {
        /// <summary>
        /// Gets Channel information.
        /// </summary>
        public ChannelInfo ChannelInfo { get; private set; } = new ChannelInfo();

        /// <summary>
        /// Gets preferred host information.
        /// </summary>
        public ReactorPreferredHostInfo PreferredHostInfo { get; private set; } = new ReactorPreferredHostInfo();

        /// <summary>
        /// Clears this object for reuse.
        /// </summary>
        public void Clear()
        {
            ChannelInfo.Clear();
            PreferredHostInfo.Clear();
        }
    }
}
