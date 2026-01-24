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
 * @file Tests_JsonDocumentAccess.cpp
 * @brief Unit tests for modern Document access patterns (getRef, rootRef, operator[])
 * @details Tests covering optimal zero-copy access methods: getRef<T>(), rootRef<T>(),
 *          and operator[] usage patterns. Demonstrates performance-optimal API usage.
 */

#include <gtest/gtest.h>

#include <nfx/Json.h>

namespace nfx::json::test
{
    using namespace nfx::json;

    //=====================================================================
    // Modern API: getRef<T>() - Zero-copy reference access with path
    //=====================================================================

    TEST( JsonDocumentAccess, GetRef_BasicTypes )
    {
        auto doc = Document::fromString( R"({"str":"hello","num":42,"dbl":3.14,"bool":true})" );
        ASSERT_TRUE( doc.has_value() );

        // getRef returns optional<reference_wrapper>
        auto strRef = doc->getRef<std::string>( "str" );
        ASSERT_TRUE( strRef.has_value() );
        EXPECT_EQ( strRef->get(), "hello" );

        auto numRef = doc->getRef<int64_t>( "num" );
        ASSERT_TRUE( numRef.has_value() );
        EXPECT_EQ( numRef->get(), 42 );

        auto dblRef = doc->getRef<double>( "dbl" );
        ASSERT_TRUE( dblRef.has_value() );
        EXPECT_DOUBLE_EQ( dblRef->get(), 3.14 );

        auto boolRef = doc->getRef<bool>( "bool" );
        ASSERT_TRUE( boolRef.has_value() );
        EXPECT_TRUE( boolRef->get() );
    }

    TEST( JsonDocumentAccess, GetRef_Modification )
    {
        auto doc = Document::fromString( R"({"value":10})" );
        ASSERT_TRUE( doc.has_value() );

        auto ref = doc->getRef<int64_t>( "value" );
        ASSERT_TRUE( ref.has_value() );

        // Modify through reference
        ref->get() = 20;

        // Verify modification
        EXPECT_EQ( ( *doc )["value"].rootRef<int64_t>().value().get(), 20 );
    }

    TEST( JsonDocumentAccess, GetRef_MissingKey )
    {
        auto doc = Document::fromString( R"({"key":123})" );
        ASSERT_TRUE( doc.has_value() );

        auto ref = doc->getRef<std::string>( "missing" );
        EXPECT_FALSE( ref.has_value() );
    }

    TEST( JsonDocumentAccess, GetRef_WrongType )
    {
        auto doc = Document::fromString( R"({"key":"string"})" );
        ASSERT_TRUE( doc.has_value() );

        auto ref = doc->getRef<int64_t>( "key" );
        EXPECT_FALSE( ref.has_value() );
    }

    TEST( JsonDocumentAccess, GetRef_NestedPath )
    {
        auto doc = Document::fromString( R"({"user":{"name":"Alice","age":30}})" );
        ASSERT_TRUE( doc.has_value() );

        // JSON Pointer path
        auto nameRef = doc->getRef<std::string>( "/user/name" );
        ASSERT_TRUE( nameRef.has_value() );
        EXPECT_EQ( nameRef->get(), "Alice" );

        // Dot notation path
        auto ageRef = doc->getRef<int64_t>( "user.age" );
        ASSERT_TRUE( ageRef.has_value() );
        EXPECT_EQ( ageRef->get(), 30 );
    }

    TEST( JsonDocumentAccess, GetRef_ArrayElement )
    {
        auto doc = Document::fromString( R"({"items":["first","second","third"]})" );
        ASSERT_TRUE( doc.has_value() );

        // Array index notation
        auto ref = doc->getRef<std::string>( "items[1]" );
        ASSERT_TRUE( ref.has_value() );
        EXPECT_EQ( ref->get(), "second" );
    }

    TEST( JsonDocumentAccess, GetRef_DeepNesting )
    {
        auto doc = Document::fromString( R"({"a":{"b":{"c":{"value":42}}}}})" );
        ASSERT_TRUE( doc.has_value() );

        auto ref = doc->getRef<int64_t>( "/a/b/c/value" );
        ASSERT_TRUE( ref.has_value() );
        EXPECT_EQ( ref->get(), 42 );

        // Modify deep value
        ref->get() = 100;
        EXPECT_EQ( ( *doc )["a"]["b"]["c"]["value"].rootRef<int64_t>().value().get(), 100 );
    }

