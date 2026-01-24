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
 * @file Document.cpp
 * @brief Implementation of Document class
 */

#include "nfx/json/Document.h"

#include "JsonWriter.h"
#include "JsonParser.h"

#include <nfx/string/Utils.h>

#include <algorithm>
#include <cctype>
#include <functional>
#include <sstream>

namespace nfx::json
{
    //=====================================================================
    // Document class
    //=====================================================================

    //----------------------------------------------
    // Comparison
    //----------------------------------------------

    std::strong_ordering Document::operator<=>( const Document& other ) const noexcept
    {
        // First compare types
        if( type() != other.type() )
        {
            return type() <=> other.type();
        }

        // Then compare values of same type
        switch( type() )
        {
            case Type::Null:
            {
                return std::strong_ordering::equal;
            }
            case Type::Boolean:
            {
                return std::get<bool>( m_data ) <=> std::get<bool>( other.m_data );
            }
            case Type::Integer:
            {
                return std::get<int64_t>( m_data ) <=> std::get<int64_t>( other.m_data );
            }
            case Type::UnsignedInteger:
            {
                return std::get<uint64_t>( m_data ) <=> std::get<uint64_t>( other.m_data );
            }
            case Type::Double:
            {
                double a = std::get<double>( m_data );
                double b = std::get<double>( other.m_data );
                if( a < b )
                {
                    return std::strong_ordering::less;
                }
                if( a > b )
                {
                    return std::strong_ordering::greater;
                }
                return std::strong_ordering::equal;
            }
            case Type::String:
            {
                return std::get<std::string>( m_data ) <=> std::get<std::string>( other.m_data );
            }
            case Type::Array:
            {
                const auto& a = std::get<Array>( m_data );
                const auto& b = std::get<Array>( other.m_data );
                return std::lexicographical_compare_three_way(
                    a.begin(), a.end(), b.begin(), b.end(), []( const Document& x, const Document& y ) {
                        return x <=> y;
                    } );
            }
            case Type::Object:
            {
                const auto& a = std::get<Object>( m_data );
                const auto& b = std::get<Object>( other.m_data );
                return std::lexicographical_compare_three_way(
                    a.begin(), a.end(), b.begin(), b.end(), []( const auto& x, const auto& y ) {
                        // Compare key first, then value
                        if( auto cmp = x.first <=> y.first; cmp != 0 )
                            return cmp;
                        return x.second <=> y.second;
                    } );
            }
        }
        return std::strong_ordering::equal;
    }

    //----------------------------------------------
    // Serialization
    //----------------------------------------------

    std::string Document::toString( int indent, size_t bufferSize ) const
    {
        JsonWriter writer( indent, bufferSize > 0 ? bufferSize : JsonWriter::DEFAULT_BUFFER_SIZE );
        return writer.write( *this );
    }

    std::vector<uint8_t> Document::toBytes() const
    {
        std::string jsonStr = toString();
        return std::vector<uint8_t>( jsonStr.begin(), jsonStr.end() );
    }

    //----------------------------------------------
    // High-level API with path support
    //----------------------------------------------

    std::optional<Document> Document::fromString( std::string_view jsonStr )
    {
        if( jsonStr.empty() )
        {
            return std::nullopt;
        }

        // Trim whitespace to check for effectively empty strings
        size_t start = 0;
        size_t end = jsonStr.length();

        // Find first non-whitespace character
        while( start < end && nfx::string::isWhitespace( jsonStr[start] ) )
        {
            ++start;
        }

        // Find last non-whitespace character
        while( end > start && nfx::string::isWhitespace( jsonStr[end - 1] ) )
        {
            --end;
        }

        // If only whitespace, it's not valid JSON
        if( start >= end )
        {
            return std::nullopt;
        }

        try
        {
            return JsonParser::parse( jsonStr );
        }
        catch( const std::runtime_error& )
        {
            return std::nullopt;
        }
    }

    bool Document::fromString( std::string_view jsonStr, Document& value )
    {
        auto result = fromString( jsonStr );
        if( result )
        {
            value = std::move( *result );
            return true;
        }
        return false;
    }

    std::optional<Document> Document::fromBytes( const std::vector<uint8_t>& bytes )
    {
        if( bytes.empty() )
        {
            return std::nullopt;
        }

        try
        {
            std::string_view jsonStr( reinterpret_cast<const char*>( bytes.data() ), bytes.size() );
            return JsonParser::parse( jsonStr );
        }
        catch( const std::runtime_error& )
        {
            return std::nullopt;
        }
    }

    bool Document::fromBytes( const std::vector<uint8_t>& bytes, Document& value )
    {
        auto result = fromBytes( bytes );
        if( result )
        {
            value = std::move( *result );
            return true;
        }
        return false;
    }

    void Document::merge( const Document& other, bool overwriteArrays )
    {
        // Recursive merge function
        std::function<void( Document&, const Document& )> mergeRecursive = [&]( Document& target,
                                                                                const Document& source ) -> void {
            if( source.type() == Type::Object && target.type() == Type::Object )
            {
                for( auto it = source.objectBegin(); it != source.objectEnd(); ++it )
                {
                    if( target.contains( it.key() ) && !overwriteArrays && it.value().type() == Type::Array &&
                        target[it.key()].type() == Type::Array )
                    {
                        // Merge arrays by appending
                        for( const auto& item : it.value() )
                        {
                            target[it.key()].push_back( item );
                        }
                    }
                    else if(
                        target.contains( it.key() ) && it.value().type() == Type::Object &&
                        target[it.key()].type() == Type::Object )
                    {
                        // Recursively merge objects
                        mergeRecursive( target[it.key()], it.value() );
                    }
                    else
                    {
                        // Overwrite or set new value
                        target[it.key()] = it.value();
                    }
                }
            }
            else
            {
                // Replace target with source
                target = source;
            }
        };

        mergeRecursive( *this, other );
    }

    void Document::update( std::string_view path, const Document& value )
    {
        // Auto-detect path syntax: paths starting with "/" are JSON Pointer, others are dot notation
        Document* node = nullptr;
        if( !path.empty() && path[0] == '/' )
        {
            // JSON Pointer (RFC 6901)
            node = navigateToJsonPointer( path, true );
        }
        else
        {
            // Dot notation
            node = navigateToPath( path, true );
        }

        if( node )
        {
            *node = value;
        }
    }

