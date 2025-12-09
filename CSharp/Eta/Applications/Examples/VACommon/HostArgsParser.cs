/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Linq;

namespace LSEG.Eta.Example.VACommon
{
    internal class HostArgsParser : IArgsParser
    {
        public List<HostArg> HostList { get; private set; } = new();

        public bool IsStart(string[] args, int argOffset)
        {
            return args[argOffset].Contains(':');
        }

        public int Parse(string[] args, int argOffset)
        {
            int retCode = IArgsParser.ERROR_RETURN_CODE;

            var parsingResult = args[argOffset]
                .Split(",", StringSplitOptions.TrimEntries)
                .Select(x => x.Split(":", StringSplitOptions.TrimEntries))
                .Select(x => x.Length == 2
                    ? new HostArg { Hostname = x[0], Port = x[1], }
                    : null)
                .ToList();
            if (parsingResult.Count == 0 || parsingResult.Any(x => x == null))
            {
                return retCode;
            }

            HostList = parsingResult!;
            retCode = argOffset + 1;

            return retCode;
        }
    }
}
