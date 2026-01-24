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
 * @file Document.inl
 * @brief Inline Implementation of Document class
 */

namespace nfx::json
{
    //=====================================================================
    // Document class
    //=====================================================================

    //----------------------------------------------
    // Construction
    //----------------------------------------------

    inline Document::Document()
        : m_data{ Object{} }
    {
    }

    inline Document::Document( bool value ) noexcept
        : m_data{ value }
    {
    }

    inline Document::Document( int64_t value ) noexcept
        : m_data{ value }
    {
    }

    inline Document::Document( uint64_t value ) noexcept
        : m_data{ value }
    {
    }

    inline Document::Document( double value ) noexcept
        : m_data{ value }
    {
    }

    inline Document::Document( std::string value )
        : m_data{ std::move( value ) }
    {
    }

    inline Document::Document( std::string_view value )
        : m_data{ std::string( value ) }
    {
    }

    inline Document::Document( const char* value )
        : m_data{ std::string( value ) }
    {
    }

    inline Document::Document( std::nullptr_t ) noexcept
        : m_data{ nullptr }
    {
    }

    inline Document::Document( Array value )
        : m_data{ std::move( value ) }
    {
    }

    inline Document::Document( Object value )
        : m_data{ std::move( value ) }
    {
    }

    //----------------------------------------------
    // Assignment
    //----------------------------------------------

    inline Document& Document::operator=( std::string value )
    {
        m_data = std::move( value );
        return *this;
    }

    inline Document& Document::operator=( bool value ) noexcept
    {
        m_data = value;
        return *this;
    }

    inline Document& Document::operator=( int64_t value ) noexcept
    {
        m_data = value;
        return *this;
    }

    inline Document& Document::operator=( uint64_t value ) noexcept
    {
        m_data = value;
        return *this;
    }

    inline Document& Document::operator=( double value ) noexcept
    {
        m_data = value;
        return *this;
    }

    inline Document& Document::operator=( std::nullptr_t ) noexcept
    {
        m_data = nullptr;
        return *this;
    }

    //----------------------------------------------
    // Comparison
    //----------------------------------------------

    inline bool Document::operator==( const Document& other ) const noexcept
    {
        return m_data == other.m_data;
    }

    //----------------------------------------------
    // Static factory methods
    //----------------------------------------------

    inline Document Document::object()
    {
        return Document{ Object{} };
    }

    inline Document Document::array()
    {
        return Document{ Array{} };
    }

    //----------------------------------------------
    // Type queries
    //----------------------------------------------

    inline Type Document::type() const noexcept
    {
        static constexpr Type typeMapping[]{ Type::Null,   Type::Boolean, Type::Integer, Type::UnsignedInteger,
                                             Type::Double, Type::String,  Type::Array,   Type::Object };
        return typeMapping[m_data.index()];
    }

    //----------------------------------------------
    // Root value access
    //----------------------------------------------

    template <Value T>
    std::optional<T> Document::root() const
    {
        if( std::holds_alternative<T>( m_data ) )
        {
            return std::get<T>( m_data );
        }
        return std::nullopt;
    }

    template <Value T>
    inline bool Document::root( T& out ) const
    {
        auto val = root<T>();
        if( val )
        {
            out = std::move( *val );
            return true;
        }
        return false;
    }

    template <Value T>
    inline std::optional<std::reference_wrapper<const T>> Document::rootRef() const
    {
        if( std::holds_alternative<T>( m_data ) )
        {
            return std::cref( std::get<T>( m_data ) );
        }
        return std::nullopt;
    }

    template <Value T>
    inline std::optional<std::reference_wrapper<T>> Document::rootRef()
    {
        if( std::holds_alternative<T>( m_data ) )
        {
            return std::ref( std::get<T>( m_data ) );
        }
        return std::nullopt;
    }

    template <Checkable T>
    inline bool Document::isRoot() const
    {
        return std::holds_alternative<T>( m_data );
    }

