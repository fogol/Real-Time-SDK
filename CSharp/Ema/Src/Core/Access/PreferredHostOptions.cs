/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Text;

namespace LSEG.Ema.Access;

/// <summary>
/// Used to specify Preferred Host configuration options.
/// </summary>
///
/// <see cref="OmmConsumer.ModifyIOCtl(LSEG.Ema.Access.IOCtlCode, object)"/>
/// <see cref="IOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS"/>
public sealed class PreferredHostOptions
{
    #region Public members

    /// <summary>
    /// Consturcts new preferred host options instance with default values.
    /// </summary>
    public PreferredHostOptions()
    {
        Clear();
    }

    /// Indicates whether preferred host feature is configured for this channel.
    ///
    /// <value>true if preferred host is enabled; false otherwise</value>
    public bool EnablePreferredHostOptions { get; set; } = false;

    /// Returns Cron time schedule to switch over to a preferred host or WSB group.
    ///
    ///  <value>time format to switch over</value>
    public string DetectionTimeSchedule { get; set; } = string.Empty;

    /// Returns time interval in second to switch over to a preferred host or WSB group.
    ///
    /// <value>time interval to switch over</value>
    public long DetectionTimeInterval { get; set; } = 0;

    /// Returns a channel name to set a preferred host.
    /// 
    /// <value>the preferred host channel name</value>
    public string ChannelName { get; set; } = string.Empty;

    /// <summary>
    /// Gets or sets the session channel name for the preferred host options.
    /// </summary>
    public string SessionChannelName { get; set; } = string.Empty;

    /// Clears this object to default.
    public void Clear()
    {
        EnablePreferredHostOptions = false;
        DetectionTimeSchedule = string.Empty;
        DetectionTimeInterval = 0;
        ChannelName = string.Empty;
        SessionChannelName = string.Empty;
    }

    /// <summary>
    /// Returns textual representation of this instance parameters.
    /// </summary>
    public override string ToString()
    {
        if (m_StringBuilder is null)
        {
            m_StringBuilder = new StringBuilder(256);
        }
        else
        {
            m_StringBuilder.Clear();
        }

        m_StringBuilder.AppendLine($"\t\tEnablePreferredHostOptions={EnablePreferredHostOptions}");
        m_StringBuilder.AppendLine($"\t\tPHDetectionTimeSchedule='{DetectionTimeSchedule}'");
        m_StringBuilder.AppendLine($"\t\tPHDetectionTimeInterval={DetectionTimeInterval}");
        m_StringBuilder.AppendLine($"\t\tPreferredChannelName='{ChannelName}'");
        m_StringBuilder.AppendLine($"\t\tSessionChannelName='{SessionChannelName}'");

        return m_StringBuilder.ToString();
    }

    #endregion

    #region Implementation details

    private StringBuilder? m_StringBuilder;

    #endregion

}
