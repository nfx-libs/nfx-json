/*
 * MIT License
 *
 * Copyright (c) 2026 nfx
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file Parser.h
 * @brief High-performance JSON parser for text-to-DOM conversion
 * @details Parses JSON text into Document representation with full UTF-8 and Unicode support.
 *          Handles UTF-8 BOM, UTF-16 surrogate pairs, and all standard JSON escape sequences.
 *          Supports parsing integers, floating-point numbers, strings, arrays, and objects.
 */

#pragma once

#include "nfx/json/Document.h"

#include <nfx/string/Utils.h>

#include <charconv>
#include <cmath>
#include <stdexcept>
#include <string>
#include <string_view>

#if defined( __SSE2__ ) || defined( _M_X64 ) || ( defined( _M_IX86_FP ) && _M_IX86_FP >= 2 )
#    include <emmintrin.h>
#    define NFX_JSON_PARSER_USE_SIMD 1
#else
#    define NFX_JSON_PARSER_USE_SIMD 0
#endif

#if defined( _MSC_VER )
#    include <intrin.h>
#endif

namespace nfx::json
{
    namespace
    {
        // String parsing constants
        constexpr size_t STRING_INITIAL_CAPACITY = 64; // Typical JSON string length
        constexpr size_t SIMD_CHUNK_SIZE = 16;         // SSE2 processes 16 bytes at a time

        /**
         * @brief Count trailing zeros in an integer (platform-independent)
         * @param value Input value to count trailing zeros
         * @return Number of trailing zero bits
         */
        inline int countTrailingZeros( unsigned int value )
        {
#if defined( _MSC_VER )
            // MSVC intrinsic
            if( value == 0 )
                return 32;
            unsigned long index;
            _BitScanForward( &index, value );
            return static_cast<int>( index );
#elif defined( __GNUC__ ) || defined( __clang__ )
            // GCC/Clang built-in
            return __builtin_ctz( value );
#else
            // Fallback implementation
            if( value == 0 )
                return 32;
            int count = 0;
            while( ( value & 1 ) == 0 )
            {
                value >>= 1;
                ++count;
            }
            return count;
#endif
        }

        // UTF-8 BOM (Byte Order Mark) - https://www.unicode.org/faq/utf_bom.html
        // EF BB BF at start of file indicates UTF-8 encoding
        constexpr uint8_t UTF8_BOM_BYTE1 = 0xEF;
        constexpr uint8_t UTF8_BOM_BYTE2 = 0xBB;
        constexpr uint8_t UTF8_BOM_BYTE3 = 0xBF;
        constexpr size_t UTF8_BOM_LENGTH = 3;

        // UTF-8 encoding constants (RFC 3629 - https://www.rfc-editor.org/rfc/rfc3629)
        // UTF-8 uses 1-4 bytes to encode Unicode code points
        constexpr uint32_t UTF8_1BYTE_MAX = 0x80;     // U+0000 to U+007F: 0xxxxxxx
        constexpr uint32_t UTF8_2BYTE_MAX = 0x800;    // U+0080 to U+07FF: 110xxxxx 10xxxxxx
        constexpr uint32_t UTF8_3BYTE_MAX = 0x10000;  // U+0800 to U+FFFF: 1110xxxx 10xxxxxx 10xxxxxx
        constexpr uint32_t UTF8_4BYTE_MAX = 0x110000; // U+10000 to U+10FFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

        // UTF-8 byte structure constants (RFC 3629)
        constexpr uint8_t UTF8_CONTINUATION_BYTE = 0x80; // 10xxxxxx - continuation byte lead bits
        constexpr uint8_t UTF8_CONTINUATION_MASK = 0x3F; // 00111111 - extracts 6 data bits from continuation byte
        constexpr uint8_t UTF8_2BYTE_LEAD = 0xC0;        // 11000000 - 2-byte sequence lead byte marker
        constexpr uint8_t UTF8_3BYTE_LEAD = 0xE0;        // 11100000 - 3-byte sequence lead byte marker
        constexpr uint8_t UTF8_4BYTE_LEAD = 0xF0;        // 11110000 - 4-byte sequence lead byte marker