    //----------------------------------------------
    // Template-based path operations
    //----------------------------------------------

    template <Value T>
    std::optional<T> Document::get( std::string_view path ) const
    {
        // Auto-detect path syntax: paths starting with "/" are JSON Pointer, others are dot notation
        const Document* node = nullptr;
        if( path.empty() )
        {
            // Empty path handling - get root document
            if constexpr( std::is_same_v<std::decay_t<T>, Document> )
            {
                return *this;
            }
            else
            {
                node = this;
            }
        }
        else if( path[0] == '/' )
        {
            // JSON Pointer (RFC 6901)
            node = const_cast<Document*>( this )->navigateToJsonPointer( path );
        }
        else
        {
            // Dot notation
            node = const_cast<Document*>( this )->navigateToPath( path );
        }

        if( !node )
        {
            return std::nullopt;
        }

        // Type-specific extraction using if constexpr
        if constexpr( std::is_same_v<std::decay_t<T>, std::string> )
        {
            if( node->type() == Type::String )
            {
                return std::get<std::string>( node->m_data );
            }
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, bool> )
        {
            if( node->type() == Type::Boolean )
            {
                return std::get<bool>( node->m_data );
            }
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, char> )
        {
            if( node->type() == Type::String )
            {
                const auto& str = std::get<std::string>( node->m_data );
                if( str.length() == 1 )
                {
                    return str[0];
                }
            }
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, int8_t> )
        {
            if( node->type() == Type::Integer )
            {
                int64_t val = std::get<int64_t>( node->m_data );
                if( val >= std::numeric_limits<int8_t>::min() && val <= std::numeric_limits<int8_t>::max() )
                {
                    return static_cast<int8_t>( val );
                }
            }
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, int16_t> )
        {
            if( node->type() == Type::Integer )
            {
                int64_t val = std::get<int64_t>( node->m_data );
                if( val >= std::numeric_limits<int16_t>::min() && val <= std::numeric_limits<int16_t>::max() )
                {
                    return static_cast<int16_t>( val );
                }
            }
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, int32_t> )
        {
            if( node->type() == Type::Integer )
            {
                return static_cast<int32_t>( std::get<int64_t>( node->m_data ) );
            }
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, int64_t> )
        {
            if( node->type() == Type::Integer )
            {
                return std::get<int64_t>( node->m_data );
            }
            else if( node->type() == Type::Double )
            {
                return static_cast<int64_t>( std::get<double>( node->m_data ) );
            }
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, uint8_t> )
        {
            if( node->type() == Type::UnsignedInteger )
            {
                uint64_t val = std::get<uint64_t>( node->m_data );
                if( val <= std::numeric_limits<uint8_t>::max() )
                {
                    return static_cast<uint8_t>( val );
                }
            }
            else if( node->type() == Type::Integer )
            {
                int64_t val = std::get<int64_t>( node->m_data );
                if( val >= 0 && val <= std::numeric_limits<uint8_t>::max() )
                {
                    return static_cast<uint8_t>( val );
                }
            }
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, uint16_t> )
        {
            if( node->type() == Type::UnsignedInteger )
            {
                uint64_t val = std::get<uint64_t>( node->m_data );
                if( val <= std::numeric_limits<uint16_t>::max() )
                {
                    return static_cast<uint16_t>( val );
                }
            }
            else if( node->type() == Type::Integer )
            {
                int64_t val = std::get<int64_t>( node->m_data );
                if( val >= 0 && val <= std::numeric_limits<uint16_t>::max() )
                {
                    return static_cast<uint16_t>( val );
                }
            }
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, uint32_t> )
        {
            if( node->type() == Type::UnsignedInteger )
            {
                uint64_t val = std::get<uint64_t>( node->m_data );
                if( val <= std::numeric_limits<uint32_t>::max() )
                {
                    return static_cast<uint32_t>( val );
                }
            }
            else if( node->type() == Type::Integer )
            {
                int64_t val = std::get<int64_t>( node->m_data );
                if( val >= 0 && val <= std::numeric_limits<uint32_t>::max() )
                {
                    return static_cast<uint32_t>( val );
                }
            }
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, uint64_t> )
        {
            if( node->type() == Type::UnsignedInteger )
            {
                return std::get<uint64_t>( node->m_data );
            }
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, float> )
        {
            if( node->type() == Type::Double )
            {
                return static_cast<float>( std::get<double>( node->m_data ) );
            }
            else if( node->type() == Type::Integer )
            {
                return static_cast<float>( std::get<int64_t>( node->m_data ) );
            }
            else if( node->type() == Type::UnsignedInteger )
            {
                return static_cast<float>( std::get<uint64_t>( node->m_data ) );
            }
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, double> )
        {
            if( node->type() == Type::Double )
            {
                return std::get<double>( node->m_data );
            }
            else if( node->type() == Type::Integer )
            {
                return static_cast<double>( std::get<int64_t>( node->m_data ) );
            }
            else if( node->type() == Type::UnsignedInteger )
            {
                return static_cast<double>( std::get<uint64_t>( node->m_data ) );
            }
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, Document> )
        {
            return *node;
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, Array> )
        {
            if( node->type() == Type::Array )
            {
                return std::get<Array>( node->m_data );
            }
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, Object> )
        {
            if( node->type() == Type::Object )
            {
                return std::get<Object>( node->m_data );
            }
        }

        return std::nullopt;
    }

    // Explicit template instantiations for get<T>(path)
    template std::optional<std::string> Document::get<std::string>( std::string_view path ) const;
    template std::optional<char> Document::get<char>( std::string_view path ) const;
    template std::optional<bool> Document::get<bool>( std::string_view path ) const;
    template std::optional<int8_t> Document::get<int8_t>( std::string_view path ) const;
    template std::optional<int16_t> Document::get<int16_t>( std::string_view path ) const;
    template std::optional<int32_t> Document::get<int32_t>( std::string_view path ) const;
    template std::optional<int64_t> Document::get<int64_t>( std::string_view path ) const;
    template std::optional<uint8_t> Document::get<uint8_t>( std::string_view path ) const;
    template std::optional<uint16_t> Document::get<uint16_t>( std::string_view path ) const;
    template std::optional<uint32_t> Document::get<uint32_t>( std::string_view path ) const;
    template std::optional<uint64_t> Document::get<uint64_t>( std::string_view path ) const;
    template std::optional<float> Document::get<float>( std::string_view path ) const;
    template std::optional<double> Document::get<double>( std::string_view path ) const;
    template std::optional<Document> Document::get<Document>( std::string_view path ) const;
    template std::optional<Array> Document::get<Array>( std::string_view path ) const;
    template std::optional<Object> Document::get<Object>( std::string_view path ) const;

