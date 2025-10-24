#include "Utils/FileSystem.hpp"
#include "Utils/Logger.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <limits.h>
#include <pwd.h>
#include <sys/statvfs.h>
#endif

namespace CE
    {

    std::string FileSystem::s_AssetsDirectory;
    std::string FileSystem::s_LogsDirectory;
    std::string FileSystem::s_ConfigDirectory;

    bool FileSystem::Exists ( const std::string & path ) {
        try
            {
            return std::filesystem::exists ( path );
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error checking existence of '{}': {}", path, e.what () );
                return false;
                }
        }

    bool FileSystem::IsFile ( const std::string & path ) {
        try
            {
            return std::filesystem::is_regular_file ( path );
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error checking if '{}' is file: {}", path, e.what () );
                return false;
                }
        }

    bool FileSystem::IsDirectory ( const std::string & path ) {
        try
            {
            return std::filesystem::is_directory ( path );
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error checking if '{}' is directory: {}", path, e.what () );
                return false;
                }
        }

    uint64_t FileSystem::GetFileSize ( const std::string & path ) {
        try
            {
            if (IsFile ( path ))
                {
                return std::filesystem::file_size ( path );
                }
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error getting size of '{}': {}", path, e.what () );
                }
            return 0;
        }

    std::string FileSystem::GetExtension ( const std::string & path ) {
        try
            {
            return std::filesystem::path ( path ).extension ().string ();
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error getting extension of '{}': {}", path, e.what () );
                return "";
                }
        }

    std::string FileSystem::GetFilename ( const std::string & path ) {
        try
            {
            return std::filesystem::path ( path ).filename ().string ();
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error getting filename of '{}': {}", path, e.what () );
                return "";
                }
        }

    std::string FileSystem::GetStem ( const std::string & path ) {
        try
            {
            return std::filesystem::path ( path ).stem ().string ();
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error getting stem of '{}': {}", path, e.what () );
                return "";
                }
        }

    std::string FileSystem::GetParentPath ( const std::string & path ) {
        try
            {
            return std::filesystem::path ( path ).parent_path ().string ();
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error getting parent path of '{}': {}", path, e.what () );
                return "";
                }
        }

    std::string FileSystem::ReadTextFile ( const std::string & path ) {
        if (!Exists ( path ) || !IsFile ( path ))
            {
            CE_ERROR ( "Cannot read text file '{}': File does not exist or is not a file", path );
            return "";
            }

        try
            {
            std::ifstream file ( path, std::ios::in );
            if (!file.is_open ())
                {
                CE_ERROR ( "Failed to open text file '{}' for reading", path );
                return "";
                }

            std::stringstream buffer;
            buffer << file.rdbuf ();
            return buffer.str ();
            }
            catch (const std::exception & e)
                {
                CE_ERROR ( "Error reading text file '{}': {}", path, e.what () );
                return "";
                }
        }

    std::vector<uint8_t> FileSystem::ReadBinaryFile ( const std::string & path ) {
        if (!Exists ( path ) || !IsFile ( path ))
            {
            CE_ERROR ( "Cannot read binary file '{}': File does not exist or is not a file", path );
            return {};
            }

        try
            {
            std::ifstream file ( path, std::ios::in | std::ios::binary | std::ios::ate );
            if (!file.is_open ())
                {
                CE_ERROR ( "Failed to open binary file '{}' for reading", path );
                return {};
                }

            std::streamsize size = file.tellg ();
            file.seekg ( 0, std::ios::beg );

            std::vector<uint8_t> buffer ( size );
            if (!file.read ( reinterpret_cast< char * >( buffer.data () ), size ))
                {
                CE_ERROR ( "Failed to read binary file '{}'", path );
                return {};
                }

            return buffer;
            }
            catch (const std::exception & e)
                {
                CE_ERROR ( "Error reading binary file '{}': {}", path, e.what () );
                return {};
                }
        }

    bool FileSystem::WriteTextFile ( const std::string & path, const std::string & content ) {
        try
            {
            CreateDirectories ( GetParentPath ( path ) );

            std::ofstream file ( path, std::ios::out | std::ios::trunc );
            if (!file.is_open ())
                {
                CE_ERROR ( "Failed to open file '{}' for writing", path );
                return false;
                }

            file << content;
            return true;
            }
            catch (const std::exception & e)
                {
                CE_ERROR ( "Error writing text file '{}': {}", path, e.what () );
                return false;
                }
        }

    bool FileSystem::WriteBinaryFile ( const std::string & path, const std::vector<uint8_t> & data ) {
        try
            {
            CreateDirectories ( GetParentPath ( path ) );

            std::ofstream file ( path, std::ios::out | std::ios::binary | std::ios::trunc );
            if (!file.is_open ())
                {
                CE_ERROR ( "Failed to open file '{}' for binary writing", path );
                return false;
                }

            file.write ( reinterpret_cast< const char * >( data.data () ), data.size () );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_ERROR ( "Error writing binary file '{}': {}", path, e.what () );
                return false;
                }
        }

    bool FileSystem::CreateDirectoryS ( const std::string & path ) {
        try
            {
            return std::filesystem::create_directory ( path );
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error creating directory '{}': {}", path, e.what () );
                return false;
                }
        }

    bool FileSystem::CreateDirectories ( const std::string & path ) {
        try
            {
            return std::filesystem::create_directories ( path );
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error creating directories '{}': {}", path, e.what () );
                return false;
                }
        }

    bool FileSystem::Delete ( const std::string & path ) {
        try
            {
            if (Exists ( path ))
                {
                std::filesystem::remove_all ( path );
                return true;
                }
            return false;
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error deleting '{}': {}", path, e.what () );
                return false;
                }
        }

    bool FileSystem::Rename ( const std::string & oldPath, const std::string & newPath ) {
        try
            {
            if (Exists ( oldPath ))
                {
                std::filesystem::rename ( oldPath, newPath );
                return true;
                }
            return false;
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error renaming '{}' to '{}': {}", oldPath, newPath, e.what () );
                return false;
                }
        }

    bool FileSystem::Copy ( const std::string & source, const std::string & destination ) {
        try
            {
            if (Exists ( source ))
                {
                CreateDirectories ( GetParentPath ( destination ) );
                std::filesystem::copy ( source, destination, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing );
                return true;
                }
            return false;
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error copying '{}' to '{}': {}", source, destination, e.what () );
                return false;
                }
        }

    std::vector<std::string> FileSystem::GetFiles ( const std::string & directory, bool recursive ) {
        std::vector<std::string> files;

        if (!Exists ( directory ) || !IsDirectory ( directory ))
            {
            CE_ERROR ( "Directory '{}' does not exist or is not a directory", directory );
            return files;
            }

        try
            {
            if (recursive)
                {
                for (const auto & entry : std::filesystem::recursive_directory_iterator ( directory ))
                    {
                    if (entry.is_regular_file ())
                        {
                        files.push_back ( entry.path ().string () );
                        }
                    }
                }
            else
                {
                for (const auto & entry : std::filesystem::directory_iterator ( directory ))
                    {
                    if (entry.is_regular_file ())
                        {
                        files.push_back ( entry.path ().string () );
                        }
                    }
                }
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error getting files from '{}': {}", directory, e.what () );
                }

            return files;
        }

    std::vector<std::string> FileSystem::GetDirectories ( const std::string & directory, bool recursive ) {
        std::vector<std::string> directories;

        if (!Exists ( directory ) || !IsDirectory ( directory ))
            {
            CE_ERROR ( "Directory '{}' does not exist or is not a directory", directory );
            return directories;
            }

        try
            {
            if (recursive)
                {
                for (const auto & entry : std::filesystem::recursive_directory_iterator ( directory ))
                    {
                    if (entry.is_directory ())
                        {
                        directories.push_back ( entry.path ().string () );
                        }
                    }
                }
            else
                {
                for (const auto & entry : std::filesystem::directory_iterator ( directory ))
                    {
                    if (entry.is_directory ())
                        {
                        directories.push_back ( entry.path ().string () );
                        }
                    }
                }
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error getting directories from '{}': {}", directory, e.what () );
                }

            return directories;
        }

    std::vector<std::string> FileSystem::GetEntries ( const std::string & directory, bool recursive ) {
        std::vector<std::string> entries;

        if (!Exists ( directory ) || !IsDirectory ( directory ))
            {
            CE_ERROR ( "Directory '{}' does not exist or is not a directory", directory );
            return entries;
            }

        try
            {
            if (recursive)
                {
                for (const auto & entry : std::filesystem::recursive_directory_iterator ( directory ))
                    {
                    entries.push_back ( entry.path ().string () );
                    }
                }
            else
                {
                for (const auto & entry : std::filesystem::directory_iterator ( directory ))
                    {
                    entries.push_back ( entry.path ().string () );
                    }
                }
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error getting entries from '{}': {}", directory, e.what () );
                }

            return entries;
        }

    std::vector<std::string> FileSystem::FindFiles ( const std::string & directory, const std::string & pattern, bool recursive ) {
        std::vector<std::string> foundFiles;
        auto files = GetFiles ( directory, recursive );

        for (const auto & file : files)
            {
            std::string filename = GetFilename ( file );
            if (filename.find ( pattern ) != std::string::npos)
                {
                foundFiles.push_back ( file );
                }
            }

        return foundFiles;
        }

    std::string FileSystem::FindFile ( const std::string & directory, const std::string & filename, bool recursive ) {
        auto files = GetFiles ( directory, recursive );

        for (const auto & file : files)
            {
            if (GetFilename ( file ) == filename)
                {
                return file;
                }
            }

        return "";
        }

    std::string FileSystem::Combine ( const std::string & path1, const std::string & path2 ) {
        try
            {
            return ( std::filesystem::path ( path1 ) / path2 ).string ();
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error combining paths '{}' and '{}': {}", path1, path2, e.what () );
                return path1 + "/" + path2;
                }
        }

    std::string FileSystem::Combine ( const std::vector<std::string> & paths ) {
        if (paths.empty ()) return "";

        std::filesystem::path result ( paths[ 0 ] );
        for (size_t i = 1; i < paths.size (); ++i)
            {
            result /= paths[ i ];
            }
        return result.string ();
        }

    std::string FileSystem::Normalize ( const std::string & path ) {
        try
            {
            return std::filesystem::path ( path ).lexically_normal ().string ();
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error normalizing path '{}': {}", path, e.what () );
                return path;
                }
        }

    std::string FileSystem::Absolute ( const std::string & path ) {
        try
            {
            return std::filesystem::absolute ( path ).string ();
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error getting absolute path for '{}': {}", path, e.what () );
                return path;
                }
        }

    std::string FileSystem::Relative ( const std::string & path, const std::string & base ) {
        try
            {
            return std::filesystem::relative ( path, base ).string ();
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error getting relative path for '{}' from '{}': {}", path, base, e.what () );
                return path;
                }
        }

    std::chrono::system_clock::time_point FileSystem::GetLastWriteTime ( const std::string & path ) {
        try
            {
            if (Exists ( path ))
                {
                auto ftime = std::filesystem::last_write_time ( path );
                return std::chrono::clock_cast< std::chrono::system_clock >( ftime );
                }
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error getting last write time for '{}': {}", path, e.what () );
                }
            return std::chrono::system_clock::time_point ();
        }

    std::chrono::system_clock::time_point FileSystem::GetCreationTime ( const std::string & path ) {
        try
            {
            if (Exists ( path ))
                {
                auto ftime = std::filesystem::last_write_time ( path );
                return std::chrono::clock_cast< std::chrono::system_clock >( ftime );
                }
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error getting creation time for '{}': {}", path, e.what () );
                }
            return std::chrono::system_clock::time_point ();
        }

    bool FileSystem::IsAbsolutePath ( const std::string & path ) {
        try
            {
            return std::filesystem::path ( path ).is_absolute ();
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error checking if path '{}' is absolute: {}", path, e.what () );
                return false;
                }
        }

    bool FileSystem::HasExtension ( const std::string & path ) {
        try
            {
            return !std::filesystem::path ( path ).extension ().empty ();
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error checking if path '{}' has extension: {}", path, e.what () );
                return false;
                }
        }

    void FileSystem::SetWorkingDirectory ( const std::string & path ) {
        try
            {
            std::filesystem::current_path ( path );
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error setting working directory to '{}': {}", path, e.what () );
                }
        }

    std::string FileSystem::GetWorkingDirectory () {
        try
            {
            return std::filesystem::current_path ().string ();
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error getting working directory: {}", e.what () );
                return "";
                }
        }

    std::string FileSystem::GetExecutablePath () {
#ifdef _WIN32
        char path[ MAX_PATH ];
        if (GetModuleFileNameA ( nullptr, path, MAX_PATH ) != 0)
            {
            return std::string ( path );
            }
#else
        char path[ PATH_MAX ];
        ssize_t count = readlink ( "/proc/self/exe", path, PATH_MAX );
        if (count != -1)
            {
            return std::string ( path, count );
            }
#endif
        CE_ERROR ( "Failed to get executable path" );
        return "";
        }

    std::string FileSystem::GetAssetsDirectory () {
        if (s_AssetsDirectory.empty ())
            {
            InitializeSpecialDirectories ();
            }
        return s_AssetsDirectory;
        }

    std::string FileSystem::GetLogsDirectory () {
        if (s_LogsDirectory.empty ())
            {
            InitializeSpecialDirectories ();
            }
        return s_LogsDirectory;
        }

    std::string FileSystem::GetConfigDirectory () {
        if (s_ConfigDirectory.empty ())
            {
            InitializeSpecialDirectories ();
            }
        return s_ConfigDirectory;
        }

    std::string FileSystem::GetTempDirectory () {
        try
            {
            return std::filesystem::temp_directory_path ().string ();
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error getting temp directory: {}", e.what () );
                return "";
                }
        }

    uint64_t FileSystem::GetAvailableSpace ( const std::string & path ) {
        try
            {
            auto space = std::filesystem::space ( path );
            return space.available;
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error getting available space for '{}': {}", path, e.what () );
                return 0;
                }
        }

    uint64_t FileSystem::GetTotalSpace ( const std::string & path ) {
        try
            {
            auto space = std::filesystem::space ( path );
            return space.capacity;
            }
            catch (const std::filesystem::filesystem_error & e)
                {
                CE_ERROR ( "FileSystem error getting total space for '{}': {}", path, e.what () );
                return 0;
                }
        }

    void FileSystem::InitializeSpecialDirectories () {
        // Get executable directory as base
        std::string exePath = GetExecutablePath ();
        std::string exeDir = GetParentPath ( exePath );

        // Assets directory - next to executable
        s_AssetsDirectory = Combine ( exeDir, "Assets" );
        CreateDirectories ( s_AssetsDirectory );

        // Logs directory - in executable directory
        s_LogsDirectory = Combine ( exeDir, "Logs" );
        CreateDirectories ( s_LogsDirectory );

        // Config directory - platform specific
#ifdef _WIN32
        char appDataPath[ MAX_PATH ];
        if (SUCCEEDED ( SHGetFolderPathA ( nullptr, CSIDL_APPDATA, nullptr, 0, appDataPath ) ))
            {
            s_ConfigDirectory = Combine ( appDataPath, "ChudEngine" );
            }
        else
            {
            s_ConfigDirectory = Combine ( exeDir, "Config" );
            }
#else
        const char * homeDir = getenv ( "HOME" );
        if (homeDir)
            {
            s_ConfigDirectory = Combine ( homeDir, ".config", "ChudEngine" );
            }
        else
            {
            s_ConfigDirectory = Combine ( exeDir, "Config" );
            }
#endif

        CreateDirectories ( s_ConfigDirectory );

        CE_DEBUG ( "Special directories initialized:" );
        CE_DEBUG ( "  Assets: {}", s_AssetsDirectory );
        CE_DEBUG ( "  Logs: {}", s_LogsDirectory );
        CE_DEBUG ( "  Config: {}", s_ConfigDirectory );
        }

    } // namespace ChudEngine