        // UTF-16 surrogate pair constants (RFC 2781 - https://www.rfc-editor.org/rfc/rfc2781)
        // Used to encode characters U+10000 to U+10FFFF (beyond BMP) in JSON \uXXXX escapes
        constexpr uint32_t UTF16_HIGH_SURROGATE_START = 0xD800; // High surrogate range: D800-DBFF
        constexpr uint32_t UTF16_HIGH_SURROGATE_END = 0xDBFF;
        constexpr uint32_t UTF16_LOW_SURROGATE_START = 0xDC00; // Low surrogate range: DC00-DFFF
        constexpr uint32_t UTF16_LOW_SURROGATE_END = 0xDFFF;
        constexpr uint32_t UTF16_SURROGATE_OFFSET = 0x10000; // Offset added when combining surrogates
    } // namespace

    /**
     * @brief High-performance JSON parser
     * @details Parses JSON text into Document representation
     */
    class Parser
    {
    public:
        /**
         * @brief Parse JSON string
         * @param json JSON text to parse
         * @return Parsed Document
         * @throws std::runtime_error on parse error
         */
        static Document parse( std::string_view json )
        {
            // Skip UTF-8 BOM if present
            if( json.size() >= UTF8_BOM_LENGTH && static_cast<unsigned char>( json[0] ) == UTF8_BOM_BYTE1 &&
                static_cast<unsigned char>( json[1] ) == UTF8_BOM_BYTE2 &&
                static_cast<unsigned char>( json[2] ) == UTF8_BOM_BYTE3 )
            {
                json.remove_prefix( UTF8_BOM_LENGTH );
            }

            Parser parser{ json };
            return parser.parseValue();
        }

    private:
        /**
         * @brief Private constructor for internal use
         * @param json JSON text to parse
         */
        explicit Parser( std::string_view json )
            : m_json{ json },
              m_pos{ 0 }
        {
        }

        /**
         * @brief Parse UTF-16 surrogate pair and return combined code point
         * @param highSurrogate The high surrogate value (0xD800-0xDBFF)
         * @return Combined Unicode code point
         * @throws std::runtime_error on invalid surrogate pair
         */
        uint32_t parseSurrogatePair( uint32_t highSurrogate )
        {
            // High surrogate - need to get low surrogate
            m_pos += 4; // Skip the 4 hex digits of high surrogate

            // Check for \uXXXX pattern
            if( m_pos + 6 >= m_json.size() || m_json[m_pos + 1] != '\\' || m_json[m_pos + 2] != 'u' )
            {
                throw std::runtime_error{ "Invalid surrogate pair: missing low surrogate" };
            }

            // Parse low surrogate directly without string allocation
            const char* lowStart = m_json.data() + m_pos + 3;
            unsigned int lowSurrogate = 0;
            auto [lowPtr, lowEc] = std::from_chars( lowStart, lowStart + 4, lowSurrogate, 16 );
            if( lowEc != std::errc{} || lowPtr != lowStart + 4 )
            {
                throw std::runtime_error{ "Invalid unicode escape in low surrogate" };
            }

            // Validate low surrogate range
            if( lowSurrogate < UTF16_LOW_SURROGATE_START || lowSurrogate > UTF16_LOW_SURROGATE_END )
            {
                throw std::runtime_error{ "Invalid low surrogate value" };
            }

            // Combine surrogates to get actual code point
            // Formula: (high - 0xD800) * 0x400 + (low - 0xDC00) + 0x10000
            uint32_t codePoint = ( ( highSurrogate - UTF16_HIGH_SURROGATE_START ) << 10 ) +
                                 ( lowSurrogate - UTF16_LOW_SURROGATE_START ) + UTF16_SURROGATE_OFFSET;

            m_pos += 6; // Skip \uXXXX of low surrogate
            return codePoint;
        }