    // Output parameter version
    template <Value T>
    bool Document::get( std::string_view path, T& value ) const
    {
        auto result = get<T>( path );
        if( result )
        {
            value = std::move( *result );
            return true;
        }
        return false;
    }

    // Explicit template instantiations for get<T>(path, value&)
    template bool Document::get<std::string>( std::string_view path, std::string& value ) const;
    template bool Document::get<char>( std::string_view path, char& value ) const;
    template bool Document::get<bool>( std::string_view path, bool& value ) const;
    template bool Document::get<int8_t>( std::string_view path, int8_t& value ) const;
    template bool Document::get<int16_t>( std::string_view path, int16_t& value ) const;
    template bool Document::get<int32_t>( std::string_view path, int32_t& value ) const;
    template bool Document::get<int64_t>( std::string_view path, int64_t& value ) const;
    template bool Document::get<uint8_t>( std::string_view path, uint8_t& value ) const;
    template bool Document::get<uint16_t>( std::string_view path, uint16_t& value ) const;
    template bool Document::get<uint32_t>( std::string_view path, uint32_t& value ) const;
    template bool Document::get<uint64_t>( std::string_view path, uint64_t& value ) const;
    template bool Document::get<float>( std::string_view path, float& value ) const;
    template bool Document::get<double>( std::string_view path, double& value ) const;
    template bool Document::get<Document>( std::string_view path, Document& value ) const;
    template bool Document::get<Array>( std::string_view path, Array& value ) const;
    template bool Document::get<Object>( std::string_view path, Object& value ) const;

    // Reference version (const)
    template <Value T>
    std::optional<std::reference_wrapper<const T>> Document::getRef( std::string_view path ) const
    {
        // Navigate to the path
        const Document* node = nullptr;
        if( path.empty() )
        {
            node = this;
        }
        else if( path[0] == '/' )
        {
            // JSON Pointer (RFC 6901)
            node = const_cast<Document*>( this )->navigateToJsonPointer( path );
        }
        else
        {
            // Dot notation
            node = const_cast<Document*>( this )->navigateToPath( path );
        }

        if( !node )
        {
            return std::nullopt;
        }

        // Special case for Document type - return reference to the node itself
        if constexpr( std::is_same_v<T, Document> )
        {
            return std::cref( *node );
        }
        else
        {
            // Check if the node holds the requested type
            if( std::holds_alternative<T>( node->m_data ) )
            {
                return std::cref( std::get<T>( node->m_data ) );
            }
        }

        return std::nullopt;
    }

    // Explicit template instantiations for getRef<T>(path) const
    template std::optional<std::reference_wrapper<const std::string>> Document::getRef<std::string>(
        std::string_view path ) const;
    template std::optional<std::reference_wrapper<const bool>> Document::getRef<bool>( std::string_view path ) const;
    template std::optional<std::reference_wrapper<const int64_t>> Document::getRef<int64_t>(
        std::string_view path ) const;
    template std::optional<std::reference_wrapper<const uint64_t>> Document::getRef<uint64_t>(
        std::string_view path ) const;
    template std::optional<std::reference_wrapper<const double>> Document::getRef<double>(
        std::string_view path ) const;
    template std::optional<std::reference_wrapper<const Document>> Document::getRef<Document>(
        std::string_view path ) const;
    template std::optional<std::reference_wrapper<const Array>> Document::getRef<Array>( std::string_view path ) const;
    template std::optional<std::reference_wrapper<const Object>> Document::getRef<Object>(
        std::string_view path ) const;

    // Reference version (mutable)
    template <Value T>
    std::optional<std::reference_wrapper<T>> Document::getRef( std::string_view path )
    {
        // Navigate to the path
        Document* node = nullptr;
        if( path.empty() )
        {
            node = this;
        }
        else if( path[0] == '/' )
        {
            // JSON Pointer (RFC 6901)
            node = navigateToJsonPointer( path );
        }
        else
        {
            // Dot notation
            node = navigateToPath( path );
        }

        if( !node )
        {
            return std::nullopt;
        }

        // Special case for Document type - return reference to the node itself
        if constexpr( std::is_same_v<T, Document> )
        {
            return std::ref( *node );
        }
        else
        {
            // Check if the node holds the requested type
            if( std::holds_alternative<T>( node->m_data ) )
            {
                return std::ref( std::get<T>( node->m_data ) );
            }
        }

        return std::nullopt;
    }

    // Explicit template instantiations for getRef<T>(path) mutable
    template std::optional<std::reference_wrapper<std::string>> Document::getRef<std::string>( std::string_view path );
    template std::optional<std::reference_wrapper<bool>> Document::getRef<bool>( std::string_view path );
    template std::optional<std::reference_wrapper<int64_t>> Document::getRef<int64_t>( std::string_view path );
    template std::optional<std::reference_wrapper<uint64_t>> Document::getRef<uint64_t>( std::string_view path );
    template std::optional<std::reference_wrapper<double>> Document::getRef<double>( std::string_view path );
    template std::optional<std::reference_wrapper<Document>> Document::getRef<Document>( std::string_view path );
    template std::optional<std::reference_wrapper<Array>> Document::getRef<Array>( std::string_view path );
    template std::optional<std::reference_wrapper<Object>> Document::getRef<Object>( std::string_view path );

