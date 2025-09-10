/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Domain.Login;
using LSEG.Ema.Rdm;
using System;
using System.Collections.Generic;
using static LSEG.Ema.Access.DataType;
using static LSEG.Ema.Access.OmmConsumerConfig;
using static System.Runtime.InteropServices.JavaScript.JSType;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent _)
    {
        if (refreshMsg.HasMsgKey)
            Console.WriteLine("Item Name: " + refreshMsg.Name() + " Service Name: " + refreshMsg.ServiceName());

        Console.WriteLine("Item State: " + refreshMsg.State());

        if (DataType.DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
            Decode(refreshMsg.Payload().FieldList());

        Console.WriteLine();
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent _)
    {
        if (updateMsg.HasMsgKey)
            Console.WriteLine("Item Name: " + updateMsg.Name() + " Service Name: " + updateMsg.ServiceName());

        if (DataType.DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
            Decode(updateMsg.Payload().FieldList());

        Console.WriteLine();
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent _)
    {
        if (statusMsg.HasMsgKey)
            Console.WriteLine("Item Name: " + statusMsg.Name() + " Service Name: " + statusMsg.ServiceName());

        if (statusMsg.HasState)
            Console.WriteLine("Item State: " + statusMsg.State());

        Console.WriteLine();
    }

    private void Decode(FieldList fieldList)
    {
        IEnumerator<FieldEntry> iter = fieldList.GetEnumerator();
        FieldEntry fieldEntry;
        while (iter.MoveNext())
        {
            fieldEntry = iter.Current;
            Console.Write("Fid: " + fieldEntry.FieldId + " Name = " + fieldEntry.Name + " DataType: " + DataType.AsString(fieldEntry.Load!.DataType) + " Value: ");

            if (Data.DataCode.BLANK == fieldEntry.Code)
                Console.WriteLine(" blank");
            else
                switch (fieldEntry.LoadType)
                {
                    case DataTypes.REAL:
                        Console.WriteLine(fieldEntry.OmmRealValue().AsDouble());
                        break;

                    case DataTypes.DATE:
                        Console.WriteLine(fieldEntry.OmmDateValue().Day + " / " + fieldEntry.OmmDateValue().Month + " / " + fieldEntry.OmmDateValue().Year);
                        break;

                    case DataTypes.TIME:
                        Console.WriteLine(fieldEntry.OmmTimeValue().Hour + ":" + fieldEntry.OmmTimeValue().Minute + ":" + fieldEntry.OmmTimeValue().Second + ":" + fieldEntry.OmmTimeValue().Millisecond);
                        break;

                    case DataTypes.INT:
                        Console.WriteLine(fieldEntry.IntValue());
                        break;

                    case DataTypes.UINT:
                        Console.WriteLine(fieldEntry.UIntValue());
                        break;

                    case DataTypes.ASCII:
                        Console.WriteLine(fieldEntry.OmmAsciiValue());
                        break;

                    case DataTypes.ENUM:
                        Console.WriteLine(fieldEntry.HasEnumDisplay ? fieldEntry.EnumDisplay() : fieldEntry.EnumValue());
                        break;

                    case DataTypes.RMTES:
                        Console.WriteLine(fieldEntry.OmmRmtesValue());
                        break;

                    case DataTypes.ERROR:
                        Console.WriteLine("(" + fieldEntry.OmmErrorValue().ErrorCodeAsString() + ")");
                        break;

                    default:
                        Console.WriteLine();
                        break;
                }
        }
    }
}

public class Consumer
{
    static void PrintHelp()
    {

        Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n"
                + "  -putf UpdateTypeFilter in programmatic config.\n"
                + "  -pnutf NegativeUpdateTypeFilter in programmatic config.\n"
                + "  -futf UpdateTypeFilter via functional call.\n"
                + "  -fnutf NegativeUpdateTypeFilter via functional call.\n"
                + "  -clmutf UpdateTypeFilter via custom Login message.\n"
                + "  -clmnutf NegativeUpdateTypeFilter via custom Login message.\n"
                + "  -utfFuncClm UpdateTypeFilter setting via functional call will precede setting the message.\n"
                + "  -nutfFuncClm NegativeUpdateTypeFilter setting via functional call will precede setting the message.\n"
                + "  -utfClmFunc UpdateTypeFilter setting via functional call will take place after setting the custom Login message.\n"
                + "  -nutfClmFunc NegativeUpdateTypeFilter setting via functional call will take place after setting the custom Login message.\n"
                + "  -setCLM if present, custom LoginRequest message will be set via config interface, otherwise it won't be set."
                + "  -service set service name. Default is DIRECT_FEED. \n"
                + "  -item set item name. Default is IBM.N. \n"
                + "  -progConf enable programmatic config. Default is disable. \n"
                + "  -progHost set host for programmatic config. Default is localhost. \n"
                + "  -progPort set port for programmatic config. Default is 14002. \n"
                + "\n");
        Console.WriteLine("For instance, the following arguments \"-putf 1 -pnutf 2 -futf 4 -fnutf 8 -clmutf 16 -clmnutf 32 -utfFuncClm -nutfClmFunc\"" +
                " indicate that UpdateTypeFilter set via programmatic config equals 1, \n" +
                " NegativeUpdateTypeFilter set via programmatic config equals 2, \n" +
                " UpdateTypeFilter set via functional call equals 4, \n" +
                " NegativeUpdateTypeFilter set via functional call equals 8, \n" +
                " UpdateTypeFilter set via custom login message equals 16, \n" +
                " NegativeUpdateTypeFilter set via custom login message equals 32, \n" +
                " setting UpdateTypeFilter via functional call will precede setting custom login message with this value set, " +
                " and setting custom login message with NegativeUpdateTypeFilter will precede setting it via function call.");
    }

    static Map CreateProgramaticConfig(long updateTypeFilter,
        long negativeUpdateTypeFilter,
        string progHost,
        string progPort)
    {
        Map innerMap = new Map();
        Map configMap = new Map();
        ElementList elementList = new ElementList();
        ElementList innerElementList = new ElementList();

        elementList.AddAscii("DefaultConsumer", "Consumer_UTF");

        innerElementList.AddAscii("Channel", "Channel_1");
        innerElementList.AddAscii("Dictionary", "Dictionary_2");
        innerElementList.AddUInt("ItemCountHint", 5000);
        innerElementList.AddUInt("ServiceCountHint", 5000);
        innerElementList.AddUInt("ObeyOpenWindow", 0);
        innerElementList.AddUInt("PostAckTimeout", 5000);
        innerElementList.AddUInt("RequestTimeout", 5000);
        innerElementList.AddUInt("MaxOutstandingPosts", 5000);
        innerElementList.AddInt("DispatchTimeoutApiThread", 1);
        innerElementList.AddUInt("HandleException", 0);
        innerElementList.AddUInt("MaxDispatchCountApiThread", 500);
        innerElementList.AddUInt("MaxDispatchCountUserThread", 500);
        innerElementList.AddInt("PipePort", 4001);
        innerElementList.AddInt("ReconnectAttemptLimit", 10);
        innerElementList.AddInt("ReconnectMinDelay", 2000);
        innerElementList.AddInt("ReconnectMaxDelay", 6000);
        innerElementList.AddUInt("XmlTraceToStdout", 0);
        innerElementList.AddUInt("MsgKeyInUpdates", 1);

        if (updateTypeFilter > 0)
        {
            innerElementList.AddUInt("UpdateTypeFilter", (ulong)updateTypeFilter);
        }

        if (negativeUpdateTypeFilter > 0)
        {
            innerElementList.AddUInt("NegativeUpdateTypeFilter", (ulong)negativeUpdateTypeFilter);
        }
        innerElementList.Complete();

        innerMap.AddKeyAscii("Consumer_UTF", MapAction.ADD, innerElementList).Complete();
        innerElementList.Clear();

        elementList.AddMap("ConsumerList", innerMap).Complete();
        innerMap.Clear();

        configMap.AddKeyAscii("ConsumerGroup", MapAction.ADD, elementList);
        elementList.Clear();

        innerElementList.AddAscii("ChannelType", "ChannelType::RSSL_SOCKET");
        innerElementList.AddAscii("CompressionType", "CompressionType::ZLib");
        innerElementList.AddInt("GuaranteedOutputBuffers", 5000);
        innerElementList.AddInt("ConnectionPingTimeout", 50000);
        innerElementList.AddAscii("Host", progHost);
        innerElementList.AddAscii("Port", progPort);
        innerElementList.AddInt("TcpNodelay", 0);
        innerElementList.Complete();

        innerMap.AddKeyAscii("Channel_1", MapAction.ADD, innerElementList);
        innerElementList.Clear();
        innerMap.Complete();

        elementList.AddMap("ChannelList", innerMap);
        innerMap.Clear();
        elementList.Complete();

        configMap.AddKeyAscii("ChannelGroup", MapAction.ADD, elementList);
        elementList.Clear();

        return configMap;
    }

    public static void Main(string[] args)
    {
        OmmConsumer consumer = null;
        try
        {
            AppClient appClient = new AppClient();

            LoginReq loginReq = new LoginReq();
            RequestMsg reqMsg = new RequestMsg();
            long pUpdateTypeFilter = 0;
            long pNegativeUpdateTypeFilter = 0;
            long fUpdateTypeFilter = 0;
            long fNegativeUpdateTypeFilter = 0;
            long clmUpdateTypeFilter = 0;
            long clmNegativeUpdateTypeFilter = 0;
            string progHost = "localhost";
            string progPort = "14002";
            string serviceName = "DIRECT_FEED";
            string itemName = "IBM.N";
            bool programmaticConfig = false;
            bool utfFuncClm = true;
            bool nutfFuncClm = true;
            bool setCLM = false;

            OmmConsumerConfig config = new OmmConsumerConfig("EmaConfig.xml").ConsumerName("Consumer_UTF");

            try
            {
                int argsCount = 0;

                while (argsCount < args.Length)
                {
                    if (0 == args[argsCount].CompareTo("-?"))
                    {
                        PrintHelp();
                    }
                    else if ("-putf".Equals(args[argsCount]))
                    {
                        pUpdateTypeFilter = int.Parse(args[++argsCount]);
                        ++argsCount;
                    }
                    else if ("-pnutf".Equals(args[argsCount]))
                    {
                        pNegativeUpdateTypeFilter = int.Parse(args[++argsCount]);
                        ++argsCount;
                    }
                    else if ("-futf".Equals(args[argsCount]))
                    {
                        fUpdateTypeFilter = int.Parse(args[++argsCount]);
                        ++argsCount;
                    }
                    else if ("-fnutf".Equals(args[argsCount]))
                    {
                        fNegativeUpdateTypeFilter = int.Parse(args[++argsCount]);
                        ++argsCount;
                    }
                    else if ("-clmutf".Equals(args[argsCount]))
                    {
                        clmUpdateTypeFilter = int.Parse(args[++argsCount]);
                        ++argsCount;
                    }
                    else if ("-clmnutf".Equals(args[argsCount]))
                    {
                        clmNegativeUpdateTypeFilter = int.Parse(args[++argsCount]);
                        ++argsCount;
                    }
                    else if ("-utfFuncClm".Equals(args[argsCount]))
                    {
                        utfFuncClm = true;
                        ++argsCount;
                    }
                    else if ("-nutfFuncClm".Equals(args[argsCount]))
                    {
                        nutfFuncClm = true;
                        ++argsCount;
                    }
                    else if ("-utfClmFunc".Equals(args[argsCount]))
                    {
                        utfFuncClm = false;
                        ++argsCount;
                    }
                    else if ("-nutfClmFunc".Equals(args[argsCount]))
                    {
                        nutfFuncClm = false;
                        ++argsCount;
                    }
                    else if ("-setCLM".Equals(args[argsCount]))
                    {
                        setCLM = true;
                        ++argsCount;
                    }
                    else if ("-service".Equals(args[argsCount]))
                    {
                        serviceName = args[++argsCount];
                        ++argsCount;
                    }
                    else if ("-item".Equals(args[argsCount]))
                    {
                        itemName = args[++argsCount];
                        ++argsCount;
                    }
                    else if ("-progHost".Equals(args[argsCount]))
                    {
                        progHost = args[++argsCount];
                        ++argsCount;
                    }
                    else if ("-progPort".Equals(args[argsCount]))
                    {
                        progPort = args[++argsCount];
                        ++argsCount;
                    }
                    else if ("-progConf".Equals(args[argsCount]))
                    {
                        programmaticConfig = true;
                        ++argsCount;
                    }
                    else // unrecognized command line argument
                    {
                        Console.WriteLine("\nPrinting help: unknown argument " + args[argsCount]);
                        PrintHelp();
                        ++argsCount;
                    }
                }
            }
            catch (Exception e)
            {
                PrintHelp();
            }

            loginReq.Name("user")
                    .NameType(EmaRdm.USER_NAME)
                    .ApplicationId("127")
                    .Position("127.0.0.1/net")
                    .AllowSuspectData(true);

            if (clmUpdateTypeFilter > 0) loginReq.UpdateTypeFilter((ulong)clmUpdateTypeFilter);
            if (clmNegativeUpdateTypeFilter > 0) loginReq.NegativeUpdateTypeFilter((ulong)clmNegativeUpdateTypeFilter);

            if (programmaticConfig)
            {
                config.Config(CreateProgramaticConfig(pUpdateTypeFilter,
                    pNegativeUpdateTypeFilter,
                    progHost,
                    progPort));
            }
            config.OperationModel(OperationModelMode.USER_DISPATCH);

            if (fUpdateTypeFilter > 0 && utfFuncClm) config.UpdateTypeFilter((ulong)fUpdateTypeFilter);
            if (fNegativeUpdateTypeFilter > 0 && nutfFuncClm) config.NegativeUpdateTypeFilter((ulong)fNegativeUpdateTypeFilter);

            if (setCLM) config.AddAdminMsg(loginReq.Message());

            if (fUpdateTypeFilter > 0 && !utfFuncClm) config.UpdateTypeFilter((ulong)fUpdateTypeFilter);
            if (fNegativeUpdateTypeFilter > 0 && !nutfFuncClm) config.NegativeUpdateTypeFilter((ulong)fNegativeUpdateTypeFilter);

            consumer = new OmmConsumer(config);

            consumer.RegisterClient(reqMsg.Clear().ServiceName(serviceName).Name("IBM.N"), appClient, null);

            long startTime = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds();
            while (startTime + 60000 > DateTimeOffset.UtcNow.ToUnixTimeMilliseconds())
                consumer.Dispatch(10);      // calls to onRefreshMsg(), onUpdateMsg(), or onStatusMsg() execute on this thread
        }
        catch (OmmException excp)
        {
            Console.WriteLine(excp);
        }
        finally
        {
            if (consumer != null) consumer.Uninitialize();
        }
    }
}