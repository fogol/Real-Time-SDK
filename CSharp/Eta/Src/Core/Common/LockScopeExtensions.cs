/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Threading;

namespace LSEG.Eta.Common
{
    /// <summary>
    /// The class for implementation of locking mechanism via RAII.
    /// It's based on idea from .NET 9 Lock.EnterScope method.
    /// Each "Enter*Scope" method performs locking and returns disposable object that will release lock on its <c>Dispose</c>
    /// method call. With <c>using</c> statement this guarantees unlocking even in case of exception without <c>try/finally</c> construct.
    /// Note that returned objects allocated on stack, so there is no additional heap allocations performed.
    /// </summary>
    internal static class LockScopeExtensions
    {
        /// <summary>
        /// Acquires read lock on <see cref="ReaderWriterLockSlim"/>.
        /// </summary>
        /// <param name="rwLock">Lock object to be acquired.</param>
        /// <returns>Returns disposable object that releases read lock on <see cref="ReadLockScope.Dispose"/> method call.</returns>
        public static ReadLockScope EnterReadLockScope(this ReaderWriterLockSlim rwLock) => new ReadLockScope(rwLock);

        /// <inheritdoc/>
        public ref struct ReadLockScope
        {
            private readonly ReaderWriterLockSlim m_RWLock;

            /// <inheritdoc/>
            public ReadLockScope(ReaderWriterLockSlim rwLock)
            {
                m_RWLock = rwLock;
                m_RWLock.EnterReadLock();
            }

            /// <inheritdoc/>
            public void Dispose() => m_RWLock.ExitReadLock();
        }

        /// <summary>
        /// Acquires read lock on <see cref="ReaderWriterLockSlim"/>.
        /// </summary>
        /// <param name="rwLock">Lock object to be acquired.</param>
        /// <returns>Returns disposable object that releases read lock on <see cref="WriteLockScope.Dispose"/> method call.</returns>
        public static WriteLockScope EnterWriteLockScope(this ReaderWriterLockSlim rwLock) => new WriteLockScope(rwLock);

        /// <inheritdoc/>
        public ref struct WriteLockScope
        {
            private readonly ReaderWriterLockSlim m_RWLock;

            /// <inheritdoc/>
            public WriteLockScope(ReaderWriterLockSlim rwLock)
            {
                m_RWLock = rwLock;
                m_RWLock.EnterWriteLock();
            }

            /// <inheritdoc/>
            public void Dispose() => m_RWLock.ExitWriteLock();
        }
    }
}

