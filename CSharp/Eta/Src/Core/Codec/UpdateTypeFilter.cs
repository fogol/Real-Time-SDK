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
using System.Text;
using System.Threading.Tasks;

namespace LSEG.Eta.Rdm
{
    /// <summary>
    /// Indicates Update Type Filter value
    /// </summary>
    public class UpdateTypeFilter
    {
        /// <summary>
        /// Update type is Unspecified
        /// </summary>
        public const ulong RDM_UPT_UNSPECIFIED = 0x001;

        /// <summary>
        /// Update Type is Quote
        /// </summary>        
        public const ulong RDM_UPT_QUOTE = 0x002;

        /// <summary>
        /// Update Type is Trade
        /// </summary>
        public const ulong RDM_UPT_TRADE = 0x004;

        /// <summary>
        /// Update Type is News Alert
        /// </summary>
        public const ulong RDM_UPT_NEWS_ALERT = 0x008;

        /// <summary>
        /// Update Type is Volume Alert
        /// </summary>
        public const ulong RDM_UPT_VOLUME_ALERT = 0x010;

        /// <summary>
        /// Update Type is Order Indication
        /// </summary>
        public const ulong RDM_UPT_ORDER_INDICATION = 0x020;

        /// <summary>
        /// Update Type is Closing Run
        /// </summary>
        public const ulong RDM_UPT_CLOSING_RUN = 0x040;

        /// <summary>
        /// Update Type is Correction
        /// </summary>
        public const ulong RDM_UPT_CORRECTION = 0x080;

        /// <summary>
        /// Update Type is Market Digest
        /// </summary>
        public const ulong RDM_UPT_MARKET_DIGEST = 0x100;

        /// <summary>
        /// Update Type is Quotes followed by a Trade
        /// </summary>
        public const ulong RDM_UPT_QUOTES_TRADE = 0x200;

        /// <summary>
        /// Update with filtering and conflation applied
        /// </summary>
        public const ulong RDM_UPT_MULTIPLE = 0x400;

        /// <summary>
        /// Fields may have changed
        /// </summary>
        public const ulong RDM_UPT_VERIFY = 0x800;

        private static ulong[] TYPE_FILTER_ENTRIES =
        {
            RDM_UPT_UNSPECIFIED,
            RDM_UPT_QUOTE,
            RDM_UPT_TRADE,
            RDM_UPT_NEWS_ALERT,
            RDM_UPT_VOLUME_ALERT,
            RDM_UPT_ORDER_INDICATION,
            RDM_UPT_CLOSING_RUN,
            RDM_UPT_CORRECTION,
            RDM_UPT_MARKET_DIGEST,
            RDM_UPT_QUOTES_TRADE,
            RDM_UPT_MULTIPLE,
            RDM_UPT_VERIFY
        };

        private static Dictionary<ulong, string> TYPE_FILTER_VAL_TO_NAME = new Dictionary<ulong, string>()
        {
            [RDM_UPT_UNSPECIFIED] = "Unspecified",
            [RDM_UPT_QUOTE] = "Quote",
            [RDM_UPT_TRADE] = "Trade",
            [RDM_UPT_NEWS_ALERT] = "News Alert",
            [RDM_UPT_VOLUME_ALERT] = "Volume Alert",
            [RDM_UPT_ORDER_INDICATION] = "Order Indication",
            [RDM_UPT_CLOSING_RUN] = "Closing Run",
            [RDM_UPT_CORRECTION] = "Correction",
            [RDM_UPT_MARKET_DIGEST] = "Market Digest",
            [RDM_UPT_QUOTES_TRADE] = "Quotes Trade",
            [RDM_UPT_MULTIPLE] = "Multiple",
            [RDM_UPT_VERIFY] = "Verify"
        };

        /// <summary>
        /// Provider String representation of the given UpdateTypeFilter value
        /// </summary>
        /// <param name="value">UpdateTypeFilter value to be converted</param>
        /// <returns>String representation</returns>
        public static string UpdateTypeFilterToString(ulong value)
        {
            StringBuilder resBuilder = new StringBuilder();

            for (int i = 0; i < TYPE_FILTER_ENTRIES.Length; i++)
            {
                if ((value & TYPE_FILTER_ENTRIES[i]) > 0)
                {
                    resBuilder.Append(TYPE_FILTER_VAL_TO_NAME[TYPE_FILTER_ENTRIES[i]]);
                    resBuilder.Append(" | ");
                }
            }
            resBuilder.Remove(resBuilder.Length - 3, 3);

            return resBuilder.ToString();
        }

    }
}
