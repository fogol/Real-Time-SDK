/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Example.VACommon;
using LSEG.Eta.Transports;

namespace LSEG.Eta.ValueAdd.Consumer
{
    /// <summary>Command line parser for the Value Add consumer application.</summary>
    internal class ConsumerCmdLineParser : ICommandLineParser
    {
        #region Configuration Properties
        internal string? BackupHostname { get; private set; }

        internal string? BackupPort { get; private set; }

        internal bool EnablePreferredHost { get; private set; }

        internal int PreferredHostIndex { get; private set; }

        internal uint DetectionTimeInterval { get; private set; }

        internal string? DetectionTimeSchedule { get; private set; }

        internal List<ConnectionArg> ConnectionList => m_ConnectionArgsParser.ConnectionList;

        internal string? UserName { get; private set; }

        internal string? Passwd { get; private set; }

        internal string? ClientId { get; private set; }

        internal string? ClientSecret { get; private set; }

        internal string? JwkFile { get; private set; }

        internal string? Audience { get; private set; }

        internal string? TokenURLV2 { get; private set; }

        internal string? serviceDiscoveryURL { get; private set; }

        internal string? TokenScope { get; private set; }

        internal bool EnableView { get; private set; }

        internal bool EnablePost { get; private set; }

        internal bool EnableOffpost { get; private set; }

        internal bool EnableSnapshot { get; private set; }

        internal string? PublisherId { get; private set; }

        internal string? PublisherAddress { get; private set; }

        internal TimeSpan Runtime { get; private set; } = TimeSpan.FromSeconds(600); // default runtime is 600 seconds

        internal bool EnableXmlTracing { get; private set; }

        internal bool EnableEncrypted { get; private set; }

        internal bool EnableProxy { get; private set; }

        internal bool EnableSessionMgnt { get; private set; }

        internal ConnectionType EncryptedProtocolType { get; private set; }

        internal string? ProxyHostname { get; private set; }

        internal string? ProxyPort { get; private set; }

        internal string? ProxyUsername { get; private set; } = "";

        internal string? ProxyPasswd { get; private set; } = "";

        internal string? RestProxyHostname { get; private set; }

        internal string? RestProxyPort { get; private set; }

        internal string? RestProxyUsername { get; private set; } = "";

        internal string? RestProxyPasswd { get; private set; } = "";

        internal string? AuthenticationToken { get; private set; }

        internal string? AuthenticationExtended { get; private set; }

        internal string? ApplicationId { get; private set; }

        internal bool EnableRtt { get; private set; }

        internal bool EnableRestLogging { get; private set; }

        internal string? RestLoggingFileName { get; private set; }

        internal string? Location { get; private set; }

        internal EncryptionProtocolFlags Protocol { get; private set; }

        public int IoctlInterval { get; private set; }

        public int FallBackInterval { get; private set; }

        public bool? IoctlEnablePH { get; private set; }

        public int? IoctlConnectListIndex { get; private set; }

        public uint? IoctlDetectionTimeInterval { get; private set; }

        public string? IoctlDetectionTimeSchedule { get; private set; }

        internal ulong UpdateTypeFilter { get; set; } = 0;

        internal ulong NegativeUpdateTypeFilter { get; set; } = 0;

        #endregion

        private ConnectionArgsParser m_ConnectionArgsParser = new ConnectionArgsParser();


        public bool IoctlOverridesPreferredHost =>
            (IoctlEnablePH is not null) ||
            (IoctlConnectListIndex is not null && IoctlConnectListIndex != PreferredHostIndex) ||
            (IoctlDetectionTimeInterval is not null && IoctlDetectionTimeInterval != DetectionTimeInterval) ||
            (IoctlDetectionTimeSchedule is not null && IoctlDetectionTimeSchedule != DetectionTimeSchedule);

