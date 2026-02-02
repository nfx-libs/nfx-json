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
 * @file Builder.h
 * @brief High-performance JSON builder for programmatic construction
 * @details Provides a fluent API for building JSON documents directly to string buffer
 *          without constructing a DOM. Optimized with SSE2 SIMD for string escaping.
 *          Supports both compact and pretty-printed output formats.
 */

#pragma once

#include "Document.h"

#include <nfx/Containers.h>

#include <charconv>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

namespace nfx::json
{
    /**
     * @class Builder
     * @brief High-performance JSON builder with incremental construction API
     * @details Constructs JSON directly to string buffer without building DOM.
     * Provides a fluent API for building JSON documents programmatically.
     * Optimized with SSE2 SIMD for string escaping on supported platforms.
     *
     * Example usage:
     * @code
     * Builder builder;
     * builder.writeStartObject()
     *        .write("name", "John Doe")
     *        .write("age", 30)
     *        .writeEndObject();
     * std::string json = builder.toString();
     * // {"name":"John Doe","age":30}
     * @endcode
     */
    class Builder
    {
    public:
        /** @brief Configuration options for Builder */
        struct Options
        {
            /** @brief Indentation level (0 = compact, >0 = pretty print with N spaces per level) */
            int indent;

            /** @brief Initial buffer capacity hint */
            size_t bufferSize;
        };

        /**
         * @brief Constructor with options
         * @param options Configuration options for the builder
         */
        inline explicit Builder( Options options = { 0, 4096 } );

        /**
         * @brief Start writing a JSON object
         * @return Reference to this builder for method chaining
         * @throws std::runtime_error if called in invalid context
         */
        inline Builder& writeStartObject();

        /**
         * @brief End writing a JSON object
         * @return Reference to this builder for method chaining
         * @throws std::runtime_error if no matching writeStartObject
         */
        inline Builder& writeEndObject();

        /**
         * @brief Start writing a JSON array
         * @return Reference to this builder for method chaining
         * @throws std::runtime_error if called in invalid context
         */
        inline Builder& writeStartArray();

        /**
         * @brief End writing a JSON array
         * @return Reference to this builder for method chaining
         * @throws std::runtime_error if no matching writeStartArray
         */
        inline Builder& writeEndArray();

        /**
         * @brief Write a property key before a nested object or array
         * @param key Property key
         * @return Reference to this builder for method chaining
         * @note Use this before writeStartObject() or writeStartArray() for nested structures
         */
        inline Builder& writeKey( std::string_view key );

        /**
         * @brief Write raw JSON string directly to output
         * @param rawJson Raw JSON string to append
         * @return Reference to this builder for method chaining
         */
        inline Builder& writeRawJson( std::string_view rawJson );

        /**
         * @brief Write a null value property
         * @param key Property key
         * @param value nullptr
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( std::string_view key, std::nullptr_t value );

        /**
         * @brief Write a boolean value property
         * @param key Property key
         * @param value Boolean value
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( std::string_view key, bool value );

        /**
         * @brief Write an integer value property
         * @param key Property key
         * @param value Integer value
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( std::string_view key, int value );

        /**
         * @brief Write an unsigned integer value property
         * @param key Property key
         * @param value Unsigned integer value
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( std::string_view key, unsigned int value );

        /**
         * @brief Write a 64-bit integer value property
         * @param key Property key
         * @param value 64-bit integer value
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( std::string_view key, int64_t value );

        /**
         * @brief Write an unsigned 64-bit integer value property
         * @param key Property key
         * @param value Unsigned 64-bit integer value
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( std::string_view key, uint64_t value );

        /**
         * @brief Write a float value property
         * @param key Property key
         * @param value Float value
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( std::string_view key, float value );

        /**
         * @brief Write a double value property
         * @param key Property key
         * @param value Double value
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( std::string_view key, double value );

        /**
         * @brief Write a string value property
         * @param key Property key
         * @param value String value
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( std::string_view key, std::string_view value );

        /**
         * @brief Write a string value property
         * @param key Property key
         * @param value C-string value
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( std::string_view key, const char* value );

        /**
         * @brief Write a string value property
         * @param key Property key
         * @param value String value
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( std::string_view key, const std::string& value );

        /**
         * @brief Write a Document value property
         * @param key Property key
         * @param value Document to serialize
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( std::string_view key, const Document& value );

        /**
         * @brief Write a null value in an array
         * @param value nullptr
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( std::nullptr_t value );

        /**
         * @brief Write a boolean value in an array
         * @param value Boolean value
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( bool value );

        /**
         * @brief Write an integer value in an array
         * @param value Integer value
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( int value );

        /**
         * @brief Write an unsigned integer value in an array
         * @param value Unsigned integer value
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( unsigned int value );

        /**
         * @brief Write a 64-bit integer value in an array
         * @param value 64-bit integer value
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( int64_t value );

        /**
         * @brief Write an unsigned 64-bit integer value in an array
         * @param value Unsigned 64-bit integer value
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( uint64_t value );

        /**
         * @brief Write a float value in an array
         * @param value Float value
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( float value );

        /**
         * @brief Write a double value in an array
         * @param value Double value
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( double value );

        /**
         * @brief Write a string value in an array
         * @param value String value
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( std::string_view value );

        /**
         * @brief Write a string value in an array
         * @param value C-string value
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( const char* value );

        /**
         * @brief Write a string value in an array
         * @param value String value
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( const std::string& value );

        /**
         * @brief Write a Document value in an array
         * @param value Document to serialize
         * @return Reference to this builder for method chaining
         */
        inline Builder& write( const Document& value );

