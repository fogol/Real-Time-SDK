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
    private const string DEFAULT_SERVICE_NAME = "DIRECT_FEED";
    private const string INFRA_SERVICE_NAME = "ELEKTRON_DD";
    private const string DEFAULT_CONSUMER_NAME = "Consumer_9";
    private const string DEFAULT_ITEM_NAME = "IBM.N";

    private const bool DEFAULT_ENABLE_PREFERRED_HOST_OPTIONS = false;
    private const string DEFAULT_DETECTION_TIME_SCHEDULE = "";
    private const int DEFAULT_DETECTION_TIME_INTERVAL = 0;
    private const string DEFAULT_CHANNEL_NAME = "";

    /// <summary>
    /// Encapsulates command-line parameters for this application.
    /// </summary>
    /// <remarks>
    /// Parameters that were not defined at the command line are left uninitialized (<c>null</c>).
    /// </remarks>
    class CommandConfig
    {
        public TimeSpan? IOCtlInterval;
        public TimeSpan? FallbackInterval;
        public bool? EnablePH;
        public string? DetectionTimeSchedule;
        public int? DetectionTimeInterval;
        public string? ChannelName;
    }

    static void PrintHelp()
    {
        Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n"
            + "-ioctlInterval  Specifies sleep time in application before ModifyIOCtl is invoked.\n"
            + "                O indicates that function won't be invoked\n"
            + "-fallBackInterval  Specifies sleep time in application before call Ad Hoc Fallback\n"
            + "                   Function is invoked. O indicates that function won't be invoked"
            + "-ioctlEnablePH  Enables preferred host feature\n"
            + "-ioctlDetectionTimeSchedule  Specifies Cron time format for detection time schedule"
            + "-ioctlDetectionTimeInterval  Specifies detection time interval in seconds.\n"
            + "                             0 indicates that the detection time interval is disabled\n"
            + "-ioctlChannelName  Specifies a channel name in the Channel or ChannelSet element.\n"
            + "                   Empty string indicates the first channel name in the ChannelSet is used\n"
            + "\n");
    }

    static CommandConfig? ReadCommandLineArgs(string[] args)
    {
        CommandConfig config = new();
        try
        {
            int argsCount = 0;

            while (argsCount < args.Length)
            {
                if ("-?".Equals(args[argsCount]))
                {
                    PrintHelp();
                    return null;
                }
                else if ("-ioctlInterval".Equals(args[argsCount]))
                {
                    string arg = args[++argsCount];
                    if (int.TryParse(arg, out var interval))
                    {
                        config.IOCtlInterval = TimeSpan.FromSeconds(interval);
                    }
                    else
                    {
                        Console.WriteLine("Could not parse -ioctlInterval value: " + arg);
                        PrintHelp();
                        return null;
                    }
                    ++argsCount;
                }
                else if ("-fallBackInterval".Equals(args[argsCount]))
                {
                    string arg = args[++argsCount];
                    if (int.TryParse(arg, out var interval))
                    {
                        config.FallbackInterval = TimeSpan.FromSeconds(interval);
                    }
                    else
                    {
                        Console.WriteLine("Could not parse -fallBackInterval value: " + arg);
                        PrintHelp();
                        return null;
                    }
                    ++argsCount;
                }
                else if ("-ioctlEnablePH".Equals(args[argsCount]))
                {
                    if (argsCount < (args.Length - 1))
                    {
                        string arg = args[++argsCount];
                        if ("true".Equals(arg))
                            config.EnablePH = true;
                        else if ("false".Equals(arg))
                            config.EnablePH = false;
                        else
                        {
                            Console.WriteLine("Unknown -ioctlEnablePH value: " + arg);
                            PrintHelp();
                            return null;
                        }
                    }
                    ++argsCount;
                }
                else if ("-ioctlDetectionTimeSchedule".Equals(args[argsCount]))
                {
                    config.DetectionTimeSchedule = argsCount < (args.Length - 1) ? args[++argsCount] : null;
                    ++argsCount;
                }
                else if ("-ioctlDetectionTimeInterval".Equals(args[argsCount]))
                {
                    if (argsCount < (args.Length - 1))
                    {
                        string arg = args[++argsCount];
                        if (int.TryParse(arg, out var interval))
                        {
                            config.DetectionTimeInterval = interval;
                        }
                        else
                        {
                            Console.WriteLine("Could not parse -ioctlDetectionTimeInterval value: " + arg);
                            PrintHelp();
                            return null;
                        }
                    }
                    ++argsCount;
                }
                else if ("-ioctlChannelName".Equals(args[argsCount]))
                {
                    config.ChannelName = argsCount < (args.Length - 1) ? args[++argsCount] : null;
                    ++argsCount;
                }
                else // unrecognized command line argument
                {
                    Console.WriteLine("Unknown argument: " + args[argsCount]);
                    PrintHelp();
                    return null;
                }
            }
            return config;
        }
        catch
        {
            PrintHelp();
            return null;
        }
    }

    private static PreferredHostOptions GetPreferredHostOptions(PreferredHostInfo? currentPreferredHostInfo,
        CommandConfig config)
    {
        PreferredHostOptions preferredHostOptions = new PreferredHostOptions();

        preferredHostOptions.EnablePreferredHostOptions = config.EnablePH
            ?? currentPreferredHostInfo?.IsPreferredHostEnabled
            ?? DEFAULT_ENABLE_PREFERRED_HOST_OPTIONS;

        preferredHostOptions.ChannelName = config.ChannelName
            ?? currentPreferredHostInfo?.ChannelName
            ?? DEFAULT_CHANNEL_NAME;

        preferredHostOptions.DetectionTimeInterval = config.DetectionTimeInterval
            ?? currentPreferredHostInfo?.DetectionTimeInterval
            ?? DEFAULT_DETECTION_TIME_INTERVAL;

        preferredHostOptions.DetectionTimeSchedule = config.DetectionTimeSchedule
            ?? currentPreferredHostInfo?.DetectionTimeSchedule
            ?? DEFAULT_DETECTION_TIME_SCHEDULE;

        return preferredHostOptions;
    }

    private static bool IsPHInfoChanged(CommandConfig config, PreferredHostInfo? info, PreferredHostOptions options)
    {

        if (info is null)
        {
            return true;
        }

        if (config.EnablePH is not null
            && info.IsPreferredHostEnabled != options.EnablePreferredHostOptions)
        {
            return true;
        }

        if (config.ChannelName is not null
            && !info.ChannelName.Equals(options.ChannelName))
        {
            return true;
        }

        if (config.DetectionTimeInterval is not null
            && info.DetectionTimeInterval != options.DetectionTimeInterval)
        {
            return true;
        }

        if (config.DetectionTimeSchedule is not null
            && !info.DetectionTimeSchedule.Equals(options.DetectionTimeSchedule))
        {
            return true;
        }

        return false;
    }

    private static void PrintModifyIOCtlData(TimeSpan ioctlInterval, TimeSpan elapsed)
    {
        Console.WriteLine("\n\tIoctl call to update PreferredHostOptions:");
        Console.WriteLine("\tTime interval: " + ioctlInterval);
        TimeSpan timeLeft = ioctlInterval - elapsed;
        Console.WriteLine("\tRemaining time: " + timeLeft);
        Console.WriteLine();
    }

    private static void PrintFallbackPreferredHostData(TimeSpan fallbackInterval, TimeSpan elapsed)
    {
        Console.WriteLine("\n\tFallback to preferred host:");
        Console.WriteLine("\tTime interval: " + fallbackInterval);
        TimeSpan timeLeft = fallbackInterval - elapsed;
        Console.WriteLine("\tRemaining time: " + timeLeft);
        Console.WriteLine();
    }

    public static void Main(string[] args)
    {
        OmmConsumer? consumer = null;
        try
        {
            AppClient appClient = new AppClient();
            ChannelInformation ci = new ChannelInformation();

            PreferredHostOptions options = new PreferredHostOptions();

            CommandConfig? config = ReadCommandLineArgs(args);
            if (config is null)
                return;

            TimeSpan ioctlInterval = config.IOCtlInterval ?? TimeSpan.Zero;
            TimeSpan fallbackInterval = config.FallbackInterval ?? TimeSpan.Zero;

            if (ioctlInterval <= TimeSpan.Zero
                && (config.EnablePH is not null
                || config.IOCtlInterval is not null
                || config.DetectionTimeSchedule is not null
                || config.DetectionTimeInterval is not null))
            {
                Console.WriteLine("\tioctlInterval should have a positive value if any ioctl parameters are specified");
                return;
            }

            consumer = new OmmConsumer(new OmmConsumerConfig()
                .ConsumerName(DEFAULT_CONSUMER_NAME));

            consumer.RegisterClient(new RequestMsg()
                .ServiceName(DEFAULT_SERVICE_NAME)
                .Name(DEFAULT_ITEM_NAME), appClient, 0);

            bool isModifyIOCtlDone = false;
            bool isFallbackDone = false;
            int printInterval = 1;

            consumer.ChannelInformation(ci);
            Console.WriteLine("\nInitial channel information (consumer):\n\t" + ci);
            Console.WriteLine();

            DateTime startTime = DateTime.Now;

            for (int i = 0; i < 600; i++)
            {
                Thread.Sleep(TimeSpan.FromSeconds(1)); // API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()

                if (i % printInterval == 0 && (isModifyIOCtlDone || ioctlInterval <= TimeSpan.Zero))
                {
                    consumer.ChannelInformation(ci);

                    if (ci.ChannelState != ChannelState.INACTIVE)
                    {
                        Console.WriteLine($"\nChannel information (consumer):\n\t {ci}");
                        Console.WriteLine();
                    }
                }

                TimeSpan elapsed = DateTime.Now - startTime;

                if (ioctlInterval > TimeSpan.Zero
                    && elapsed >= ioctlInterval
                    && !isModifyIOCtlDone)
                {
                    bool isChanged = IsPHInfoChanged(config, ci.PreferredHostInfo, options);

                    if (isChanged)
                    {
                        PreferredHostOptions preferredHostOptions = GetPreferredHostOptions(ci.PreferredHostInfo, config);

                        consumer.ModifyIOCtl(IOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, preferredHostOptions);
                        Console.WriteLine("ModifyIOCtl() is called!");
                    }

                    isModifyIOCtlDone = true;
                }

                if (fallbackInterval > TimeSpan.Zero
                    && elapsed >= fallbackInterval
                    && !isFallbackDone)
                {
                    consumer.FallbackPreferredHost();
                    Console.WriteLine("FallbackPreferredHost() is called!");

                    isFallbackDone = true;
                }

                if (i % printInterval == 0)
                {
                    if (!isModifyIOCtlDone
                        && ioctlInterval > TimeSpan.Zero)
                    {
                        PrintModifyIOCtlData(ioctlInterval, elapsed);
                    }

                    if (!isFallbackDone
                        && fallbackInterval > TimeSpan.Zero)
                    {
                        PrintFallbackPreferredHostData(fallbackInterval, elapsed);
                    }
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