        public bool ParseArgs(string[] args)
        {
            int argsCount = 0;

            while (argsCount < args.Length)
            {
                if (m_ConnectionArgsParser.IsStart(args, argsCount))
                {
                    if ((argsCount = m_ConnectionArgsParser.Parse(args, argsCount)) < 0)
                    {
                        // error
                        Console.WriteLine("\nError parsing connection arguments...\n");
                        return false;
                    }
                }
                else if ("-bc".Equals(args[argsCount]))
                {
                    if (args[argsCount + 1].Contains(':'))
                    {
                        string[] tokens = args[++argsCount].Split(":");
                        if (tokens.Length == 2)
                        {
                            BackupHostname = tokens[0];
                            BackupPort = tokens[1];
                            ++argsCount;
                        }
                        else
                        {
                            // error
                            Console.WriteLine("\nError parsing backup connection arguments...\n");
                            return false;
                        }
                    }
                    else
                    {
                        // error
                        Console.WriteLine("\nError parsing backup connection arguments...\n");
                        return false;
                    }
                }
                else if ("-enablePH".Equals(args[argsCount]))
                {
                    EnablePreferredHost = true;
                    ++argsCount;
                }
                else if ("-preferredHostIndex".Equals(args[argsCount]))
                {
                    if (Int32.TryParse(args[++argsCount], out var preferredHostIndex))
                    {
                        PreferredHostIndex = preferredHostIndex;
                    }
                    else
                    {
                        Console.WriteLine($"Error: failed to parse preferred host index '{args[argsCount]}'");
                        return false;
                    }
                    ++argsCount;
                }
                else if ("-detectionTimeInterval".Equals(args[argsCount]))
                {
                    if (UInt32.TryParse(args[++argsCount], out var detectionTimeInterval))
                    {
                        DetectionTimeInterval = detectionTimeInterval;
                    }
                    else
                    {
                        Console.WriteLine($"Error: failed to parse detection time interval '{args[argsCount]}'");
                        return false;
                    }
                    ++argsCount;
                }
                else if ("-detectionTimeSchedule".Equals(args[argsCount]))
                {
                    DetectionTimeSchedule = args[++argsCount];
                    ++argsCount;
                }
                else if ("-uname".Equals(args[argsCount]))
                {
                    UserName = args[++argsCount];
                    ++argsCount;
                }
                else if ("-passwd".Equals(args[argsCount]))
                {
                    Passwd = args[++argsCount];
                    ++argsCount;
                }
                else if ("-sessionMgnt".Equals(args[argsCount]))
                {
                    EnableSessionMgnt = true;
                    ++argsCount;
                }
                else if ("-clientId".Equals(args[argsCount]))
                {
                    ClientId = args[++argsCount];
                    ++argsCount;
                }
                else if ("-clientSecret".Equals(args[argsCount]))
                {
                    ClientSecret = args[++argsCount];
                    ++argsCount;
                }
                else if ("-jwkFile".Equals(args[argsCount]))
                {
                    JwkFile = args[++argsCount];
                    ++argsCount;
                }
                else if ("-audience".Equals(args[argsCount]))
                {
                    Audience = args[++argsCount];
                    ++argsCount;
                }
                else if ("-tokenURLV2".Equals(args[argsCount]))
                {
                    TokenURLV2 = args[++argsCount];
                    ++argsCount;
                }
                else if ("-serviceDiscoveryURL".Equals(args[argsCount]))
                {
                    serviceDiscoveryURL = args[++argsCount];
                    ++argsCount;
                }
                else if ("-tokenScope".Equals(args[argsCount]))
                {
                    TokenScope = args[++argsCount];
                    ++argsCount;
                }
                else if ("-view".Equals(args[argsCount]))
                {
                    EnableView = true;
                    ++argsCount;
                }
                else if ("-post".Equals(args[argsCount]))
                {
                    EnablePost = true;
                    ++argsCount;
                }
                else if ("-offpost".Equals(args[argsCount]))
                {
                    EnableOffpost = true;
                    ++argsCount;
                }
                else if ("-publisherInfo".Equals(args[argsCount]))
                {
                    String value = args[++argsCount];
                    if (value != null)
                    {
                        String[] pieces = value.Split(",");

                        if (pieces.Length > 1)
                        {
                            PublisherId = pieces[0];

                            PublisherAddress = pieces[1];
                        }
                        else
                        {
                            Console.WriteLine("Error loading command line arguments for publisherInfo [id, address]:\t");
                            return false;
                        }
                    }
                    ++argsCount;
                }
                else if ("-snapshot".Equals(args[argsCount]))
                {
                    EnableSnapshot = true;
                    ++argsCount;
                }
                else if ("-runtime".Equals(args[argsCount]))
                {
                    if (Int32.TryParse(args[++argsCount], out var runTimeSeconds))
                    {
                        Runtime = TimeSpan.FromSeconds(runTimeSeconds);
                    }
                    else
                    {
                        Console.WriteLine($"Error: failed to parse runtime '{args[argsCount]}'");
                        return false;
                    }
                    ++argsCount;
                }
                else if ("-x".Equals(args[argsCount]))
                {
                    EnableXmlTracing = true;
                    ++argsCount;
                }
                else if ("-connectionType".Equals(args[argsCount]))
                {
                    // will overwrite connectionArgsParser's connectionList's connectionType based on the flag
                    string connectionType = args[++argsCount];
                    ++argsCount;
                    if (connectionType.Equals("encrypted"))
                    {
                        EnableEncrypted = true;
                    }
                }
                else if ("-encryptedProtocolType".Equals(args[argsCount]))
                {
                    // will overwrite connectionArgsParser's connectionList's connectionType based on the flag
                    String connectionType = args[++argsCount];
                    ++argsCount;
                    if (connectionType.Equals("socket"))
                    {
                        EncryptedProtocolType = ConnectionType.SOCKET;
                    }
                }
                else if ("-proxy".Equals(args[argsCount]))
                {
                    EnableProxy = true;
                    ++argsCount;
                }
                else if ("-ph".Equals(args[argsCount]))
                {
                    ProxyHostname = args[++argsCount];
                    ++argsCount;
                }
                else if ("-pp".Equals(args[argsCount]))
                {
                    ProxyPort = argsCount < (args.Length - 1) ? args[++argsCount] : null;
                    ++argsCount;
                }
                else if ("-plogin".Equals(args[argsCount]))
                {
                    ProxyUsername = args[++argsCount];
                    ++argsCount;
                }
                else if ("-ppasswd".Equals(args[argsCount]))
                {
                    ProxyPasswd = argsCount < (args.Length - 1) ? args[++argsCount] : null;
                    ++argsCount;
                }
                else if ("-restProxyHost".Equals(args[argsCount]))
                {
                    RestProxyHostname = args[++argsCount];
                    ++argsCount;
                }
                else if ("-restProxyPort".Equals(args[argsCount]))
                {
                    RestProxyPort = args[++argsCount];
                    ++argsCount;
                }
                else if ("-restProxyUsername".Equals(args[argsCount]))
                {
                    RestProxyUsername = args[++argsCount];
                    ++argsCount;
                }
                else if ("-restProxyPasswd".Equals(args[argsCount]))
                {
                    RestProxyPasswd = args[++argsCount];
                    ++argsCount;
                }
                else if ("-at".Equals(args[argsCount]))
                {
                    AuthenticationToken = args[++argsCount];
                    ++argsCount;
                }
                else if ("-ax".Equals(args[argsCount]))
                {
                    AuthenticationExtended = args[++argsCount];
                    ++argsCount;
                }
                else if ("-aid".Equals(args[argsCount]))
                {
                    ApplicationId = args[++argsCount];
                    ++argsCount;
                }
                else if ("-rtt".Equals(args[argsCount]))
                {
                    EnableRtt = true;
                    ++argsCount;
                }
                else if ("-restEnableLog".Equals(args[argsCount]))
                {
                    EnableRestLogging = true;
                    ++argsCount;
                }
                else if ("-restLogFileName".Equals(args[argsCount]))
                {
                    RestLoggingFileName = args[++argsCount];
                    ++argsCount;
                }
                else if ("-location".Equals(args[argsCount]))
                {
                    Location = args[++argsCount];
                    ++argsCount;
                }
                else if ("-ioctlInterval".Equals(args[argsCount]))
                {
                    if (int.TryParse(args[++argsCount], out var ioctlInterval))
                    {
                        IoctlInterval = ioctlInterval;
                    }
                    else
                    {
                        Console.WriteLine($"Error: failed to parse ioctlInterval '{args[argsCount]}'");
                        return false;
                    }
                    ++argsCount;
                }
                else if ("-fallBackInterval".Equals(args[argsCount]))
                {
                    if (int.TryParse(args[++argsCount], out var fallBackInterval))
                    {
                        FallBackInterval = fallBackInterval;
                    }
                    else
                    {
                        Console.WriteLine($"Error: failed to parse FallBackInterval '{args[argsCount]}'");
                        return false;
                    }
                    ++argsCount;
                }
                else if ("-ioctlEnablePH".Equals(args[argsCount]))
                {
                    if (bool.TryParse(args[++argsCount], out var ioctlEnablePH))
                    {
                        IoctlEnablePH = ioctlEnablePH;
                    }
                    else
                    {
                        Console.WriteLine($"Error: failed to parse ioctlEnablePH '{args[argsCount]}'");
                        return false;
                    }
                    ++argsCount;
                }
                else if ("-ioctlConnectListIndex".Equals(args[argsCount]))
                {
                    if (int.TryParse(args[++argsCount], out var ioctlConnectListIndex))
                    {
                        IoctlConnectListIndex = ioctlConnectListIndex;
                    }
                    else
                    {
                        Console.WriteLine($"Error: failed to parse IoctlConnectListIndex '{args[argsCount]}'");
                        return false;
                    }
                    ++argsCount;
                }
                else if ("-ioctlDetectionTimeInterval".Equals(args[argsCount]))
                {
                    if (uint.TryParse(args[++argsCount], out var ioctlDetectionTimeInterval))
                    {
                        IoctlDetectionTimeInterval = ioctlDetectionTimeInterval;
                    }
                    else
                    {
                        Console.WriteLine($"Error: failed to parse IoctlDetectionTimeInterval '{args[argsCount]}'");
                        return false;
                    }
                    ++argsCount;
                }
                else if ("-ioctlDetectionTimeSchedule".Equals(args[argsCount]))
                {
                    IoctlDetectionTimeSchedule = args[++argsCount];
                    ++argsCount;
                }
                else if ("-encryptionProtocol".Equals(args[argsCount]))
                {
                    var protocol = args[++argsCount];
                    if (EncryptionProtocolFlagsExtension.TryParse(protocol, out var parsedProtocol))
                    {
                        Protocol = parsedProtocol;
                    }
                    else
                    {
                        Console.WriteLine("\nUnrecognized protocol type...\n");
                        return false;
                    }
                }
                else if ("-updateTypeFilter".Equals(args[argsCount]))
                {
                    UpdateTypeFilter = ulong.Parse(args[++argsCount]);
                    ++argsCount;
                }
                else if ("-negativeUpdateTypeFilter".Equals(args[argsCount]))
                {
                    NegativeUpdateTypeFilter = ulong.Parse(args[++argsCount]);
                    ++argsCount;
                }
                else // unrecognized command line argument
                {
                    Console.WriteLine($"\nUnrecognized command line argument '{args[argsCount]}'\n");
                    return false;
                }
            }

            return ValidateParsedArguments();
        }

