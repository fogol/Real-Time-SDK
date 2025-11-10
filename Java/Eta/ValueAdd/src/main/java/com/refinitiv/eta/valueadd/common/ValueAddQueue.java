/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.common;

public interface ValueAddQueue
{
    void add(VaNode node);
    VaNode poll();
    VaNode peek();
    boolean remove(VaNode node);
    int size();
    default void setLimit(int newLimit) {}
}
