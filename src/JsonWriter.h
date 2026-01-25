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

#include <charconv>
#include <cstdio>
#include <string>
#include <string_view>

#if defined( __SSE2__ ) || defined( _M_X64 ) || ( defined( _M_IX86_FP ) && _M_IX86_FP >= 2 )
#    include <emmintrin.h>
#endif

namespace nfx::json
{
    /**
     * @brief High-performance JSON writer
     * @details Writes JSON directly to string buffer without building DOM.
     */
    class JsonWriter
    {
    public:
        static constexpr int DEFAULT_INDENT = 0;
        static constexpr size_t DEFAULT_BUFFER_SIZE = 4096;

        /**
         * @brief Constructor
         * @param indent Indentation level (0 = compact, >0 = pretty print)
         * @param bufferSize Initial buffer capacity (default 4KB)
         */
        explicit JsonWriter( int indent = DEFAULT_INDENT, size_t bufferSize = DEFAULT_BUFFER_SIZE )
            : m_indent{ indent },
              m_currentIndent{ 0 },
              m_bufferSize{ bufferSize }
        {
            m_buffer.reserve( m_bufferSize );
        }

        /**
         * @brief Write a Document to string
         * @param value The JSON value to write
         * @return JSON string representation
         */
        std::string write( const Document& value )
        {
            m_buffer.clear();
            m_currentIndent = 0;
            writeValue( value );
            return std::move( m_buffer );
        }

    private:
        //----------------------------------------------
        // Core writing methods
        //----------------------------------------------

        void writeValue( const Document& value )
        {
            switch( value.type() )
            {
                case Type::Null:
                    writeNull();
                    break;
                case Type::Boolean:
                    writeBool( *value.root<bool>() );
                    break;
                case Type::Integer:
                    writeInt( *value.root<int64_t>() );
                    break;
                case Type::UnsignedInteger:
                    writeUInt( *value.root<uint64_t>() );
                    break;
                case Type::Double:
                    writeDouble( *value.root<double>() );
                    break;
                case Type::String:
                    writeString( value.rootInternal<std::string>() );
                    break;
                case Type::Array:
                    writeArray( value.rootInternal<Array>() );
                    break;
                case Type::Object:
                    writeObject( value.rootInternal<Object>() );
                    break;
            }
        }

        void writeNull()
        {
            m_buffer.append( "null" );
        }

        void writeBool( bool value )
        {
            m_buffer.append( value ? "true" : "false" );
        }

        void writeInt( int64_t value )
        {
            char buf[32];
            auto* end = std::to_chars( buf, buf + sizeof( buf ), value ).ptr;
            m_buffer.append( buf, end - buf );
        }

        void writeUInt( uint64_t value )
        {
            char buf[32];
            auto* end = std::to_chars( buf, buf + sizeof( buf ), value ).ptr;
            m_buffer.append( buf, end - buf );
        }

        void writeDouble( double value )
        {
            char buf[32];
            auto* end = std::to_chars( buf, buf + sizeof( buf ), value ).ptr;
            m_buffer.append( buf, end - buf );
        }