        /**
         * @brief Write a long integer value property (SFINAE overload for platforms where long != int64_t)
         * @param key Property key
         * @param value Long integer value
         * @return Reference to this builder for method chaining
         */
        template <typename T>
        inline std::enable_if_t<
            std::is_same_v<T, long> && !std::is_same_v<long, int> && !std::is_same_v<long, int64_t>,
            Builder&>
        write( std::string_view key, T value );

        /**
         * @brief Write an unsigned long integer value property (SFINAE overload for platforms where unsigned long !=
         * uint64_t)
         * @param key Property key
         * @param value Unsigned long integer value
         * @return Reference to this builder for method chaining
         */
        template <typename T>
        inline std::enable_if_t<
            std::is_same_v<T, unsigned long> && !std::is_same_v<unsigned long, unsigned int> &&
                !std::is_same_v<unsigned long, uint64_t>,
            Builder&>
        write( std::string_view key, T value );

        /**
         * @brief Write a long long integer value property (SFINAE overload for platforms where long long != int64_t)
         * @param key Property key
         * @param value Long long integer value
         * @return Reference to this builder for method chaining
         */
        template <typename T>
        inline std::enable_if_t<std::is_same_v<T, long long> && !std::is_same_v<long long, int64_t>, Builder&> write(
            std::string_view key, T value );

        /**
         * @brief Write an unsigned long long integer value property (SFINAE overload for platforms where unsigned long
         * long != uint64_t)
         * @param key Property key
         * @param value Unsigned long long integer value
         * @return Reference to this builder for method chaining
         */
        template <typename T>
        inline std::enable_if_t<
            std::is_same_v<T, unsigned long long> && !std::is_same_v<unsigned long long, uint64_t>,
            Builder&>
        write( std::string_view key, T value );

        /**
         * @brief Write a long integer value in an array (SFINAE overload for platforms where long != int64_t)
         * @param value Long integer value
         * @return Reference to this builder for method chaining
         */
        template <typename T>
        inline std::enable_if_t<
            std::is_same_v<T, long> && !std::is_same_v<long, int> && !std::is_same_v<long, int64_t>,
            Builder&>
        write( T value );

        /**
         * @brief Write an unsigned long integer value in an array (SFINAE overload for platforms where unsigned long !=
         * uint64_t)
         * @param value Unsigned long integer value
         * @return Reference to this builder for method chaining
         */
        template <typename T>
        inline std::enable_if_t<
            std::is_same_v<T, unsigned long> && !std::is_same_v<unsigned long, unsigned int> &&
                !std::is_same_v<unsigned long, uint64_t>,
            Builder&>
        write( T value );

        /**
         * @brief Write a long long integer value in an array (SFINAE overload for platforms where long long != int64_t)
         * @param value Long long integer value
         * @return Reference to this builder for method chaining
         */
        template <typename T>
        inline std::enable_if_t<std::is_same_v<T, long long> && !std::is_same_v<long long, int64_t>, Builder&> write(
            T value );

        /**
         * @brief Write an unsigned long long integer value in an array (SFINAE overload for platforms where unsigned
         * long long != uint64_t)
         * @param value Unsigned long long integer value
         * @return Reference to this builder for method chaining
         */
        template <typename T>
        inline std::enable_if_t<
            std::is_same_v<T, unsigned long long> && !std::is_same_v<unsigned long long, uint64_t>,
            Builder&>
        write( T value );