    template <typename Visitor>
    inline decltype( auto ) Document::visit( Visitor&& visitor ) const
    {
        return std::visit( std::forward<Visitor>( visitor ), m_data );
    }

    template <typename Visitor>
    inline decltype( auto ) Document::visit( Visitor&& visitor )
    {
        return std::visit( std::forward<Visitor>( visitor ), m_data );
    }

    //----------------------------------------------
    // Object operations
    //----------------------------------------------

    inline Document* Document::find( std::string_view key ) noexcept
    {
        if( type() != Type::Object )
        {
            return nullptr;
        }

        auto& obj = std::get<Object>( m_data );
        auto it = std::find_if( obj.begin(), obj.end(), [key]( const auto& pair ) { return pair.first == key; } );
        if( it != obj.end() )
        {
            return &it->second;
        }
        return nullptr;
    }

    inline const Document* Document::find( std::string_view key ) const noexcept
    {
        return const_cast<Document*>( this )->find( key );
    }

    inline void Document::set( std::string key, Document value )
    {
        if( type() != Type::Object )
        {
            m_data = Object{};
        }

        auto& obj = std::get<Object>( m_data );
        auto it = std::find_if( obj.begin(), obj.end(), [&key]( const auto& pair ) { return pair.first == key; } );
        if( it != obj.end() )
        {
            it->second = std::move( value );
        }
        else
        {
            obj.emplace_back( std::move( key ), std::move( value ) );
        }
    }

    inline bool Document::contains( std::string_view key ) const noexcept
    {
        // Empty string means root document (always exists)
        if( key.empty() )
        {
            return true;
        }

        // If key contains '.', '/', or '[', treat it as a path
        if( key.find( '.' ) != std::string_view::npos || key.find( '/' ) != std::string_view::npos ||
            key.find( '[' ) != std::string_view::npos )
        {
            return containsPath( key );
        }

        // Otherwise, simple key lookup
        if( type() != Type::Object )
        {
            return false;
        }
        const auto& obj = std::get<Object>( m_data );
        return std::find_if( obj.begin(), obj.end(), [key]( const auto& pair ) { return pair.first == key; } ) !=
               obj.end();
    }

    inline Document& Document::at( std::string_view key )
    {
        if( type() != Type::Object )
        {
            throw std::runtime_error{ "Not an object" };
        }

        auto& obj = std::get<Object>( m_data );
        auto it = std::find_if( obj.begin(), obj.end(), [key]( const auto& pair ) { return pair.first == key; } );
        if( it == obj.end() )
        {
            throw std::out_of_range{ "Key not found" };
        }

        return it->second;
    }

    inline const Document& Document::at( std::string_view key ) const
    {
        return const_cast<Document*>( this )->at( key );
    }

    inline Document& Document::at( size_t index )
    {
        if( type() != Type::Array )
        {
            throw std::runtime_error{ "Not an array" };
        }

        auto& arr = std::get<Array>( m_data );
        if( index >= arr.size() )
        {
            throw std::out_of_range{ "Array index out of bounds" };
        }

        return arr[index];
    }

    inline const Document& Document::at( size_t index ) const
    {
        return const_cast<Document*>( this )->at( index );
    }

    inline Document& Document::operator[]( std::string_view key )
    {
        if( type() == Type::Null )
        {
            m_data = Object{};
        }

#ifndef NDEBUG
        if( type() != Type::Object )
        {
            throw std::runtime_error( "operator[]: Cannot use key access on non-Object type" );
        }
#endif

        auto& obj = std::get<Object>( m_data );
        auto it = std::find_if( obj.begin(), obj.end(), [key]( const auto& pair ) { return pair.first == key; } );
        if( it != obj.end() )
        {
            return it->second;
        }
        obj.emplace_back( std::string( key ), Document{} );
        return obj.back().second;
    }

