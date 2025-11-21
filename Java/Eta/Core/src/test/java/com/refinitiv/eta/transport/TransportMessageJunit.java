/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2021,2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import static com.refinitiv.eta.transport.TransportTestRunner.testRunner;
import static com.refinitiv.eta.transport.TransportTestRunner.testRunnerPacked;
import static org.junit.Assert.*;

import com.refinitiv.eta.transport.TransportTestRunner.TestArgs;
import com.refinitiv.eta.transport.TransportTestRunner.PackedTestArgs;
import com.refinitiv.eta.transport.TransportTestRunner.MessageContentType;

import com.refinitiv.eta.JUnitConfigVariables;
import com.refinitiv.eta.RetryRule;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;

import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.transport.Ripc.CompressionTypes;
import org.junit.rules.TestName;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

/**
 * 
 * The {@link TransportMessageJunit} creates a Channel between
 * a client and server. Messages are sent from client to server,
 * and the received message is verified to be identical to what
 * was sent. 
 * <p>
 * The test options include <ul>
 * <li>An array of messages sizes; one message will be sent of each message size.</li> 
 * <li>The compression type and level for the channel.</li>
 * <li>The content of the message from {@link MessageContentType}:
 * UNIFORM (all 1's), SEQUENCE (1,2,3...), or RANDOM (good for 
 * simulating poor compression). </li>
 * </ul>
 * <p>
 * <p>
 * The client/server framework for this test was based on {@link TransportLockJunit}.
 *
 */
public class TransportMessageJunit
{
    @Rule
    public RetryRule retryRule = new RetryRule(JUnitConfigVariables.TEST_RETRY_COUNT);

    @Rule
    public TestName testName = new TestName();

    @Before
    public void printTestName() {
        System.out.println(">>>>>>>>>>>>>>>>>>>>  " + testName.getMethodName() + " Test <<<<<<<<<<<<<<<<<<<<<<<");
    }

    @Test
    // Primarily for debugging during development
    public void test0()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = false;
        args.writeLocking = true;
        args.blocking = true;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 6;
        
        int[] sizes = { 6140};
        args.messageSizes = sizes;
        args.printReceivedData = true;
        args.debug = true;
        args.messageContent = MessageContentType.RANDOM;
        
