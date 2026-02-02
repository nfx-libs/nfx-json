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
 * @file Builder.inl
 * @brief Inline Implementation of Builder class
 */

#if defined( __SSE2__ ) || defined( _M_X64 ) || ( defined( _M_IX86_FP ) && _M_IX86_FP >= 2 )
#    include <emmintrin.h>
#    define NFX_JSON_BUILDER_USE_SIMD 1
#else
#    define NFX_JSON_BUILDER_USE_SIMD 0
#endif

namespace nfx::json
{
    inline Builder::Builder( Options options )
        : m_indent{ options.indent },
          m_currentIndent{ 0 }
    {
        m_buffer.reserve( options.bufferSize );
    }

    inline Builder& Builder::writeStartObject()
    {
        writeCommaIfNeeded();
        m_buffer.push_back( '{' );
        m_contextStack.push_back( { true, true, false } );
        if( m_indent > 0 )
        {
            m_currentIndent += m_indent;
        }
        return *this;
    }

    inline Builder& Builder::writeEndObject()
    {
        if( m_contextStack.isEmpty() || !m_contextStack.back().isObject )
        {
            throw std::runtime_error{ "writeEndObject called without matching writeStartObject" };
        }

        if( m_indent > 0 )
        {
            m_currentIndent -= m_indent;
            if( !m_contextStack.back().isEmpty )
            {
                writeNewlineAndIndent();
            }
        }

        m_contextStack.pop_back();
        m_buffer.push_back( '}' );
        return *this;
    }

    inline Builder& Builder::writeStartArray()
    {
        writeCommaIfNeeded();
        m_buffer.push_back( '[' );
        m_contextStack.push_back( { false, true, false } );
        if( m_indent > 0 )
        {
            m_currentIndent += m_indent;
        }
        return *this;
    }

    inline Builder& Builder::writeEndArray()
    {
        if( m_contextStack.isEmpty() || m_contextStack.back().isObject )
        {
            throw std::runtime_error{ "writeEndArray called without matching writeStartArray" };
        }

        if( m_indent > 0 )
        {
            m_currentIndent -= m_indent;
            if( !m_contextStack.back().isEmpty )
            {
                writeNewlineAndIndent();
            }
        }

        m_contextStack.pop_back();
        m_buffer.push_back( ']' );
        return *this;
    }

    inline Builder& Builder::writeKey( std::string_view key )
    {
        if( m_contextStack.isEmpty() || !m_contextStack.back().isObject )
        {
            throw std::runtime_error{ "writeKey can only be called inside an object" };
        }
        writeCommaIfNeeded();
        writeString( key );
        if( m_indent > 0 )
        {
            m_buffer.append( ": " );
        }
        else
        {
            m_buffer.push_back( ':' );
        }
        m_contextStack.back().expectingValue = true;
        return *this;
    }

    inline Builder& Builder::writeRawJson( std::string_view rawJson )
    {
        writeCommaIfNeeded();
        m_buffer.append( rawJson );
        return *this;
    }

    inline Builder& Builder::write( std::string_view key, std::nullptr_t )
    {
        if( m_contextStack.isEmpty() || !m_contextStack.back().isObject )
        {
            throw std::runtime_error{ "write can only be called inside an object" };
        }
        writeCommaIfNeeded();
        writeString( key );
        if( m_indent > 0 )
        {
            m_buffer.append( ": " );
        }
        else
        {
            m_buffer.push_back( ':' );
        }
        m_contextStack.back().expectingValue = true;
        writeCommaIfNeeded();
        m_buffer.append( "null" );
        return *this;
    }

    inline Builder& Builder::write( std::string_view key, bool value )
    {
        if( m_contextStack.isEmpty() || !m_contextStack.back().isObject )
        {
            throw std::runtime_error{ "write can only be called inside an object" };
        }
        writeCommaIfNeeded();
        writeString( key );
        if( m_indent > 0 )
        {
            m_buffer.append( ": " );
        }
        else
        {
            m_buffer.push_back( ':' );
        }
        m_contextStack.back().expectingValue = true;
        writeCommaIfNeeded();
        m_buffer.append( value ? "true" : "false" );
        return *this;
    }

