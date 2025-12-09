/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using NLog.Targets;

namespace LSEG.Ema.Access
{
	internal class LoggerConfig
	{
        // Name of this Logger
        public string Name { get; set; } = string.Empty;
        public string FileName { get; set; } = string.Empty;
        public ulong IncludeDateInLoggerOutput { get; set; }
        public ulong NumberOfLogFiles { get; set; }
        public ulong MaxLogFileSize { get; set; }
        public LoggerLevel LoggerSeverity { get; set; }
        public LoggerType LoggerType { get; set; }
        public FileArchivePeriod FileArchivePeriod { get; set; } = FileArchivePeriod.None;
        
        public LoggerConfig()
        {
            Clear();
        }

        public LoggerConfig(LoggerConfig oldConfig)
        {
            Name = oldConfig.Name;
            IncludeDateInLoggerOutput = oldConfig.IncludeDateInLoggerOutput;
            NumberOfLogFiles = oldConfig.NumberOfLogFiles;
            MaxLogFileSize = oldConfig.MaxLogFileSize;
            LoggerSeverity = oldConfig.LoggerSeverity;
            LoggerType = oldConfig.LoggerType;
            FileArchivePeriod = oldConfig.FileArchivePeriod;
        }

        // Clears the Logger info and sets the default options.
        public void Clear()
        {
            Name = string.Empty;
            FileName = "emaLog";
            IncludeDateInLoggerOutput = 0;
            NumberOfLogFiles = 0;
            MaxLogFileSize = 0;
            LoggerSeverity = LoggerLevel.INFO;
            LoggerType = LoggerType.FILE;
            FileArchivePeriod = FileArchivePeriod.None;
        }

        public void Copy(LoggerConfig destConfig)
        {
            destConfig.Name = Name;
            destConfig.IncludeDateInLoggerOutput = IncludeDateInLoggerOutput;
            destConfig.NumberOfLogFiles = NumberOfLogFiles;
            destConfig.MaxLogFileSize = MaxLogFileSize;
            destConfig.FileName = FileName;
            destConfig.LoggerSeverity = LoggerSeverity;
            destConfig.LoggerType = LoggerType;
            destConfig.FileArchivePeriod = FileArchivePeriod;
        }

        internal static LoggerLevel StringToLoggerLevel(string logLevel) => logLevel switch
        {
            "Trace"     => LoggerLevel.TRACE,
            "Verbose"   => LoggerLevel.TRACE,
            "Info"      => LoggerLevel.INFO,
            "Success"   => LoggerLevel.INFO,
            "Warning"   => LoggerLevel.WARNING,
            "Error"     => LoggerLevel.ERROR,
            "NoLogMsg"  => LoggerLevel.OFF,
            _           => throw new OmmInvalidConfigurationException("Logger Severity: " + logLevel + " not recognized. Acceptable inputs: \"Trace\", \"Info\" or \"Success\", \"Warning\", \"Error\" or \"Verbose\", \"NoLogMsg\".")
        };

        internal static LoggerType StringToLoggerType(string logType) => logType switch
        {
            "File"      => LoggerType.FILE,
            "Stdout"    => LoggerType.STDOUT,
            _           => throw new OmmInvalidConfigurationException("Logger Type: " + logType + " not recognized. Acceptable inputs: \"File\", \"Stdout\".")
        };

        internal static FileArchivePeriod StringToFileArchivePeriod(string fileArchivePeriod) => fileArchivePeriod switch
        {
            "Day" => FileArchivePeriod.Day,
            "Hour" => FileArchivePeriod.Hour,
            "Minute" => FileArchivePeriod.Minute,
            "Month" => FileArchivePeriod.Month,
            "Year" => FileArchivePeriod.Year,
            "None" => FileArchivePeriod.None,
            "Sunday" => FileArchivePeriod.Sunday,
            "Monday" => FileArchivePeriod.Monday,
            "Tuesday" => FileArchivePeriod.Tuesday,
            "Wednesday" => FileArchivePeriod.Wednesday,
            "Thursday" => FileArchivePeriod.Thursday,
            "Friday" => FileArchivePeriod.Friday,
            "Saturday" => FileArchivePeriod.Saturday,
            _ => throw new OmmInvalidConfigurationException("File Archive Period: " + fileArchivePeriod + " not recognized. Acceptable inputs are of the form \"FileArchivePeriod::<Period>\", " +
                "where Period is from NLog.Targets.FileArchivePeriod, e.g. FileArchivePeriod::Month..")
        };
    }
}