    // Copy version
    template <Value T>
    void Document::set( std::string_view path, const T& value )
    {
        // Auto-detect path syntax: paths starting with "/" are JSON Pointer, others are dot notation
        Document* node = nullptr;
        if( !path.empty() && path[0] == '/' )
        {
            // JSON Pointer (RFC 6901)
            node = navigateToJsonPointer( path, true );
        }
        else
        {
            // Dot notation
            node = navigateToPath( path, true );
        }

        if( !node )
        {
            return;
        }

        // Type-specific assignment using if constexpr
        if constexpr(
            std::is_same_v<std::decay_t<T>, std::string> || std::is_same_v<std::decay_t<T>, std::string_view> )
        {
            *node = std::string( value );
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, char> )
        {
            *node = std::string( 1, value );
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, bool> )
        {
            *node = value;
        }
        else if constexpr(
            std::is_same_v<std::decay_t<T>, int8_t> || std::is_same_v<std::decay_t<T>, int16_t> ||
            std::is_same_v<std::decay_t<T>, int32_t> )
        {
            *node = static_cast<int64_t>( value );
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, int64_t> )
        {
            *node = value;
        }
        else if constexpr(
            std::is_same_v<std::decay_t<T>, uint8_t> || std::is_same_v<std::decay_t<T>, uint16_t> ||
            std::is_same_v<std::decay_t<T>, uint32_t> )
        {
            *node = static_cast<uint64_t>( value );
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, uint64_t> )
        {
            *node = value;
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, float> )
        {
            *node = static_cast<double>( value );
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, double> )
        {
            *node = value;
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, Document> )
        {
            *node = value;
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, Array> )
        {
            *node = Document{ value };
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, Object> )
        {
            *node = Document{ value };
        }
    }

    // Explicit template instantiations for set<T>(path, const T&)
    template void Document::set<std::string>( std::string_view path, const std::string& );
    template void Document::set<char>( std::string_view path, const char& );
    template void Document::set<bool>( std::string_view path, const bool& );
    template void Document::set<int8_t>( std::string_view path, const int8_t& );
    template void Document::set<int16_t>( std::string_view path, const int16_t& );
    template void Document::set<int32_t>( std::string_view path, const int32_t& );
    template void Document::set<int64_t>( std::string_view path, const int64_t& );
    template void Document::set<uint8_t>( std::string_view path, const uint8_t& );
    template void Document::set<uint16_t>( std::string_view path, const uint16_t& );
    template void Document::set<uint32_t>( std::string_view path, const uint32_t& );
    template void Document::set<uint64_t>( std::string_view path, const uint64_t& );
    template void Document::set<float>( std::string_view path, const float& );
    template void Document::set<double>( std::string_view path, const double& );
    template void Document::set<Document>( std::string_view path, const Document& );
    template void Document::set<Array>( std::string_view path, const Array& );
    template void Document::set<Object>( std::string_view path, const Object& );

    // Move version
    template <Value T>
    void Document::set( std::string_view path, T&& value )
    {
        // Auto-detect path syntax: paths starting with "/" are JSON Pointer, others are dot notation
        Document* node = nullptr;
        if( !path.empty() && path[0] == '/' )
        {
            // JSON Pointer (RFC 6901)
            node = navigateToJsonPointer( path, true );
        }
        else
        {
            // Dot notation
            node = navigateToPath( path, true );
        }

        if( !node )
        {
            return;
        }

        // Type-specific assignment using if constexpr (move semantics)
        if constexpr(
            std::is_same_v<std::decay_t<T>, std::string> || std::is_same_v<std::decay_t<T>, std::string_view> )
        {
            *node = std::move( value );
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, char> )
        {
            *node = std::string( 1, value );
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, bool> )
        {
            *node = value;
        }
        else if constexpr(
            std::is_same_v<std::decay_t<T>, int8_t> || std::is_same_v<std::decay_t<T>, int16_t> ||
            std::is_same_v<std::decay_t<T>, int32_t> )
        {
            *node = static_cast<int64_t>( value );
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, int64_t> )
        {
            *node = value;
        }
        else if constexpr(
            std::is_same_v<std::decay_t<T>, uint8_t> || std::is_same_v<std::decay_t<T>, uint16_t> ||
            std::is_same_v<std::decay_t<T>, uint32_t> )
        {
            *node = static_cast<uint64_t>( value );
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, uint64_t> )
        {
            *node = value;
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, float> )
        {
            *node = static_cast<double>( value );
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, double> )
        {
            *node = value;
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, Document> )
        {
            *node = std::move( value );
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, Array> )
        {
            *node = Document{ std::move( value ) };
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, Object> )
        {
            *node = Document{ std::move( value ) };
        }
    }

    // Explicit template instantiations for set<T>(path, T&&)
    template void Document::set<std::string>( std::string_view path, std::string&& );
    template void Document::set<char>( std::string_view path, char&& );
    template void Document::set<bool>( std::string_view path, bool&& );
    template void Document::set<int8_t>( std::string_view path, int8_t&& );
    template void Document::set<int16_t>( std::string_view path, int16_t&& );
    template void Document::set<int32_t>( std::string_view path, int32_t&& );
    template void Document::set<int64_t>( std::string_view path, int64_t&& );
    template void Document::set<uint8_t>( std::string_view path, uint8_t&& );
    template void Document::set<uint16_t>( std::string_view path, uint16_t&& );
    template void Document::set<uint32_t>( std::string_view path, uint32_t&& );
    template void Document::set<uint64_t>( std::string_view path, uint64_t&& );
    template void Document::set<float>( std::string_view path, float&& );
    template void Document::set<double>( std::string_view path, double&& );
    template void Document::set<Document>( std::string_view path, Document&& );
    template void Document::set<Array>( std::string_view path, Array&& );
    template void Document::set<Object>( std::string_view path, Object&& );

    // Type-only creation for containers
    template <Container T>
    void Document::set( std::string_view path )
    {
        // Auto-detect path syntax: paths starting with "/" are JSON Pointer, others are dot notation
        Document* node = nullptr;
        if( !path.empty() && path[0] == '/' )
        {
            // JSON Pointer (RFC 6901)
            node = navigateToJsonPointer( path, true );
        }
        else
        {
            // Dot notation
            node = navigateToPath( path, true );
        }

        if( node )
        {
            if constexpr( std::is_same_v<std::decay_t<T>, Document> )
            {
                *node = Document{};
            }
            else if constexpr( std::is_same_v<std::decay_t<T>, Object> )
            {
                *node = Document{ Object() };
            }
            else if constexpr( std::is_same_v<std::decay_t<T>, Array> )
            {
                *node = Document{ Array() };
            }
        }
    }

