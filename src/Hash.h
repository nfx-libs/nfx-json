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
 * @file Hash.h
 * @brief std::hash specialization for JSON Document
 * @details Provides CRC32-C hardware-accelerated hashing for Document types.
 *          Used by FastHashSet/FastHashMap for O(n) uniqueItems validation.
 */

#pragma once

#include <nfx/json/Document.h>

#include <nfx/hashing/Algorithms.h>

namespace std
{
    /**
     * @brief std::hash specialization for nfx::json::Document
     * @details Uses CRC32-C algorithm (hardware-accelerated with SSE4.2) for 32-bit hash.
     *          Hash is type-discriminated (different types produce different hashes) and
     *          recursive for arrays and objects.
     */
    template <>
    struct hash<nfx::json::Document>
    {
        /** @brief Hash result type (32-bit unsigned integer) */
        using result_type = uint32_t;

        /** @brief Hash argument type (const reference to Document) */
        using argument_type = nfx::json::Document;

        /** @brief Maximum recursion depth to prevent stack overflow */
        static constexpr size_t MAX_RECURSION_DEPTH = 256;

        /**
         * @brief Hash a JSON Document using CRC32-C
         * @param doc The document to hash
         * @param depth Current recursion depth (internal use)
         * @return 32-bit hash value
         * @note noexcept - hashing never throws
         */
        result_type operator()( const nfx::json::Document& doc, size_t depth = 0 ) const noexcept
        {
            using namespace nfx::hashing;

            if( depth >= MAX_RECURSION_DEPTH )
            {
                // Return deterministic "too deep" hash (all documents beyond limit hash the same)
                return ~static_cast<uint32_t>( 0xDEADBEEF );
            }

            uint32_t h = constants::FNV_OFFSET_BASIS_32;

            h = crc32c( h, static_cast<uint8_t>( doc.type() ) );

            switch( doc.type() )
            {
                case nfx::json::Type::Null:
                {
                    break;
                }

                case nfx::json::Type::Boolean:
                {
                    auto val = doc.get<bool>( "" );
                    if( val )
                    {
                        h = crc32c( h, *val ? uint8_t{ 1 } : uint8_t{ 0 } );
                    }
                    break;
                }

                case nfx::json::Type::Integer:
                {
                    auto val = doc.get<int64_t>( "" );
                    if( val )
                    {
                        const uint8_t* bytes = reinterpret_cast<const uint8_t*>( &( *val ) );
                        for( size_t i = 0; i < sizeof( *val ); ++i )
                        {
                            h = crc32c( h, bytes[i] );
                        }
                    }
                    break;
                }

                case nfx::json::Type::UnsignedInteger:
                {
                    auto val = doc.get<uint64_t>( "" );
                    if( val )
                    {
                        const uint8_t* bytes = reinterpret_cast<const uint8_t*>( &( *val ) );
                        for( size_t i = 0; i < sizeof( *val ); ++i )
                        {
                            h = crc32c( h, bytes[i] );
                        }
                    }
                    break;
                }

                case nfx::json::Type::Double:
                {
                    auto valOpt = doc.get<double>( "" );
                    if( valOpt )
                    {
                        double val = *valOpt;

                        // Normalize ±0.0 to +0.0 for consistent hashing
                        if( val == 0.0 )
                        {
                            val = 0.0;
                        }

                        // Hash bit representation
                        const uint8_t* bytes = reinterpret_cast<const uint8_t*>( &val );
                        for( size_t i = 0; i < sizeof( val ); ++i )
                        {
                            h = crc32c( h, bytes[i] );
                        }
                    }
                    break;
                }

                case nfx::json::Type::String:
                {
                    auto str = doc.get<std::string>( "" );
                    if( str )
                    {
                        for( char c : *str )
                        {
                            h = crc32c( h, static_cast<uint8_t>( c ) );
                        }
                    }
                    break;
                }

                case nfx::json::Type::Array:
                {
                    auto arr = doc.get<nfx::json::Array>( "" );
                    if( arr )
                    {
                        // Hash array size first (prevents collision with different-sized arrays)
                        uint64_t size = arr->size();
                        const uint8_t* sizeBytes = reinterpret_cast<const uint8_t*>( &size );
                        for( size_t i = 0; i < sizeof( size ); ++i )
                        {
                            h = crc32c( h, sizeBytes[i] );
                        }

                        // Recursively hash each element (pass incremented depth)
                        for( const auto& elem : *arr )
                        {
                            uint32_t elemHash = ( *this )( elem, depth + 1 );
                            const uint8_t* elemBytes = reinterpret_cast<const uint8_t*>( &elemHash );
                            for( size_t i = 0; i < sizeof( elemHash ); ++i )
                            {
                                h = crc32c( h, elemBytes[i] );
                            }
                        }
                    }
                    break;
                }

                case nfx::json::Type::Object:
                {
                    auto obj = doc.get<nfx::json::Object>( "" );
                    if( obj )
                    {
                        // Hash object size first
                        uint64_t size = obj->size();
                        const uint8_t* sizeBytes = reinterpret_cast<const uint8_t*>( &size );
                        for( size_t i = 0; i < sizeof( size ); ++i )
                        {
                            h = crc32c( h, sizeBytes[i] );
                        }

                        for( const auto& [key, value] : *obj )
                        {
                            for( char c : key )
                            {
                                h = crc32c( h, static_cast<uint8_t>( c ) );
                            }

                            uint32_t valueHash = ( *this )( value, depth + 1 );
                            const uint8_t* valueBytes = reinterpret_cast<const uint8_t*>( &valueHash );
                            for( size_t i = 0; i < sizeof( valueHash ); ++i )
                            {
                                h = crc32c( h, valueBytes[i] );
                            }
                        }
                    }
                    break;
                }
            }

            return ~h;
        }
    };
} // namespace std
