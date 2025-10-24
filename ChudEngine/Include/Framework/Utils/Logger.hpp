// Source/Framework/Core/Logger.h
#pragma once

#include <iostream>
#include <string>
#include <format>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>

namespace CE
    {
    enum class LogLevel
        {
        Trace,
        Debug,  // Добавлен уровень Debug
        Info,
        Warn,
        Error,
        Critical
        };

    class Logger
        {
        private:
            static std::ofstream logFile;
            static std::mutex logMutex;
            static bool writeToFile;
            static std::string logFileName;
            static LogLevel currentLogLevel;

            static std::string GetCurrentTime ();
            static void WriteToFile ( const std::string & message );
            static bool ShouldLog ( LogLevel level );

        public:
            static void Init ( const std::string & fileName = "app.log",
                               bool enableFileLogging = true,
                               LogLevel level = LogLevel::Info );
            static void Shutdown ();
            static void SetLogLevel ( LogLevel level );
            static LogLevel GetLogLevel ();
            static void Log ( LogLevel level, const std::string & message );

            // Core logger methods
            static inline void CoreTrace ( const std::string & message )
                { if (ShouldLog ( LogLevel::Trace )) Log ( LogLevel::Trace, "[CORE] " + message ); }
            static inline void CoreDebug ( const std::string & message )
                { if (ShouldLog ( LogLevel::Debug )) Log ( LogLevel::Debug, "[CORE] " + message ); }
            static inline void CoreInfo ( const std::string & message )
                { if (ShouldLog ( LogLevel::Info )) Log ( LogLevel::Info, "[CORE] " + message ); }
            static inline void CoreWarn ( const std::string & message )
                { if (ShouldLog ( LogLevel::Warn )) Log ( LogLevel::Warn, "[CORE] " + message ); }
            static inline void CoreError ( const std::string & message )
                { if (ShouldLog ( LogLevel::Error )) Log ( LogLevel::Error, "[CORE] " + message ); }
            static inline void CoreCritical ( const std::string & message )
                { if (ShouldLog ( LogLevel::Critical )) Log ( LogLevel::Critical, "[CORE] " + message ); }

            // Client logger methods
            static inline void ClientTrace ( const std::string & message )
                { if (ShouldLog ( LogLevel::Trace )) Log ( LogLevel::Trace, "[APP] " + message ); }
            static inline void ClientDebug ( const std::string & message )
                { if (ShouldLog ( LogLevel::Debug )) Log ( LogLevel::Debug, "[APP] " + message ); }
            static inline void ClientInfo ( const std::string & message )
                { if (ShouldLog ( LogLevel::Info )) Log ( LogLevel::Info, "[APP] " + message ); }
            static inline void ClientWarn ( const std::string & message )
                { if (ShouldLog ( LogLevel::Warn )) Log ( LogLevel::Warn, "[APP] " + message ); }
            static inline void ClientError ( const std::string & message )
                { if (ShouldLog ( LogLevel::Error )) Log ( LogLevel::Error, "[APP] " + message ); }
            static inline void ClientCritical ( const std::string & message )
                { if (ShouldLog ( LogLevel::Critical )) Log ( LogLevel::Critical, "[APP] " + message ); }
        };

    } // namespace CE

// Макросы для Core логирования
#define CE_CORE_TRACE(...)    ::CE::Logger::CoreTrace(std::format(__VA_ARGS__))
#define CE_CORE_DEBUG(...)    ::CE::Logger::CoreDebug(std::format(__VA_ARGS__))
#define CE_CORE_INFO(...)     ::CE::Logger::CoreInfo(std::format(__VA_ARGS__))
#define CE_CORE_WARN(...)     ::CE::Logger::CoreWarn(std::format(__VA_ARGS__))
#define CE_CORE_ERROR(...)    ::CE::Logger::CoreError(std::format(__VA_ARGS__))
#define CE_CORE_CRITICAL(...) ::CE::Logger::CoreCritical(std::format(__VA_ARGS__))

// Макросы для Client логирования
#define CE_TRACE(...)         ::CE::Logger::ClientTrace(std::format(__VA_ARGS__))
#define CE_DEBUG(...)         ::CE::Logger::ClientDebug(std::format(__VA_ARGS__))
#define CE_INFO(...)          ::CE::Logger::ClientInfo(std::format(__VA_ARGS__))
#define CE_WARN(...)          ::CE::Logger::ClientWarn(std::format(__VA_ARGS__))
#define CE_ERROR(...)         ::CE::Logger::ClientError(std::format(__VA_ARGS__))
#define CE_CRITICAL(...)      ::CE::Logger::ClientCritical(std::format(__VA_ARGS__))