        /**
         * @brief Get the constructed JSON string
         * @return The complete JSON string
         */
        inline std::string toString();

        /**
         * @brief Reset builder to initial state for reuse
         * @return Reference to this builder for method chaining
         * @note Clears all buffered data and context stack, allowing builder reuse without reallocation
         */
        inline Builder& reset();

        /**
         * @brief Get current JSON string size
         * @return Size of generated JSON in bytes
         */
        [[nodiscard]] inline size_t size() const noexcept;

        /**
         * @brief Check if builder is empty
         * @return true if no JSON has been written
         */
        [[nodiscard]] inline bool isEmpty() const noexcept;

        /**
         * @brief Reserve buffer capacity
         * @param capacity Minimum buffer size in bytes to reserve
         * @return Reference to this builder for method chaining
         * @note Pre-allocating capacity avoids reallocations during construction of large JSON documents
         */
        inline Builder& reserve( size_t capacity );

        /**
         * @brief Get non-destructive view of generated JSON
         * @return String view to the internal buffer
         * @note Does not invalidate the builder state, unlike toString()
         */
        [[nodiscard]] inline std::string_view toStringView() const noexcept;

        /**
         * @brief Get current buffer capacity
         * @return Allocated capacity of internal buffer in bytes
         */
        [[nodiscard]] inline size_t capacity() const noexcept;

        /**
         * @brief Get current nesting depth
         * @return Number of nested objects/arrays
         * @note Returns 0 when at root level
         */
        [[nodiscard]] inline size_t depth() const noexcept;

        /**
         * @brief Check if builder state is valid
         * @return true if all opened objects/arrays have been closed
         */
        [[nodiscard]] inline bool isValid() const noexcept;

        /**
         * @brief Write a complete array from a STL container as object property
         * @tparam Container STL container type (vector, list, set, etc.)
         * @param key Property key
         * @param values Container of values to write as array
         * @return Reference to this builder for method chaining
         * @note Automatically iterates container and writes each element
         */
        template <typename Container>
        inline Builder& writeArray( std::string_view key, const Container& values );

        /**
         * @brief Write a complete array from a STL container
         * @tparam Container STL container type (vector, list, set, etc.)
         * @param values Container of values to write as array
         * @return Reference to this builder for method chaining
         * @note Automatically iterates container and writes each element
         */
        template <typename Container>
        inline Builder& writeArray( const Container& values );

    private:
        /**
         * @brief Context frame for tracking nested object/array state
         */
        struct ContextFrame
        {
            bool isObject;       ///< True if current context is an object, false if array
            bool isEmpty;        ///< True if no elements have been written yet
            bool expectingValue; ///< True if expecting a value after property name
        };

        /**
         * @brief Write a signed integer to the buffer
         * @param value Signed 64-bit integer value
         */
        inline void writeInt( int64_t value );

        /**
         * @brief Write an unsigned integer to the buffer
         * @param value Unsigned 64-bit integer value
         */
        inline void writeUInt( uint64_t value );

        /**
         * @brief Write a double to the buffer
         * @param value Double precision floating point value
         */
        inline void writeDouble( double value );

        /**
         * @brief Write an escaped and quoted string to the buffer
         * @param str String view to write (uses SSE2 SIMD optimization when available)
         */
        inline void writeString( std::string_view str );

        /**
         * @brief Write newline and indentation if pretty-printing is enabled
         */
        inline void writeNewlineAndIndent();

        /**
         * @brief Write a comma separator if needed based on current context
         */
        inline void writeCommaIfNeeded();

        /**
         * @brief Write a Document directly by traversing its DOM
         * @param doc Document to write
         * @note Avoids temporary string allocation by traversing DOM directly
         */
        inline void writeDocument( const Document& doc );

        /**
         * @brief Write a Document array to the buffer
         * @param arrayRef Reference to the array to write
         */
        inline void writeDocumentArray( const Array& array );

        /**
         * @brief Write a Document object to the buffer
         * @param objectRef Reference to the object to write
         */
        inline void writeDocumentObject( const Object& object );

        std::string m_buffer;                                    ///< JSON output buffer
        int m_indent;                                            ///< Indentation level (0 = compact, >0 = pretty print)
        int m_currentIndent;                                     ///< Current indentation depth
        containers::SmallVector<ContextFrame, 8> m_contextStack; ///< Stack of nested object/array contexts
    };
} // namespace nfx::json

#include "detail/Builder.inl"