    inline Builder& Builder::write( std::string_view key, int value )
    {
        if( m_contextStack.isEmpty() || !m_contextStack.back().isObject )
        {
            throw std::runtime_error{ "write can only be called inside an object" };
        }
        writeCommaIfNeeded();
        writeString( key );
        if( m_indent > 0 )
        {
            m_buffer.append( ": " );
        }
        else
        {
            m_buffer.push_back( ':' );
        }
        m_contextStack.back().expectingValue = true;
        writeCommaIfNeeded();
        writeInt( static_cast<int64_t>( value ) );
        return *this;
    }

    inline Builder& Builder::write( std::string_view key, unsigned int value )
    {
        if( m_contextStack.isEmpty() || !m_contextStack.back().isObject )
        {
            throw std::runtime_error{ "write can only be called inside an object" };
        }
        writeCommaIfNeeded();
        writeString( key );
        if( m_indent > 0 )
        {
            m_buffer.append( ": " );
        }
        else
        {
            m_buffer.push_back( ':' );
        }
        m_contextStack.back().expectingValue = true;
        writeCommaIfNeeded();
        writeUInt( static_cast<uint64_t>( value ) );
        return *this;
    }

    inline Builder& Builder::write( std::string_view key, int64_t value )
    {
        if( m_contextStack.isEmpty() || !m_contextStack.back().isObject )
        {
            throw std::runtime_error{ "write can only be called inside an object" };
        }
        writeCommaIfNeeded();
        writeString( key );
        if( m_indent > 0 )
        {
            m_buffer.append( ": " );
        }
        else
        {
            m_buffer.push_back( ':' );
        }
        m_contextStack.back().expectingValue = true;
        writeCommaIfNeeded();
        writeInt( value );
        return *this;
    }

    inline Builder& Builder::write( std::string_view key, uint64_t value )
    {
        if( m_contextStack.isEmpty() || !m_contextStack.back().isObject )
        {
            throw std::runtime_error{ "write can only be called inside an object" };
        }
        writeCommaIfNeeded();
        writeString( key );
        if( m_indent > 0 )
        {
            m_buffer.append( ": " );
        }
        else
        {
            m_buffer.push_back( ':' );
        }
        m_contextStack.back().expectingValue = true;
        writeCommaIfNeeded();
        writeUInt( value );
        return *this;
    }

    inline Builder& Builder::write( std::string_view key, float value )
    {
        if( m_contextStack.isEmpty() || !m_contextStack.back().isObject )
        {
            throw std::runtime_error{ "write can only be called inside an object" };
        }
        writeCommaIfNeeded();
        writeString( key );
        if( m_indent > 0 )
        {
            m_buffer.append( ": " );
        }
        else
        {
            m_buffer.push_back( ':' );
        }
        m_contextStack.back().expectingValue = true;
        writeCommaIfNeeded();
        writeDouble( static_cast<double>( value ) );
        return *this;
    }

    inline Builder& Builder::write( std::string_view key, double value )
    {
        if( m_contextStack.isEmpty() || !m_contextStack.back().isObject )
        {
            throw std::runtime_error{ "write can only be called inside an object" };
        }
        writeCommaIfNeeded();
        writeString( key );
        if( m_indent > 0 )
        {
            m_buffer.append( ": " );
        }
        else
        {
            m_buffer.push_back( ':' );
        }
        m_contextStack.back().expectingValue = true;
        writeCommaIfNeeded();
        writeDouble( value );
        return *this;
    }

    inline Builder& Builder::write( std::string_view key, std::string_view value )
    {
        if( m_contextStack.isEmpty() || !m_contextStack.back().isObject )
        {
            throw std::runtime_error{ "write can only be called inside an object" };
        }
        writeCommaIfNeeded();
        writeString( key );
        if( m_indent > 0 )
        {
            m_buffer.append( ": " );
        }
        else
        {
            m_buffer.push_back( ':' );
        }
        m_contextStack.back().expectingValue = true;
        writeCommaIfNeeded();
        writeString( value );
        return *this;
    }

    inline Builder& Builder::write( std::string_view key, const char* value )
    {
        return write( key, std::string_view{ value } );
    }

    inline Builder& Builder::write( std::string_view key, const std::string& value )
    {
        return write( key, std::string_view{ value } );
    }

    inline Builder& Builder::write( std::string_view key, const Document& value )
    {
        writeCommaIfNeeded();
        writeString( key );
        if( m_indent > 0 )
        {
            m_buffer.append( ": " );
        }
        else
        {
            m_buffer.push_back( ':' );
        }
        writeDocument( value );
        return *this;
    }

