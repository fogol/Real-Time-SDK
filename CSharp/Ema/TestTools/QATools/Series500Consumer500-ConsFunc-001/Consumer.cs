/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
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
            {
                Console.WriteLine(" blank");
            }
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
    private const string DEFAULT_CONSUMER_NAME = "Consumer_9";
    private static string serviceName1 = "DIRECT_FEED";
    private static string serviceName2 = "ELEKTRON_DD";
    private static string itemName1 = "TRI.N";
    private static string itemName2 = "IBM.N";

    static void PrintHelp()
    {
        Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n"
                + "  -itemName1 Request item name (optional).\n"
                + "  -serviceName1 Service name (optional).\n"
                + "  -itemName2 Request item name (optional).\n"
                + "  -serviceName2 Service name (optional).\n"
                + "\n");
    }

    static bool ReadCommandLineArgs(string[] args)
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
                else if ("-itemName1".Equals(args[argsCount]))
                {
                    if (argsCount < (args.Length - 1))
                    {
                        itemName1 = args[++argsCount];
                    }
                    ++argsCount;
                }
				 else if ("-serviceName1".Equals(args[argsCount]))
                {
                    if (argsCount < (args.Length - 1))
                    {
                        serviceName1 = args[++argsCount];
                    }
                    ++argsCount;
                }
                else if ("-itemName2".Equals(args[argsCount]))
                {
                    if (argsCount < (args.Length - 1))
                    {
                        itemName2 = args[++argsCount];
                    }
                    ++argsCount;
                }
				 else if ("-serviceName2".Equals(args[argsCount]))
                {
                    if (argsCount < (args.Length - 1))
                    {
                        serviceName2 = args[++argsCount];
                    }
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
            AppClient appClient1 = new AppClient();
            AppClient appClient2 = new AppClient();

            if (!ReadCommandLineArgs(args)) return;

            consumer = new OmmConsumer(new OmmConsumerConfig()
                .ConsumerName(DEFAULT_CONSUMER_NAME));

            consumer.RegisterClient(new RequestMsg()
                .ServiceName(serviceName1)
                .Name(itemName1), appClient1, 0);
             consumer.RegisterClient(new RequestMsg()
                .ServiceName(serviceName2)
                .Name(itemName2), appClient2, 0);

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