    inline const Document& Document::operator[]( std::string_view key ) const
    {
        static const Document null_value{};
        if( type() != Type::Object )
        {
            return null_value;
        }

        const auto& obj = std::get<Object>( m_data );
        auto it = std::find_if( obj.begin(), obj.end(), [key]( const auto& pair ) { return pair.first == key; } );
        if( it == obj.end() )
        {
            return null_value;
        }
        return it->second;
    }

    inline Document& Document::operator[]( size_t index )
    {
        if( type() == Type::Null )
        {
            m_data = Array{};
        }

#ifndef NDEBUG
        if( type() != Type::Array )
        {
            throw std::runtime_error( "operator[]: Cannot use index access on non-Array type" );
        }
#endif

        auto& arr = std::get<Array>( m_data );
        if( index >= arr.size() )
        {
            arr.resize( index + 1, Document{ nullptr } );
        }
        return arr[index];
    }

    inline const Document& Document::operator[]( size_t index ) const
    {
        static const Document nullValue{ nullptr };
        if( type() != Type::Array )
        {
            return nullValue;
        }

        const auto& arr = std::get<Array>( m_data );
        if( index >= arr.size() )
        {
            return nullValue;
        }
        return arr[index];
    }

    inline void Document::clear()
    {
        if( type() == Type::Object )
        {
            std::get<Object>( m_data ).clear();
        }
        else if( type() == Type::Array )
        {
            std::get<Array>( m_data ).clear();
        }
    }

    inline size_t Document::erase( std::string_view key )
    {
        // If key contains '.', '/', or '[', treat it as a path
        if( key.find( '.' ) != std::string_view::npos || key.find( '/' ) != std::string_view::npos ||
            key.find( '[' ) != std::string_view::npos )
        {
            return erasePath( key );
        }

        // Otherwise, simple key lookup
        if( type() != Type::Object )
        {
            return 0;
        }
        auto& obj = std::get<Object>( m_data );
        auto it = std::find_if( obj.begin(), obj.end(), [key]( const auto& pair ) { return pair.first == key; } );
        if( it != obj.end() )
        {
            obj.erase( it );
            return 1;
        }
        return 0;
    }

    inline size_t Document::erase( const char* key )
    {
        return erase( std::string_view( key ) );
    }

    inline size_t Document::erase( const std::string& key )
    {
        return erase( std::string_view( key ) );
    }

    template <typename iterator, typename>
    inline iterator Document::erase( iterator it )
    {
        if( type() == Type::Array )
            return std::get<Array>( m_data ).erase( it );
        return it;
    }

    template <typename iterator>
    inline iterator Document::insert( iterator pos, Document value )
    {
        if( type() != Type::Array )
        {
            m_data = Array{};
        }
        return std::get<Array>( m_data ).insert( pos, std::move( value ) );
    }

    //----------------------------------------------
    // Array operations
    //----------------------------------------------

    inline size_t Document::size() const noexcept
    {
        if( type() == Type::Array )
        {
            return std::get<Array>( m_data ).size();
        }
        if( type() == Type::Object )
        {
            return std::get<Object>( m_data ).size();
        }
        return 0;
    }

    inline bool Document::empty() const noexcept
    {
        switch( type() )
        {
            case Type::Array:
                return std::get<Array>( m_data ).empty();
            case Type::Object:
                return std::get<Object>( m_data ).empty();
            case Type::String:
                return std::get<std::string>( m_data ).empty();
            default:
                return false;
        }
    }

    inline void Document::push_back( Document value )
    {
        if( type() != Type::Array )
        {
            m_data = Array{};
        }
        std::get<Array>( m_data ).push_back( std::move( value ) );
    }

    inline void Document::reserve( size_t capacity )
    {
        if( type() == Type::Array )
        {
            std::get<Array>( m_data ).reserve( capacity );
        }
        else if( type() == Type::Object )
        {
            std::get<Object>( m_data ).reserve( capacity );
        }
    }