    template <typename T>
    inline std::
        enable_if_t<std::is_same_v<T, long> && !std::is_same_v<long, int> && !std::is_same_v<long, int64_t>, Builder&>
        Builder::write( std::string_view key, T value )
    {
        return write( key, static_cast<int64_t>( value ) );
    }

    template <typename T>
    inline std::enable_if_t<
        std::is_same_v<T, unsigned long> && !std::is_same_v<unsigned long, unsigned int> &&
            !std::is_same_v<unsigned long, uint64_t>,
        Builder&>
    Builder::write( std::string_view key, T value )
    {
        return write( key, static_cast<uint64_t>( value ) );
    }

    template <typename T>
    inline std::enable_if_t<std::is_same_v<T, long long> && !std::is_same_v<long long, int64_t>, Builder&> Builder::
        write( std::string_view key, T value )
    {
        return write( key, static_cast<int64_t>( value ) );
    }

    template <typename T>
    inline std::
        enable_if_t<std::is_same_v<T, unsigned long long> && !std::is_same_v<unsigned long long, uint64_t>, Builder&>
        Builder::write( std::string_view key, T value )
    {
        return write( key, static_cast<uint64_t>( value ) );
    }

    // Array value implementations
    inline Builder& Builder::write( std::nullptr_t )
    {
        writeCommaIfNeeded();
        m_buffer.append( "null" );
        return *this;
    }

    inline Builder& Builder::write( bool value )
    {
        writeCommaIfNeeded();
        m_buffer.append( value ? "true" : "false" );
        return *this;
    }

    inline Builder& Builder::write( int value )
    {
        writeCommaIfNeeded();
        writeInt( static_cast<int64_t>( value ) );
        return *this;
    }

    inline Builder& Builder::write( unsigned int value )
    {
        writeCommaIfNeeded();
        writeUInt( static_cast<uint64_t>( value ) );
        return *this;
    }

    inline Builder& Builder::write( int64_t value )
    {
        writeCommaIfNeeded();
        writeInt( value );
        return *this;
    }

    inline Builder& Builder::write( uint64_t value )
    {
        writeCommaIfNeeded();
        writeUInt( value );
        return *this;
    }

    inline Builder& Builder::write( float value )
    {
        writeCommaIfNeeded();
        writeDouble( static_cast<double>( value ) );
        return *this;
    }

    inline Builder& Builder::write( double value )
    {
        writeCommaIfNeeded();
        writeDouble( value );
        return *this;
    }

    inline Builder& Builder::write( std::string_view value )
    {
        writeCommaIfNeeded();
        writeString( value );
        return *this;
    }

    inline Builder& Builder::write( const char* value )
    {
        return write( std::string_view{ value } );
    }

    inline Builder& Builder::write( const std::string& value )
    {
        return write( std::string_view{ value } );
    }

    inline Builder& Builder::write( const Document& value )
    {
        writeCommaIfNeeded();
        writeDocument( value );
        return *this;
    }

    template <typename T>
    inline std::
        enable_if_t<std::is_same_v<T, long> && !std::is_same_v<long, int> && !std::is_same_v<long, int64_t>, Builder&>
        Builder::write( T value )
    {
        return write( static_cast<int64_t>( value ) );
    }

    template <typename T>
    inline std::enable_if_t<
        std::is_same_v<T, unsigned long> && !std::is_same_v<unsigned long, unsigned int> &&
            !std::is_same_v<unsigned long, uint64_t>,
        Builder&>
    Builder::write( T value )
    {
        return write( static_cast<uint64_t>( value ) );
    }

    template <typename T>
    inline std::enable_if_t<std::is_same_v<T, long long> && !std::is_same_v<long long, int64_t>, Builder&> Builder::
        write( T value )
    {
        return write( static_cast<int64_t>( value ) );
    }

    template <typename T>
    inline std::
        enable_if_t<std::is_same_v<T, unsigned long long> && !std::is_same_v<unsigned long long, uint64_t>, Builder&>
        Builder::write( T value )
    {
        return write( static_cast<uint64_t>( value ) );
    }

    inline std::string Builder::toString()
    {
        std::string result = std::move( m_buffer );
        m_buffer.clear();
        m_contextStack.clear();
        m_currentIndent = 0;
        return result;
    }

