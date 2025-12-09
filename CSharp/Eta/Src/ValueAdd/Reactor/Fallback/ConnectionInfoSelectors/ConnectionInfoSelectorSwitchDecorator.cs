/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;

namespace LSEG.Eta.ValueAdd.Reactor.Fallback.ConnectionInfoSelectors
{
    /// <summary>
    /// Allows to change <see cref="IConnectionInfoSelector"/> instance on fly as well as access to its members, both in thread safe manner.
    /// </summary>
    internal sealed class ConnectionInfoSelectorSwitchDecorator : IConnectionInfoSelector, IConnectionInfoSelectorSwitcher
    {
        private ReaderWriterLockSlim m_ReaderWriterLock = new();
        private IConnectionInfoSelector? m_Wrapee;
        private bool m_IsDisposed;

        public void SetConnectionInfoSelector(IConnectionInfoSelector connectionInfoSelector)
        {
            using var lockScope = m_ReaderWriterLock.EnterWriteLockScope();
            var stateNext = connectionInfoSelector as IConnectionInfoSelectorPreservedState;
            var stateCurrent = m_Wrapee as IConnectionInfoSelectorPreservedState;
            if (stateCurrent != null && stateNext != null)
            {
                stateNext.CurrentIndex = stateCurrent.CurrentIndex;
            }

            m_Wrapee = connectionInfoSelector;
        }

        public ReactorConnectInfo Current
        {
            get
            {
                using var lockScope = m_ReaderWriterLock.EnterReadLockScope();
                return m_Wrapee!.Current;
            }
        }

        public ReactorConnectInfo Next
        {
            get
            {
                using var lockScope = m_ReaderWriterLock.EnterReadLockScope();
                return m_Wrapee!.Next;
            }
        }

        public bool ShouldSwitchPrematurely
        {
            get
            {
                using var lockScope = m_ReaderWriterLock.EnterReadLockScope();
                return m_Wrapee!.ShouldSwitchPrematurely;
            }
        }

        public ReactorConnectInfo SwitchToNext(int reconnectAttempt)
        {
            using var lockScope = m_ReaderWriterLock.EnterWriteLockScope();
            return m_Wrapee!.SwitchToNext(reconnectAttempt);
        }

        public override string ToString()
        {
            using var lockScope = m_ReaderWriterLock.EnterReadLockScope();
            return m_Wrapee?.ToString() ?? "";
        }

        #region IDisposable implementation
        private void Dispose(bool disposing)
        {
            if (m_IsDisposed)
                return;

            if (disposing)
            {
                // dispose managed state (managed objects)
                m_ReaderWriterLock.Dispose();
                m_ReaderWriterLock = null!;
            }

            // free unmanaged resources (unmanaged objects) and override finalizer
            // set large fields to null

            m_IsDisposed = true;
        }

        ~ConnectionInfoSelectorSwitchDecorator()
        {
            Dispose(disposing: false);
        }

        public void Dispose()
        {
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }
        #endregion
    }
}