        /**
         * @brief Encode Unicode code point as UTF-8 and append to string
         * @param result String to append UTF-8 bytes to
         * @param codePoint Unicode code point to encode
         * @throws std::runtime_error on invalid code point
         */
        void encodeUtf8( std::string& result, uint32_t codePoint )
        {
            // UTF-8 encoding of the code point
            if( codePoint < UTF8_1BYTE_MAX )
            {
                // 1-byte sequence: 0xxxxxxx
                result.push_back( static_cast<char>( codePoint ) );
            }
            else if( codePoint < UTF8_2BYTE_MAX )
            {
                // 2-byte sequence: 110xxxxx 10xxxxxx
                result.push_back( static_cast<char>( UTF8_2BYTE_LEAD | ( codePoint >> 6 ) ) );
                result.push_back(
                    static_cast<char>( UTF8_CONTINUATION_BYTE | ( codePoint & UTF8_CONTINUATION_MASK ) ) );
            }
            else if( codePoint < UTF8_3BYTE_MAX )
            {
                // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
                result.push_back( static_cast<char>( UTF8_3BYTE_LEAD | ( codePoint >> 12 ) ) );
                result.push_back(
                    static_cast<char>( UTF8_CONTINUATION_BYTE | ( ( codePoint >> 6 ) & UTF8_CONTINUATION_MASK ) ) );
                result.push_back(
                    static_cast<char>( UTF8_CONTINUATION_BYTE | ( codePoint & UTF8_CONTINUATION_MASK ) ) );
            }
            else if( codePoint < UTF8_4BYTE_MAX )
            {
                // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                result.push_back( static_cast<char>( UTF8_4BYTE_LEAD | ( codePoint >> 18 ) ) );
                result.push_back(
                    static_cast<char>( UTF8_CONTINUATION_BYTE | ( ( codePoint >> 12 ) & UTF8_CONTINUATION_MASK ) ) );
                result.push_back(
                    static_cast<char>( UTF8_CONTINUATION_BYTE | ( ( codePoint >> 6 ) & UTF8_CONTINUATION_MASK ) ) );
                result.push_back(
                    static_cast<char>( UTF8_CONTINUATION_BYTE | ( codePoint & UTF8_CONTINUATION_MASK ) ) );
            }
            else
            {
                throw std::runtime_error{ "Invalid unicode code point" };
            }
        }

        /**
         * @brief Parse Unicode escape sequence (\uXXXX)
         * @param result String to append decoded UTF-8 bytes to
         * @throws std::runtime_error on invalid escape sequence or surrogate pair
         * @details Handles UTF-16 surrogate pairs for characters beyond the Basic Multilingual Plane
         */
        void parseUnicodeEscape( std::string& result )
        {
            // Unicode escape \uXXXX
            if( m_pos + 4 >= m_json.size() )
            {
                throw std::runtime_error{ "Invalid unicode escape" };
            }

            // Parse hex code point directly without string allocation
            const char* start = m_json.data() + m_pos + 1;
            unsigned int codePoint = 0;
            auto [ptr, ec] = std::from_chars( start, start + 4, codePoint, 16 );
            if( ec != std::errc{} || ptr != start + 4 )
            {
                throw std::runtime_error{ "Invalid unicode escape" };
            }

            // Check for UTF-16 surrogate pairs (for characters beyond BMP)
            if( codePoint >= UTF16_HIGH_SURROGATE_START && codePoint <= UTF16_HIGH_SURROGATE_END )
            {
                // Parse surrogate pair and get combined code point
                codePoint = parseSurrogatePair( codePoint );
            }
            else if( codePoint >= UTF16_LOW_SURROGATE_START && codePoint <= UTF16_LOW_SURROGATE_END )
            {
                // Low surrogate without high surrogate
                throw std::runtime_error{ "Invalid surrogate pair: unexpected low surrogate" };
            }
            else
            {
                m_pos += 4; // Skip the 4 hex digits
            }

            // Encode code point as UTF-8
            encodeUtf8( result, codePoint );
        }

        /**
         * @brief Process escape sequence in JSON string
         * @param result String to append unescaped character to
         * @throws std::runtime_error on invalid escape sequence
         * @note m_pos should be at the character after backslash; will be incremented after processing
         */
        void parseEscapeSequence( std::string& result )
        {
            if( m_pos >= m_json.size() )
            {
                throw std::runtime_error{ "Unexpected end in string escape" };
            }

            char escaped = m_json[m_pos];
            switch( escaped )
            {
                case '"':
                    result.push_back( '"' );
                    break;
                case '\\':
                    result.push_back( '\\' );
                    break;
                case '/':
                    result.push_back( '/' );
                    break;
                case 'b':
                    result.push_back( '\b' );
                    break;
                case 'f':
                    result.push_back( '\f' );
                    break;
                case 'n':
                    result.push_back( '\n' );
                    break;
                case 'r':
                    result.push_back( '\r' );
                    break;
                case 't':
                    result.push_back( '\t' );
                    break;
                case 'u':
                    parseUnicodeEscape( result );
                    break;
                default:
                    throw std::runtime_error{ std::string{ "Invalid escape sequence: \\" } + escaped };
            }
            ++m_pos;
        }