        void writeString( std::string_view str )
        {
            m_buffer.push_back( '"' );

#if defined( __SSE2__ ) || defined( _M_X64 ) || ( defined( _M_IX86_FP ) && _M_IX86_FP >= 2 )
            static constexpr char hex[] = "0123456789abcdef";
            size_t lastPos = 0;
            size_t i = 0;

            // Process 16 bytes at a time with SIMD
            const __m128i quote = _mm_set1_epi8( '"' );
            const __m128i backslash = _mm_set1_epi8( '\\' );
            const __m128i control = _mm_set1_epi8( 0x1F ); // Characters < 0x20

            while( i + 16 <= str.size() )
            {
                __m128i chunk = _mm_loadu_si128( reinterpret_cast<const __m128i*>( str.data() + i ) );

                // Check for quote, backslash, or control chars
                __m128i cmp_quote = _mm_cmpeq_epi8( chunk, quote );
                __m128i cmp_backslash = _mm_cmpeq_epi8( chunk, backslash );
                __m128i cmp_control = _mm_cmplt_epi8( chunk, control ); // Signed comparison for < 0x20

                __m128i needs_escape = _mm_or_si128( _mm_or_si128( cmp_quote, cmp_backslash ), cmp_control );

                int mask = _mm_movemask_epi8( needs_escape );

                if( mask == 0 )
                {
                    // No escaping needed in this chunk, skip forward
                    i += 16;
                }
                else
                {
                    // Found character that needs escaping, fall back to scalar
                    break;
                }
            }

            // Append the SIMD-processed portion (if any)
            if( i > lastPos )
            {
                m_buffer.append( str.data() + lastPos, i - lastPos );
                lastPos = i;
            }

            // Scalar fallback for remaining characters
            for( ; i < str.size(); ++i )
            {
                unsigned char c = static_cast<unsigned char>( str[i] );

                if( c == '"' || c == '\\' || c < 0x20 )
                {
                    // Append unescaped portion
                    if( i > lastPos )
                    {
                        m_buffer.append( str.data() + lastPos, i - lastPos );
                    }

                    // Escape character
                    m_buffer.push_back( '\\' );
                    switch( c )
                    {
                        case '"':
                            m_buffer.push_back( '"' );
                            break;
                        case '\\':
                            m_buffer.push_back( '\\' );
                            break;
                        case '\b':
                            m_buffer.push_back( 'b' );
                            break;
                        case '\f':
                            m_buffer.push_back( 'f' );
                            break;
                        case '\n':
                            m_buffer.push_back( 'n' );
                            break;
                        case '\r':
                            m_buffer.push_back( 'r' );
                            break;
                        case '\t':
                            m_buffer.push_back( 't' );
                            break;
                        default:
                            // \uXXXX format with lookup table
                            m_buffer.append( "u00" );
                            m_buffer.push_back( hex[( c >> 4 ) & 0xF] );
                            m_buffer.push_back( hex[c & 0xF] );
                            break;
                    }

                    lastPos = i + 1;
                }
            }

            // Append remaining unescaped portion
            if( lastPos < str.size() )
            {
                m_buffer.append( str.data() + lastPos, str.size() - lastPos );
            }
#else
            // Fallback scalar path for non-SSE2 platforms
            static constexpr char hex[] = "0123456789abcdef";
            size_t lastPos = 0;
            for( size_t i = 0; i < str.size(); ++i )
            {
                unsigned char c = static_cast<unsigned char>( str[i] );

                if( c == '"' || c == '\\' || c < 0x20 )
                {
                    if( i > lastPos )
                    {
                        m_buffer.append( str.data() + lastPos, i - lastPos );
                    }

                    m_buffer.push_back( '\\' );
                    switch( c )
                    {
                        case '"':
                            m_buffer.push_back( '"' );
                            break;
                        case '\\':
                            m_buffer.push_back( '\\' );
                            break;
                        case '\b':
                            m_buffer.push_back( 'b' );
                            break;
                        case '\f':
                            m_buffer.push_back( 'f' );
                            break;
                        case '\n':
                            m_buffer.push_back( 'n' );
                            break;
                        case '\r':
                            m_buffer.push_back( 'r' );
                            break;
                        case '\t':
                            m_buffer.push_back( 't' );
                            break;
                        default:
                            m_buffer.append( "u00" );
                            m_buffer.push_back( hex[( c >> 4 ) & 0xF] );
                            m_buffer.push_back( hex[c & 0xF] );
                            break;
                    }

                    lastPos = i + 1;
                }
            }

            if( lastPos < str.size() )
            {
                m_buffer.append( str.data() + lastPos, str.size() - lastPos );
            }
#endif
            m_buffer.push_back( '"' );
        }

        void writeArray( const Array& arr )
        {
            m_buffer.push_back( '[' );

            if( !arr.empty() )
            {
                if( m_indent > 0 )
                {
                    m_currentIndent += m_indent;
                    for( size_t i = 0; i < arr.size(); ++i )
                    {
                        if( i > 0 )
                            m_buffer.push_back( ',' );
                        writeNewlineAndIndent();
                        writeValue( arr[i] );
                    }
                    m_currentIndent -= m_indent;
                    writeNewlineAndIndent();
                }
                else
                {
                    // Compact mode
                    for( size_t i = 0; i < arr.size(); ++i )
                    {
                        if( i > 0 )
                        {
                            m_buffer.push_back( ',' );
                        }
                        writeValue( arr[i] );
                    }
                }
            }

            m_buffer.push_back( ']' );
        }

        void writeObject( const Object& obj )
        {
            m_buffer.push_back( '{' );

            if( !obj.empty() )
            {
                if( m_indent > 0 )
                {
                    m_currentIndent += m_indent;
                    bool first = true;
                    for( const auto& [key, value] : obj )
                    {
                        if( !first )
                        {
                            m_buffer.push_back( ',' );
                        }
                        first = false;
                        writeNewlineAndIndent();
                        writeString( key );
                        m_buffer.append( ": " );
                        writeValue( value );
                    }
                    m_currentIndent -= m_indent;
                    writeNewlineAndIndent();
                }
                else
                {
                    // Compact mode
                    bool first = true;
                    for( const auto& [key, value] : obj )
                    {
                        if( !first )
                        {
                            m_buffer.push_back( ',' );
                        }
                        first = false;
                        writeString( key );
                        m_buffer.push_back( ':' );
                        writeValue( value );
                    }
                }
            }

            m_buffer.push_back( '}' );
        }

        void writeNewlineAndIndent()
        {
            m_buffer.push_back( '\n' );
            m_buffer.append( m_currentIndent, ' ' );
        }

        //----------------------------------------------
        // Members
        //----------------------------------------------

        std::string m_buffer;
        int m_indent;
        int m_currentIndent;
        size_t m_bufferSize;
    };
} // namespace nfx::json
