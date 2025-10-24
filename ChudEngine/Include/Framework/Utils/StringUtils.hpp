#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace CE
    {

    class StringUtils
        {
        public:
            // String manipulation
            static std::string ToLower ( const std::string & str );
            static std::string ToUpper ( const std::string & str );
            static std::string Trim ( const std::string & str );
            static std::string TrimLeft ( const std::string & str );
            static std::string TrimRight ( const std::string & str );

            // String splitting/joining
            static std::vector<std::string> Split ( const std::string & str, char delimiter );
            static std::vector<std::string> Split ( const std::string & str, const std::string & delimiter );
            static std::string Join ( const std::vector<std::string> & strings, const std::string & delimiter );

            // String searching
            static bool StartsWith ( const std::string & str, const std::string & prefix );
            static bool EndsWith ( const std::string & str, const std::string & suffix );
            static bool Contains ( const std::string & str, const std::string & substring );
            static size_t Count ( const std::string & str, char character );

            // String replacement
            static std::string Replace ( const std::string & str, const std::string & from, const std::string & to );
            static std::string ReplaceAll ( const std::string & str, const std::string & from, const std::string & to );

            // String formatting
            static std::string Format ( const std::string & format, ... );

            // String validation
            static bool IsNumber ( const std::string & str );
            static bool IsInteger ( const std::string & str );
            static bool IsFloat ( const std::string & str );
            static bool IsWhitespace ( const std::string & str );

            // Encoding/decoding
            static std::string UrlEncode ( const std::string & str );
            static std::string UrlDecode ( const std::string & str );
            static std::string Base64Encode ( const std::string & str );
            static std::string Base64Decode ( const std::string & str );

            // Utility functions
            static std::string Repeat ( const std::string & str, int count );
            static std::string PadLeft ( const std::string & str, size_t length, char padChar = ' ' );
            static std::string PadRight ( const std::string & str, size_t length, char padChar = ' ' );

            // Conversion functions
            static int ToInt ( const std::string & str );
            static float ToFloat ( const std::string & str );
            static double ToDouble ( const std::string & str );
            static bool ToBool ( const std::string & str );

        private:
            static const std::string BASE64_CHARS;
        };

    } // namespace ChudEngine