    //=====================================================================
    // Modern API: rootRef<T>() - Direct root value access
    //=====================================================================

    TEST( JsonDocumentAccess, RootRef_BasicTypes )
    {
        Document strDoc{ std::string( "hello" ) };
        Document intDoc{ int64_t{ 42 } };
        Document dblDoc{ 3.14 };
        Document boolDoc{ true };

        auto& str = strDoc.rootRef<std::string>().value().get();
        EXPECT_EQ( str, "hello" );

        auto& num = intDoc.rootRef<int64_t>().value().get();
        EXPECT_EQ( num, 42 );

        auto& dbl = dblDoc.rootRef<double>().value().get();
        EXPECT_DOUBLE_EQ( dbl, 3.14 );

        auto& boolean = boolDoc.rootRef<bool>().value().get();
        EXPECT_TRUE( boolean );
    }

    TEST( JsonDocumentAccess, RootRef_WrongType )
    {
        Document doc{ std::string( "string" ) };

        auto ref = doc.rootRef<int64_t>();
        EXPECT_FALSE( ref.has_value() );
    }

    TEST( JsonDocumentAccess, RootRef_ArrayModification )
    {
        auto doc = Document::fromString( R"([1,2,3])" );
        ASSERT_TRUE( doc.has_value() );

        auto& arr = doc->rootRef<Array>().value().get();
        arr.push_back( Document{ int64_t{ 4 } } );
        arr.push_back( Document{ int64_t{ 5 } } );

        EXPECT_EQ( doc->size(), 5 );
        EXPECT_EQ( ( *doc )[4].rootRef<int64_t>().value().get(), 5 );
    }

    TEST( JsonDocumentAccess, RootRef_ObjectModification )
    {
        auto doc = Document::fromString( R"({"count":0})" );
        ASSERT_TRUE( doc.has_value() );

        // Modify through Document operator[], not Object directly
        ( *doc )["count"] = Document{ int64_t{ 10 } };
        ( *doc )["new_key"] = Document{ std::string( "new_value" ) };

        EXPECT_EQ( ( *doc )["count"].rootRef<int64_t>().value().get(), 10 );
        EXPECT_EQ( ( *doc )["new_key"].rootRef<std::string>().value().get(), "new_value" );
    }

    TEST( JsonDocumentAccess, RootRef_ConstAccess )
    {
        auto doc = Document::fromString( R"({"value":42})" );
        ASSERT_TRUE( doc.has_value() );
        const Document& constDoc = *doc;

        const auto& obj = constDoc.rootRef<Object>().value().get();
        EXPECT_EQ( obj.size(), 1 );

        // Check if key exists
        bool hasValue = false;
        for( const auto& [key, val] : obj )
        {
            if( key == "value" )
            {
                hasValue = true;
                break;
            }
        }
        EXPECT_TRUE( hasValue );
    }

    //=====================================================================
    // Modern API: operator[] - Optimal navigation pattern
    //=====================================================================

    TEST( JsonDocumentAccess, OperatorBracket_ChainedAccess )
    {
        auto doc = Document::fromString( R"({"user":{"profile":{"name":"Alice"}}})" );
        ASSERT_TRUE( doc.has_value() );

        // Optimal: chain operator[] then rootRef
        const auto& name = ( *doc )["user"]["profile"]["name"].rootRef<std::string>().value().get();

        EXPECT_EQ( name, "Alice" );
    }

    TEST( JsonDocumentAccess, OperatorBracket_CreateIfMissing )
    {
        Document doc;

        // operator[] creates keys if missing (for objects)
        doc["new_key"] = Document{ std::string( "new_value" ) };

        EXPECT_TRUE( doc.contains( "new_key" ) );
        EXPECT_EQ( doc["new_key"].rootRef<std::string>().value().get(), "new_value" );
    }

