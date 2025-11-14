/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Threading;

using LSEG.Ema.Access;
using static LSEG.Ema.Access.DataType;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
    private bool updateCalled = false;

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent evt)
    {
        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

        Console.WriteLine("Item State: " + refreshMsg.State());

        if (DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
        {
            Decode(refreshMsg.Payload().FieldList());
        }

        Console.WriteLine("\nEvent channel info (refresh)\n" + evt.ChannelInformation());
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent evt)
    {
        if (!updateCalled)
        {
            updateCalled = true;
            Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
            Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));

            if (DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
            {
                Decode(updateMsg.Payload().FieldList());
            }

            Console.WriteLine("\nEvent channel info (update)\n" + evt.ChannelInformation());
        }
        else
        {
            Console.WriteLine("skipped printing updateMsg");
        }
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent evt)
    {
        Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

        if (statusMsg.HasState)
        {
            Console.WriteLine("Item State: " + statusMsg.State());
        }

        Console.WriteLine("\nEvent channel info (status)\n" + evt.ChannelInformation());
    }

    void Decode(FieldList fieldList)
    {
        foreach (FieldEntry fieldEntry in fieldList)
        {
            Console.Write($"Fid: {fieldEntry.FieldId} Name = {fieldEntry.Name} DataType: {DataType.AsString(fieldEntry.Load!.DataType)} Value: ");

            if (Data.DataCode.BLANK == fieldEntry.Code)
                Console.WriteLine(" blank");
            else
                switch (fieldEntry.LoadType)
                {
                    case DataTypes.REAL:
                        Console.WriteLine(fieldEntry.OmmRealValue());
                        break;
                    case DataTypes.DATE:
                        Console.WriteLine(fieldEntry.OmmDateValue().Day + " / " + fieldEntry.OmmDateValue().Month + " / " + fieldEntry.OmmDateValue().Year);
                        break;
                    case DataTypes.TIME:
                        Console.WriteLine(fieldEntry.OmmTimeValue().Hour + ":" + fieldEntry.OmmTimeValue().Minute + ":" + fieldEntry.OmmTimeValue().Second + ":" + fieldEntry.OmmTimeValue().Millisecond);
                        break;
                    case DataTypes.DATETIME:
                        Console.WriteLine(fieldEntry.OmmDateTimeValue().Day + " / " + fieldEntry.OmmDateTimeValue().Month + " / " +
                                fieldEntry.OmmDateTimeValue().Year + "." + fieldEntry.OmmDateTimeValue().Hour + ":" +
                                fieldEntry.OmmDateTimeValue().Minute + ":" + fieldEntry.OmmDateTimeValue().Second + ":" +
                                fieldEntry.OmmDateTimeValue().Millisecond + ":" + fieldEntry.OmmDateTimeValue().Microsecond + ":" +
                                fieldEntry.OmmDateTimeValue().Nanosecond);
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
    // PORTS, HOSTS and CHANNEL_NAMES should have the same size
    // List of ports for channels
    private static readonly string[] PORTS = {
        "15001", "15002", "15003",
        "15004", "15005", "15006",
        "15007", "15008", "15009"
    };

    // List of hosts for channels
    private static readonly string[] HOSTS = {
        "host1", "host2", "host3",
        "host4", "host5", "host6",
        "host7", "host8", "host9"
    };

    // List of channel names
    private static readonly string[] CHANNEL_NAMES = {
        "Channel_A", "Channel_B", "Channel_C",
        "Channel_D", "Channel_E", "Channel_F",
        "Channel_G", "Channel_H", "Channel_I"
    };

    private const string DEFAULT_SERVICE_NAME = "DIRECT_FEED";
    private const string DEFAULT_ITEM_NAME = "IBM.N";
    private const string CS_CONSUMER_NAME = "Consumer_A";

    private const bool DEFAULT_ENABLE_PREFERRED_HOST_OPTIONS = true;
    private const string DEFAULT_DETECTION_TIME_SCHEDULE = "";
    private const int DEFAULT_DETECTION_TIME_INTERVAL = 15;
	// Default channel name is a last name from CHANNEL_NAMES
    private static readonly string DEFAULT_CHANNEL_NAME = CHANNEL_NAMES[^1];

    private static Map CreateProgrammaticConfig(PreferredHostOptions options)
    {
        // Channel set contains last three channels from CHANNEL_NAMES
        string channelSet = string.Join(',', CHANNEL_NAMES[^3], CHANNEL_NAMES[^2], CHANNEL_NAMES[^1]);

        Map innerMap = new Map();
        Map configMap = new Map();
        ElementList elementList = new ElementList();
        ElementList innerElementList = new ElementList();

        elementList.AddAscii("DefaultConsumer", "Consumer_A");

        // ConsumerGroup
        // CS_Consumer
        innerElementList.AddAscii("ChannelSet", channelSet);
        innerElementList.AddAscii("PreferredChannelName", options.ChannelName);
        innerElementList.AddAscii("Dictionary", "Dictionary_1");
        if (!string.IsNullOrEmpty(options.DetectionTimeSchedule))
        {
            innerElementList.AddAscii("PHDetectionTimeSchedule", options.DetectionTimeSchedule);
        }
        innerElementList.AddUInt("EnablePreferredHostOptions", options.IsPreferredHostEnabled ? (uint)1 : (uint)0);
        innerElementList.AddUInt("PHDetectionTimeInterval", (uint)options.DetectionTimeInterval);
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
        innerElementList.AddInt("ReconnectAttemptLimit", 10);
        innerElementList.AddInt("ReconnectMinDelay", 2000);
        innerElementList.AddInt("ReconnectMaxDelay", 6000);
        innerElementList.AddUInt("XmlTraceToStdout", 0);
        innerElementList.AddUInt("XmlTraceToFile", 0);
        innerElementList.AddUInt("XmlTraceWrite", 0);
        innerElementList.AddUInt("XmlTraceRead", 0);
        innerElementList.AddUInt("XmlTracePing", 0);
        innerElementList.AddUInt("MsgKeyInUpdates", 1);

        innerMap.AddKeyAscii(CS_CONSUMER_NAME, MapAction.ADD, innerElementList.Complete());
        innerElementList.Clear();

        elementList.AddMap("ConsumerList", innerMap.Complete());
        innerMap.Clear();

        configMap.AddKeyAscii("ConsumerGroup", MapAction.ADD, elementList.Complete());
        elementList.Clear();

        // ChannelGroup
        // Channels
        for (int i = 0; i < PORTS.Length; i++)
        {
            innerElementList.AddEnum("ChannelType", EmaConfig.ConnectionTypeEnum.SOCKET);
            innerElementList.AddEnum("CompressionType", EmaConfig.CompressionTypeEnum.ZLIB);
            innerElementList.AddUInt("GuaranteedOutputBuffers", 5000);
            innerElementList.AddUInt("ConnectionPingTimeout", 50_000);
            innerElementList.AddUInt("InitializationTimeout", 30_000);
            innerElementList.AddAscii("Host", HOSTS[i]);
            innerElementList.AddAscii("Port", PORTS[i]);
            innerElementList.AddUInt("TcpNodelay", 0);

            innerMap.AddKeyAscii(CHANNEL_NAMES[i], MapAction.ADD, innerElementList.Complete());
            innerElementList.Clear();
        }

        elementList.AddMap("ChannelList", innerMap.Complete());
        innerMap.Clear();

        configMap.AddKeyAscii("ChannelGroup", MapAction.ADD, elementList.Complete());
        elementList.Clear();

        // DictionaryGroup
        // Dictionary_1
        innerElementList.AddEnum("DictionaryType", EmaConfig.DictionaryTypeEnum.FILE);
        innerElementList.AddAscii("RdmFieldDictionaryFileName", "RDMFieldDictionary");
        innerElementList.AddAscii("EnumTypeDefFileName", "enumtype.def");
        innerMap.AddKeyAscii("Dictionary_1", MapAction.ADD, innerElementList.Complete());
        innerElementList.Clear();

        elementList.AddMap("DictionaryList", innerMap.Complete());
        innerMap.Clear();

        configMap.AddKeyAscii("DictionaryGroup", MapAction.ADD, elementList.Complete());
        elementList.Clear();

        return configMap.Complete();
    }

    static void PrintHelp()
    {
        Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n"
            + "-enablePH  Enables preferred host feature\n"
            + "-detectionTimeSchedule  Specifies Cron time format for detection time schedule\n"
            + "-detectionTimeInterval  Specifies detection time interval in seconds.\n"
            + "                        0 indicates that the detection time interval is disabled\n"
            + "-channelNamePreferred  Specifies a channel name in the Channel or ChannelSet element.\n"
            + "                       Empty string indicates the first channel name in the ChannelSet is used\n"
            + "\n");
    }

    static bool ReadCommandLineArgs(string[] args, PreferredHostOptions options)
    {
        try
        {
            int argsCount = 0;

            while (argsCount < args.Length)
            {
                if ("-?".Equals(args[argsCount]))
                {
                    PrintHelp();
                    return false;
                }
                else if ("-enablePH".Equals(args[argsCount]))
                {
                    if (argsCount < (args.Length - 1))
                    {
                        string arg = args[++argsCount];
                        if ("true".Equals(arg))
                            options.IsPreferredHostEnabled = true;
                        else if ("false".Equals(arg))
                            options.IsPreferredHostEnabled = false;
                        else
                        {
                            Console.WriteLine("Unknown -enablePH value: " + arg);
                            PrintHelp();
                            return false;
                        }
                    }
                    ++argsCount;
                }
                else if ("-detectionTimeSchedule".Equals(args[argsCount]))
                {
                    options.DetectionTimeSchedule = argsCount < (args.Length - 1) ? args[++argsCount] : string.Empty;
                    ++argsCount;
                }
                else if ("-detectionTimeInterval".Equals(args[argsCount]))
                {
                    if (argsCount < (args.Length - 1))
                    {
                        string arg = args[++argsCount];
                        if (int.TryParse(arg, out var interval))
                        {
                            options.DetectionTimeInterval = interval;
                        }
                        else
                        {
                            Console.WriteLine("Could not parse -detectionTimeInterval value: " + arg);
                            PrintHelp();
                            return false;
                        }
                    }
                    ++argsCount;
                }
                else if ("-channelNamePreferred".Equals(args[argsCount]))
                {
                    options.ChannelName = argsCount < (args.Length - 1) ? args[++argsCount] : string.Empty;
                    ++argsCount;
                }
                else // unrecognized command line argument
                {
                    Console.WriteLine("Unknown argument: " + args[argsCount]);
                    PrintHelp();
                    return false;
                }
            }
        }
        catch
        {
            PrintHelp();
            return false;
        }

        return true;
    }

    public static void Main(string[] args)
    {
        OmmConsumer? consumer = null;
        try
        {
            PreferredHostOptions options = new PreferredHostOptions()
            {
                IsPreferredHostEnabled = DEFAULT_ENABLE_PREFERRED_HOST_OPTIONS,
                DetectionTimeSchedule = DEFAULT_DETECTION_TIME_SCHEDULE,
                DetectionTimeInterval = DEFAULT_DETECTION_TIME_INTERVAL,
                ChannelName = DEFAULT_CHANNEL_NAME
            };

            if (!ReadCommandLineArgs(args, options))
                return;

            AppClient appClient = new AppClient();

            Map progConfig = CreateProgrammaticConfig(options);
            consumer = new OmmConsumer(new OmmConsumerConfig()
                .Config(progConfig)
                .ConsumerName(CS_CONSUMER_NAME));

            consumer.RegisterClient(new RequestMsg()
                .ServiceName(DEFAULT_SERVICE_NAME)
                .Name(DEFAULT_ITEM_NAME), appClient);

            int printInterval = 1;
            ChannelInformation ci = new ChannelInformation();
            for (int i = 0; i < 600; i++)
            {
                Thread.Sleep(TimeSpan.FromSeconds(1)); // API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()

                if ((i % printInterval == 0))
                {
                    consumer.ChannelInformation(ci);
                    Console.WriteLine("\nChannel information (consumer):\n\t" + ci);
                    Console.WriteLine();
                }
            }
        }
        catch (ThreadInterruptedException ex)
        {
            Console.WriteLine(ex.Message);
        }
        catch (OmmException ex)
        {
            Console.WriteLine(ex.Message);
        }
        finally
        {
            consumer?.Uninitialize();
        }
    }
}