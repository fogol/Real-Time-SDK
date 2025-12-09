/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Example.VACommon
{
    internal interface IArgsParser
    {
        const int ERROR_RETURN_CODE = -1;

        /// <summary>
        /// Checks if arguments (<paramref name="args"/>) are parsable by this parser starting the specified offset (<paramref name="argOffset"/>).
        /// </summary>
        /// <param name="args"></param>
        /// <param name="argOffset"></param>
        /// <returns></returns>
        bool IsStart(string[] args, int argOffset);

        /// <summary>
        /// Parses arguments from given <paramref name="args"/> and returns new offset in argument list.
        /// </summary>
        /// <param name="args"></param>
        /// <param name="argOffset"></param>
        /// <returns>returns offset past item arguments or <see cref="IArgsParser.ERROR_RETURN_CODE"/> if error</returns>
        int Parse(string[] args, int argOffset);
    }
}