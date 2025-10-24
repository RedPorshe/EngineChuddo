#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <chrono>

namespace CE
    {

    class FileSystem
        {
        public:
            // File operations
            static bool Exists ( const std::string & path );
            static bool IsFile ( const std::string & path );
            static bool IsDirectory ( const std::string & path );
            static uint64_t GetFileSize ( const std::string & path );
            static std::string GetExtension ( const std::string & path );
            static std::string GetFilename ( const std::string & path );
            static std::string GetStem ( const std::string & path );
            static std::string GetParentPath ( const std::string & path );

            // File reading/writing
            static std::string ReadTextFile ( const std::string & path );
            static std::vector<uint8_t> ReadBinaryFile ( const std::string & path );
            static bool WriteTextFile ( const std::string & path, const std::string & content );
            static bool WriteBinaryFile ( const std::string & path, const std::vector<uint8_t> & data );

            // Directory operations
            static bool CreateDirectoryS ( const std::string & path );
            static bool CreateDirectories ( const std::string & path );
            static bool Delete ( const std::string & path );
            static bool Rename ( const std::string & oldPath, const std::string & newPath );
            static bool Copy ( const std::string & source, const std::string & destination );

            // Directory listing
            static std::vector<std::string> GetFiles ( const std::string & directory, bool recursive = false );
            static std::vector<std::string> GetDirectories ( const std::string & directory, bool recursive = false );
            static std::vector<std::string> GetEntries ( const std::string & directory, bool recursive = false );

            // File search
            static std::vector<std::string> FindFiles ( const std::string & directory, const std::string & pattern, bool recursive = false );
            static std::string FindFile ( const std::string & directory, const std::string & filename, bool recursive = false );

            // Path operations
            static std::string Combine ( const std::string & path1, const std::string & path2 );
            static std::string Combine ( const std::vector<std::string> & paths );
            static std::string Normalize ( const std::string & path );
            static std::string Absolute ( const std::string & path );
            static std::string Relative ( const std::string & path, const std::string & base );

            // File time operations
            static std::chrono::system_clock::time_point GetLastWriteTime ( const std::string & path );
            static std::chrono::system_clock::time_point GetCreationTime ( const std::string & path );

            // Utility functions
            static bool IsAbsolutePath ( const std::string & path );
            static bool HasExtension ( const std::string & path );
            static void SetWorkingDirectory ( const std::string & path );
            static std::string GetWorkingDirectory ();
            static std::string GetExecutablePath ();

            // Special directories
            static std::string GetAssetsDirectory ();
            static std::string GetLogsDirectory ();
            static std::string GetConfigDirectory ();
            static std::string GetTempDirectory ();

            // File system info
            static uint64_t GetAvailableSpace ( const std::string & path );
            static uint64_t GetTotalSpace ( const std::string & path );

        private:
            static std::string s_AssetsDirectory;
            static std::string s_LogsDirectory;
            static std::string s_ConfigDirectory;

            static void InitializeSpecialDirectories ();
        };

    } // namespace ChudEngine