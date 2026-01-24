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

namespace nfx::json
{
    /**
     * @brief High-performance JSON writer
     * @details Writes JSON directly to string buffer without building DOM.
     *          Optimized for speed similar to System.Text.Json in .NET.
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
            auto [ptr, ec] = std::to_chars( buf, buf + sizeof( buf ), value );
            m_buffer.append( buf, ptr - buf );
        }

        void writeUInt( uint64_t value )
        {
            char buf[32];
            auto [ptr, ec] = std::to_chars( buf, buf + sizeof( buf ), value );
            m_buffer.append( buf, ptr - buf );
        }

        void writeDouble( double value )
        {
            char buf[32];
            auto [ptr, ec] = std::to_chars( buf, buf + sizeof( buf ), value );
            m_buffer.append( buf, ptr - buf );
        }

        void writeString( std::string_view str )
        {
            m_buffer.push_back( '"' );

            // Fast path: no escaping needed
            size_t lastPos = 0;
            for( size_t i = 0; i < str.size(); ++i )
            {
                unsigned char c = static_cast<unsigned char>( str[i] );

                // Check if escaping needed
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
                            // \uXXXX format for control characters
                            char hexBuf[7];
                            snprintf( hexBuf, sizeof( hexBuf ), "u%04x", c );
                            m_buffer.append( hexBuf );
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