    // Explicit template instantiations for set<T>(path)
    template void Document::set<Document>( std::string_view path );
    template void Document::set<Object>( std::string_view path );
    template void Document::set<Array>( std::string_view path );

    void Document::setNull( std::string_view path )
    {
        // Auto-detect path syntax: paths starting with "/" are JSON Pointer, others are dot notation
        Document* node = nullptr;
        if( !path.empty() && path[0] == '/' )
        {
            // JSON Pointer (RFC 6901)
            node = navigateToJsonPointer( path, true );
        }
        else
        {
            // Dot notation
            node = navigateToPath( path, true );
        }

        if( node )
        {
            *node = nullptr;
        }
    }

    template <Checkable T>
    bool Document::is( std::string_view path ) const
    {
        // Auto-detect path syntax: paths starting with "/" are JSON Pointer, others are dot notation
        const Document* node = nullptr;
        if( !path.empty() && path[0] == '/' )
        {
            // JSON Pointer (RFC 6901)
            node = const_cast<Document*>( this )->navigateToJsonPointer( path );
        }
        else
        {
            // Dot notation
            node = const_cast<Document*>( this )->navigateToPath( path );
        }

        if( !node )
        {
            return false;
        }

        // Type-specific checking using if constexpr
        if constexpr(
            std::is_same_v<std::decay_t<T>, std::string> || std::is_same_v<std::decay_t<T>, std::string_view> )
        {
            return node->type() == Type::String;
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, char> )
        {
            return node->type() == Type::String && node->rootInternal<std::string>().length() == 1;
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, bool> )
        {
            return node->type() == Type::Boolean;
        }
        else if constexpr(
            std::is_same_v<std::decay_t<T>, int8_t> || std::is_same_v<std::decay_t<T>, int16_t> ||
            std::is_same_v<std::decay_t<T>, int32_t> || std::is_same_v<std::decay_t<T>, int64_t> )
        {
            return node->type() == Type::Integer;
        }
        else if constexpr(
            std::is_same_v<std::decay_t<T>, uint8_t> || std::is_same_v<std::decay_t<T>, uint16_t> ||
            std::is_same_v<std::decay_t<T>, uint32_t> || std::is_same_v<std::decay_t<T>, uint64_t> )
        {
            return node->type() == Type::UnsignedInteger || node->type() == Type::Integer;
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, float> )
        {
            return node->type() == Type::Double;
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, double> )
        {
            return node->type() == Type::Double;
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, Array> )
        {
            return node->type() == Type::Array;
        }
        else if constexpr( std::is_same_v<std::decay_t<T>, Object> )
        {
            return node->type() == Type::Object;
        }

        return false;
    }

    // Explicit template instantiations for is<T>(path)
    template bool Document::is<std::string>( std::string_view path ) const;
    template bool Document::is<char>( std::string_view path ) const;
    template bool Document::is<bool>( std::string_view path ) const;
    template bool Document::is<int8_t>( std::string_view path ) const;
    template bool Document::is<int16_t>( std::string_view path ) const;
    template bool Document::is<int32_t>( std::string_view path ) const;
    template bool Document::is<int64_t>( std::string_view path ) const;
    template bool Document::is<uint8_t>( std::string_view path ) const;
    template bool Document::is<uint16_t>( std::string_view path ) const;
    template bool Document::is<uint32_t>( std::string_view path ) const;
    template bool Document::is<uint64_t>( std::string_view path ) const;
    template bool Document::is<float>( std::string_view path ) const;
    template bool Document::is<double>( std::string_view path ) const;
    template bool Document::is<Array>( std::string_view path ) const;
    template bool Document::is<Object>( std::string_view path ) const;

    bool Document::isNull( std::string_view path ) const
    {
        // Auto-detect path syntax: paths starting with "/" are JSON Pointer, others are dot notation
        const Document* node = nullptr;
        if( !path.empty() && path[0] == '/' )
        {
            // JSON Pointer (RFC 6901)
            node = const_cast<Document*>( this )->navigateToJsonPointer( path );
        }
        else
        {
            // Dot notation
            node = const_cast<Document*>( this )->navigateToPath( path );
        }

        return node && node->type() == Type::Null;
    }

    bool Document::isValid() const
    {
        return true; // Document is always valid
    }

    //----------------------------------------------
    // Document::PathView class
    //----------------------------------------------

    //-----------------------------
    // Path entry structure
    //-----------------------------

    Document::PathView::Entry::Entry( const Entry& other )
        : path{ other.path },
          valuePtr{ other.valuePtr ? std::make_unique<Document>( *other.valuePtr ) : nullptr },
          depth{ other.depth },
          isLeaf{ other.isLeaf }
    {
    }

    Document::PathView::Entry& Document::PathView::Entry::operator=( const Entry& other )
    {
        if( this != &other )
        {
            path = other.path;
            valuePtr = other.valuePtr ? std::make_unique<Document>( *other.valuePtr ) : nullptr;
            depth = other.depth;
            isLeaf = other.isLeaf;
        }
        return *this;
    }

    //-----------------------------
    // Construction
    //-----------------------------

    Document::PathView::PathView( const Document& doc, Format format, bool includeContainers )
        : m_format{ format },
          m_includeContainers{ includeContainers }
    {
        if( doc.isValid() )
        {
            buildEntries( doc );
        }
    }

    //-----------------------------
    // Filtering
    //-----------------------------

    std::vector<Document::PathView::Entry> Document::PathView::leaves() const
    {
        std::vector<Entry> result;
        for( const auto& entry : m_entries )
        {
            if( entry.isLeaf )
            {
                result.push_back( entry );
            }
        }
        return result;
    }

    //-----------------------------
    // Private helper methods
    //-----------------------------