    inline Builder& Builder::reset()
    {
        m_buffer.clear();
        m_contextStack.clear();
        m_currentIndent = 0;
        return *this;
    }

    inline size_t Builder::size() const noexcept
    {
        return m_buffer.size();
    }

    inline bool Builder::isEmpty() const noexcept
    {
        return m_buffer.empty();
    }

    inline Builder& Builder::reserve( size_t capacity )
    {
        m_buffer.reserve( capacity );
        return *this;
    }

    inline std::string_view Builder::toStringView() const noexcept
    {
        return m_buffer;
    }

    inline size_t Builder::capacity() const noexcept
    {
        return m_buffer.capacity();
    }

    inline size_t Builder::depth() const noexcept
    {
        return m_contextStack.size();
    }

    inline bool Builder::isValid() const noexcept
    {
        return m_contextStack.isEmpty();
    }

    template <typename Container>
    inline Builder& Builder::writeArray( std::string_view key, const Container& values )
    {
        writeKey( key );
        return writeArray( values );
    }

    template <typename Container>
    inline Builder& Builder::writeArray( const Container& values )
    {
        writeStartArray();
        for( const auto& value : values )
        {
            write( value );
        }
        return writeEndArray();
    }

    inline void Builder::writeInt( int64_t value )
    {
        char buf[32];
        auto* end = std::to_chars( buf, buf + sizeof( buf ), value ).ptr;
        m_buffer.append( buf, static_cast<size_t>( end - buf ) );
    }

    inline void Builder::writeUInt( uint64_t value )
    {
        char buf[32];
        auto* end = std::to_chars( buf, buf + sizeof( buf ), value ).ptr;
        m_buffer.append( buf, static_cast<size_t>( end - buf ) );
    }

    inline void Builder::writeDouble( double value )
    {
        char buf[32];
        auto* end = std::to_chars( buf, buf + sizeof( buf ), value ).ptr;
        m_buffer.append( buf, static_cast<size_t>( end - buf ) );
    }

