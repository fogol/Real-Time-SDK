/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Ema.Access.Tests;

public class EmaObjectPoolTests
{
    private void PoolTest<TArray>(
        Func<EmaObjectManager, TArray> getFromPool,
        Action<EmaObjectManager, TArray> returnToPool,
        Func<TArray, bool> isOwnedByPool,
        Func<EmaObjectManager, int> getPoolBorrowedCount,
        Func<EmaObjectManager, int> getPoolLimit)
    {
        // Test must be independent from EmaGlobalObjectPool.Instance & its configuration in order to be executed
        // on Linux without intermittent failures.
        const int INITIAL_POOL_SIZE = 10;
        var objectPoolManager = new EmaObjectManager(INITIAL_POOL_SIZE, global: false);

        var arrays = new TArray[INITIAL_POOL_SIZE * 2];

        // no objects were returned from the pool yet
        Assert.Equal(0, getPoolBorrowedCount(objectPoolManager));

        for (int i = 0; i < arrays.Length; i++)
        {
            arrays[i] = getFromPool(objectPoolManager);
        }

        // pool has been drained dry to the limit
        Assert.Equal(getPoolLimit(objectPoolManager), getPoolBorrowedCount(objectPoolManager));

        foreach (var array in arrays)
        {
            if (isOwnedByPool(array))
                returnToPool(objectPoolManager, array);
        }

        Assert.Equal(0, getPoolBorrowedCount(objectPoolManager));

        // test has passed when no exceptions were thrown
    }

    [Fact]
    public void ObjectPrimitivePoolTest()
    {
        PoolTest<EmaObjectManager.DataArray>(
            objManager => objManager.GetPrimitiveDataArrayFromPool(),
            (objManager, arr) => objManager.ReturnPrimitiveDataArrayToPool(arr),
            arr => arr.OwnedByPool,
            objManager => objManager.primitivePool.current,
            objManager => objManager.primitivePool.limit
            );
    }

    [Fact]
    public void ObjectComplexPoolTest()
    {
        PoolTest<EmaObjectManager.ComplexTypeArray>(
            objManager => objManager.GetComplexTypeArrayFromPool(),
            (objManager, arr) => objManager.ReturnComplexTypeArrayToPool(arr),
            arr => arr.OwnedByPool,
            objManager => objManager.complexTypePool.current,
            objManager => objManager.complexTypePool.limit
            );
    }

    [Fact]
    public void ObjectMsgTypePoolTest()
    {
        PoolTest<EmaObjectManager.MsgTypeArray>(
            objManager => objManager.GetMsgTypeArrayFromPool(),
            (objManager, arr) => objManager.ReturnMsgTypeArrayToPool(arr),
            arr => arr.OwnedByPool,
            objManager => objManager.msgTypePool.current,
            objManager => objManager.msgTypePool.limit
            );
    }
}