    void Document::PathView::buildEntries( const Document& doc )
    {
        // Stack for depth-first traversal
        struct StackEntry
        {
            const Document* node;
            std::vector<std::string> segments;
            size_t depth;
        };

        std::vector<StackEntry> stack;
        stack.push_back( { &doc, {}, 0 } );

        while( !stack.empty() )
        {
            auto current = stack.back();
            stack.pop_back();

            bool isContainer = current.node->type() == Type::Object || current.node->type() == Type::Array;
            bool isLeaf = !isContainer;

            // Should we include this entry?
            bool shouldInclude = ( m_includeContainers || isLeaf );

            // Don't include root with empty path unless it's a leaf
            if( current.segments.empty() && isContainer )
            {
                shouldInclude = false;
            }

            if( shouldInclude && !current.segments.empty() )
            {
                Entry entry;
                entry.path = formatPath( current.segments );
                entry.depth = current.depth;
                entry.isLeaf = isLeaf;

                // Create a Document copy for this value
                entry.valuePtr = std::make_unique<Document>( *current.node );

                m_entries.push_back( std::move( entry ) );
            }

            // Push children in reverse order for correct traversal order
            if( current.node->type() == Type::Object )
            {
                std::vector<std::string> keys;

                for( auto it = current.node->objectBegin(); it != current.node->objectEnd(); ++it )
                {
                    keys.push_back( it.key() );
                }

                for( auto rit = keys.rbegin(); rit != keys.rend(); ++rit )
                {
                    std::vector<std::string> childSegments = current.segments;
                    childSegments.push_back( *rit );

                    // Navigate to child
                    auto childDoc = current.node->navigateToPath( *rit );
                    if( childDoc )
                    {
                        stack.push_back( { childDoc, childSegments, current.depth + 1 } );
                    }
                }
            }
            else if( current.node->type() == Type::Array )
            {
                size_t size = current.node->size();
                for( size_t i = size; i > 0; --i )
                {
                    std::vector<std::string> childSegments = current.segments;
                    childSegments.push_back( std::to_string( i - 1 ) );

                    // Navigate to child using JSON Pointer
                    std::string indexPath = "/" + std::to_string( i - 1 );
                    auto childDoc = current.node->navigateToJsonPointer( indexPath );
                    if( childDoc )
                    {
                        stack.push_back( { childDoc, childSegments, current.depth + 1 } );
                    }
                }
            }
        }
    }

    std::string Document::PathView::formatPath( const std::vector<std::string>& segments ) const
    {
        if( segments.empty() )
        {
            return "";
        }

        std::string result;

        if( m_format == Format::JsonPointer )
        {
            // RFC 6901: /segment1/segment2/...
            for( const auto& segment : segments )
            {
                result += '/';
                // Escape ~ as ~0 and / as ~1
                for( char c : segment )
                {
                    if( c == '~' )
                    {
                        result += "~0";
                    }
                    else if( c == '/' )
                    {
                        result += "~1";
                    }
                    else
                    {
                        result += c;
                    }
                }
            }
        }
        else // DotNotation
        {
            // user.addresses[0].city
            bool first = true;
            for( const auto& segment : segments )
            {
                // Check if segment is a numeric index
                bool isIndex = !segment.empty() && std::all_of( segment.begin(), segment.end(), []( char c ) {
                    return nfx::string::isDigit( c );
                } );

                if( isIndex )
                {
                    result += '[';
                    result += segment;
                    result += ']';
                }
                else
                {
                    if( !first )
                    {
                        result += '.';
                    }
                    result += segment;
                }
                first = false;
            }
        }

        return result;
    }

    //----------------------------------------------
    // Private path helpers
    //----------------------------------------------

    bool Document::containsPath( std::string_view path ) const
    {
        // Auto-detect path syntax: paths starting with "/" are JSON Pointer, others are dot notation
        const Document* node = nullptr;
        if( !path.empty() && path[0] == '/' )
        {
            // JSON Pointer (RFC 6901)
            node = navigateToJsonPointer( path );
        }
        else
        {
            // Dot notation
            node = navigateToPath( path );
        }

        // Return true if any JSON value exists at the path
        return node != nullptr;
    }

    size_t Document::erasePath( std::string_view path )
    {
        if( path.empty() )
        {
            return 0;
        }

        // Auto-detect path syntax: paths starting with "/" are JSON Pointer
        bool isJsonPointer = !path.empty() && path[0] == '/';

        // Find the last separator to split parent path from final key
        std::string_view parentPath;
        std::string_view finalKey;

        if( isJsonPointer )
        {
            // JSON Pointer: find last '/'
            size_t lastSlash = path.rfind( '/' );
            if( lastSlash == 0 )
            {
                // Root level like "/key"
                parentPath = "";
                finalKey = path.substr( 1 );
            }
            else if( lastSlash != std::string_view::npos )
            {
                // Nested like "/user/profile/name"
                parentPath = path.substr( 0, lastSlash );
                finalKey = path.substr( lastSlash + 1 );
            }
            else
            {
                return 0; // Invalid JSON Pointer
            }
        }
        else
        {
            // Dot notation: find last '.' or '['
            size_t lastDot = path.rfind( '.' );
            size_t lastBracket = path.rfind( '[' );
            size_t lastSep = std::string_view::npos;

            if( lastDot != std::string_view::npos && lastBracket != std::string_view::npos )
            {
                lastSep = std::max( lastDot, lastBracket );
            }
            else if( lastDot != std::string_view::npos )
            {
                lastSep = lastDot;
            }
            else if( lastBracket != std::string_view::npos )
            {
                lastSep = lastBracket;
            }

            if( lastSep == std::string_view::npos )
            {
                // Root level like "key"
                parentPath = "";
                finalKey = path;
            }
            else
            {
                // Nested like "user.profile.name" or "data[0]"
                parentPath = path.substr( 0, lastSep );
                finalKey = path.substr( lastSep + 1 );

                // For array notation, we need to handle "data[0]" specially
                if( lastSep == lastBracket )
                {
                    // finalKey is like "0]" - extract the index
                    size_t closeBracket = finalKey.find( ']' );
                    if( closeBracket != std::string_view::npos )
                    {
                        finalKey = finalKey.substr( 0, closeBracket );
                    }
                }
            }
        }

        // Navigate to parent
        Document* parent = nullptr;
        if( parentPath.empty() )
        {
            parent = this;
        }
        else if( isJsonPointer )
        {
            parent = navigateToJsonPointer( parentPath, false );
        }
        else
        {
            parent = navigateToPath( parentPath, false );
        }

        if( !parent )
        {
            return 0; // Parent doesn't exist
        }

        // Erase the final key
        std::string finalKeyStr( finalKey );
        return parent->erase( finalKeyStr );
    }

    //----------------------------------------------
    // Private navigation methods
    //----------------------------------------------