        public void PrintUsage()
        {
            Console.WriteLine("Usage: Consumer or\nConsumer [-c <hostname>:<port>[,...] <service name> <domain>:<item name>,...] [-bc <hostname>:<port>] [-uname <LoginUsername>] [-view] [-post] [-offpost] [-snapshot] [-runtime <seconds>]" +
                               "\n -c specifies a connection to open and a list of items to request:" +
                               "\n     hostname:        Hostname of provider to connect to" +
                               "\n     port:            Port of provider to connect to" +
                               "\n     service:         Name of service to request items from on this connection" +
                               "\n     domain:itemName: Domain and name of an item to request" +
                               "\n         A comma-separated list of these may be specified." +
                               "\n         The domain may be any of: mp(MarketPrice), mbo(MarketByOrder), mbp(MarketByPrice), yc(YieldCurve), sl(SymbolList)" +
                               "\n         The domain may also be any of the private stream domains: mpps(MarketPrice PS), mbops(MarketByOrder PS), mbpps(MarketByPrice PS), ycps(YieldCurve PS)" +
                               "\n         Example Usage: -c localhost:14002 DIRECT_FEED mp:TRI,mp:GOOG,mpps:FB,mbo:MSFT,mbpps:IBM,sl" +
                               "\n           (for SymbolList requests, a name can be optionally specified)" +
                               "\n -bc specifies a backup connection that is attempted if the primary connection fails" +
                               "\n -enablePH enables preferred host backup connection switching instead of default round-robin" +
                               "\n -preferredHostIndex specifies which connection among backup & primary should be preferred" +
                               "\n -detectionTimeInterval specifies time interval in seconds to switch over to a preferred host when preferred host is enabled" +
                               "\n -detectionTimeSchedule specifies CRON expression to switch over to a preferred host when preferred host is enabled" +
                               "\n -uname changes the username used when logging into the provider" +
                               "\n -passwd changes the password used when logging into the provider" +
                               "\n -clientId specifies a unique ID for application making the request to RDP token service" +
                               "\n -clientSecret specifies the associated secret with the client ID" +
                               "\n -jwkFile specifies a file containing the JWK encoded private key for V2 JWT logins." +
                               "\n -audience audience claim for v2 JWT logins." +
                               "\n -sessionMgnt enables the session management in the Reactor" +
                               "\n -tokenURLV2 specifies the URL for the token service to override the default value." +
                               "\n -serviceDiscoveryURL specifies the RDP Service Discovery URL to override the default value." +
                               "\n -tokenScope specifies a scope for the token service." +
                               "\n -view specifies each request using a basic dynamic view" +
                               "\n -post specifies that the application should attempt to send post messages on the first requested Market Price item" +
                               "\n -offpost specifies that the application should attempt to send post messages on the login stream (i.e., off-stream)" +
                               "\n -publisherInfo specifies that the application should add user provided publisher Id and publisher ipaddress when posting" +
                               "\n -snapshot specifies each request using non-streaming" +
                               "\n -connectionType specifies the connection type that the connection should use (possible values are: 'socket', 'encrypted')" +
                               "\n -encryptedProtocolType specifies the encrypted protocol type that the connection should use (possible values are: 'socket')" +
                               "\n -proxy specifies that proxy is used for connectionType" +
                               "\n -ph specifies proxy server host name" +
                               "\n -pp specifies proxy port number" +
                               "\n -plogin specifies user name on proxy server" +
                               "\n -ppasswd specifies password on proxy server" +
                               "\n -restProxyHost specifies REST proxy server host name" +
                               "\n -restProxyPort specifies REST proxy port number" +
                               "\n -restProxyUsername specifies user name on REST proxy server" +
                               "\n -restProxyPasswd specifies password on REST proxy server" +
                               "\n -x provides an XML trace of messages" +
                               "\n -runtime adjusts the running time of the application" +
                               "\n -aid Specifies the Application ID" +
                               "\n -restEnableLog enable REST logging message" +
                               "\n -restLogFileName set REST logging output stream" +
                               "\n -rtt Enables rtt support by a consumer. If provider makes distribution of RTT messages, consumer will return back them. In another case, consumer will ignore them." +
                               "\n -location specifies location/region when doing service discovery" +
                               "\n -updateTypeFilter specifies UpdateTypeFilter in the Login Request message" +
                               "\n -negativeUpdateTypeFilter specifies NegativeUpdateTypeFilter in the Login Request message" +
                               "\nOptions for IOCtl and Fallback calls (optional):" +
                               "\n -fallBackInterval <time interval> specifies time interval (in second) in application before Ad Hoc Fallback function is invoked. O indicates that function won't be invoked" +
                               "\n -ioctlInterval <time interval> specifies time interval (in second) before IOCtl function is invoked. O indicates that function won't be invoked" +
                               "\n -ioctlEnablePH <true/false> enables Preferred host feature" +
                               "\n -ioctlConnectListIndex <index> specifies the preferred host as the index in the connection list" +
                               "\n -ioctlDetectionTimeInterval <time interval> specifies time interval (in second) to switch over to a preferred host. 0 indicates that the detection time interval is disabled" +
                               "\n -ioctlDetectionTimeSchedule <Cron time> specifies Cron time format to switch over to a preferred host");
        }

