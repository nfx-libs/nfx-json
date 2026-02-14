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
 * @file Document.h
 * @brief JSON Document type with low-level and high-level APIs
 */

#pragma once

#include "Concepts.h"

#include <algorithm>
#include <compare>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace nfx::json
{
    // Forward declarations
    class Document;

    /**
     * @brief JSON value types
     */
    enum class Type : uint8_t
    {
        Null,
        Boolean,
        Integer,
        UnsignedInteger,
        Double,
        String,
        Array,
        Object
    };

    /**
     * @brief Ordered map for JSON objects (preserves insertion order)
     */
    using Object = std::vector<std::pair<std::string, Document>>;

    /**
     * @brief Array for JSON arrays
     */
    using Array = std::vector<Document>;

    //----------------------------------------------
    // Type trait specializations for Object and Array
    //----------------------------------------------

    /**
     * @brief Specialization of is_json_container for Object
     */
    template <>
    struct is_json_container<Object> : std::true_type
    {
    };

    /**
     * @brief Specialization of is_json_container for Array
     */
    template <>
    struct is_json_container<Array> : std::true_type
    {
    };

    //=====================================================================
    // Document class
    //=====================================================================

    /**
     * @brief Low-level JSON value storage type
     * @details Represents a single JSON value (null, bool, number, string, array, or object).
     *          Uses std::variant for efficient type-tagged storage with minimal overhead.
     *          Object preserves insertion order using std::vector of key-value pairs.
     *          This is the internal storage used by Document - prefer using Document for most operations.
     */
    class Document final
    {
    public:
        //----------------------------------------------
        // Construction
        //----------------------------------------------

        /** @brief Default constructor */
        inline Document();

        /**
         * @brief Boolean constructor
         * @param value The boolean value
         */
        inline Document( bool value ) noexcept;

        /**
         * @brief Integer constructor
         * @param value The integer value
         */
        inline Document( int value ) noexcept;

        /**
         * @brief Unsigned integer constructor
         * @param value The unsigned integer value
         */
        inline Document( unsigned int value ) noexcept;

        /**
         * @brief 64-bit integer constructor
         * @param value The 64-bit integer value
         */
        inline Document( int64_t value ) noexcept;

        /**
         * @brief Unsigned integer constructor
         * @param value The unsigned integer value
         */
        inline Document( uint64_t value ) noexcept;

        /**
         * @brief Double constructor
         * @param value The double value
         */
        inline Document( double value ) noexcept;

        /**
         * @brief String constructor
         * @param value The string value
         */
        inline explicit Document( std::string value );

        /**
         * @brief String view constructor
         * @param value The string view to convert to string
         */
        inline explicit Document( std::string_view value );

        /**
         * @brief C-string constructor
         * @param value The C-string to convert to string
         */
        inline explicit Document( const char* value );

        /**
         * @brief String literal constructor (non-explicit for convenience)
         * @tparam N Size of the string literal (including null terminator)
         * @param value String literal
         */
        template <size_t N>
        inline Document( const char ( &value )[N] );

        /**
         * @brief nullptr constructor (creates null JSON value)
         */
        inline Document( std::nullptr_t ) noexcept;

        /**
         * @brief Array constructor
         * @param value The array value
         */
        inline explicit Document( Array value );

        /**
         * @brief Object constructor
         * @param value The object value
         */
        inline explicit Document( Object value );

        /**
         * @brief Long integer constructor (SFINAE overload for platforms where long != int && long != int64_t)
         * @param value The long integer value
         */
        template <typename T>
        inline Document(
            T value,
            std::enable_if_t<
                std::is_same_v<T, long> && !std::is_same_v<long, int> && !std::is_same_v<long, int64_t>,
                int> = 0 ) noexcept;

        /**
         * @brief Unsigned long integer constructor (SFINAE overload for platforms where unsigned long != unsigned int
         * && unsigned long != uint64_t)
         * @param value The unsigned long integer value
         */
        template <typename T>
        inline Document(
            T value,
            std::enable_if_t<
                std::is_same_v<T, unsigned long> && !std::is_same_v<unsigned long, unsigned int> &&
                    !std::is_same_v<unsigned long, uint64_t>,
                int> = 0 ) noexcept;

        /**
         * @brief Long long integer constructor (SFINAE overload for platforms where long long != int64_t)
         * @param value The long long integer value
         */
        template <typename T>
        inline Document(
            T value,
            std::enable_if_t<std::is_same_v<T, long long> && !std::is_same_v<long long, int64_t>, int> = 0 ) noexcept;

        /**
         * @brief Unsigned long long integer constructor (SFINAE overload for platforms where unsigned long long !=
         * uint64_t)
         * @param value The unsigned long long integer value
         */
        template <typename T>
        inline Document(
            T value,
            std::enable_if_t<
                std::is_same_v<T, unsigned long long> && !std::is_same_v<unsigned long long, uint64_t>,
                int> = 0 ) noexcept;

        /**
         * @brief Copy constructor
         * @param other The Document to copy from
         */
        Document( const Document& other ) = default;

        /**
         * @brief Move constructor
         * @param other The Document to move from
         */
#if defined( __GNUC__ ) && !defined( __clang__ )
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
        Document( Document&& other ) noexcept = default;
#if defined( __GNUC__ ) && !defined( __clang__ )
#    pragma GCC diagnostic pop
#endif

        /** @brief Destructor */
        ~Document() = default;

        //----------------------------------------------
        // Assignment
        //----------------------------------------------

        /**
         * @brief Copy assignment
         * @param other The Document to copy from
         * @return Reference to this Document
         */
        Document& operator=( const Document& other ) = default;

        /**
         * @brief Move assignment
         * @param other The Document to move from
         * @return Reference to this Document
         */
        Document& operator=( Document&& other ) noexcept = default;

        /**
         * @brief Assignment from string
         * @param value The string value to assign
         * @return Reference to this Document
         */
        inline Document& operator=( std::string value );

        /**
         * @brief Assignment from bool
         * @param value The boolean value to assign
         * @return Reference to this Document
         */
        inline Document& operator=( bool value ) noexcept;

        /**
         * @brief Assignment from int64_t
         * @param value The integer value to assign
         * @return Reference to this Document
         */
        inline Document& operator=( int64_t value ) noexcept;

        /**
         * @brief Assignment from uint64_t
         * @param value The unsigned integer value to assign
         * @return Reference to this Document
         */
        inline Document& operator=( uint64_t value ) noexcept;

        /**
         * @brief Assignment from double
         * @param value The double value to assign
         * @return Reference to this Document
         */
        inline Document& operator=( double value ) noexcept;

        /**
         * @brief Assignment from nullptr (null)
         * @return Reference to this Document
         */
        inline Document& operator=( std::nullptr_t ) noexcept;

        //----------------------------------------------
        // Comparison
        //----------------------------------------------

        /**
         * @brief Equality comparison
         * @param other The Document to compare with
         * @return true if the values are equal, false otherwise
         */
        inline bool operator==( const Document& other ) const noexcept;

        /**
         * @brief Three-way comparison
         * @param other The Document to compare with
         * @return Ordering relationship (strong_ordering)
         * @note Automatically provides <, <=, >, >=, != operators
         */
        std::strong_ordering operator<=>( const Document& other ) const noexcept;

        //----------------------------------------------
        // Static factory methods
        //----------------------------------------------

        /**
         * @brief Create empty object
         * @return A new Document containing an empty object
         */
        [[nodiscard]] inline static Document object();

        /**
         * @brief Create empty array
         * @return A new Document containing an empty array
         */
        [[nodiscard]] inline static Document array();

        //----------------------------------------------
        // Type queries
        //----------------------------------------------

        /**
         * @brief Get the JSON type
         * @return The JSON type enumeration value
         */
        [[nodiscard]] inline Type type() const noexcept;

        //----------------------------------------------
        // Root value access
        //----------------------------------------------

        /**
         * @brief Get the root value of this document (safe, returns optional)
         * @tparam T Type to retrieve
         * @return std::optional<T> containing the value if type matches, empty otherwise
         *
         * @note This method never throws exceptions
         * @note For primitives (int, double, string): lightweight copy
         * @note For large containers (Object, Array): consider rootRef<T>() to avoid copy overhead
         */
        template <Value T>
        [[nodiscard]] inline std::optional<T> root() const;

        /**
         * @brief Get the root value of this document (output parameter version)
         * @tparam T Type to retrieve
         * @param[out] out Variable to fill with the value
         * @return true if value was extracted successfully, false otherwise
         *
         * @note Use this variant when you prefer output parameter style over optional
         */
        template <Value T>
        [[nodiscard]] inline bool root( T& out ) const;

        /**
         * @brief Get a reference to the root value (safe, no copy for large objects)
         * @tparam T Type to retrieve reference to
         * @return std::optional<std::reference_wrapper<const T>> containing reference if type matches
         *
         * @note Preferred for large Object/Array to avoid copy overhead
         * @note Returns reference wrapped in optional - always safe (no raw pointers)
         * @note Use root<T>() for primitives where copy is cheap
         */
        template <Value T>
        [[nodiscard]] inline std::optional<std::reference_wrapper<const T>> rootRef() const;

        /**
         * @brief Get a mutable reference to the root value (safe, no copy)
         * @tparam T Type to retrieve reference to
         * @return std::optional<std::reference_wrapper<T>> containing mutable reference if type matches
         *
         * @note Allows direct modification of large containers without copy
         * @note Returns reference wrapped in optional - always safe (no raw pointers)
         */
        template <Value T>
        [[nodiscard]] inline std::optional<std::reference_wrapper<T>> rootRef();

        /**
         * @brief Check if the root value is of a specific type
         * @tparam T Type to check for (must satisfy Checkable concept)
         * @return true if root value is of type T, false otherwise
         * @note More explicit than is<T>("") - clearly indicates root check
         */
        template <Checkable T>
        [[nodiscard]] inline bool isRoot() const;

        /**
         * @brief Visit the root value with a visitor (const version)
         * @tparam Visitor Callable accepting any variant alternative type
         * @param visitor Function/lambda to call with the underlying value
         * @return Result of the visitor call
         *
         * @note Enables compile-time type-safe pattern matching on JSON types
         * @note More efficient than runtime is<T>() checks - uses std::visit
         *
         * Example usage with generic lambda:
         * @code
         * doc.visit([](auto&& value) {
         *     using T = std::decay_t<decltype(value)>;
         *     if constexpr (std::is_same_v<T, int64_t>) {
         *         std::cout << "Integer: " << value << "\n";
         *     } else if constexpr (std::is_same_v<T, std::string>) {
         *         std::cout << "String: " << value << "\n";
         *     }
         *     // ... handle other types
         * });
         * @endcode
         *
         * Example with overloaded lambdas:
         * @code
         * doc.visit(overloaded{
         *     [](int64_t i) { std::cout << "Int: " << i; },
         *     [](const std::string& s) { std::cout << "String: " << s; },
         *     [](const Array& a) { std::cout << "Array[" << a.size() << "]"; },
         *     [](auto&&) { std::cout << "Other type"; }
         * });
         * @endcode
         */
        template <typename Visitor>
        inline decltype( auto ) visit( Visitor&& visitor ) const;

        /**
         * @brief Visit the root value with a visitor (mutable version)
         * @tparam Visitor Callable accepting any variant alternative type
         * @param visitor Function/lambda to call with the underlying value
         * @return Result of the visitor call
         * @note Allows modification of the underlying value through the visitor
         */
        template <typename Visitor>
        inline decltype( auto ) visit( Visitor&& visitor );

        //----------------------------------------------
        // Object operations
        //----------------------------------------------

        /**
         * @brief Get object member by key (mutable, returns nullptr if not found)
         * @param key The key to search for
         * @return Pointer to the value if found, nullptr otherwise
         */
        [[nodiscard]] inline Document* find( std::string_view key ) noexcept;

        /**
         * @brief Get object member by key (const version)
         * @param key The key to search for
         * @return Pointer to the value if found, nullptr otherwise
         */
        [[nodiscard]] inline const Document* find( std::string_view key ) const noexcept;

        /**
         * @brief Insert or assign object member
         * @param key The key to set
         * @param value The value to assign
         */
        inline void set( std::string key, Document value );

        /**
         * @brief Check if object contains key
         * @param key The key to search for (supports dot notation like "user.name" and array access like "data[0]")
         * @return true if the key exists, false otherwise
         */
        [[nodiscard]] inline bool contains( std::string_view key ) const noexcept;

        /**
         * @brief Access object member by key (with bounds checking)
         * @param key The key to access
         * @return Reference to the value at the key
         * @throws std::runtime_error if not an object
         * @throws std::out_of_range if key not found
         */
        [[nodiscard]] inline Document& at( std::string_view key );

        /**
         * @brief Access object member by key (const version)
         * @param key The key to access
         * @return Const reference to the value at the key
         * @throws std::runtime_error if not an object
         * @throws std::out_of_range if key not found
         */
        [[nodiscard]] inline const Document& at( std::string_view key ) const;

        /**
         * @brief Access array element with bounds checking
         * @param index The index to access
         * @return Reference to the value at the index
         * @throws std::out_of_range if index >= array size
         * @throws std::runtime_error if not an array
         */
        [[nodiscard]] inline Document& at( size_t index );

        /**
         * @brief Access array element with bounds checking (const version)
         * @param index The index to access
         * @return Const reference to the value at the index
         * @throws std::out_of_range if index >= array size
         * @throws std::runtime_error if not an array
         */
        [[nodiscard]] inline const Document& at( size_t index ) const;

        /**
         * @brief Access or create object field
         * @param key The key to access or create
         * @return Reference to the value at the key
         *
         * @note Auto-converts Null to Object on first access (common pattern)
         * @note In DEBUG builds: throws std::runtime_error on type mismatch
         * @note In RELEASE builds: undefined behavior on type mismatch (no overhead)
         * @note For safe access with error checking, use at() or find()
         *
         * @warning Performance-critical path - minimal error checking in release builds
         */
        inline Document& operator[]( std::string_view key );

        /**
         * @brief Access object field (const)
         * @param key The key to access
         * @return Const reference to the value, or null value if not found
         */
        [[nodiscard]] inline const Document& operator[]( std::string_view key ) const;

        /**
         * @brief Access or create array element
         * @param index The index to access or create
         * @return Reference to the value at the index
         *
         * @note Auto-converts Null to Array on first access (common pattern)
         * @note Extends array with null values if index >= size
         * @note In DEBUG builds: throws std::runtime_error on type mismatch
         * @note In RELEASE builds: undefined behavior on type mismatch (no overhead)
         * @note For safe access with bounds checking, use at()
         *
         * @warning Performance-critical path - minimal error checking in release builds
         */
        [[nodiscard]] inline Document& operator[]( size_t index );

        /**
         * @brief Access array element (const)
         * @param index The index to access
         * @return Const reference to the value, or null value if out of bounds
         */
        [[nodiscard]] inline const Document& operator[]( size_t index ) const;

        /** @brief Clear object or array */
        inline void clear();

        /**
         * @brief Erase key from object
         * @param key The key to remove (supports simple keys, dot notation, and JSON Pointer paths)
         * @return 1 if the key was found and removed, 0 otherwise
         */
        inline size_t erase( std::string_view key );

        /**
         * @brief Erase key from object (overload for const char*)
         * @param key The key to remove
         * @return 1 if the key was found and removed, 0 otherwise
         */
        inline size_t erase( const char* key );

        /**
         * @brief Erase key from object (overload for std::string)
         * @param key The key to remove
         * @return 1 if the key was found and removed, 0 otherwise
         */
        inline size_t erase( const std::string& key );

        /**
         * @brief Erase element from array by iterator
         * @tparam iterator The iterator type
         * @param it The iterator pointing to the element to remove
         * @return iterator to the element following the removed element
         */
        template <typename iterator, typename = std::enable_if_t<!std::is_convertible_v<iterator, std::string_view>>>
        inline iterator erase( iterator it );

        /**
         * @brief Insert element into array at iterator position
         * @tparam iterator The iterator type
         * @param pos The position to insert at
         * @param value The value to insert
         * @return iterator pointing to the inserted element
         */
        template <typename iterator>
        inline iterator insert( iterator pos, Document value );

        //----------------------------------------------
        // Array operations
        //----------------------------------------------

        /**
         * @brief Get array size
         * @return The number of elements in the array or object, 0 otherwise
         */
        [[nodiscard]] inline size_t size() const noexcept;

        /**
         * @brief Check if container is empty
         * @return True if array/object has no elements, or string has length 0, false otherwise
         */
        [[nodiscard]] inline bool empty() const noexcept;

        /**
         * @brief Append to array
         * @param value The value to append
         */
        inline void push_back( Document value );

        /**
         * @brief Reserve capacity for array or object
         * @param capacity The number of elements to reserve space for
         * @note For arrays, reserves space in the underlying vector
         * @note For objects, reserves space in the underlying vector of key-value pairs
         */
        inline void reserve( size_t capacity );

        /**
         * @brief Get capacity of array or object
         * @return The capacity of the underlying container, 0 otherwise
         */
        [[nodiscard]] inline size_t capacity() const noexcept;

        /**
         * @brief Get first element of array
         * @return Reference to the first element
         * @throws std::out_of_range if array is empty
         */
        [[nodiscard]] inline Document& front();

        /**
         * @brief Get first element of array (const)
         * @return Const reference to the first element
         * @throws std::out_of_range if array is empty
         */
        [[nodiscard]] inline const Document& front() const;

        /**
         * @brief Get last element of array
         * @return Reference to the last element
         */
        [[nodiscard]] inline Document& back();

        /**
         * @brief Get last element of array (const)
         * @return Const reference to the last element
         */
        [[nodiscard]] inline const Document& back() const;

        /**
         * @brief Begin iterator for arrays
         * @return iterator to the beginning
         */
        [[nodiscard]] inline auto begin();

        /**
         * @brief End iterator for arrays
         * @return iterator to the end
         */
        [[nodiscard]] inline auto end();

        /**
         * @brief Begin iterator for arrays (const)
         * @return Const iterator to the beginning
         */
        [[nodiscard]] inline auto begin() const;

        /**
         * @brief End iterator for arrays (const)
         * @return Const iterator to the end
         */
        [[nodiscard]] inline auto end() const;

        //----------------------------------------------
        // Document::ObjectIterator
        //----------------------------------------------

        /** @brief Object iterator wrapper that provides key() and value() */
        class ObjectIterator
        {
        public:
            /** @brief iterator category (forward iterator) */
            using iterator_category = std::forward_iterator_tag;

            /** @brief iterator value type */
            using value_type = std::pair<const std::string, Document>;

            /** @brief iterator difference type */
            using difference_type = std::ptrdiff_t;

            /** @brief iterator pointer type */
            using pointer = const value_type*;

            /** @brief iterator reference type */
            using reference = const value_type&;

            /** @brief Underlying iterator type for Object */
            using MapIterator = Object::const_iterator;

            /** @brief The underlying iterator */
            MapIterator it;

            /**
             * @brief Construct from underlying iterator
             * @param iter The underlying iterator to wrap
             */
            inline ObjectIterator( MapIterator iter );

            /** @brief Copy constructor */
            ObjectIterator( const ObjectIterator& ) = default;

            /** @brief Move constructor */
            ObjectIterator( ObjectIterator&& ) noexcept = default;

            /** @brief Copy assignment
             * @return Reference to this iterator */
            ObjectIterator& operator=( const ObjectIterator& ) = default;

            /** @brief Move assignment
             * @return Reference to this iterator */
            ObjectIterator& operator=( ObjectIterator&& ) noexcept = default;

            /**
             * @brief Get the key of the current element
             * @return Const reference to the key string
             */
            inline const std::string& key() const;

            /**
             * @brief Get the value of the current element (const)
             * @return Const reference to the Document
             */
            inline const Document& value() const;

            /**
             * @brief Get the value of the current element (mutable)
             * @return Mutable reference to the Document
             */
            inline Document& value();

            /**
             * @brief Dereference operator (const)
             * @return Const reference to the Document
             */
            inline const Document& operator*() const;

            /**
             * @brief Dereference operator (mutable)
             * @return Mutable reference to the Document
             */
            inline Document& operator*();

            /**
             * @brief Pre-increment operator
             * @return Reference to this iterator after increment
             */
            inline ObjectIterator& operator++();

            /**
             * @brief Inequality comparison
             * @param other The iterator to compare with
             * @return true if iterators are not equal, false otherwise
             */
            inline bool operator!=( const ObjectIterator& other ) const;

            /**
             * @brief Equality comparison
             * @param other The iterator to compare with
             * @return true if iterators are equal, false otherwise
             */
            inline bool operator==( const ObjectIterator& other ) const;

            /**
             * @brief Get the underlying iterator
             * @return The underlying MapIterator
             */
            inline MapIterator base() const;
        };

        //----------------------------------------------
        // Object iteration
        //----------------------------------------------

        /**
         * @brief Begin iterator for objects (returns ObjectIterator with key/value)
         * @return iterator to the beginning of the object
         */
        [[nodiscard]] inline ObjectIterator objectBegin() const;

        /**
         * @brief End iterator for objects
         * @return iterator to the end of the object
         */
        [[nodiscard]] inline ObjectIterator objectEnd() const;

        //----------------------------------------------
        // Document::KeysView class
        //----------------------------------------------

        /** @brief Range adapter for object keys */
        class KeysView
        {
            const Object* m_obj;

        public:
            /** @brief Key iterator */
            class iterator
            {
                Object::const_iterator m_it;

            public:
                /** @brief iterator category (forward iterator) */
                using iterator_category = std::forward_iterator_tag;

                /** @brief iterator value type */
                using value_type = std::string;

                /** @brief iterator difference type */
                using difference_type = std::ptrdiff_t;

                /** @brief iterator pointer type */
                using pointer = const std::string*;

                /** @brief iterator reference type */
                using reference = const std::string&;

                /**
                 * @brief Construct iterator from underlying Object iterator
                 * @param it The underlying Object const_iterator
                 */
                inline iterator( Object::const_iterator it );

                /** @brief Copy constructor */
                iterator( const iterator& ) = default;

                /** @brief Move constructor */
                iterator( iterator&& ) noexcept = default;

                /** @brief Copy assignment
                 * @return Reference to this iterator */
                iterator& operator=( const iterator& ) = default;

                /** @brief Move assignment
                 * @return Reference to this iterator */
                iterator& operator=( iterator&& ) noexcept = default;

                /**
                 * @brief Dereference operator
                 * @return Const reference to the key string
                 */
                inline reference operator*() const;

                /**
                 * @brief Arrow operator
                 * @return Pointer to the key string
                 */
                inline pointer operator->() const;

                /**
                 * @brief Pre-increment operator
                 * @return Reference to this iterator after increment
                 */
                inline iterator& operator++();

                /**
                 * @brief Post-increment operator
                 * @return Copy of iterator before increment
                 */
                inline iterator operator++( int );

                /**
                 * @brief Equality comparison
                 * @param other The iterator to compare with
                 * @return true if iterators are equal, false otherwise
                 */
                inline bool operator==( const iterator& other ) const;

                /**
                 * @brief Inequality comparison
                 * @param other The iterator to compare with
                 * @return true if iterators are not equal, false otherwise
                 */
                inline bool operator!=( const iterator& other ) const;
            };

            /**
             * @brief Construct KeysView from object pointer
             * @param obj Pointer to the object to iterate keys from
             */
            inline KeysView( const Object* obj );

            /**
             * @brief Get iterator to first key
             * @return iterator pointing to first key
             */
            inline iterator begin() const;

            /**
             * @brief Get iterator past last key
             * @return iterator representing end position
             */
            inline iterator end() const;
        };

        /** @brief Range adapter for object values */
        class ValuesView
        {
            const Object* m_obj;

        public:
            /** @brief Value iterator (const) */
            class const_iterator
            {
                Object::const_iterator m_it;

            public:
                /** @brief iterator category (forward iterator) */
                using iterator_category = std::forward_iterator_tag;

                /** @brief iterator value type */
                using value_type = Document;

                /** @brief iterator difference type */
                using difference_type = std::ptrdiff_t;

                /** @brief iterator pointer type */
                using pointer = const Document*;

                /** @brief iterator reference type */
                using reference = const Document&;

                /**
                 * @brief Construct const_iterator from underlying Object iterator
                 * @param it The underlying Object const_iterator
                 */
                inline const_iterator( Object::const_iterator it );

                /** @brief Copy constructor */
                const_iterator( const const_iterator& ) = default;

                /** @brief Move constructor */
                const_iterator( const_iterator&& ) noexcept = default;

                /** @brief Copy assignment
                 * @return Reference to this iterator */
                const_iterator& operator=( const const_iterator& ) = default;

                /** @brief Move assignment
                 * @return Reference to this iterator */
                const_iterator& operator=( const_iterator&& ) noexcept = default;

                /**
                 * @brief Dereference operator
                 * @return Const reference to the Document value
                 */
                inline reference operator*() const;

                /**
                 * @brief Arrow operator
                 * @return Pointer to the Document value
                 */
                inline pointer operator->() const;

                /**
                 * @brief Pre-increment operator
                 * @return Reference to this iterator after increment
                 */
                inline const_iterator& operator++();

                /**
                 * @brief Post-increment operator
                 * @return Copy of iterator before increment
                 */
                inline const_iterator operator++( int );

                /**
                 * @brief Equality comparison
                 * @param other The iterator to compare with
                 * @return true if iterators are equal, false otherwise
                 */
                inline bool operator==( const const_iterator& other ) const;

                /**
                 * @brief Inequality comparison
                 * @param other The iterator to compare with
                 * @return true if iterators are not equal, false otherwise
                 */
                inline bool operator!=( const const_iterator& other ) const;
            };

            /** @brief Value iterator (mutable) */
            class iterator
            {
                Object::iterator m_it;

            public:
                /** @brief iterator category (forward iterator) */
                using iterator_category = std::forward_iterator_tag;

                /** @brief iterator value type */
                using value_type = Document;

                /** @brief iterator difference type */
                using difference_type = std::ptrdiff_t;

                /** @brief iterator pointer type */
                using pointer = Document*;

                /** @brief iterator reference type */
                using reference = Document&;

                /**
                 * @brief Construct iterator from underlying Object iterator
                 * @param it The underlying Object iterator
                 */
                inline iterator( Object::iterator it );

                /** @brief Copy constructor */
                iterator( const iterator& ) = default;

                /** @brief Move constructor */
                iterator( iterator&& ) noexcept = default;

                /** @brief Copy assignment
                 * @return Reference to this iterator */
                iterator& operator=( const iterator& ) = default;

                /** @brief Move assignment
                 * @return Reference to this iterator */
                iterator& operator=( iterator&& ) noexcept = default;

                /**
                 * @brief Dereference operator
                 * @return Reference to the Document value
                 */
                inline reference operator*() const;

                /**
                 * @brief Arrow operator
                 * @return Pointer to the Document value
                 */
                inline pointer operator->() const;

                /**
                 * @brief Pre-increment operator
                 * @return Reference to this iterator after increment
                 */
                inline iterator& operator++();

                /**
                 * @brief Post-increment operator
                 * @return Copy of iterator before increment
                 */
                inline iterator operator++( int );

                /**
                 * @brief Equality comparison
                 * @param other The iterator to compare with
                 * @return true if iterators are equal, false otherwise
                 */
                inline bool operator==( const iterator& other ) const;

                /**
                 * @brief Inequality comparison
                 * @param other The iterator to compare with
                 * @return true if iterators are not equal, false otherwise
                 */
                inline bool operator!=( const iterator& other ) const;
            };

            /**
             * @brief Construct ValuesView from object pointer
             * @param obj Pointer to the object to iterate values from
             */
            inline ValuesView( const Object* obj );

            /**
             * @brief Get iterator to first value
             * @return Const iterator pointing to first value
             */
            inline const_iterator begin() const;

            /**
             * @brief Get iterator past last value
             * @return Const iterator representing end position
             */
            inline const_iterator end() const;
        };

        /** @brief Mutable values view */
        class MutableValuesView
        {
            Object* m_obj;

        public:
            /** @brief Value iterator (mutable) */
            class iterator
            {
                Object::iterator m_it;

            public:
                /** @brief iterator category (forward iterator) */
                using iterator_category = std::forward_iterator_tag;
                /** @brief iterator value type */
                using value_type = Document;
                /** @brief iterator difference type */
                using difference_type = std::ptrdiff_t;
                /** @brief iterator pointer type */
                using pointer = Document*;
                /** @brief iterator reference type */
                using reference = Document&;

                /**
                 * @brief Construct iterator from underlying Object iterator
                 * @param it The underlying Object iterator
                 */
                inline iterator( Object::iterator it );

                /** @brief Copy constructor */
                iterator( const iterator& ) = default;

                /** @brief Move constructor */
                iterator( iterator&& ) noexcept = default;

                /** @brief Copy assignment
                 * @return Reference to this iterator */
                iterator& operator=( const iterator& ) = default;

                /** @brief Move assignment
                 * @return Reference to this iterator */
                iterator& operator=( iterator&& ) noexcept = default;

                /**
                 * @brief Dereference operator
                 * @return Reference to the Document value
                 */
                inline reference operator*() const;

                /**
                 * @brief Arrow operator
                 * @return Pointer to the Document value
                 */
                inline pointer operator->() const;

                /**
                 * @brief Pre-increment operator
                 * @return Reference to this iterator after increment
                 */
                inline iterator& operator++();

                /**
                 * @brief Post-increment operator
                 * @return Copy of iterator before increment
                 */
                inline iterator operator++( int );

                /**
                 * @brief Equality comparison
                 * @param other The iterator to compare with
                 * @return true if iterators are equal, false otherwise
                 */
                inline bool operator==( const iterator& other ) const;

                /**
                 * @brief Inequality comparison
                 * @param other The iterator to compare with
                 * @return true if iterators are not equal, false otherwise
                 */
                inline bool operator!=( const iterator& other ) const;
            };

            /**
             * @brief Construct MutableValuesView from object pointer
             * @param obj Pointer to the object to iterate values from
             */
            inline MutableValuesView( Object* obj );

            /**
             * @brief Get iterator to first value
             * @return Mutable iterator pointing to first value
             */
            inline iterator begin();

            /**
             * @brief Get iterator past last value
             * @return Mutable iterator representing end position
             */
            inline iterator end();
        };

        /**
         * @brief Get a range view of object keys
         * @return Range adapter that iterates over keys only
         * @note Returns empty range for non-object types
         */
        [[nodiscard]] inline KeysView keys() const;

        /**
         * @brief Get a range view of object values (const)
         * @return Range adapter that iterates over values only
         * @note Returns empty range for non-object types
         */
        [[nodiscard]] inline ValuesView values() const;

        /**
         * @brief Get a range view of object values (mutable)
         * @return Range adapter that iterates over mutable values
         * @note Returns empty range for non-object types
         */
        [[nodiscard]] inline MutableValuesView values();

        //----------------------------------------------
        // Serialization
        //----------------------------------------------

        /**
         * @brief Convert to JSON string
         * @param indent Indentation level (0 = compact output, >0 = pretty printing with N spaces)
         * @param bufferSize Initial buffer capacity hint (0 = use default 4KB)
         * @return JSON string representation
         */
        [[nodiscard]] std::string toString( int indent = 0, size_t bufferSize = 0 ) const;

        /**
         * @brief Convert to JSON bytes
         * @return JSON byte representation
         */
        [[nodiscard]] inline std::vector<uint8_t> toBytes() const;

        //----------------------------------------------
        // High-level API with path support
        //----------------------------------------------

        /**
         * @brief Create Document from JSON string
         * @param jsonStr JSON string to parse
         * @return Optional Document if parsing succeeds, empty optional otherwise
         */
        [[nodiscard]] static std::optional<Document> fromString( std::string_view jsonStr );

        /**
         * @brief Parse JSON string into existing Document
         * @param jsonStr JSON string to parse
         * @param[out] value Document to populate
         * @return true if parsing succeeds, false otherwise
         */
        [[nodiscard]] static bool fromString( std::string_view jsonStr, Document& value );

        /**
         * @brief Create Document from JSON bytes
         * @param bytes JSON bytes to parse
         * @return Optional Document if parsing succeeds, empty optional otherwise
         */
        [[nodiscard]] static std::optional<Document> fromBytes( const std::vector<uint8_t>& bytes );

        /**
         * @brief Parse JSON bytes into existing Document
         * @param bytes JSON bytes to parse
         * @param[out] value Document to populate
         * @return true if parsing succeeds, false otherwise
         */
        [[nodiscard]] static bool fromBytes( const std::vector<uint8_t>& bytes, Document& value );

        /**
         * @brief Merge another Document into this one
         * @param other Document to merge
         * @param overwriteArrays Whether to overwrite arrays or merge them
         */
        void merge( const Document& other, bool overwriteArrays = true );

        /**
         * @brief Update value at specific path
         * @param path Path to update (supports: "/key/subkey" JSON Pointer, "key.subkey" dot notation, "arr[0]" array
         * syntax)
         * @param value New value to set
         */
        void update( std::string_view path, const Document& value );

        //----------------------------------------------
        // Template-based path operations
        //----------------------------------------------

        /**
         * @brief Get typed value at specified path
         * @tparam T Type to retrieve (string, int, double, bool, Document)
         * @param path Path to value (supports: "/key/subkey" JSON Pointer, "key.subkey" dot notation, "arr[0]" array
         * syntax)
         * @return Optional containing value if exists and correct type, empty otherwise
         */
        template <Value T>
        [[nodiscard]] std::optional<T> get( std::string_view path ) const;

        /**
         * @brief Get typed value at specified path into output parameter
         * @tparam T Type to retrieve (string, int, double, bool, Document)
         * @param path Path to value (supports: "/key/subkey" JSON Pointer, "key.subkey" dot notation, "arr[0]" array
         * syntax)
         * @param[out] value Output parameter to store the result
         * @return true if value exists and was successfully retrieved, false otherwise
         */
        template <Value T>
        [[nodiscard]] bool get( std::string_view path, T& value ) const;

        /**
         * @brief Get a const reference to value at specified path
         * @tparam T Type to retrieve reference to
         * @param path Path to value (supports: "/key/subkey" JSON Pointer, "key.subkey" dot notation, "arr[0]" array
         * syntax)
         * @return std::optional<std::reference_wrapper<const T>> containing reference if exists and correct type
         *
         * @note Preferred for large Object/Array to avoid copying
         * @note Returns reference wrapped in optional - always safe
         * @note Example: auto obj_ref = doc.getRef<Object>("settings"); if(obj_ref) { const Object& obj =
         * obj_ref->get(); }
         */
        template <Value T>
        [[nodiscard]] std::optional<std::reference_wrapper<const T>> getRef( std::string_view path ) const;

        /**
         * @brief Get a mutable reference to value at specified path (zero-copy)
         * @tparam T Type to retrieve reference to
         * @param path Path to value (supports: "/key/subkey" JSON Pointer, "key.subkey" dot notation, "arr[0]" array
         * syntax)
         * @return std::optional<std::reference_wrapper<T>> containing mutable reference if exists and correct type
         *
         * @note Allows direct modification of large containers
         * @note Returns reference wrapped in optional - always safe
         */
        template <Value T>
        [[nodiscard]] std::optional<std::reference_wrapper<T>> getRef( std::string_view path );

        /**
         * @brief Set typed value at specified path (copy version)
         * @tparam T Type to set (string, int, double, bool, Document)
         * @param path Path where to set value (supports: "/key/subkey" JSON Pointer, "key.subkey" dot notation,
         * "arr[0]" array syntax)
         * @param value Value to set (copied)
         */
        template <Value T>
        void set( std::string_view path, const T& value );

        /**
         * @brief Set typed value at specified path (move version)
         * @tparam T Type to set (string, int, double, bool, Document)
         * @param path Path where to set value (supports: "/key/subkey" JSON Pointer, "key.subkey" dot notation,
         * "arr[0]" array syntax)
         * @param value Value to set (moved)
         */
        template <Value T>
        void set( std::string_view path, T&& value );

        //-----------------------------
        // Type-only creation
        //-----------------------------

        /**
         * @brief Create empty container at specified path
         * @tparam T Container type (Document, Object, Array)
         * @param path Path where to create container (supports: "/key/subkey" JSON Pointer, "key.subkey" dot notation,
         * "arr[0]" array syntax)
         */
        template <Container T>
        void set( std::string_view path );

        /**
         * @brief Set null value at specified path
         * @param path Path where to set null (supports: "/key/subkey" JSON Pointer, "key.subkey" dot notation, "arr[0]"
         * array syntax)
         */
        void setNull( std::string_view path );

        /**
         * @brief Check if value at path is of specified type
         * @tparam T Type to check for
         * @param path Path to check (supports: "/key/subkey" JSON Pointer, "key.subkey" dot notation, "arr[0]" array
         * syntax)
         * @return true if value exists and is of type T, false otherwise
         */
        template <Checkable T>
        [[nodiscard]] bool is( std::string_view path ) const;

        /**
         * @brief Check if value at path is null
         * @param path Path to check (supports: "/key/subkey" JSON Pointer, "key.subkey" dot notation, "arr[0]" array
         * syntax)
         * @return true if value is null, false otherwise
         */
        [[nodiscard]] bool isNull( std::string_view path ) const;

        /**
         * @brief Check if Document is in valid state
         * @return true if valid, false otherwise
         */
        [[nodiscard]] bool isValid() const;

        //----------------------------------------------
        // Document::PathView class
        //----------------------------------------------

        /**
         * @brief Path iterator for traversing all paths in a JSON document
         * @details Provides depth-first traversal of all JSON paths with their values.
         *          Supports both JSON Pointer format ("/user/name") and dot notation ("user.name").
         *
         * Example usage:
         * @code
         * for (const auto& entry : Document::PathView(doc)) {
         *     std::cout << entry.path << " = " << entry.value().toString() << "\n";
         * }
         * @endcode
         */
        class PathView final
        {
        public:
            //-----------------------------
            // Forward declaration
            //-----------------------------

            class iterator;

            //-----------------------------
            // Format
            //-----------------------------

            /**
             * @brief Format for path string representation
             */
            enum class Format : bool
            {
                JsonPointer = 0, ///< RFC 6901 JSON Pointer format (e.g., "/user/addresses/0/city")
                DotNotation      ///< Dot notation format (e.g., "user.addresses[0].city")
            };

            //-----------------------------
            // Path entry structure
            //-----------------------------

            /**
             * @brief Represents a single path entry in the document
             */
            struct Entry
            {
                std::string path;                   ///< Full path to this value
                std::unique_ptr<Document> valuePtr; ///< The value at this path (owned pointer)
                size_t depth;                       ///< Nesting depth (0 = root level)
                bool isLeaf;                        ///< True if value is a primitive (not object/array)

                /// @brief Default constructor
                inline Entry();

                /**
                 * @brief Move constructor
                 * @param other Entry to move from
                 */
                Entry( Entry&& other ) noexcept = default;

                /**
                 * @brief Move assignment
                 * @param other Entry to move from
                 * @return Reference to this entry
                 */
                Entry& operator=( Entry&& other ) noexcept = default;

                /**
                 * @brief Copy constructor
                 * @param other Entry to copy from
                 */
                Entry( const Entry& other );

                /**
                 * @brief Copy assignment
                 * @param other Entry to copy from
                 * @return Reference to this entry
                 */
                Entry& operator=( const Entry& other );

                /**
                 * @brief Get the value as a Document reference
                 * @return Const reference to the Document value
                 */
                inline const Document& value() const;
            };

            //-----------------------------
            // Construction
            //-----------------------------

            /**
             * @brief Construct PathView for a document
             * @param doc Document to iterate
             * @param format Path format to use
             * @param includeContainers Whether to include object/array container paths
             *
             * Example usage:
             * @code
             * for (const auto& entry : Document::PathView(doc)) {
             *     std::cout << entry.path << "\n";
             * }
             * @endcode
             */
            explicit PathView(
                const Document& doc, Format format = Format::JsonPointer, bool includeContainers = true );

            //-----------------------------
            // Range interface
            //-----------------------------

            /**
             * @brief Get iterator to first entry
             * @return iterator pointing to first path entry
             */
            inline iterator begin() const;

            /**
             * @brief Get iterator past last entry
             * @return iterator representing end position
             */
            inline iterator end() const;

            //-----------------------------
            // Direct access
            //-----------------------------

            /**
             * @brief Get number of path entries
             * @return Number of entries
             */
            inline size_t size() const;

            /**
             * @brief Check if empty
             * @return true if no entries
             */
            inline bool empty() const;

            /**
             * @brief Access entry by index
             * @param index Index of entry
             * @return Reference to entry at index
             */
            inline const Entry& operator[]( size_t index ) const;

            //-----------------------------
            // Filtering
            //-----------------------------

            /**
             * @brief Get only leaf entries (primitives)
             * @return Vector of leaf entries
             */
            std::vector<Entry> leaves() const;

            //-----------------------------
            // Document::PathView::iterator class
            //-----------------------------

            /**
             * @brief Forward iterator for path entries
             */
            class iterator
            {
            public:
                /** @brief iterator category tag for STL compatibility */
                using iterator_category = std::forward_iterator_tag;

                /** @brief Type of value yielded by iterator */
                using value_type = Entry;

                /** @brief Type for iterator difference */
                using difference_type = std::ptrdiff_t;

                /** @brief Pointer to value type */
                using pointer = const Entry*;

                /** @brief Reference to value type */
                using reference = const Entry&;

                /// @brief Default constructor
                inline iterator();

                /**
                 * @brief Construct iterator at position
                 * @param entries Pointer to entries vector
                 * @param index Current position
                 */
                inline iterator( const std::vector<Entry>* entries, size_t index );

                /**
                 * @brief Dereference operator
                 * @return Reference to current entry
                 */
                inline reference operator*() const;

                /**
                 * @brief Arrow operator
                 * @return Pointer to current entry
                 */
                inline pointer operator->() const;

                /**
                 * @brief Pre-increment operator
                 * @return Reference to this iterator
                 */
                inline iterator& operator++();

                /**
                 * @brief Post-increment operator
                 * @return Copy of iterator before increment
                 */
                inline iterator operator++( int );

                /**
                 * @brief Equality comparison
                 * @param other iterator to compare with
                 * @return true if equal
                 */
                inline bool operator==( const iterator& other ) const;

                /**
                 * @brief Inequality comparison
                 * @param other iterator to compare with
                 * @return true if not equal
                 */
                inline bool operator!=( const iterator& other ) const;

            private:
                const std::vector<Entry>* m_entries; ///< Pointer to entries
                size_t m_index;                      ///< Current position
            };

        private:
            //-----------------------------
            // Private helper methods
            //-----------------------------

            /**
             * @brief Build entries via depth-first traversal
             * @param doc Document to traverse
             */
            void buildEntries( const Document& doc );

            /**
             * @brief Format path string based on format setting
             * @param segments Path segments
             * @return Formatted path string
             */
            std::string formatPath( const std::vector<std::string>& segments ) const;

            //----------------------------------------------
            // Private data members
            //----------------------------------------------

            Format m_format;              ///< Path format to use
            bool m_includeContainers;     ///< Whether to include container nodes
            std::vector<Entry> m_entries; ///< Pre-computed entries
        };

    private:
        //----------------------------------------------
        // Private internal access
        //----------------------------------------------

        /**
         * @brief Get direct const reference to root value
         * @tparam T Type of the value
         * @return Direct const reference to the value
         * @throws std::bad_variant_access if type doesn't match
         */
        template <typename T>
        const T& rootInternal() const;

        /**
         * @brief Get direct mutable reference to root value
         * @tparam T Type of the value
         * @return Direct mutable reference to the value
         * @throws std::bad_variant_access if type doesn't match
         */
        template <typename T>
        T& rootInternal();

        //----------------------------------------------
        // Private path helpers
        //----------------------------------------------

        /**
         * @brief Internal helper for contains() when dealing with paths
         * @param path Path with special characters (supports: "/key/subkey" JSON Pointer, "key.subkey" dot notation,
         * "arr[0]" array syntax)
         * @return true if path exists
         */
        bool containsPath( std::string_view path ) const;

        /**
         * @brief Internal helper for erase() when dealing with paths
         * @param path Path with special characters (supports: "/key/subkey" JSON Pointer, "key.subkey" dot notation,
         * "arr[0]" array syntax)
         * @return Number of elements erased
         */
        size_t erasePath( std::string_view path );

        //----------------------------------------------
        // Private navigation methods
        //----------------------------------------------

        /**
         * @brief Navigate to a JSON node at the specified path
         * @param path Path to navigate (supports: "/key/subkey" JSON Pointer, "key.subkey" dot notation, "arr[0]" array
         * syntax)
         * @param createPath If true, creates intermediate objects/arrays if they don't exist
         * @return Pointer to the JSON node at the path, or nullptr if path doesn't exist
         */
        Document* navigateToPath( std::string_view path, bool createPath = false );

        /**
         * @brief Navigate to a JSON node at the specified path (const version)
         * @param path Path to navigate (supports: "/key/subkey" JSON Pointer, "key.subkey" dot notation, "arr[0]" array
         * syntax)
         * @return Pointer to the JSON node at the path, or nullptr if path doesn't exist
         */
        const Document* navigateToPath( std::string_view path ) const;

        /**
         * @brief Navigate to a JSON node using RFC 6901 JSON Pointer syntax
         * @param pointer JSON Pointer string (e.g., "/users/0/name" or "/data/items/-")
         * @param createPath If true, creates intermediate objects/arrays if they don't exist
         * @return Pointer to the JSON node at the pointer path, or nullptr if path doesn't exist
         */
        Document* navigateToJsonPointer( std::string_view pointer, bool createPath = false );

        /**
         * @brief Navigate to a JSON node using RFC 6901 JSON Pointer syntax (const version)
         * @param pointer JSON Pointer string (e.g., "/users/0/name" or "/data/items/-")
         * @return Pointer to the JSON node at the pointer path, or nullptr if path doesn't exist
         */
        const Document* navigateToJsonPointer( std::string_view pointer ) const;

        /**
         * @brief Unescape JSON Pointer token according to RFC 6901
         */
        static std::string unescapeJsonPointerToken( std::string_view token ) noexcept;

        /**
         * @brief Check if a token represents a valid array index
         */
        static bool isValidArrayIndex( std::string_view token ) noexcept;

        //----------------------------------------------
        // Private members
        //----------------------------------------------

#if defined( __GNUC__ ) && !defined( __clang__ )
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
        std::variant<std::nullptr_t, bool, int64_t, uint64_t, double, std::string, Array, Object> m_data;
#if defined( __GNUC__ ) && !defined( __clang__ )
#    pragma GCC diagnostic pop
#endif
    };
} // namespace nfx::json

#include "detail/Document.inl"