    Document* Document::navigateToPath( std::string_view path, bool createPath )
    {
        if( path.empty() )
        {
            return this;
        }

        // If document is null/empty and we need to create path, initialize as object
        if( createPath && type() == Type::Null )
        {
            *this = Document{ Object{} };
        }

        Document* current = this;
        size_t start = 0;

        while( start < path.length() )
        {
            size_t pos = path.find( '.', start );
            std::string_view segmentView = path.substr( start, pos == std::string_view::npos ? pos : pos - start );

            if( segmentView.empty() )
            {
                start = pos + 1;
                continue;
            }

            // Handle bracket notation - can be nested like "data[0][1]" or "matrix[0][1][2]"
            size_t bracketPos = segmentView.find( '[' );
            if( bracketPos != std::string_view::npos )
            {
                // Early validation: check all brackets are valid before creating anything
                if( createPath && bracketPos > 0 )
                {
                    size_t validatePos = bracketPos;
                    while( validatePos < segmentView.length() && segmentView[validatePos] == '[' )
                    {
                        size_t closeBracket = segmentView.find( ']', validatePos );
                        if( closeBracket == std::string_view::npos )
                        {
                            return nullptr; // Malformed bracket
                        }

                        std::string_view indexCheck =
                            segmentView.substr( validatePos + 1, closeBracket - validatePos - 1 );
                        if( indexCheck.empty() )
                        {
                            return nullptr; // Empty brackets
                        }

                        // Validate numeric
                        if( !nfx::string::isAllDigits( indexCheck ) )
                        {
                            return nullptr; // Non-numeric index
                        }

                        validatePos = closeBracket + 1;
                    }
                }

                // First handle the field name before the first bracket (if any)
                if( bracketPos > 0 )
                {
                    std::string_view fieldNameView = segmentView.substr( 0, bracketPos );
                    std::string fieldName( fieldNameView );

                    if( createPath && current->find( fieldName ) == nullptr )
                    {
                        // Validate that current node can have object fields
                        if( current->type() != Type::Object && current->type() != Type::Null )
                        {
                            return nullptr;
                        }

                        current->set( fieldName, Document{ Array{} } );
                    }

                    current = current->find( fieldName );
                    if( !current )
                    {
                        return nullptr;
                    }
                }

                // Now process all bracket indices in sequence (e.g., [0][1][2])
                size_t bracketStart = bracketPos;
                while( bracketStart < segmentView.length() && segmentView[bracketStart] == '[' )
                {
                    size_t bracketEnd = segmentView.find( ']', bracketStart );
                    if( bracketEnd == std::string_view::npos )
                    {
                        // Malformed bracket - no closing ']'
                        return nullptr;
                    }

                    std::string_view indexView = segmentView.substr( bracketStart + 1, bracketEnd - bracketStart - 1 );

                    if( indexView.empty() )
                    {
                        // Empty brackets []
                        return nullptr;
                    }

                    // Validate that current node is or can become an array
                    if( createPath && current->type() == Type::Null )
                    {
                        *current = Document{ Array{} };
                    }

                    if( current->type() != Type::Array )
                    {
                        return nullptr;
                    }

                    try
                    {
                        std::string indexStr( indexView );

                        // Validate that index string contains only digits
                        if( !nfx::string::isAllDigits( indexView ) )
                        {
                            return nullptr; // Non-numeric index
                        }

                        size_t index = std::stoull( indexStr );

                        if( createPath )
                        {
                            // Extend array if needed, filling gaps with null
                            while( current->rootInternal<Array>().size() <= index )
                            {
                                current->push_back( Document{ nullptr } ); // Explicitly null
                            }

                            // Check if this is the last bracket in the chain
                            bool isLastBracket =
                                ( bracketEnd + 1 >= segmentView.length() || segmentView[bracketEnd + 1] != '[' );

                            // Only initialize the specific element we're accessing, not the gaps
                            if( !isLastBracket )
                            {
                                // More brackets follow - ensure this specific element is an array
                                if( current->rootInternal<Array>()[index].type() == Type::Null )
                                {
                                    current->rootInternal<Array>()[index] = Document{ Array{} };
                                }
                            }
                            else if( pos != std::string_view::npos )
                            {
                                // More path segments follow (after the dot) - ensure this specific element is an object
                                if( current->rootInternal<Array>()[index].type() == Type::Null )
                                {
                                    current->rootInternal<Array>()[index] = Document{ Object{} };
                                }
                            }
                            // else: This is the final element, leave it as Null - the caller will set its value
                        }
                        else if( index >= current->rootInternal<Array>().size() )
                        {
                            return nullptr;
                        }

                        current = &current->rootInternal<Array>()[index];
                    }
                    catch( ... )
                    {
                        return nullptr;
                    }

                    // Move to next bracket (if any)
                    bracketStart = bracketEnd + 1;
                }
            }
            else
            {
                // Regular field access - check if it's a numeric index for array access
                std::string segment( segmentView );

                // Check if current node is an array and segment is numeric
                bool isNumericIndex = !segment.empty() && nfx::string::isAllDigits( segment );

                if( current->type() == Type::Array && isNumericIndex )
                {
                    // Treat numeric segment as array index (supports dot notation like "items.0")
                    try
                    {
                        size_t index = std::stoull( segment );

                        if( createPath )
                        {
                            // Extend array if needed
                            while( current->rootInternal<Array>().size() <= index )
                            {
                                current->push_back( Document{ nullptr } ); // Explicitly null
                            }

                            // If more path segments follow, ensure element is an object
                            if( pos != std::string_view::npos &&
                                current->rootInternal<Array>()[index].type() == Type::Null )
                            {
                                current->rootInternal<Array>()[index] = Document{ Object{} };
                            }
                        }
                        else if( index >= current->rootInternal<Array>().size() )
                        {
                            return nullptr;
                        }

                        current = &current->rootInternal<Array>()[index];
                    }
                    catch( ... )
                    {
                        return nullptr;
                    }
                }
                else
                {
                    // Regular object field access
                    if( createPath && current->find( segment ) == nullptr )
                    {
                        // Validate that current node is a container type that can have fields
                        if( current->type() != Type::Object && current->type() != Type::Null )
                        {
                            return nullptr;
                        }

                        // Look ahead to next segment to determine what type to create
                        Document valueToSet{ Object{} }; // Default to object

                        if( pos != std::string_view::npos )
                        {
                            // There's a next segment - check if it's numeric
                            size_t nextStart = pos + 1;
                            size_t nextDot = path.find( '.', nextStart );
                            std::string_view nextSegmentView = path.substr(
                                nextStart,
                                nextDot == std::string_view::npos ? std::string_view::npos : nextDot - nextStart );

                            // Check if next segment is numeric or starts with bracket
                            if( !nextSegmentView.empty() )
                            {
                                if( nextSegmentView[0] == '[' || nfx::string::isAllDigits( nextSegmentView ) )
                                {
                                    valueToSet = Document{ Array{} };
                                }
                            }
                        }

                        current->set( segment, std::move( valueToSet ) );
                    }

                    current = current->find( segment );

                    if( !current )
                    {
                        return nullptr;
                    }
                }
            }

            if( pos == std::string_view::npos )
            {
                break;
            }

            start = pos + 1;
        }

        return current;
    }