    inline size_t Document::capacity() const noexcept
    {
        if( type() == Type::Array )
        {
            return std::get<Array>( m_data ).capacity();
        }
        if( type() == Type::Object )
        {
            return std::get<Object>( m_data ).capacity();
        }
        return 0;
    }

    inline Document& Document::front()
    {
        auto& arr = std::get<Array>( m_data );
        if( arr.empty() )
        {
            throw std::out_of_range{ "Array is empty" };
        }
        return arr.front();
    }

    inline const Document& Document::front() const
    {
        return const_cast<Document*>( this )->front();
    }

    inline Document& Document::back()
    {
        return std::get<Array>( m_data ).back();
    }

    inline const Document& Document::back() const
    {
        return std::get<Array>( m_data ).back();
    }

    inline auto Document::begin()
    {
        return std::get<Array>( m_data ).begin();
    }

    inline auto Document::end()
    {
        return std::get<Array>( m_data ).end();
    }

    inline auto Document::begin() const
    {
        return std::get<Array>( m_data ).begin();
    }

    inline auto Document::end() const
    {
        return std::get<Array>( m_data ).end();
    }

    //----------------------------------------------
    // Document::ObjectIterator
    //----------------------------------------------

    inline Document::ObjectIterator::ObjectIterator( MapIterator iter )
        : it{ iter }
    {
    }

    inline const std::string& Document::ObjectIterator::key() const
    {
        return it->first;
    }

    inline const Document& Document::ObjectIterator::value() const
    {
        return it->second;
    }

    inline Document& Document::ObjectIterator::value()
    {
        return const_cast<Document&>( it->second );
    }

    inline const Document& Document::ObjectIterator::operator*() const
    {
        return it->second;
    }

    inline Document& Document::ObjectIterator::operator*()
    {
        return const_cast<Document&>( it->second );
    }

    inline Document::ObjectIterator& Document::ObjectIterator::operator++()
    {
        ++it;
        return *this;
    }

    inline bool Document::ObjectIterator::operator!=( const ObjectIterator& other ) const
    {
        return it != other.it;
    }

    inline bool Document::ObjectIterator::operator==( const ObjectIterator& other ) const
    {
        return it == other.it;
    }

    inline Document::ObjectIterator::MapIterator Document::ObjectIterator::base() const
    {
        return it;
    }

    //----------------------------------------------
    // Object iteration
    //----------------------------------------------

    inline Document::ObjectIterator Document::objectBegin() const
    {
        if( type() != Type::Object )
        {
            return ObjectIterator( Object{}.end() );
        }
        return ObjectIterator( std::get<Object>( m_data ).begin() );
    }

    inline Document::ObjectIterator Document::objectEnd() const
    {
        if( type() != Type::Object )
        {
            return ObjectIterator( Object{}.end() );
        }
        return ObjectIterator( std::get<Object>( m_data ).end() );
    }

    //----------------------------------------------
    // Document::KeysView class
    //----------------------------------------------

    inline Document::KeysView::iterator::iterator( Object::const_iterator it )
        : m_it{ it }
    {
    }

    inline Document::KeysView::iterator::reference Document::KeysView::iterator::operator*() const
    {
        return m_it->first;
    }

    inline Document::KeysView::iterator::pointer Document::KeysView::iterator::operator->() const
    {
        return &m_it->first;
    }

    inline Document::KeysView::iterator& Document::KeysView::iterator::operator++()
    {
        ++m_it;
        return *this;
    }

    inline Document::KeysView::iterator Document::KeysView::iterator::operator++( int )
    {
        iterator tmp = *this;
        ++m_it;
        return tmp;
    }

    inline bool Document::KeysView::iterator::operator==( const iterator& other ) const
    {
        return m_it == other.m_it;
    }

    inline bool Document::KeysView::iterator::operator!=( const iterator& other ) const
    {
        return m_it != other.m_it;
    }

