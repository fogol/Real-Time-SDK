/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import com.refinitiv.eta.JUnitConfigVariables;
import com.refinitiv.eta.RetryRule;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

import static com.refinitiv.eta.transport.TransportTestRunner.testRunner;

@RunWith(Parameterized.class)
public class TransportMessageEncryptedJunit {

    private final String serverSecurityProvider;
    private final String clientSecurityProvider;

    @Rule
    public RetryRule retryRule = new RetryRule(JUnitConfigVariables.TEST_RETRY_COUNT);

    @Rule
    public TestName testName = new TestName();

    @Before
    public void printTestName() {
        System.out.println(">>>>>>>>>>>>>>>>>>>>  " + testName.getMethodName() + " Test <<<<<<<<<<<<<<<<<<<<<<<");
    }

    @Parameterized.Parameters
    public static Object[][] data()
    {
        return new Object[][]{
                {"SunJSSE", "SunJSSE"},
                {"Conscrypt", "Conscrypt"},
                {"SunJSSE", "Conscrypt"},
                {"Conscrypt", "SunJSSE"}
        };
    }

    public TransportMessageEncryptedJunit(String serverSecurityProvider, String clientSecurityProvider)
    {
        this.serverSecurityProvider = serverSecurityProvider;
        this.clientSecurityProvider = clientSecurityProvider;
    }

    @Test
    public void test0_encrypted()
    {
        TransportTestRunner.TestArgs args = TransportTestRunner.TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = false;
        args.writeLocking = true;
        args.blocking = true;
        args.compressionType = Ripc.CompressionTypes.LZ4;
        args.compressionLevel = 6;

        int[] sizes = { 6140};
        args.messageSizes = sizes;
        args.printReceivedData = true;
        args.debug = true;
        args.messageContent = TransportTestRunner.MessageContentType.RANDOM;
        args.encrypted = true;
        args.clientSecurityProvider = clientSecurityProvider;
        args.serverSecurityProvider = serverSecurityProvider;

        testRunner("test0_encrypted: debugging", args);
    }

    @Test
    public void test1_encrypted()
    {
        TransportTestRunner.TestArgs args = TransportTestRunner.TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = Ripc.CompressionTypes.ZLIB;
        args.compressionLevel = 0;

        int[] sizes = { 6142 };
        args.messageSizes = sizes;
        args.printReceivedData = true;
        args.messageContent = TransportTestRunner.MessageContentType.UNIFORM;
        args.encrypted = true;
        args.clientSecurityProvider = clientSecurityProvider;
        args.serverSecurityProvider = serverSecurityProvider;

        testRunner("test1_encrypted: basic", args);
    }