    TEST( JsonDocumentAccess, OperatorBracket_ArrayAccess )
    {
        auto doc = Document::fromString( R"([10,20,30,40,50])" );
        ASSERT_TRUE( doc.has_value() );

        // Array index access
        EXPECT_EQ( ( *doc )[0].rootRef<int64_t>().value().get(), 10 );
        EXPECT_EQ( ( *doc )[2].rootRef<int64_t>().value().get(), 30 );
        EXPECT_EQ( ( *doc )[4].rootRef<int64_t>().value().get(), 50 );
    }

    TEST( JsonDocumentAccess, OperatorBracket_OutOfBounds )
    {
        auto doc = Document::fromString( R"([1,2,3])" );
        ASSERT_TRUE( doc.has_value() );

        // Out of bounds access returns a null value (safe default behavior)
        auto& element = ( *doc )[10];

        EXPECT_EQ( element.type(), Type::Null );
    }

    //=====================================================================
    // Performance Comparison: Old API vs Modern API
    //=====================================================================

    TEST( JsonDocumentAccess, API_Comparison_AllMethods )
    {
        auto doc = Document::fromString( R"({"str":"test","num":42})" );
        ASSERT_TRUE( doc.has_value() );

        auto strCopy = doc->get<std::string>( "str" );
        ASSERT_TRUE( strCopy.has_value() );
        EXPECT_EQ( strCopy.value(), "test" );

        auto strRef = doc->getRef<std::string>( "str" );
        ASSERT_TRUE( strRef.has_value() );
        EXPECT_EQ( strRef->get(), "test" );

        const auto& strDirect = ( *doc )["str"].rootRef<std::string>().value().get();
        EXPECT_EQ( strDirect, "test" );
    }

    TEST( JsonDocumentAccess, API_Comparison_NestedAccess )
    {
        auto doc = Document::fromString( R"({"user":{"data":{"value":123}}})" );
        ASSERT_TRUE( doc.has_value() );

        // Old way: multiple get calls
        auto userOpt = doc->get<Object>( "user" );
        ASSERT_TRUE( userOpt.has_value() );
        Document userDoc{ userOpt.value() };
        auto dataOpt = userDoc.get<Object>( "data" );
        ASSERT_TRUE( dataOpt.has_value() );
        Document dataDoc{ dataOpt.value() };
        auto valueOpt = dataDoc.get<int64_t>( "value" );
        ASSERT_TRUE( valueOpt.has_value() );
        EXPECT_EQ( valueOpt.value(), 123 );

        // New way: chained operator[] (cleaner, faster)
        const auto& value = ( *doc )["user"]["data"]["value"].rootRef<int64_t>().value().get();
        EXPECT_EQ( value, 123 );
    }

    TEST( JsonDocumentAccess, API_Comparison_Modification )
    {
        auto doc = Document::fromString( R"({"counter":0})" );
        ASSERT_TRUE( doc.has_value() );

        // Old way: get, modify copy, set back
        auto counterCopy = doc->get<int64_t>( "counter" ).value();
        counterCopy++;
        doc->set<int64_t>( "counter", counterCopy );
        EXPECT_EQ( doc->get<int64_t>( "counter" ).value(), 1 );

        // New way: direct reference modification
        auto counterRef = doc->getRef<int64_t>( "counter" );
        counterRef->get()++;
        EXPECT_EQ( ( *doc )["counter"].rootRef<int64_t>().value().get(), 2 );
    }

    //=====================================================================
    // Mixed usage patterns
    //=====================================================================

    TEST( JsonDocumentAccess, MixedPattern_ReadWriteWorkflow )
    {
        auto doc = Document::fromString( R"({"users":[{"name":"Alice","score":85},{"name":"Bob","score":90}]})" );
        ASSERT_TRUE( doc.has_value() );

        // Read using optimal pattern
        auto& users = ( *doc )["users"].rootRef<Array>().value().get();
        EXPECT_EQ( users.size(), 2 );

        // Modify first user's score
        auto scoreRef = doc->getRef<int64_t>( "users[0].score" );
        ASSERT_TRUE( scoreRef.has_value() );
        scoreRef->get() = 95;

        // Verify with operator[]
        EXPECT_EQ( ( *doc )["users"][0]["score"].rootRef<int64_t>().value().get(), 95 );
    }
} // namespace nfx::json::test