        /**
         * @brief Skip whitespace characters (space, tab, newline, carriage return)
         */
        void skipWhitespace()
        {
#if NFX_JSON_PARSER_USE_SIMD
            // SIMD fast path: scan for non-whitespace characters
            const __m128i space_vec = _mm_set1_epi8( ' ' );
            const __m128i tab_vec = _mm_set1_epi8( '\t' );
            const __m128i newline_vec = _mm_set1_epi8( '\n' );
            const __m128i cr_vec = _mm_set1_epi8( '\r' );

            while( m_pos + SIMD_CHUNK_SIZE <= m_json.size() )
            {
                // Load 16 bytes
                __m128i chunk = _mm_loadu_si128( reinterpret_cast<const __m128i*>( m_json.data() + m_pos ) );

                // Check if all bytes are whitespace
                __m128i is_space = _mm_cmpeq_epi8( chunk, space_vec );
                __m128i is_tab = _mm_cmpeq_epi8( chunk, tab_vec );
                __m128i is_newline = _mm_cmpeq_epi8( chunk, newline_vec );
                __m128i is_cr = _mm_cmpeq_epi8( chunk, cr_vec );

                __m128i whitespace =
                    _mm_or_si128( _mm_or_si128( is_space, is_tab ), _mm_or_si128( is_newline, is_cr ) );

                int mask = _mm_movemask_epi8( whitespace );

                if( mask == 0xFFFF )
                {
                    // All 16 bytes are whitespace - skip entire block
                    m_pos += SIMD_CHUNK_SIZE;
                }
                else
                {
                    // Found non-whitespace - count leading whitespace bytes
                    int non_ws_mask = ~mask & 0xFFFF;
                    int leading_ws = countTrailingZeros( static_cast<unsigned int>( non_ws_mask ) );
                    m_pos += leading_ws;
                    break;
                }
            }
#endif
            // Fallback path: byte-by-byte for remainder
            while( m_pos < m_json.size() )
            {
                char c = m_json[m_pos];
                if( c == ' ' || c == '\t' || c == '\n' || c == '\r' )
                {
                    ++m_pos;
                }
                else
                {
                    break;
                }
            }
        }

        //----------------------------------------------
        // Core parsing methods
        //----------------------------------------------

        /**
         * @brief Parse any JSON value (determines type and dispatches to specific parser)
         * @return Parsed Document containing the value
         * @throws std::runtime_error on unexpected character or end of input
         */
        inline Document parseValue()
        {
            skipWhitespace();

            if( m_pos >= m_json.size() )
            {
                throw std::runtime_error{ "Unexpected end of JSON" };
            }

            char c = m_json[m_pos];

            switch( c )
            {
                case 'n':
                    return parseNull();
                case 't':
                case 'f':
                    return parseBool();
                case '"':
                    return parseString();
                case '[':
                    return parseArray();
                case '{':
                    return parseObject();
                case '-':
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    return parseNumber();
                default:
                    throw std::runtime_error{ std::string{ "Unexpected character: " } + c };
            }
        }

        /**
         * @brief Parse JSON null value
         * @return Document containing null
         * @throws std::runtime_error if input doesn't match "null"
         */
        inline Document parseNull()
        {
            if( m_json.substr( m_pos, 4 ) != "null" )
            {
                throw std::runtime_error{ "Expected 'null'" };
            }
            m_pos += 4;
            return Document{ nullptr };
        }