    // No compression: message sizes from no-frag to fragmentation
    @Test
    public void test2_encrypted()
    {
        TransportTestRunner.TestArgs args = TransportTestRunner.TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.encrypted = true;
        args.clientSecurityProvider = clientSecurityProvider;
        args.serverSecurityProvider = serverSecurityProvider;

        args.compressionType = Ripc.CompressionTypes.NONE;
        args.compressionLevel = 0;

        int[] sizes = { 6140, 6141, 6142, 6143, 6144, 6145, 6146, 6147, 6148, 6149 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = TransportTestRunner.MessageContentType.UNIFORM;

        testRunner("test2_encrypted: no compression fragmentation boundary", args);
    }

    // lz4 compression growth: messages sizes from no-frag to fragmentation
    @Test
    public void test3_encrypted()
    {
        TransportTestRunner.TestArgs args = TransportTestRunner.TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = false;
        args.writeLocking = true;
        args.blocking = true;
        args.compressionType = Ripc.CompressionTypes.LZ4;
        args.compressionLevel = 6;
        args.encrypted = true;
        args.clientSecurityProvider = clientSecurityProvider;
        args.serverSecurityProvider = serverSecurityProvider;

        int[] sizes = {6100, 6101, 6102, 6103, 6104, 6105, 6106, 6107, 6108, 6109, 6110};
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = TransportTestRunner.MessageContentType.RANDOM;

        testRunner("test3_encrypted: lz4 fragmentation boundary test poor compression", args);
    }

    // zlib compression growth: message sizes from no-frag to fragmentation
    @Test
    public void test4_encrypted()
    {
        TransportTestRunner.TestArgs args = TransportTestRunner.TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = Ripc.CompressionTypes.ZLIB;
        args.compressionLevel = 6;
        args.encrypted = true;
        args.clientSecurityProvider = clientSecurityProvider;
        args.serverSecurityProvider = serverSecurityProvider;

        int[] sizes = { 6123, 6124, 6125, 6126, 6127, 6128, 6129, 6130, 6131, 6132, 6133, 6134, 6135, 6136, 6137, 6138, 6139, 6140 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = TransportTestRunner.MessageContentType.RANDOM;

        testRunner("test4_encrypted: zlib fragmentation boundary test poor compression", args);
    }

    // Alternate fragments with Random and Uniform data.
    // Starting with Random forces compression fragmentation on the first fragment
    @Test
    public void test5_encrypted()
    {
        TransportTestRunner.TestArgs args = TransportTestRunner.TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = Ripc.CompressionTypes.LZ4;
        args.compressionLevel = 6;
        args.encrypted = true;
        args.clientSecurityProvider = clientSecurityProvider;
        args.serverSecurityProvider = serverSecurityProvider;

        int[] sizes = { 50000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = TransportTestRunner.MessageContentType.MIXED_RANDOM_START;

        testRunner("test5_encrypted: mixed data random start", args);
    }

    @Test
    public void test5z_encrypted()
    {
        TransportTestRunner.TestArgs args = TransportTestRunner.TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = Ripc.CompressionTypes.ZLIB;
        args.compressionLevel = 6;
        args.encrypted = true;
        args.clientSecurityProvider = clientSecurityProvider;
        args.serverSecurityProvider = serverSecurityProvider;

        int[] sizes = { 50000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = TransportTestRunner.MessageContentType.MIXED_RANDOM_START;

        testRunner("test5_encrypted: mixed data random start", args);
    }

    // Alternate fragments with Random and Uniform data.
    // Starting with Uniform forces compression fragmentation on the second fragment
    @Test
    public void test6_encrypted()
    {
        TransportTestRunner.TestArgs args = TransportTestRunner.TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = Ripc.CompressionTypes.LZ4;
        args.compressionLevel = 6;
        args.encrypted = true;
        args.clientSecurityProvider = clientSecurityProvider;
        args.serverSecurityProvider = serverSecurityProvider;

        int[] sizes = { 50000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = TransportTestRunner.MessageContentType.MIXED_UNIFORM_START;

        testRunner("test6_encrypted: mised data uniform start", args);
    }

    @Test
    public void test6z_encrypted()
    {
        TransportTestRunner.TestArgs args = TransportTestRunner.TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = Ripc.CompressionTypes.ZLIB;
        args.compressionLevel = 6;
        args.encrypted = true;
        args.clientSecurityProvider = clientSecurityProvider;
        args.serverSecurityProvider = serverSecurityProvider;

        int[] sizes = { 50000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = TransportTestRunner.MessageContentType.MIXED_UNIFORM_START;

        testRunner("test6z_encrypted: mixed data uniform start", args);
    }

    // Alternate fragments with Random and Uniform data.
    // Starting with Uniform forces compression fragmentation on the second fragment
    @Test
    public void test7_encrypted()
    {
        TransportTestRunner.TestArgs args = TransportTestRunner.TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = Ripc.CompressionTypes.LZ4;
        args.compressionLevel = 6;
        args.encrypted = true;
        args.clientSecurityProvider = clientSecurityProvider;
        args.serverSecurityProvider = serverSecurityProvider;

        int[] sizes = { 500000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = TransportTestRunner.MessageContentType.MIXED_UNIFORM_START;

        testRunner("test7_encrypted: mixed data large message", args);
    }

    @Test
    public void test8_encrypted()
    {
        TransportTestRunner.TestArgs args = TransportTestRunner.TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = Ripc.CompressionTypes.NONE;
        args.compressionLevel = 0;
        args.encrypted = true;
        args.clientSecurityProvider = clientSecurityProvider;
        args.serverSecurityProvider = serverSecurityProvider;

        int[] sizes = { 7000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = TransportTestRunner.MessageContentType.RANDOM;

        // frag 1: 6147
        // frag 2:  869
        args.expectedTotalBytes = 7016;

        testRunner("test8_encrypted: ", args);
    }

    // fragment + compressed frag testing writeArgs bytes written
    @Test
    public void test8lz4_encrypted()
    {
        TransportTestRunner.TestArgs args = TransportTestRunner.TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = Ripc.CompressionTypes.LZ4;
        args.compressionLevel = 0;
        args.encrypted = true;
        args.clientSecurityProvider = clientSecurityProvider;
        args.serverSecurityProvider = serverSecurityProvider;

        int[] sizes = { 7000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = TransportTestRunner.MessageContentType.RANDOM;

        // frag 1: 6147
        // frag 1b: 29
        // frag 2: 874
        args.expectedTotalBytes = 7050;

        // frag 1: 10
        // frag 1b: 3
        // frag 2: 6
        // data: 7000
        args.expectedUncompressedBytes = 7019;

        testRunner("test8lz4_encrypted: fragment with compFragment testing write args bytes written", args);
    }

    // fragment + compressed frag testing writeArgs bytes written
    @Test
    public void test8z_encrypted()
    {
        TransportTestRunner.TestArgs args = TransportTestRunner.TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = Ripc.CompressionTypes.ZLIB;
        args.compressionLevel = 0;
        args.debug = true;
        args.encrypted = true;
        args.clientSecurityProvider = clientSecurityProvider;
        args.serverSecurityProvider = serverSecurityProvider;

        int[] sizes = { 7000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = TransportTestRunner.MessageContentType.RANDOM;

        // frag 1: 6147
        // frag 1b: 15
        // frag 2: 879
        args.expectedTotalBytes = 7041;

        // frag 1: 10
        // frag 1b: 3
        // frag 2: 6
        // data: 7000
        args.expectedUncompressedBytes = 7019;

        testRunner("test8z_encrypted: zlib fragment with compFragment testing write args bytes written", args);
    }

    // compressed frag normal: testing writeArgs bytes written
    @Test
    public void test9lz4_encrypted()
    {
        TransportTestRunner.TestArgs args = TransportTestRunner.TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = Ripc.CompressionTypes.LZ4;
        args.compressionLevel = 0;
        args.clientSecurityProvider = clientSecurityProvider;
        args.serverSecurityProvider = serverSecurityProvider;

        int[] sizes = { 6140 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = TransportTestRunner.MessageContentType.RANDOM;
        args.encrypted = true;

        // 1: 6147
        // 1b: 25
        args.expectedTotalBytes = 6172;

        // 1: 3
        // 1b: 3
        // data: 6140
        args.expectedUncompressedBytes = 6146;

        testRunner("test9lz4_encrypted: compFragment normal testing write args bytes written", args);
    }

    @Test
    public void test10_encrypted()
    {
        TransportTestRunner.TestArgs args = TransportTestRunner.TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = Ripc.CompressionTypes.NONE;
        args.compressionLevel = 0;
        args.encrypted = true;
        args.clientSecurityProvider = clientSecurityProvider;
        args.serverSecurityProvider = serverSecurityProvider;

        int[] sizes = { 12300 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = TransportTestRunner.MessageContentType.RANDOM;

        testRunner("test10_encrypted: ", args);
    }

    // Designed so that the last fragment size is below the compression threshold
    @Test
    public void test10lz4_encrypted()
    {
        TransportTestRunner.TestArgs args = TransportTestRunner.TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = Ripc.CompressionTypes.LZ4;
        args.compressionLevel = 0;
        args.encrypted = true;
        args.clientSecurityProvider = clientSecurityProvider;
        args.serverSecurityProvider = serverSecurityProvider;

        int[] sizes = { 12300 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = TransportTestRunner.MessageContentType.RANDOM;

        testRunner("test10lz4_encrypted: last fragment not compressed", args);
    }

    // Designed so that the last fragment size is below the compression threshold
    @Test
    public void test10z_encrypted()
    {
        TransportTestRunner.TestArgs args = TransportTestRunner.TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = Ripc.CompressionTypes.ZLIB;
        args.compressionLevel = 0;
        args.encrypted = true;
        args.clientSecurityProvider = clientSecurityProvider;
        args.serverSecurityProvider = serverSecurityProvider;

        int[] sizes = { 12300 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = TransportTestRunner.MessageContentType.RANDOM;

        testRunner("test10z_encrypted: last fragment not compressed", args);
    }

    @Test
    public void test11z_encrypted()
    {
        TransportTestRunner.TestArgs args = TransportTestRunner.TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = Ripc.CompressionTypes.ZLIB;
        args.compressionLevel = 0;
        args.encrypted = true;
        args.clientSecurityProvider = clientSecurityProvider;
        args.serverSecurityProvider = serverSecurityProvider;

        int[] sizes = { 6128, 6129 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = TransportTestRunner.MessageContentType.RANDOM;

        testRunner("test11z_encrypted: ", args);
    }

}