    inline void Builder::writeString( std::string_view str )
    {
        m_buffer.push_back( '"' );

#if NFX_JSON_BUILDER_USE_SIMD
        static constexpr char hex[] = "0123456789abcdef";
        size_t lastPos = 0;
        size_t i = 0;

        // Process 16 bytes at a time with SIMD
        const __m128i quote = _mm_set1_epi8( '"' );
        const __m128i backslash = _mm_set1_epi8( '\\' );
        const __m128i control_threshold = _mm_set1_epi8( 0x20 );
        const __m128i sign_flip = _mm_set1_epi8( static_cast<char>( 0x80 ) );

        while( i + 16 <= str.size() )
        {
            __m128i chunk = _mm_loadu_si128( reinterpret_cast<const __m128i*>( str.data() + i ) );

            __m128i cmp_quote = _mm_cmpeq_epi8( chunk, quote );
            __m128i cmp_backslash = _mm_cmpeq_epi8( chunk, backslash );

            // Unsigned comparison: chunk < 0x20 (control characters)
            __m128i chunk_unsigned = _mm_xor_si128( chunk, sign_flip );
            __m128i threshold_unsigned = _mm_xor_si128( control_threshold, sign_flip );
            __m128i cmp_control = _mm_cmplt_epi8( chunk_unsigned, threshold_unsigned );

            __m128i needs_escape = _mm_or_si128( _mm_or_si128( cmp_quote, cmp_backslash ), cmp_control );

            int mask = _mm_movemask_epi8( needs_escape );

            if( mask == 0 )
            {
                // Fast path: no escaping needed for this 16-byte chunk
                i += 16;
            }
            else
            {
                // Found characters needing escape - process them individually
                // but append clean prefix first
                if( i > lastPos )
                {
                    m_buffer.append( str.data() + lastPos, i - lastPos );
                    lastPos = i;
                }

                // Process each byte in this chunk that needs escaping
                for( size_t j = 0; j < 16 && i + j < str.size(); ++j )
                {
                    if( mask & ( 1 << j ) )
                    {
                        // Append clean bytes before this one
                        if( i + j > lastPos )
                        {
                            m_buffer.append( str.data() + lastPos, ( i + j ) - lastPos );
                        }

                        unsigned char c = static_cast<unsigned char>( str[i + j] );
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

                        lastPos = i + j + 1;
                    }
                }

                i += 16;
            }
        }

        // Handle remaining bytes (< 16) with scalar code
        for( ; i < str.size(); ++i )
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
#else
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

    inline void Builder::writeNewlineAndIndent()
    {
        m_buffer.push_back( '\n' );
        m_buffer.append( static_cast<size_t>( m_currentIndent ), ' ' );
    }

    inline void Builder::writeCommaIfNeeded()
    {
        if( m_contextStack.isEmpty() )
        {
            return;
        }

        auto& context = m_contextStack.back();

        if( context.expectingValue )
        {
            context.expectingValue = false;
            context.isEmpty = false;
            return;
        }

        if( !context.isEmpty )
        {
            m_buffer.push_back( ',' );
            if( m_indent > 0 )
            {
                writeNewlineAndIndent();
            }
        }
        else
        {
            context.isEmpty = false;
            if( m_indent > 0 )
            {
                writeNewlineAndIndent();
            }
        }
    }

    inline void Builder::writeDocument( const Document& doc )
    {
        // Serialize a Document to JSON by dispatching on its runtime type.
        // This enables DOM-to-stream conversion for mixed construction patterns.
        switch( doc.type() )
        {
            case Type::Null:
                // Write JSON null literal
                m_buffer.append( "null" );
                break;

            case Type::Boolean:
                // Write JSON boolean literal (true/false)
                if( auto val = doc.root<bool>() )
                {
                    m_buffer.append( *val ? "true" : "false" );
                }
                break;

            case Type::Integer:
                // Write signed integer value
                if( auto val = doc.root<int64_t>() )
                {
                    writeInt( *val );
                }
                break;

            case Type::UnsignedInteger:
                // Write unsigned integer value
                if( auto val = doc.root<uint64_t>() )
                {
                    writeUInt( *val );
                }
                break;

            case Type::Double:
                // Write floating-point value
                if( auto val = doc.root<double>() )
                {
                    writeDouble( *val );
                }
                break;

            case Type::String:
                // Write escaped JSON string
                if( auto val = doc.root<std::string>() )
                {
                    writeString( *val );
                }
                break;

            case Type::Array:
                // Write JSON array with recursive element serialization
                if( auto arrayRef = doc.rootRef<Array>() )
                {
                    writeDocumentArray( arrayRef->get() );
                }
                break;

            case Type::Object:
                // Write JSON object with recursive key-value pair serialization
                if( auto objectRef = doc.rootRef<Object>() )
                {
                    writeDocumentObject( objectRef->get() );
                }
                break;
        }
    }

    inline void Builder::writeDocumentArray( const Array& array )
    {
        // Start array: write '[', push context, increase indent
        m_buffer.push_back( '[' );
        m_contextStack.push_back( { false, true, false } );
        if( m_indent > 0 )
        {
            m_currentIndent += m_indent;
        }

        // Serialize each array element recursively
        for( const auto& element : array )
        {
            writeCommaIfNeeded();
            writeDocument( element );
        }

        // End array: decrease indent, write newline if needed, write ']'
        if( m_indent > 0 )
        {
            m_currentIndent -= m_indent;
            if( !m_contextStack.back().isEmpty )
            {
                writeNewlineAndIndent();
            }
        }
        m_buffer.push_back( ']' );
        m_contextStack.pop_back();
    }

    inline void Builder::writeDocumentObject( const Object& object )
    {
        // Start object: write '{', push context, increase indent
        m_buffer.push_back( '{' );
        m_contextStack.push_back( { true, true, false } );
        if( m_indent > 0 )
        {
            m_currentIndent += m_indent;
        }

        // Serialize each key-value pair recursively
        for( const auto& [key, value] : object )
        {
            writeCommaIfNeeded();
            writeString( key );
            // Add space after colon in pretty-print mode
            if( m_indent > 0 )
            {
                m_buffer.append( ": " );
            }
            else
            {
                m_buffer.push_back( ':' );
            }
            writeDocument( value );
        }

        // End object: decrease indent, write newline if needed, write '}'
        if( m_indent > 0 )
        {
            m_currentIndent -= m_indent;
            if( !m_contextStack.back().isEmpty )
            {
                writeNewlineAndIndent();
            }
        }
        m_buffer.push_back( '}' );
        m_contextStack.pop_back();
    }
} // namespace nfx::json