    inline Document::KeysView::KeysView( const Object* obj )
        : m_obj{ obj }
    {
    }

    inline Document::KeysView::iterator Document::KeysView::begin() const
    {
        return iterator( m_obj->begin() );
    }

    inline Document::KeysView::iterator Document::KeysView::end() const
    {
        return iterator( m_obj->end() );
    }

    //----------------------------------------------
    // Document::ValuesView class
    //----------------------------------------------

    inline Document::ValuesView::const_iterator::const_iterator( Object::const_iterator it )
        : m_it{ it }
    {
    }

    inline Document::ValuesView::const_iterator::reference Document::ValuesView::const_iterator::operator*() const
    {
        return m_it->second;
    }

    inline Document::ValuesView::const_iterator::pointer Document::ValuesView::const_iterator::operator->() const
    {
        return &m_it->second;
    }

    inline Document::ValuesView::const_iterator& Document::ValuesView::const_iterator::operator++()
    {
        ++m_it;
        return *this;
    }

    inline Document::ValuesView::const_iterator Document::ValuesView::const_iterator::operator++( int )
    {
        const_iterator tmp = *this;
        ++m_it;
        return tmp;
    }

    inline bool Document::ValuesView::const_iterator::operator==( const const_iterator& other ) const
    {
        return m_it == other.m_it;
    }

    inline bool Document::ValuesView::const_iterator::operator!=( const const_iterator& other ) const
    {
        return m_it != other.m_it;
    }

    inline Document::ValuesView::iterator::iterator( Object::iterator it )
        : m_it{ it }
    {
    }

    inline Document::ValuesView::iterator::reference Document::ValuesView::iterator::operator*() const
    {
        return m_it->second;
    }

    inline Document::ValuesView::iterator::pointer Document::ValuesView::iterator::operator->() const
    {
        return &m_it->second;
    }

    inline Document::ValuesView::iterator& Document::ValuesView::iterator::operator++()
    {
        ++m_it;
        return *this;
    }

    inline Document::ValuesView::iterator Document::ValuesView::iterator::operator++( int )
    {
        iterator tmp = *this;
        ++m_it;
        return tmp;
    }

    inline bool Document::ValuesView::iterator::operator==( const iterator& other ) const
    {
        return m_it == other.m_it;
    }

    inline bool Document::ValuesView::iterator::operator!=( const iterator& other ) const
    {
        return m_it != other.m_it;
    }

    inline Document::ValuesView::ValuesView( const Object* obj )
        : m_obj{ obj }
    {
    }

    inline Document::ValuesView::const_iterator Document::ValuesView::begin() const
    {
        return const_iterator( m_obj->begin() );
    }

    inline Document::ValuesView::const_iterator Document::ValuesView::end() const
    {
        return const_iterator( m_obj->end() );
    }

    //----------------------------------------------
    // Document::MutableValuesView class
    //----------------------------------------------

    inline Document::MutableValuesView::iterator::iterator( Object::iterator it )
        : m_it{ it }
    {
    }

    inline Document::MutableValuesView::iterator::reference Document::MutableValuesView::iterator::operator*() const
    {
        return m_it->second;
    }

    inline Document::MutableValuesView::iterator::pointer Document::MutableValuesView::iterator::operator->() const
    {
        return &m_it->second;
    }

    inline Document::MutableValuesView::iterator& Document::MutableValuesView::iterator::operator++()
    {
        ++m_it;
        return *this;
    }

    inline Document::MutableValuesView::iterator Document::MutableValuesView::iterator::operator++( int )
    {
        iterator tmp = *this;
        ++m_it;
        return tmp;
    }

    inline bool Document::MutableValuesView::iterator::operator==( const iterator& other ) const
    {
        return m_it == other.m_it;
    }

    inline bool Document::MutableValuesView::iterator::operator!=( const iterator& other ) const
    {
        return m_it != other.m_it;
    }

