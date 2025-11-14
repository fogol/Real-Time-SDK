/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Common;

namespace LSEG.Eta.ValueAdd.Reactor.Fallback
{
    /// <summary>
    /// Holds necessary state for PH fallback (e.g. <see cref="IsSwitchingToPreferredHost"/>)
    /// as well as several <see cref="ReactorChannel"/> properties which values need to be preserved until some point in fallback flow.
    /// 
    /// The idea is:
    /// <list type="bullet">
    ///     <item>During PH fallback allow to change several <see cref="ReactorChannel"/> properties via this class but apply those changes at particular point (<see cref="FallbackReconnectionComplete"/>).</item>
    ///     <item>During normal connection recovery allow to change <see cref="ReactorChannel"/> properties via this class immediately.</item>
    /// </list>
    /// That allows to reuse code across normal connection recovery and PH fallback flow, just by changing it from
    /// for example <code>ReactorChannel.State</code> to <code>ReactorChannel.FallbackContext.State</code>.
    /// </summary>
    internal class FallbackContext
    {
        private readonly ReactorChannel m_ReactorChannel;

        private ReactorChannelState m_State;
        private IChannel? m_Channel;
        private object? m_UserSpecObj;
        private NotifierEvent m_NotifierEvent;
        private ReactorChannelInitializationTimeout m_Initialization;

        private bool m_IsShadowingEnabled;
        private bool m_IsSwitchingToPreferredHost;
        private bool m_IsFirstTimeConnect;

        private readonly ReaderWriterLockSlim m_Lock;

        public FallbackContext(ReactorChannel reactorChannel)
        {
            m_ReactorChannel = reactorChannel;

            m_NotifierEvent = new();
            m_Initialization = new();

            m_IsFirstTimeConnect = true;

            m_Lock = new();
        }

        public bool IsSwitchingToPreferredHost
        {
            get
            {
                using var lockScope = m_Lock.EnterReadLockScope();
                return m_IsSwitchingToPreferredHost;
            }
            private set
            {
                using var lockScope = m_Lock.EnterWriteLockScope();
                m_IsSwitchingToPreferredHost = value;
            }
        }

        public bool IsFirstTimeConnect
        {
            get
            {
                using var lockScope = m_Lock.EnterReadLockScope();
                return m_IsFirstTimeConnect;
            }
            private set
            {
                using var lockScope = m_Lock.EnterWriteLockScope();
                m_IsFirstTimeConnect = value;
            }
        }

        public void BeginEnforcedFallback()
        {
            IsSwitchingToPreferredHost = true;
            m_IsShadowingEnabled = true;
            m_State = m_ReactorChannel.State;
            IsFirstTimeConnect = false;
        }

        public void FallbackReconnectionComplete()
        {
            if (!IsSwitchingToPreferredHost)
                return;

            m_ReactorChannel.UserSpecObj = m_UserSpecObj;
            OldChannel = m_ReactorChannel.Channel;
            m_ReactorChannel.Channel = m_Channel;
            m_ReactorChannel.Socket = m_Channel?.Socket;
            m_ReactorChannel.NotifierEvent.EventSocket = m_Channel?.Socket;
            m_Channel = null;

            m_IsShadowingEnabled = false;
        }

        public void FallbackSwitchoverComplete()
        {
            if (!IsSwitchingToPreferredHost)
                return;

            OldChannel?.Close(out _);
            OldChannel = null;
            IsSwitchingToPreferredHost = false;
            IsFirstTimeConnect = false;
        }

        public void RollbackEnforcedFallback()
        {
            if (!IsSwitchingToPreferredHost)
                return;

            m_Channel?.Close(out _);
            m_Channel = null;
            IsSwitchingToPreferredHost = false;
            m_IsShadowingEnabled = false;
            IsFirstTimeConnect = false;
        }

        public IChannel? OldChannel { get; private set; }

        #region ReactorChannel state that should not be affected during fallback process until reconnection complete

        public ReactorChannelState State
        {
            get => m_IsShadowingEnabled ? m_State : m_ReactorChannel.State;
            set
            {
                if (m_IsShadowingEnabled)
                    m_State = value;
                else
                    m_ReactorChannel.State = value;
            }
        }

        public IChannel? Channel
        {
            get => m_IsShadowingEnabled ? m_Channel : m_ReactorChannel.Channel;
            set
            {
                if (m_IsShadowingEnabled)
                {
                    m_Channel = value;
                    m_NotifierEvent.EventSocket = m_Channel?.Socket;
                }
                else
                    m_ReactorChannel.Channel = value;
            }
        }

        public object? UserSpecObj
        {
            get => m_IsShadowingEnabled ? m_UserSpecObj : m_ReactorChannel.UserSpecObj;
            set
            {
                if (m_IsShadowingEnabled)
                    m_UserSpecObj = value;
                else
                    m_ReactorChannel.UserSpecObj = value;
            }
        }

        public NotifierEvent NotifierEvent =>
            m_IsShadowingEnabled ? m_NotifierEvent : m_ReactorChannel.NotifierEvent;

        public void SetChannel(IChannel? channel)
        {
            if (m_IsShadowingEnabled)
                m_Channel = channel;
            else
                m_ReactorChannel.SetChannel(channel);
        }

        public void InitializationTimeout(int timeout)
        {
            if (m_IsShadowingEnabled)
                m_Initialization.Timeout(timeout);
            else
                m_ReactorChannel.InitializationTimeout(timeout);
        }

        public long InitializationEndTimeMs() =>
            m_IsShadowingEnabled
                ? m_Initialization.EndTimeMs()
                : m_ReactorChannel.InitializationEndTimeMs();

        #endregion
    }
}