    const Document* Document::navigateToPath( std::string_view path ) const
    {
        return const_cast<Document*>( this )->navigateToPath( path, false );
    }

    Document* Document::navigateToJsonPointer( std::string_view pointer, bool createPath )
    {
        // RFC 6901: Empty string means root document
        if( pointer.empty() )
        {
            return this;
        }

        // RFC 6901: JSON Pointer must start with "/"
        if( pointer[0] != '/' )
        {
            return nullptr;
        }

        Document* current = this;

        // If document is null/empty and we need to create path, initialize appropriately
        if( createPath && ( type() == Type::Null || ( type() == Type::Object && size() == 0 ) ) )
        {
            // Look at first token to determine if we need array or object
            size_t firstSlash = pointer.find( '/', 1 );
            size_t firstTokenEnd = ( firstSlash == std::string_view::npos ) ? pointer.length() : firstSlash;
            std::string_view firstTokenView = pointer.substr( 1, firstTokenEnd - 1 );
            std::string firstToken = unescapeJsonPointerToken( firstTokenView );

            // Initialize as array if first token is a valid array index or "-"
            if( isValidArrayIndex( firstToken ) || firstToken == "-" )
            {
                *this = Document{ Array{} };
            }
            else
            {
                *this = Document{ Object{} };
            }
        }

        size_t start = 1; // Skip initial "/"

        while( start < pointer.length() )
        {
            // Find next "/" or end of string
            size_t pos = pointer.find( '/', start );
            if( pos == std::string_view::npos )
            {
                pos = pointer.length();
            }

            std::string_view tokenView = pointer.substr( start, pos - start );

            if( tokenView.empty() )
            {
                // Empty token - invalid pointer
                return nullptr;
            }

            // Unescape the token according to RFC 6901
            std::string token = unescapeJsonPointerToken( tokenView );

            // Handle array indexing
            if( current->type() == Type::Array )
            {
                // Special case: "-" means append to array (only valid for creation)
                if( token == "-" )
                {
                    if( createPath && pos == pointer.length() )
                    {
                        // Append new element to array
                        current->push_back( Document{ Object{} } );
                        return &current->rootInternal<Array>().back();
                    }
                    else
                    {
                        // "-" not valid for navigation without creation
                        return nullptr;
                    }
                }

                // Validate array index
                if( !isValidArrayIndex( token ) )
                {
                    return nullptr;
                }

                try
                {
                    size_t index = std::stoull( token );

                    if( createPath )
                    {
                        // Extend array if needed
                        while( current->size() <= index )
                        {
                            current->push_back( Document{ Object{} } );
                        }
                    }
                    else if( index >= current->size() )
                    {
                        return nullptr;
                    }

                    current = &current->rootInternal<Array>()[index];
                }
                catch( ... )
                {
                    return nullptr;
                }
            }
            // Handle object property access
            else if( current->type() == Type::Object )
            {
                if( createPath && current->find( token ) == nullptr )
                {
                    // Determine if next level should be array or object
                    if( pos < pointer.length() )
                    {
                        // Look ahead to next token to determine type
                        size_t nextStart = pos + 1;
                        size_t nextEnd = pointer.find( '/', nextStart );
                        if( nextEnd == std::string_view::npos )
                        {
                            nextEnd = pointer.length();
                        }

                        std::string_view nextTokenView = pointer.substr( nextStart, nextEnd - nextStart );
                        std::string nextToken = unescapeJsonPointerToken( nextTokenView );

                        if( isValidArrayIndex( nextToken ) || nextToken == "-" )
                        {
                            current->set( token, Document{ Array{} } );
                        }
                        else
                        {
                            current->set( token, Document{ Object{} } );
                        }
                    }
                    else
                    {
                        // This is the final token, create as object by default
                        current->set( token, Document{ Object{} } );
                    }
                }

                current = current->find( token );
                if( !current )
                {
                    return nullptr;
                }
            }
            else
            {
                // Current node is neither array nor object
                return nullptr;
            }

            start = pos + 1;
        }

        return current;
    }

    const Document* Document::navigateToJsonPointer( std::string_view pointer ) const
    {
        return const_cast<Document*>( this )->navigateToJsonPointer( pointer, false );
    }

    std::string Document::unescapeJsonPointerToken( std::string_view token ) noexcept
    {
        std::string result;
        result.reserve( token.length() );

        for( size_t i = 0; i < token.length(); ++i )
        {
            if( token[i] == '~' && i + 1 < token.length() )
            {
                if( token[i + 1] == '1' )
                {
                    result += '/';
                    ++i; // Skip the '1'
                }
                else if( token[i + 1] == '0' )
                {
                    result += '~';
                    ++i; // Skip the '0'
                }
                else
                {
                    // Invalid escape sequence, include as-is
                    result += token[i];
                }
            }
            else
            {
                result += token[i];
            }
        }

        return result;
    }

    bool Document::isValidArrayIndex( std::string_view token ) noexcept
    {
        if( token.empty() )
        {
            return false;
        }

        // RFC 6901: Array index must be either "0" or not start with "0"
        if( token.length() > 1 && token[0] == '0' )
        {
            return false;
        }

        return nfx::string::isAllDigits( token );
    }
} // namespace nfx::json