        /**
         * @brief Parse JSON boolean value (true or false)
         * @return Document containing boolean
         * @throws std::runtime_error if input doesn't match "true" or "false"
         */
        inline Document parseBool()
        {
            if( m_json.substr( m_pos, 4 ) == "true" )
            {
                m_pos += 4;
                return Document{ true };
            }
            else if( m_json.substr( m_pos, 5 ) == "false" )
            {
                m_pos += 5;
                return Document{ false };
            }
            else
            {
                throw std::runtime_error{ "Expected 'true' or 'false'" };
            }
        }

        /**
         * @brief Parse JSON number (integer or floating-point)
         * @return Document containing int64_t, uint64_t, or double
         * @throws std::runtime_error on invalid number format
         * @details Supports scientific notation and attempts int64_t before falling back to uint64_t or double
         */
        inline Document parseNumber()
        {
            size_t start = m_pos;

            // Skip minus
            if( m_pos < m_json.size() && m_json[m_pos] == '-' )
            {
                ++m_pos;
            }

            // Skip digits
            while( m_pos < m_json.size() && nfx::string::isDigit( m_json[m_pos] ) )
            {
                ++m_pos;
            }

            bool isDouble = false;

            // Check for decimal point
            if( m_pos < m_json.size() && m_json[m_pos] == '.' )
            {
                isDouble = true;
                ++m_pos;
                while( m_pos < m_json.size() && nfx::string::isDigit( m_json[m_pos] ) )
                {
                    ++m_pos;
                }
            }

            // Check for exponent
            if( m_pos < m_json.size() && ( m_json[m_pos] == 'e' || m_json[m_pos] == 'E' ) )
            {
                isDouble = true;
                ++m_pos;
                if( m_pos < m_json.size() && ( m_json[m_pos] == '+' || m_json[m_pos] == '-' ) )
                {
                    ++m_pos;
                }
                while( m_pos < m_json.size() && nfx::string::isDigit( m_json[m_pos] ) )
                {
                    ++m_pos;
                }
            }

            std::string_view numStr = m_json.substr( start, m_pos - start );

            if( isDouble )
            {
                double value = 0.0;
                auto ec = std::from_chars( numStr.data(), numStr.data() + numStr.size(), value ).ec;
                if( ec != std::errc{} )
                {
                    throw std::runtime_error{ "Invalid number format" };
                }

                return Document{ value };
            }
            else
            {
                // Try parsing as int64_t first
                int64_t intValue = 0;
                auto ec = std::from_chars( numStr.data(), numStr.data() + numStr.size(), intValue ).ec;
                if( ec == std::errc{} )
                {
                    return Document{ intValue };
                }

                // Try as uint64_t
                uint64_t uintValue = 0;
                auto ec2 = std::from_chars( numStr.data(), numStr.data() + numStr.size(), uintValue ).ec;
                if( ec2 == std::errc{} )
                {
                    return Document{ uintValue };
                }

                throw std::runtime_error{ "Invalid number format" };
            }
        }

        /**
         * @brief Parse JSON string with escape sequence handling
         * @return Document containing string
         * @throws std::runtime_error on unterminated string or invalid escape sequence
         */
        inline Document parseString()
        {
            if( m_json[m_pos] != '"' )
            {
                throw std::runtime_error{ "Expected '\"'" };
            }

            ++m_pos;

            std::string result;
            result.reserve( STRING_INITIAL_CAPACITY );

#if NFX_JSON_PARSER_USE_SIMD
            // SIMD fast path: scan for special characters (quote and backslash)
            const __m128i quote_vec = _mm_set1_epi8( '"' );
            const __m128i backslash_vec = _mm_set1_epi8( '\\' );

            while( m_pos + SIMD_CHUNK_SIZE <= m_json.size() )
            {
                // Load 16 bytes
                __m128i chunk = _mm_loadu_si128( reinterpret_cast<const __m128i*>( m_json.data() + m_pos ) );

                // Check for quote or backslash
                __m128i cmp_quote = _mm_cmpeq_epi8( chunk, quote_vec );
                __m128i cmp_backslash = _mm_cmpeq_epi8( chunk, backslash_vec );
                __m128i special = _mm_or_si128( cmp_quote, cmp_backslash );

                int mask = _mm_movemask_epi8( special );

                if( mask == 0 )
                {
                    // No special characters in these 16 bytes - bulk copy
                    result.append( m_json.data() + m_pos, SIMD_CHUNK_SIZE );
                    m_pos += SIMD_CHUNK_SIZE;
                }
                else
                {
                    // Found special character - copy bytes before it and break to slow path
                    int special_pos = countTrailingZeros( static_cast<unsigned int>( mask ) );
                    result.append( m_json.data() + m_pos, special_pos );
                    m_pos += special_pos;
                    break;
                }
            }
#endif
            // Fallback path: byte-by-byte processing for remainder and special characters
            while( m_pos < m_json.size() )
            {
                char c = m_json[m_pos];

                if( c == '"' )
                {
                    ++m_pos;
                    return Document{ std::move( result ) };
                }
                else if( c == '\\' )
                {
                    ++m_pos;
                    parseEscapeSequence( result );
                }
                else
                {
                    // JSON spec (RFC 8259) requires control characters (0x00-0x1F) to be escaped
                    unsigned char uc = static_cast<unsigned char>( c );
                    if( uc < 0x20 )
                    {
                        throw std::runtime_error{ "Unescaped control character in string" };
                    }
                    result.push_back( c );
                    ++m_pos;
                }
            }

            throw std::runtime_error{ "Unterminated string" };
        }