    inline Document::MutableValuesView::MutableValuesView( Object* obj )
        : m_obj{ obj }
    {
    }

    inline Document::MutableValuesView::iterator Document::MutableValuesView::begin()
    {
        return iterator( m_obj->begin() );
    }

    inline Document::MutableValuesView::iterator Document::MutableValuesView::end()
    {
        return iterator( m_obj->end() );
    }

    //----------------------------------------------
    // Object keys/values accessors
    //----------------------------------------------

    inline Document::KeysView Document::keys() const
    {
        static const Object emptyObj;
        if( type() != Type::Object )
        {
            return KeysView( &emptyObj );
        }
        return KeysView( &std::get<Object>( m_data ) );
    }

    inline Document::ValuesView Document::values() const
    {
        static const Object emptyObj;
        if( type() != Type::Object )
        {
            return ValuesView( &emptyObj );
        }
        return ValuesView( &std::get<Object>( m_data ) );
    }

    inline Document::MutableValuesView Document::values()
    {
        static Object emptyObj;
        if( type() != Type::Object )
        {
            return MutableValuesView( &emptyObj );
        }
        return MutableValuesView( &std::get<Object>( m_data ) );
    }

    //----------------------------------------------
    // Document::PathView class
    //----------------------------------------------

    inline Document::PathView::Entry::Entry()
        : depth{ 0 },
          isLeaf{ false }
    {
    }

    inline const Document& Document::PathView::Entry::value() const
    {
        return *valuePtr;
    }

    inline Document::PathView::iterator Document::PathView::begin() const
    {
        return iterator{ &m_entries, 0 };
    }

    inline Document::PathView::iterator Document::PathView::end() const
    {
        return iterator{ &m_entries, m_entries.size() };
    }

    inline size_t Document::PathView::size() const
    {
        return m_entries.size();
    }

    inline bool Document::PathView::empty() const
    {
        return m_entries.empty();
    }

    inline const Document::PathView::Entry& Document::PathView::operator[]( size_t index ) const
    {
        return m_entries[index];
    }

    //-----------------------------
    // Document::PathView::iterator class
    //-----------------------------

    inline Document::PathView::iterator::iterator()
        : m_entries{ nullptr },
          m_index{ 0 }
    {
    }

    inline Document::PathView::iterator::iterator( const std::vector<Entry>* entries, size_t index )
        : m_entries{ entries },
          m_index{ index }
    {
    }

    inline Document::PathView::iterator::reference Document::PathView::iterator::operator*() const
    {
        return ( *m_entries )[m_index];
    }

    inline Document::PathView::iterator::pointer Document::PathView::iterator::operator->() const
    {
        return &( *m_entries )[m_index];
    }

    inline Document::PathView::iterator& Document::PathView::iterator::operator++()
    {
        ++m_index;
        return *this;
    }

    inline Document::PathView::iterator Document::PathView::iterator::operator++( int )
    {
        iterator tmp = *this;
        ++m_index;
        return tmp;
    }

    inline bool Document::PathView::iterator::operator==( const iterator& other ) const
    {
        if( !m_entries && !other.m_entries )
        {
            return true;
        }
        if( !m_entries )
        {
            return other.m_index >= other.m_entries->size();
        }
        if( !other.m_entries )
        {
            return m_index >= m_entries->size();
        }
        return m_entries == other.m_entries && m_index == other.m_index;
    }

    inline bool Document::PathView::iterator::operator!=( const iterator& other ) const
    {
        return !( *this == other );
    }

    //----------------------------------------------
    // Private internal access
    //----------------------------------------------

    template <typename T>
    inline const T& Document::rootInternal() const
    {
        return std::get<T>( m_data );
    }

    template <typename T>
    inline T& Document::rootInternal()
    {
        return std::get<T>( m_data );
    }
} // namespace nfx::json
