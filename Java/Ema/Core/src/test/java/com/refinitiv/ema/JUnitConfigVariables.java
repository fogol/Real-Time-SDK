/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema;

public class JUnitConfigVariables
{
    public static final int WAIT_AFTER_TEST; // Time in milliseconds
    public static final int SERVER_BIND_RETRY_COUNT; // The number of times server tries to bind to the socket before it fails.

    public static final int MSGQUEUE_POLL_TIMEOUT_MS;
    public static final int TEST_RETRY_COUNT;

    static
    {
        int waitAfterTestVal = 500;
        int serverBindRetryCountVal = 5;
        int testRetryCountVal = 3;
        int pollTimeoutVal = 5000;

        String waitAfterTest = System.getProperty("junitWaitAfterTest");
        String serverBindRetryCount = System.getProperty("junitServerBindRetryCount");
        String testRetryCount = System.getProperty("junitTestRetryCount");
        String pollTimeout = System.getProperty("junitMsgQueuePollTimeout");

        try {
            if (waitAfterTest != null) waitAfterTestVal = Integer.parseInt(waitAfterTest);
            if (serverBindRetryCount != null) serverBindRetryCountVal = Integer.parseInt(serverBindRetryCount);
            if (testRetryCount != null) testRetryCountVal = Integer.parseInt(testRetryCount);
            if (pollTimeout != null) pollTimeoutVal = Integer.parseInt(pollTimeout);
        }
        catch (Exception e) {}

        WAIT_AFTER_TEST = waitAfterTestVal;
        SERVER_BIND_RETRY_COUNT = serverBindRetryCountVal;
        TEST_RETRY_COUNT = testRetryCountVal;
        MSGQUEUE_POLL_TIMEOUT_MS = pollTimeoutVal;
    }
}
