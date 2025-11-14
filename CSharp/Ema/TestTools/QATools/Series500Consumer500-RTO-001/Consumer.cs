/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.IO;
using System.Runtime.CompilerServices;
using System.Threading;

using LSEG.Ema.Access;
using static LSEG.Ema.Access.DataType;
using static LSEG.Ema.Access.EmaConfig;

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
    private const string DEFAULT_CONSUMER_NAME = "Consumer_9"; // EMA C# doesn't support the WEBSOCKET connection yet.

    private static string? clientId;
    private static string? clientSecret;
    private static string? clientJwk;
    private static string? audience;
    private static string? proxyHostName;
    private static string? proxyPort = "";
    private static string? proxyUserName;
    private static string? proxyPassword;
    private static string itemName = "IBM.N";
    private static string serviceName = "ELEKTRON_DD";
    private static string? tokenUrlV2;
    private static string? serviceDiscoveryUrl;
    private static uint tlsProtocolFlags = 0;

    static void PrintHelp()
    {
        Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n"
                + "  -clientId client ID for application making the request to(mandatory for V2 client credentials grant) \n"
                + "  -clientSecret service account secret (mandatory for V2 client credentials grant).\n"
                + "  -jwkFile file containing the private JWK encoded in JSON format.\n"
                + "  -audience audience location for the JWK (optional). \n"
                + "  -tokenURLV2 V2 URL to perform authentication to get access and refresh tokens (optional).\n"
                + "  -serviceDiscoveryURL URL for RDP service discovery to get global endpoints (optional).\n"
                + "  -encryptedTLSv1.2 Enable TLS 1.2 encrypted protocol. Default enables both TLS 1.2 and TLS 1.3 (optional). \n"
                + "  -encryptedTLSv1.3 Enable TLS 1.3 encrypted protocol. Default enables both TLS 1.2 and TLS 1.3 (optional). \n"
                + "  -itemName Request item name (optional).\n"
                + "  -serviceName Service name (optional).\n"
                + "\nOptional parameters for establishing a connection and sending requests through a proxy server:\n"
                + "  -ph Proxy host name (optional).\n"
                + "  -pp Proxy port number (optional).\n"
                + "  -plogin User name on proxy server (optional).\n"
                + "  -ppasswd Password on proxy server (optional).\n"
                + "\n");
    }

    static bool ReadCommandlineArgs(string[] args, OmmConsumerConfig config)
    {
        try
        {
            int argsCount = 0;

            while (argsCount < args.Length)
            {
                if (args[argsCount].Equals("-?"))
                {
                    PrintHelp();
                    return false;
                }
                else if ("-clientId".Equals(args[argsCount]))
                {
                    clientId = argsCount < (args.Length - 1) ? args[++argsCount] : null;
                    if(clientId != null) config.ClientId(clientId);
                    ++argsCount;
                }
                else if ("-clientSecret".Equals(args[argsCount]))
                {
                    clientSecret = argsCount < (args.Length - 1) ? args[++argsCount] : null;
                    if (clientSecret != null) config.ClientSecret(clientSecret);
                    ++argsCount;
                }
                else if ("-jwkFile".Equals(args[argsCount]))
                {
                    string? jwkFile = argsCount < (args.Length - 1) ? args[++argsCount] : null;
                    if (jwkFile != null)
                    {
                        try
                        {
                            // Get the full contents of the JWK file.
                            string jwkText = File.ReadAllText(jwkFile);

                            clientJwk = jwkText;
                            config.ClientJwk(clientJwk);
                        }
                        catch (Exception e)
                        {
                            Console.Error.WriteLine("Error loading JWK file: " + e.Message);
                            Console.Error.WriteLine();
                            Console.WriteLine("Consumer exits...");
                            Environment.Exit((int)Eta.Codec.CodecReturnCode.FAILURE);
                        }
                    }
                    ++argsCount;
                }
                else if ("-audience".Equals(args[argsCount]))
                {
                    audience = argsCount < (args.Length - 1) ? args[++argsCount] : null;
                    if(audience != null) config.Audience(audience);
                    ++argsCount;
                }
                else if ("-itemName".Equals(args[argsCount]))
                {
                    if (argsCount < (args.Length - 1))
                    {
                        itemName = args[++argsCount];
                    }
                    ++argsCount;
                }
				 else if ("-serviceName".Equals(args[argsCount]))
                {
                    if (argsCount < (args.Length - 1))
                    {
                        serviceName = args[++argsCount];
                    }
                    ++argsCount;
                }
                else if ("-ph".Equals(args[argsCount]))
                {
                    proxyHostName = argsCount < (args.Length - 1) ? args[++argsCount] : null;
                    if(proxyHostName != null) config.ProxyHost(proxyHostName);
                    ++argsCount;
                }
                else if ("-pp".Equals(args[argsCount]))
                {
                    proxyPort = argsCount < (args.Length - 1) ? args[++argsCount] : null;
                    if (proxyPort != null) config.ProxyPort(proxyPort);
                    ++argsCount;
                }
                else if ("-plogin".Equals(args[argsCount]))
                {
                    proxyUserName = argsCount < (args.Length - 1) ? args[++argsCount] : null;
                    if (proxyUserName != null) config.ProxyUserName(proxyUserName);
                    ++argsCount;
                }
                else if ("-ppasswd".Equals(args[argsCount]))
                {
                    proxyPassword = argsCount < (args.Length - 1) ? args[++argsCount] : null;
                    if (proxyPassword != null) config.ProxyPassword(proxyPassword);
                    ++argsCount;
                }
                else if ("-tokenURLV2".Equals(args[argsCount]))
                {
                    if (argsCount < (args.Length - 1))
                    {
                        tokenUrlV2 = args[++argsCount];
                        config.TokenUrlV2(tokenUrlV2);
                    }
                    ++argsCount;
                }
                else if ("-serviceDiscoveryURL".Equals(args[argsCount]))
                {
                    if (argsCount < (args.Length - 1))
                    {
                        serviceDiscoveryUrl = args[++argsCount];
                        config.ServiceDiscoveryUrl(serviceDiscoveryUrl);
                    }
                    ++argsCount;
                }
                else if ("-encryptedTLSv1.2".Equals(args[argsCount]))
                {
                    tlsProtocolFlags |= EncryptedTLSProtocolFlags.TLSv1_2;
                    ++argsCount;
                }
                else if ("-encryptedTLSv1.3".Equals(args[argsCount]))
                {
                    tlsProtocolFlags |= EncryptedTLSProtocolFlags.TLSv1_3;
                    ++argsCount;
                }
                else // unrecognized command line argument
                {
                    PrintHelp();
                    return false;
                }
            }

            if (clientId == null || (clientSecret == null && clientJwk == null))
            {
                Console.WriteLine("clientId/clientSecret or jwkFile must be specified on the command line. Exiting...");
                PrintHelp();
                return false;
            }
        }
        catch
        {
            PrintHelp();
            return false;
        }

        // Checks whether the encrypted protocol is specified and set them to OmmConsumerConfig.
        if (tlsProtocolFlags != 0)
            config.EncryptedProtocolFlags(tlsProtocolFlags);

        return true;
    }

    public static void Main(string[] args)
    {
        OmmConsumer? consumer = null;
        try
        {
            AppClient appClient = new AppClient();
            var config = new OmmConsumerConfig();

            if (!ReadCommandlineArgs(args, config)) return;

            consumer = new OmmConsumer(config.ConsumerName(DEFAULT_CONSUMER_NAME));

            consumer.RegisterClient(new RequestMsg()
                .ServiceName(serviceName)
                .Name(itemName), appClient, 0);

            int printInterval = 1;
            ChannelInformation ci = new ChannelInformation();
            for (int i = 0; i < 600; i++)
            {
                Thread.Sleep(TimeSpan.FromSeconds(1)); // API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()

                if ((i % printInterval == 0))
                {
                    consumer.ChannelInformation(ci);

                    if (ci.ChannelState != ChannelState.INACTIVE)
                    {
                        Console.WriteLine("\nChannel information (consumer):\n\t" + ci);
                        Console.WriteLine();
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