        private bool ValidateParsedArguments()
        {
            var connectionOptionWithMultipleHostsExists = ConnectionList.Any(x => x.HostList.Count > 1);
            if (ConnectionList.Count > 1 && connectionOptionWithMultipleHostsExists)
            {
                Console.WriteLine($"Error: Connection option \"-c\" can be used only once when specifying multiple hosts.");
                return false;
            }
            if (connectionOptionWithMultipleHostsExists && (!string.IsNullOrEmpty(BackupHostname) || !string.IsNullOrEmpty(BackupPort)))
            {
                Console.WriteLine($"Error: Backup connection option \"-bc\" cannot be used with connection option \"-c\" having multiple hosts.");
                return false;
            }

            if (IoctlInterval == 0)
            {
                const string Condition = "ioctlInterval value should be set and be greater than zero.";
                if (IoctlEnablePH is not null)
                {
                    Console.WriteLine($"When ioctlEnablePH is set, {Condition}");
                    return false;
                }
                if (IoctlConnectListIndex is not null)
                {
                    Console.WriteLine($"When ioctlConnectListIndex is set, {Condition}");
                    return false;
                }
                if (IoctlDetectionTimeInterval is not null)
                {
                    Console.WriteLine($"When ioctlDetectionTimeInterval is set, {Condition}");
                    return false;
                }
                if (IoctlDetectionTimeSchedule is not null)
                {
                    Console.WriteLine($"When ioctlDetectionTimeSchedule is set, {Condition}");
                    return false;
                }
            }
            
            return true;
        }
    }
}
