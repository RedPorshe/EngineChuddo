#include "Logger.hpp"
#include "FileSystem.hpp"

// Определение статических членов
namespace CE
    {
    std::ofstream Logger::logFile;
    std::mutex Logger::logMutex;
    bool Logger::writeToFile = false;
    std::string Logger::logFileName = "app.log";
    LogLevel Logger::currentLogLevel = LogLevel::Info;

    bool Logger::ShouldLog ( LogLevel level )
        {
        return static_cast< int >( level ) >= static_cast< int >( currentLogLevel );
        }

    std::string Logger::GetCurrentTime ()
        {
        auto now = std::chrono::system_clock::now ();
        auto time_t = std::chrono::system_clock::to_time_t ( now );
        auto ms = std::chrono::duration_cast< std::chrono::milliseconds >(
            now.time_since_epoch () ) % 1000;

        std::stringstream ss;

        // Безопасная версия для Windows
#ifdef _WIN32
        std::tm tm;
        localtime_s ( &tm, &time_t );
        ss << std::put_time ( &tm, "%Y-%m-%d %H:%M:%S" );
#else
        ss << std::put_time ( std::localtime ( &time_t ), "%Y-%m-%d %H:%M:%S" );
#endif

        ss << "." << std::setfill ( '0' ) << std::setw ( 3 ) << ms.count ();
        return ss.str ();
        }

    void Logger::WriteToFile ( const std::string & message )
        {
        if (writeToFile && logFile.is_open ())
            {
            try
                {
                logFile << message << std::endl;
                logFile.flush ();
                }
                catch (const std::exception & e)
                    {
                    std::cerr << "Logger WriteToFile error: " << e.what () << std::endl;
                    }
            }
        }

    void Logger::Init ( const std::string & fileName, bool enableFileLogging, LogLevel level )
        {
        try
            {
            std::lock_guard<std::mutex> lock ( logMutex );

            currentLogLevel = level;

            if (logFile.is_open ())
                {
                logFile.close ();
                }

            writeToFile = enableFileLogging;
            logFileName = fileName;

            if (writeToFile)
                {
                std::string logsDir = FileSystem::GetLogsDirectory ();
                std::string fullLogPath = FileSystem::Combine ( logsDir, logFileName );

                FileSystem::CreateDirectories ( logsDir );

                logFile.open ( fullLogPath, std::ios::app );
                if (!logFile.is_open ())
                    {
                    std::cerr << "ERROR: Cannot open log file: " << fullLogPath << std::endl;
                    writeToFile = false;
                    }
                else
                    {
                    
                    logFile << "\n\n=== Logging started at " << GetCurrentTime () << " ===\n" << std::endl;
                    logFile << "=== Log level: " << static_cast< int >( level ) << " ===\n" << std::endl;
                    }
                }
            }
            catch (const std::exception & e)
                {
                std::cerr << "Logger Init error: " << e.what () << std::endl;
                writeToFile = false;
                }
        }

    void Logger::SetLogLevel ( LogLevel level )
        {
        std::lock_guard<std::mutex> lock ( logMutex );
        currentLogLevel = level;
        }

    LogLevel Logger::GetLogLevel ()
        {
        std::lock_guard<std::mutex> lock ( logMutex );
        return currentLogLevel;
        }

    void Logger::Shutdown ()
        {
        try
            {
            std::lock_guard<std::mutex> lock ( logMutex );

            if (logFile.is_open ())
                {
                logFile << "=== Logging stopped at " << GetCurrentTime () << " ===\n" << std::endl;
                logFile.close ();
                }
            }
            catch (const std::exception & e)
                {
                std::cerr << "Logger Shutdown error: " << e.what () << std::endl;
                }
        }

    void Logger::Log ( LogLevel level, const std::string & message )
        {
        try
            {
            if (!ShouldLog ( level ))
                return;

            std::lock_guard<std::mutex> lock ( logMutex );

            const char * levelStr = "";
            const char * color = "";

            switch (level)
                {
                    case LogLevel::Trace:    levelStr = "TRACE"; color = "\033[37m"; break;    // White
                    case LogLevel::Debug:    levelStr = "DEBUG"; color = "\033[36m"; break;    // Cyan (новый цвет для Debug)
                    case LogLevel::Info:     levelStr = "INFO";  color = "\033[32m"; break;    // Green
                    case LogLevel::Warn:     levelStr = "WARN";  color = "\033[33m"; break;    // Yellow
                    case LogLevel::Error:    levelStr = "ERROR"; color = "\033[31m"; break;    // Red
                    case LogLevel::Critical: levelStr = "CRITICAL"; color = "\033[41m"; break; // Red background
                }

            std::string timestamp = GetCurrentTime ();
            std::string formattedMessage = "[" + timestamp + "] [" + levelStr + "] " + message;

            // Вывод в консоль с цветом
            std::cout << color << formattedMessage << "\033[0m" << std::endl;

            // Запись в файл (без цветовых кодов)
            WriteToFile ( formattedMessage );
            }
            catch (const std::exception & e)
                {
                std::cerr << "Logger Log error: " << e.what () << std::endl;
                }
        }
    }