        testRunner("test0: debugging", args);
    }

    @Test
    // Verify zlib level 0 split/re-combine with CompFragment
    public void test1()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 0;
        
        int[] sizes = { 6142 };
        args.messageSizes = sizes;
        args.printReceivedData = true;
        args.messageContent = MessageContentType.UNIFORM;
        
        testRunner("test1: basic", args);
    }

    // No compression: message sizes from no-frag to fragmentation
    @Test
    public void test2()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        
        args.compressionType = CompressionTypes.NONE;
        args.compressionLevel = 0;
        
        int[] sizes = { 6140, 6141, 6142, 6143, 6144, 6145, 6146, 6147, 6148, 6149 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.UNIFORM;
        
        testRunner("test2: no compression fragmentation boundary", args);
    }
    
    // lz4 compression growth: messages sizes from no-frag to fragmentation
    @Test
    public void test3()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 6;
        
        int[] sizes = { 6100, 6101, 6102, 6103, 6104, 6105, 6106, 6107, 6108, 6109, 6110};
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        
        testRunner("test3: lz4 fragmentation boundary test poor compression", args);
    }

    // zlib compression growth: message sizes from no-frag to fragmentation
    @Test
    public void test4()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 6;
        
        int[] sizes = { 6123, 6124, 6125, 6126, 6127, 6128, 6129, 6130, 6131, 6132, 6133, 6134, 6135, 6136, 6137, 6138, 6139, 6140 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        
        testRunner("test4: zlib fragmentation boundary test poor compression", args);
    }

    // Alternate fragments with Random and Uniform data.
    // Starting with Random forces compression fragmentation on the first fragment
    @Test
    public void test5()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 6;

        int[] sizes = { 50000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.MIXED_RANDOM_START;
        
        testRunner("test5: mixed data random start", args);
    }

    @Test
    public void test5z()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 6;

        int[] sizes = { 50000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.MIXED_RANDOM_START;
        
        testRunner("test5: mixed data random start", args);
    }

    // Alternate fragments with Random and Uniform data.
    // Starting with Uniform forces compression fragmentation on the second fragment
    @Test
    public void test6()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 6;

        int[] sizes = { 50000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.MIXED_UNIFORM_START;
        
        testRunner("test6: mised data uniform start", args);
    }

    @Test
    public void test6z()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 6;

        int[] sizes = { 50000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.MIXED_UNIFORM_START;
        
        testRunner("test6z: mixed data uniform start", args);
    }

    // Alternate fragments with Random and Uniform data.
    // Starting with Uniform forces compression fragmentation on the second fragment
    @Test
    public void test7()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 6;

        int[] sizes = { 500000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.MIXED_UNIFORM_START;
        
        testRunner("test7: mixed data large message", args);
    }

    @Test
    public void test8()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.NONE;
        args.compressionLevel = 0;

        int[] sizes = { 7000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        
        // frag 1: 6147
        // frag 2:  869
        args.expectedTotalBytes = 7016;
        
        testRunner("test8: ", args);
    }

    // fragment + compressed frag testing writeArgs bytes written
    @Test
    public void test8lz4()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 0;

        int[] sizes = { 7000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        
        // frag 1: 6147
        // frag 1b: 29
        // frag 2: 874
        args.expectedTotalBytes = 7050;
        
        // frag 1: 10
        // frag 1b: 3
        // frag 2: 6
        // data: 7000
        args.expectedUncompressedBytes = 7019;
        
        testRunner("test8lz4: fragment with compFragment testing write args bytes written", args);
    }

    // fragment + compressed frag testing writeArgs bytes written
    @Test
    public void test8z()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 0;
        args.debug = true;

        int[] sizes = { 7000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        
        // frag 1: 6147
        // frag 1b: 15
        // frag 2: 879
        args.expectedTotalBytes = 7041;
        
        // frag 1: 10
        // frag 1b: 3
        // frag 2: 6
        // data: 7000
        args.expectedUncompressedBytes = 7019;
        
        testRunner("test8z: zlib fragment with compFragment testing write args bytes written", args);
    }

    // compressed frag normal: testing writeArgs bytes written
    @Test
    public void test9lz4()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 0;

        int[] sizes = { 6140 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        
        // 1: 6147
        // 1b: 25
        args.expectedTotalBytes = 6172;
        
        // 1: 3
        // 1b: 3
        // data: 6140
        args.expectedUncompressedBytes = 6146;
        
        testRunner("test9lz4: compFragment normal testing write args bytes written", args);
    }
    
    @Test
    public void test10()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.NONE;
        args.compressionLevel = 0;

        int[] sizes = { 12300 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        
        testRunner("test10: ", args);
    }

    // Designed so that the last fragment size is below the compression threshold
    @Test
    public void test10lz4()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 0;

        int[] sizes = { 12300 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        
        testRunner("test10lz4: last fragment not compressed", args);
    }

    // Designed so that the last fragment size is below the compression threshold
    @Test
    public void test10z()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 0;

        int[] sizes = { 12300 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        
        testRunner("test10z: last fragment not compressed", args);
    }

    @Test
    public void test11z()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 0;

        int[] sizes = { 6128, 6129 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        
        testRunner("test11z: ", args);
    }

    @Test
    public void ptest1()
    {
        PackedTestArgs args = PackedTestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 0;

        args.packedMessageSize = 612;
        args.packedCount = 10;
        args.printReceivedData = false;
        args.debug = false;
        args.messageContent = MessageContentType.RANDOM;
        
        testRunnerPacked("ptest1: packing with random data and lz4 ", args);
    }

    @Test
    public void ptest2()
    {
        TransportTestRunner.PackedTestArgs args = TransportTestRunner.PackedTestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 6;

        args.packedMessageSize = 612;
        args.packedCount = 10;
        args.printReceivedData = false;
        args.debug = false;
        args.messageContent = TransportTestRunner.MessageContentType.RANDOM;
        
        testRunnerPacked("ptest2: packing with random data and zlib ", args);
    }

    @Test
    public void ptest3()
    {
        TransportTestRunner.PackedTestArgs args = TransportTestRunner.PackedTestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.NONE;
        args.compressionLevel = 6;

        args.packedMessageSize = 612;
        args.packedCount = 10;
        args.printReceivedData = false;
        args.debug = false;
        args.messageContent = MessageContentType.RANDOM;
        
        testRunnerPacked("ptest3: packing with random data and no compression ", args);
    }

    @Test
    public void test_BufferBackedByArray()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 10;
        args.guaranteedOutputBuffers = 500;
        args.blocking = false;

        int[] sizes = {};
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;

        args.compressionType = CompressionTypes.ZLIB;

        testRunner("test_BufferBackedByArray, BigBuffer: ", args, (s, c) ->{
            TransportBuffer buf = c._channel.getBuffer(10000, false, new ErrorImpl());
            assertTrue(buf.data().hasArray());
        });

        testRunner("test_BufferBackedByArray, buffer: ", args, (s, c) ->{
            TransportBuffer buf = c._channel.getBuffer(1000, false, new ErrorImpl());
            assertTrue(buf.data().hasArray());
        });

        args.compressionType = CompressionTypes.NONE;

        testRunner("test_BufferBackedByArray: ", args, (s, c) ->{
            TransportBuffer buf = c._channel.getBuffer(10000, false, new ErrorImpl());
            assertFalse(buf.data().hasArray());
        });
    }
}
