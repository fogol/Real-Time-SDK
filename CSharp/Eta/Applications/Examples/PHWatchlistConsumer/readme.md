# WatchlistConsumer Mod1a

WatchlistConsumer application modified for JSON config input with the connection list and the prefered host options.

## Summary

The purpose of this application is to demonstrate consuming data from an ADS device, OMM Provider application or Refinitiv Real-Time - Optimized using ValueAdd components. It is a single-threaded client application. 

This application leverages the consumer watchlist feature provided by the RsslReactor to provide recovery and aggregation of items. Using the consumer watchlist feature also enables this application to consume from either an OMM Provider or ADS over a socket-based connection, or from an ADS over a multicast network.  It requests items within the channelOpenCallback function, which are automatically requested by the RsslReactor on its behalf when the channel connects to the providing component.

If the dictionary is found in the directory of execution, then it is loaded directly from the file.  However, the default configuration for this application is to request the dictionary from the provider. Hence, no link to the dictionary is made in the execution directory by the build script. The user can change this behavior by manually creating a link to the dictionary in the execution directory.

This application supports consuming Level I Market Price, Level II Market By Order, Level II Market By Price.

## Command line usage

```cmd
WatchlistConsumer
	[-help  <Display help information and exit. Default is:False> ]
	[-s  <Default service name for requests. This will be used for dictionary requests as well as any items that do not have a default service specified in -mp, -mbo, or -mbp.. Default is:DIRECT_FEED> ]
	[-u  <Login user name. Default is system user name.> ]
	[-passwd  <Password for the user name.> ]
	[-runTime  <Program runtime in seconds. Default is:600> ]
	[-x  <Provides XML tracing of messages.> ]
	[-clientId  <Specifies the client Id for Refinitiv login V2, or specifies a unique ID with login V1 for applications making the request to EDP token service, this is also known as AppKey generated using an AppGenerator.> ]
	[-clientSecret  <Specifies the associated client Secret with a provided clientId for V2 logins.> ]
	[-tokenURLV2  <Specifies the token URL for V2 token oauthclientcreds grant type.> ]
	[-tokenScope  <Specifies the token scope.. Default is:> ]
	[-serviceDiscoveryURL  <Specifies the service discovery URL.> ]
	[-mp  <For each occurrence, requests item using Market Price domain.> ]
	[-mbo  <For each occurrence, requests item using Market By Order domain. Default is no market by order requests.> ]
	[-mbp  <For each occurrence, requests item using Market By Price domain. Default is no market by price requests.> ]
	[-at  <Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN.. Default is:> ]
	[-ax  <Specifies the Authentication Extended information.. Default is:> ]
	[-aid  <Specifies the Application ID.. Default is:> ]
	[-audience  <Optionally specifies the audience used with V2 JWT logins. Default is:> ]
	[-jwkFile  <Specifies the file location containing the JWK encoded private key for V2 logins.> ]
	[-configJson  <Json config file path. Default is:Config.json> ]
	[-restProxyHost  <Proxy server host name for REST requests> ]
	[-restProxyPort  <Proxy port number for REST requests> ]
	[-restProxyUsername  <User Name on proxy server for REST requests> ]
	[-restProxyPasswd  <Password on proxy server for REST requests> ]
	[-reconnectLimit  <Reconnection attempt count. Default is:-1> ]
```

The -configJson allows passing connection list and prefered host configuration as JSON format from a file. See an [example](Config.json).

The -clientId option specifies an unique ID for authenticating with the RDP token service (mandatory). You can generate and manage client Ids by using the Eikon App Key Generator. This is found by visiting my.Refinitiv.Com, launching Eikon (need valid login), and searching for "App Key Generator". The App Key is the Client ID. 

For a client credentials grant(Service Account) V2 login: -clientId <service account> -clientSecret <client secret>. 

The user can specify multiple instances of -mp, -mbo, -mbp, where each occurrence is associated with a single item. For example, specifying "-mp TRI -mp GOOG -mbo AAPL" will issue requests for two MarketPrice items and one MarketByOrder item.

Specifying the -at option configures the token used for UserAuthn Authentication. This should be used in place of a userName.  This token is retrieved from a token generator, and passed to Refinitiv Real-Time Distribution, which will verify the token against a token validator. For more information about the UserAuthn Authentication feature, please see the Developers guide and the UserAuthn Authentication guide.

Specifying the -ax option configures the authentication extended information used for UserAuthn Authentication.

Specifying the -aid option configures the Application Id.

Specifying the -runTime option controls the time the application will run before exiting, in seconds.

- WatchlistConsumer -? displays command line options.  