        /**
         * @brief Parse JSON array
         * @return Document containing Array
         * @throws std::runtime_error on malformed array syntax
         */
        inline Document parseArray()
        {
            if( m_json[m_pos] != '[' )
            {
                throw std::runtime_error{ "Expected '['" };
            }
            ++m_pos;

            Array arr;
            skipWhitespace();

            // Empty array
            if( m_pos < m_json.size() && m_json[m_pos] == ']' )
            {
                ++m_pos;
                return Document{ std::move( arr ) };
            }

            while( true )
            {
                arr.push_back( parseValue() );
                skipWhitespace();

                if( m_pos >= m_json.size() )
                {
                    throw std::runtime_error{ "Unexpected end of array" };
                }

                if( m_json[m_pos] == ']' )
                {
                    ++m_pos;
                    return Document{ std::move( arr ) };
                }
                else if( m_json[m_pos] == ',' )
                {
                    ++m_pos;
                    skipWhitespace();
                }
                else
                {
                    throw std::runtime_error{ "Expected ',' or ']' in array" };
                }
            }
        }

        /**
         * @brief Parse JSON object
         * @return Document containing Object
         * @throws std::runtime_error on malformed object syntax or non-string keys
         */
        inline Document parseObject()
        {
            if( m_json[m_pos] != '{' )
            {
                throw std::runtime_error{ "Expected '{'" };
            }
            ++m_pos;

            Object obj;
            skipWhitespace();

            // Empty object
            if( m_pos < m_json.size() && m_json[m_pos] == '}' )
            {
                ++m_pos;
                return Document{ std::move( obj ) };
            }

            while( true )
            {
                // Parse key
                skipWhitespace();
                if( m_json[m_pos] != '"' )
                {
                    throw std::runtime_error{ "Expected string key in object" };
                }

                Document keyValue = parseString();
                auto keyOpt = keyValue.root<std::string>();
                if( !keyOpt )
                {
                    throw std::runtime_error{ "Failed to extract string key from parsed value" };
                }
                std::string key = std::move( keyOpt.value() );

                // Parse colon
                skipWhitespace();
                if( m_pos >= m_json.size() || m_json[m_pos] != ':' )
                {
                    throw std::runtime_error{ "Expected ':' in object" };
                }
                ++m_pos;

                // Parse value
                Document value = parseValue();
                obj.emplace_back( std::move( key ), std::move( value ) );

                skipWhitespace();

                if( m_pos >= m_json.size() )
                {
                    throw std::runtime_error{ "Unexpected end of object" };
                }

                if( m_json[m_pos] == '}' )
                {
                    ++m_pos;
                    return Document{ std::move( obj ) };
                }
                else if( m_json[m_pos] == ',' )
                {
                    ++m_pos;
                }
                else
                {
                    throw std::runtime_error{ "Expected ',' or '}' in object" };
                }
            }
        }

        //----------------------------------------------
        // Private members
        //----------------------------------------------

        std::string_view m_json; ///< JSON text being parsed
        size_t m_pos;            ///< Current position in the JSON string
    };
} // namespace nfx::json
