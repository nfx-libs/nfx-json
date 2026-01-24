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

#pragma once

#include "nfx/json/Document.h"

#include <nfx/string/Utils.h>

#include <charconv>
#include <cmath>
#include <stdexcept>
#include <string>
#include <string_view>

namespace nfx::json
{
    namespace
    {
        // String parsing constants
        constexpr size_t STRING_INITIAL_CAPACITY = 64; // Typical JSON string length

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
    class JsonParser
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

            JsonParser parser{ json };
            return parser.parseValue();
        }

    private:
        explicit JsonParser( std::string_view json )
            : m_json{ json },
              m_pos{ 0 }
        {
        }

        //----------------------------------------------
        // Core parsing methods
        //----------------------------------------------

        Document parseValue()
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

        Document parseNull()
        {
            if( m_json.substr( m_pos, 4 ) != "null" )
            {
                throw std::runtime_error{ "Expected 'null'" };
            }
            m_pos += 4;
            return Document{ nullptr };
        }

        Document parseBool()
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

        Document parseNumber()
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
                auto [ptr, ec] = std::from_chars( numStr.data(), numStr.data() + numStr.size(), value );
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
                auto [ptr, ec] = std::from_chars( numStr.data(), numStr.data() + numStr.size(), intValue );
                if( ec == std::errc{} )
                {
                    return Document{ intValue };
                }

                // Try as uint64_t
                uint64_t uintValue = 0;
                auto [ptr2, ec2] = std::from_chars( numStr.data(), numStr.data() + numStr.size(), uintValue );
                if( ec2 == std::errc{} )
                {
                    return Document{ uintValue };
                }

                throw std::runtime_error{ "Invalid number format" };
            }
        }

        Document parseString()
        {
            if( m_json[m_pos] != '"' )
            {
                throw std::runtime_error{ "Expected '\"'" };
            }

            ++m_pos;

            std::string result;
            result.reserve( STRING_INITIAL_CAPACITY );

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
                        {
                            // Unicode escape \uXXXX
                            if( m_pos + 4 >= m_json.size() )
                            {
                                throw std::runtime_error{ "Invalid unicode escape" };
                            }

                            std::string hexStr{ m_json.substr( m_pos + 1, 4 ) };
                            unsigned int codePoint = 0;
                            auto [ptr, ec] = std::from_chars( hexStr.data(), hexStr.data() + 4, codePoint, 16 );
                            if( ec != std::errc{} )
                            {
                                throw std::runtime_error{ "Invalid unicode escape" };
                            }

                            // Check for UTF-16 surrogate pairs (for characters beyond BMP)
                            if( codePoint >= UTF16_HIGH_SURROGATE_START && codePoint <= UTF16_HIGH_SURROGATE_END )
                            {
                                // High surrogate - need to get low surrogate
                                m_pos += 4; // Skip the 4 hex digits

                                // Check for \uXXXX pattern
                                if( m_pos + 6 >= m_json.size() || m_json[m_pos + 1] != '\\' ||
                                    m_json[m_pos + 2] != 'u' )
                                {
                                    throw std::runtime_error{ "Invalid surrogate pair: missing low surrogate" };
                                }

                                // Parse low surrogate
                                std::string lowHexStr( m_json.substr( m_pos + 3, 4 ) );
                                unsigned int lowSurrogate = 0;
                                auto [lowPtr, lowEc] =
                                    std::from_chars( lowHexStr.data(), lowHexStr.data() + 4, lowSurrogate, 16 );
                                if( lowEc != std::errc{} )
                                {
                                    throw std::runtime_error{ "Invalid unicode escape in low surrogate" };
                                }

                                // Validate low surrogate range
                                if( lowSurrogate < UTF16_LOW_SURROGATE_START || lowSurrogate > UTF16_LOW_SURROGATE_END )
                                {
                                    throw std::runtime_error{ "Invalid low surrogate value" };
                                }

                                // Combine surrogates to get actual code point Formula:
                                // (high - 0xD800) * 0x400 + (low -0xDC00) + 0x10000
                                codePoint = ( ( codePoint - UTF16_HIGH_SURROGATE_START ) << 10 ) +
                                            ( lowSurrogate - UTF16_LOW_SURROGATE_START ) + UTF16_SURROGATE_OFFSET;

                                m_pos += 6; // Skip \uXXXX  of low surrogate
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
                                    static_cast<char>(
                                        UTF8_CONTINUATION_BYTE | ( codePoint & UTF8_CONTINUATION_MASK ) ) );
                            }
                            else if( codePoint < UTF8_3BYTE_MAX )
                            {
                                // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
                                result.push_back( static_cast<char>( UTF8_3BYTE_LEAD | ( codePoint >> 12 ) ) );
                                result.push_back(
                                    static_cast<char>(
                                        UTF8_CONTINUATION_BYTE | ( ( codePoint >> 6 ) & UTF8_CONTINUATION_MASK ) ) );
                                result.push_back(
                                    static_cast<char>(
                                        UTF8_CONTINUATION_BYTE | ( codePoint & UTF8_CONTINUATION_MASK ) ) );
                            }
                            else if( codePoint < UTF8_4BYTE_MAX )
                            {
                                // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                                result.push_back( static_cast<char>( UTF8_4BYTE_LEAD | ( codePoint >> 18 ) ) );
                                result.push_back(
                                    static_cast<char>(
                                        UTF8_CONTINUATION_BYTE | ( ( codePoint >> 12 ) & UTF8_CONTINUATION_MASK ) ) );
                                result.push_back(
                                    static_cast<char>(
                                        UTF8_CONTINUATION_BYTE | ( ( codePoint >> 6 ) & UTF8_CONTINUATION_MASK ) ) );
                                result.push_back(
                                    static_cast<char>(
                                        UTF8_CONTINUATION_BYTE | ( codePoint & UTF8_CONTINUATION_MASK ) ) );
                            }
                            else
                            {
                                throw std::runtime_error{ "Invalid unicode code point" };
                            }

                            break;
                        }
                        default:
                            throw std::runtime_error{ std::string( "Invalid escape sequence: \\" ) + escaped };
                    }
                    ++m_pos;
                }
                else
                {
                    result.push_back( c );
                    ++m_pos;
                }
            }

            throw std::runtime_error{ "Unterminated string" };
        }

        Document parseArray()
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

        Document parseObject()
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

        void skipWhitespace()
        {
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
        // Members
        //----------------------------------------------

        std::string_view m_json;
        size_t m_pos;
    };
} // namespace nfx::json
