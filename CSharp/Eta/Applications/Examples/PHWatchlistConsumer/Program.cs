/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2024 LSEG. All rights reserved.                   --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.WatchlistConsumer;
using System;
using System.Linq;

WatchlistConsumerConfig config = new ();

if(config.Errors.Any())
{
    foreach (var error in config.Errors)
    {
        Console.Error.WriteLine(error);
    }
    Console.Error.WriteLine();
    Console.Error.WriteLine(WatchlistConsumerConfig.OptionHelpString());
    Console.WriteLine("Consumer exits...");
    Environment.Exit((int)CodecReturnCode.FAILURE);
}

WatchlistConsumer consumer = new(config);
consumer.Init();
consumer.Run();
consumer.Uninitialize();