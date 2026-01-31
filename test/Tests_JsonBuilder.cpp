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
 * @file Tests_JsonBuilder.cpp
 * @brief Unit tests for JSON Builder class functionality
 */

#include <gtest/gtest.h>

#include <nfx/Json.h>

#include <deque>
#include <list>
#include <set>

namespace nfx::json::test
{
    //=====================================================================
    // Builder construction tests
    //=====================================================================

    class BuilderTest : public ::testing::Test
    {
    protected:
    };

    //----------------------------------------------
    // Basic construction
    //----------------------------------------------

    TEST_F( BuilderTest, DefaultConstructor )
    {
        Builder builder;
        std::string json = builder.toString();
        EXPECT_EQ( json, "" );
    }

    TEST_F( BuilderTest, ConstructorWithIndent )
    {
        Builder builder{ { 2 } }; // 2-space indentation
        builder.writeStartObject();
        builder.write( "key", "value" );
        builder.writeEndObject();

        std::string json = builder.toString();
        EXPECT_TRUE( json.find( "\n" ) != std::string::npos ); // Should have newlines for pretty print
    }

    TEST_F( BuilderTest, ConstructorWithBufferSize )
    {
        Builder builder{ { 0, 8192 } }; // Compact output, 8KB buffer

        builder.writeStartObject();
        builder.writeEndObject();

        std::string json = builder.toString();
        EXPECT_EQ( json, "{}" );
    }

    //----------------------------------------------
    // Empty structures
    //----------------------------------------------

    TEST_F( BuilderTest, EmptyObject )
    {
        Builder builder;
        builder.writeStartObject();
        builder.writeEndObject();

        std::string json = builder.toString();
        EXPECT_EQ( json, "{}" );
    }

    TEST_F( BuilderTest, EmptyArray )
    {
        Builder builder;
        builder.writeStartArray();
        builder.writeEndArray();

        std::string json = builder.toString();
        EXPECT_EQ( json, "[]" );
    }

    //----------------------------------------------
    // Simple objects
    //----------------------------------------------

    TEST_F( BuilderTest, SimpleObjectWithString )
    {
        Builder builder;
        builder.writeStartObject();
        builder.write( "name", "Alice" );
        builder.writeEndObject();

        std::string json = builder.toString();
        EXPECT_EQ( json, R"({"name":"Alice"})" );
    }

    TEST_F( BuilderTest, SimpleObjectWithInteger )
    {
        Builder builder;
        builder.writeStartObject();
        builder.write( "age", int64_t( 30 ) );
        builder.writeEndObject();

        std::string json = builder.toString();
        EXPECT_EQ( json, R"({"age":30})" );
    }

    TEST_F( BuilderTest, SimpleObjectWithUnsignedInteger )
    {
        Builder builder;
        builder.writeStartObject();
        builder.write( "count", uint64_t( 42 ) );
        builder.writeEndObject();

        std::string json = builder.toString();
        EXPECT_EQ( json, R"({"count":42})" );
    }

    TEST_F( BuilderTest, SimpleObjectWithDouble )
    {
        Builder builder;
        builder.writeStartObject();
        builder.write( "pi", 3.14159 );
        builder.writeEndObject();

        std::string json = builder.toString();

        // Parse to verify it's valid JSON
        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        auto pi = doc->get<double>( "pi" );
        ASSERT_TRUE( pi.has_value() );
        EXPECT_NEAR( *pi, 3.14159, 0.00001 );
    }

    TEST_F( BuilderTest, SimpleObjectWithBoolean )
    {
        Builder builder;
        builder.writeStartObject();
        builder.write( "active", true );
        builder.writeEndObject();

        std::string json = builder.toString();
        EXPECT_EQ( json, R"({"active":true})" );
    }

    TEST_F( BuilderTest, SimpleObjectWithNull )
    {
        Builder builder;
        builder.writeStartObject();
        builder.write( "spouse", nullptr );
        builder.writeEndObject();

        std::string json = builder.toString();
        EXPECT_EQ( json, R"({"spouse":null})" );
    }

    //----------------------------------------------
    // Objects with multiple properties
    //----------------------------------------------

    TEST_F( BuilderTest, ObjectWithMultipleProperties )
    {
        Builder builder;
        builder.writeStartObject();
        builder.write( "name", "Bob" );
        builder.write( "age", int64_t( 25 ) );
        builder.write( "active", false );
        builder.writeEndObject();

        std::string json = builder.toString();

        // Verify by parsing
        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->get<std::string>( "name" ), "Bob" );
        EXPECT_EQ( doc->get<int64_t>( "age" ), 25 );
        EXPECT_EQ( doc->get<bool>( "active" ), false );
    }

    //----------------------------------------------
    // Simple arrays
    //----------------------------------------------

    TEST_F( BuilderTest, SimpleArrayWithIntegers )
    {
        Builder builder;
        builder.writeStartArray();
        builder.write( 1 );
        builder.write( 2 );
        builder.write( 3 );
        builder.writeEndArray();

        std::string json = builder.toString();
        EXPECT_EQ( json, "[1,2,3]" );
    }

    TEST_F( BuilderTest, SimpleArrayWithStrings )
    {
        Builder builder;
        builder.writeStartArray();
        builder.write( "apple" );
        builder.write( "banana" );
        builder.write( "cherry" );
        builder.writeEndArray();

        std::string json = builder.toString();
        EXPECT_EQ( json, R"(["apple","banana","cherry"])" );
    }

    TEST_F( BuilderTest, SimpleArrayWithMixedTypes )
    {
        Builder builder;
        builder.writeStartArray();
        builder.write( "text" );
        builder.write( 42 );
        builder.write( true );
        builder.write( nullptr );
        builder.writeEndArray();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_TRUE( doc->isRoot<Array>() );
        EXPECT_EQ( doc->size(), 4 );
    }

    //----------------------------------------------
    // Nested objects
    //----------------------------------------------

    TEST_F( BuilderTest, NestedObject )
    {
        Builder builder;
        builder.writeStartObject();
        builder.writeKey( "user" );
        builder.writeStartObject();
        builder.write( "name", "Alice" );
        builder.write( "age", int64_t( 30 ) );
        builder.writeEndObject();
        builder.writeEndObject();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->get<std::string>( "/user/name" ), "Alice" );
        EXPECT_EQ( doc->get<int64_t>( "/user/age" ), 30 );
    }

    TEST_F( BuilderTest, DeeplyNestedObjects )
    {
        Builder builder;
        builder.writeStartObject();
        builder.writeKey( "level1" );
        builder.writeStartObject();
        builder.writeKey( "level2" );
        builder.writeStartObject();
        builder.write( "level3", "deep" );
        builder.writeEndObject();
        builder.writeEndObject();
        builder.writeEndObject();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->get<std::string>( "/level1/level2/level3" ), "deep" );
    }

    //----------------------------------------------
    // Nested arrays
    //----------------------------------------------

    TEST_F( BuilderTest, ArrayOfArrays )
    {
        Builder builder;
        builder.writeStartArray();
        builder.writeStartArray();
        builder.write( 1 );
        builder.write( 2 );
        builder.writeEndArray();
        builder.writeStartArray();
        builder.write( 3 );
        builder.write( 4 );
        builder.writeEndArray();
        builder.writeEndArray();

        std::string json = builder.toString();
        EXPECT_EQ( json, "[[1,2],[3,4]]" );
    }

    //----------------------------------------------
    // Mixed nesting
    //----------------------------------------------

    TEST_F( BuilderTest, ObjectContainingArray )
    {
        Builder builder;
        builder.writeStartObject();
        builder.writeKey( "items" );
        builder.writeStartArray();
        builder.write( 1 );
        builder.write( 2 );
        builder.write( 3 );
        builder.writeEndArray();
        builder.writeEndObject();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        auto items = doc->get<Array>( "items" );
        ASSERT_TRUE( items.has_value() );
        EXPECT_EQ( items->size(), 3 );
    }

    TEST_F( BuilderTest, ArrayContainingObjects )
    {
        Builder builder;
        builder.writeStartArray();
        builder.writeStartObject();
        builder.write( "id", 1 );
        builder.writeEndObject();
        builder.writeStartObject();
        builder.write( "id", 2 );
        builder.writeEndObject();
        builder.writeEndArray();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_TRUE( doc->isRoot<Array>() );
        EXPECT_EQ( doc->size(), 2 );
    }

    //----------------------------------------------
    // String escaping
    //----------------------------------------------

    TEST_F( BuilderTest, StringEscaping_Quotes )
    {
        Builder builder;
        builder.writeStartObject();
        builder.write( "text", R"(He said "Hello")" );
        builder.writeEndObject();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->get<std::string>( "text" ), R"(He said "Hello")" );
    }

    TEST_F( BuilderTest, StringEscaping_Backslash )
    {
        Builder builder;
        builder.writeStartObject();
        builder.write( "path", R"(C:\Windows\System32)" );
        builder.writeEndObject();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->get<std::string>( "path" ), R"(C:\Windows\System32)" );
    }

    TEST_F( BuilderTest, StringEscaping_ControlCharacters )
    {
        Builder builder;
        builder.writeStartObject();
        builder.write( "text", "Line1\nLine2\tTab\rReturn" );
        builder.writeEndObject();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->get<std::string>( "text" ), "Line1\nLine2\tTab\rReturn" );
    }

    TEST_F( BuilderTest, StringEscaping_Unicode )
    {
        Builder builder;
        builder.writeStartObject();
        builder.write( "emoji", "😀🎉🔥" );
        builder.writeEndObject();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->get<std::string>( "emoji" ), "😀🎉🔥" );
    }

    TEST_F( BuilderTest, StringEscaping_AllSpecialChars )
    {
        Builder builder;
        builder.writeStartObject();
        builder.write( "special", "\"\\\b\f\n\r\t" );
        builder.writeEndObject();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->get<std::string>( "special" ), "\"\\\b\f\n\r\t" );
    }

    //----------------------------------------------
    // Pretty printing (indentation)
    //----------------------------------------------

    TEST_F( BuilderTest, PrettyPrint_SimpleObject )
    {
        Builder builder{ { 2 } }; // 2-space indentation

        builder.writeStartObject().write( "name", "Alice" ).write( "age", 30 ).writeEndObject();

        std::string json = builder.toString();

        // Should have newlines and proper indentation
        EXPECT_TRUE( json.find( "\n" ) != std::string::npos );
        EXPECT_TRUE( json.find( "  " ) != std::string::npos ); // 2 spaces

        // Verify it parses correctly
        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->get<std::string>( "name" ), "Alice" );
    }

    TEST_F( BuilderTest, PrettyPrint_NestedObject )
    {
        Builder builder{ { 4 } }; // 4-space indentation
        builder.writeStartObject()
            .writeKey( "user" )
            .writeStartObject()
            .write( "name", "Bob" )
            .writeEndObject()
            .writeEndObject();

        std::string json = builder.toString();

        // Should have multiple levels of indentation
        EXPECT_TRUE( json.find( "\n" ) != std::string::npos );

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->get<std::string>( "/user/name" ), "Bob" );
    }

    TEST_F( BuilderTest, PrettyPrint_Array )
    {
        Builder builder{ { 2 } }; // 1-space indentation
        builder.writeStartArray().write( 1 ).write( 2 ).write( 3 ).writeEndArray();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->size(), 3 );
    }

    //----------------------------------------------
    // Edge cases - Large numbers
    //----------------------------------------------

    TEST_F( BuilderTest, LargeInteger )
    {
        Builder builder;
        builder.writeStartObject()
            .write( "bignum", 9223372036854775807 ) // INT64_MAX
            .writeEndObject();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->get<int64_t>( "bignum" ), 9223372036854775807LL );
    }

    TEST_F( BuilderTest, LargeUnsignedInteger )
    {
        Builder builder;
        builder.writeStartObject()
            .write( "bignum", 18446744073709551615ULL ) // UINT64_MAX
            .writeEndObject();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->get<uint64_t>( "bignum" ), 18446744073709551615ULL );
    }

    TEST_F( BuilderTest, NegativeInteger )
    {
        Builder builder;
        builder.writeStartObject()
            .write( "negative", -9223372036854775807 ) // Near INT64_MIN
            .writeEndObject();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->get<int64_t>( "negative" ), -9223372036854775807LL );
    }

    TEST_F( BuilderTest, ZeroValue )
    {
        Builder builder;
        builder.writeStartObject().write( "zero", 0 ).writeEndObject();

        std::string json = builder.toString();
        EXPECT_EQ( json, R"({"zero":0})" );
    }

    //----------------------------------------------
    // Edge cases - Floating point
    //----------------------------------------------

    TEST_F( BuilderTest, FloatingPoint_Zero )
    {
        Builder builder;
        builder.writeStartObject().write( "zero", 0.0 ).writeEndObject();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_DOUBLE_EQ( doc->get<double>( "zero" ).value_or( -1.0 ), 0.0 );
    }

    TEST_F( BuilderTest, FloatingPoint_Negative )
    {
        Builder builder;
        builder.writeStartObject().write( "negative", -123.456 ).writeEndObject();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_NEAR( doc->get<double>( "negative" ).value_or( 0.0 ), -123.456, 0.001 );
    }

    TEST_F( BuilderTest, FloatingPoint_Scientific )
    {
        Builder builder;
        builder.writeStartObject().write( "scientific", 1.23e10 ).writeEndObject();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_NEAR( doc->get<double>( "scientific" ).value_or( 0.0 ), 1.23e10, 1e5 );
    }

    //----------------------------------------------
    // Edge cases - Empty strings
    //----------------------------------------------

    TEST_F( BuilderTest, EmptyString )
    {
        Builder builder;
        builder.writeStartObject().write( "empty", "" ).writeEndObject();

        std::string json = builder.toString();
        EXPECT_EQ( json, R"({"empty":""})" );
    }

    TEST_F( BuilderTest, EmptyPropertyName )
    {
        Builder builder;
        builder.writeStartObject().write( "", "value" ).writeEndObject();

        std::string json = builder.toString();
        EXPECT_EQ( json, R"({"":"value"})" );
    }

    //----------------------------------------------
    // Round-trip compatibility with Document
    //----------------------------------------------

    TEST_F( BuilderTest, RoundTrip_ComplexDocument )
    {
        // Build with Builder
        Builder builder;
        builder.writeStartObject()
            .writeKey( "user" )
            .writeStartObject()
            .write( "name", "Alice Johnson" )
            .write( "age", 30 )
            .write( "active", true )
            .writeKey( "scores" )
            .writeStartArray()
            .write( 95 )
            .write( 87 )
            .write( 92 )
            .writeEndArray()
            .writeEndObject()
            .writeEndObject();

        std::string json = builder.toString();

        // Parse with Document
        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );

        // Verify all values
        EXPECT_EQ( doc->get<std::string>( "/user/name" ), "Alice Johnson" );
        EXPECT_EQ( doc->get<int64_t>( "/user/age" ), 30 );
        EXPECT_EQ( doc->get<bool>( "/user/active" ), true );

        auto scores = doc->get<Array>( "/user/scores" );
        ASSERT_TRUE( scores.has_value() );
        EXPECT_EQ( scores->size(), 3 );

        // Serialize with Document and compare
        std::string docJson = doc->toString();
        auto doc2 = Document::fromString( docJson );
        ASSERT_TRUE( doc2.has_value() );

        EXPECT_EQ( doc2->get<std::string>( "/user/name" ), "Alice Johnson" );
    }

    //----------------------------------------------
    // Performance - Large documents
    //----------------------------------------------

    TEST_F( BuilderTest, LargeArray )
    {
        Builder builder;
        builder.writeStartArray();
        for( int i = 0; i < 1000; ++i )
        {
            builder.write( i );
        }
        builder.writeEndArray();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->size(), 1000 );
    }

    TEST_F( BuilderTest, LargeObject )
    {
        Builder builder;
        builder.writeStartObject();
        for( int i = 0; i < 100; ++i )
        {
            builder.write( "key" + std::to_string( i ), "value" + std::to_string( i ) );
        }
        builder.writeEndObject();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->get<std::string>( "key0" ), "value0" );
        EXPECT_EQ( doc->get<std::string>( "key99" ), "value99" );
    }

    //----------------------------------------------
    // Error handling - Invalid sequences
    //----------------------------------------------

    TEST_F( BuilderTest, ErrorHandling_PropertyNameOutsideObject )
    {
        Builder builder;
        EXPECT_THROW( builder.writeKey( "invalid" ), std::runtime_error );
    }

    TEST_F( BuilderTest, ErrorHandling_ValueAfterPropertyNameWithoutObject )
    {
        Builder builder;
        builder.writeStartObject();
        builder.writeKey( "key" );
        builder.writeKey( "another" ); // Should throw - expecting value
        // Note: This depends on Builder's validation logic
    }

    TEST_F( BuilderTest, ErrorHandling_UnmatchedEndObject )
    {
        Builder builder;
        EXPECT_THROW( builder.writeEndObject(), std::runtime_error );
    }

    TEST_F( BuilderTest, ErrorHandling_UnmatchedEndArray )
    {
        Builder builder;
        EXPECT_THROW( builder.writeEndArray(), std::runtime_error );
    }

    TEST_F( BuilderTest, ErrorHandling_MismatchedObjectArray )
    {
        Builder builder;
        builder.writeStartObject();
        EXPECT_THROW( builder.writeEndArray(), std::runtime_error );
    }

    TEST_F( BuilderTest, ErrorHandling_MismatchedArrayObject )
    {
        Builder builder;
        builder.writeStartArray();
        EXPECT_THROW( builder.writeEndObject(), std::runtime_error );
    }

    // SFINAE tests: long long and unsigned long long literals without casts
    TEST_F( BuilderTest, LongLongLiteral_WithoutCast )
    {
        Builder builder;
        builder.writeStartObject()
            .write( "maxInt64", 9223372036854775807LL ) // Direct LL literal
            .writeEndObject();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->get<int64_t>( "maxInt64" ), 9223372036854775807LL );
    }

    TEST_F( BuilderTest, UnsignedLongLongLiteral_WithoutCast )
    {
        Builder builder;
        builder.writeStartObject();
        builder.write( "maxUInt64", 18446744073709551615ULL ); // Direct ULL literal
        builder.writeEndObject();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->get<uint64_t>( "maxUInt64" ), 18446744073709551615ULL );
    }

    TEST_F( BuilderTest, NegativeLongLongLiteral_WithoutCast )
    {
        Builder builder;
        builder.writeStartObject();
        builder.write( "minInt64", -9223372036854775807LL ); // Direct negative LL literal
        builder.writeEndObject();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->get<int64_t>( "minInt64" ), -9223372036854775807LL );
    }

    TEST_F( BuilderTest, MixedIntegerTypes_WithoutCasts )
    {
        Builder builder;
        builder.writeStartObject();
        builder.write( "int32", 42 );                          // int literal
        builder.write( "uint32", 100u );                       // unsigned int literal
        builder.write( "int64LL", 9223372036854775807LL );     // long long literal
        builder.write( "uint64ULL", 18446744073709551615ULL ); // unsigned long long literal
        builder.writeEndObject();

        std::string json = builder.toString();

        auto doc = Document::fromString( json );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->get<int>( "int32" ), 42 );
        EXPECT_EQ( doc->get<unsigned int>( "uint32" ), 100u );
        EXPECT_EQ( doc->get<int64_t>( "int64LL" ), 9223372036854775807LL );
        EXPECT_EQ( doc->get<uint64_t>( "uint64ULL" ), 18446744073709551615ULL );
    }

    //=====================================================================
    // Builder state management tests
    //=====================================================================

    TEST_F( BuilderTest, Reset_AllowsReuse )
    {
        Builder builder;

        // First document
        builder.writeStartObject();
        builder.write( "name", "Alice" );
        builder.writeEndObject();
        std::string json1 = builder.toString();
        EXPECT_EQ( json1, R"({"name":"Alice"})" );

        // Reset and create second document
        builder.reset();
        builder.writeStartObject();
        builder.write( "age", 30 );
        builder.writeEndObject();
        std::string json2 = builder.toString();
        EXPECT_EQ( json2, R"({"age":30})" );
    }

    TEST_F( BuilderTest, Reset_ClearsState )
    {
        Builder builder;

        // Build incomplete structure
        builder.writeStartObject();
        builder.write( "key", "value" );
        // Intentionally don't close object

        // Reset should clear everything
        builder.reset();
        EXPECT_TRUE( builder.isEmpty() );
        EXPECT_EQ( builder.size(), 0u );
    }

    TEST_F( BuilderTest, Size_ReturnsBufferSize )
    {
        Builder builder;
        EXPECT_EQ( builder.size(), 0u );

        builder.writeStartObject();
        EXPECT_GT( builder.size(), 0u ); // "{" has been written

        builder.write( "name", "John" );
        size_t sizeAfterFirstProperty = builder.size();
        EXPECT_GT( sizeAfterFirstProperty, 1u );

        builder.write( "age", 25 );
        EXPECT_GT( builder.size(), sizeAfterFirstProperty );

        builder.writeEndObject();
        size_t finalSize = builder.size();
        EXPECT_GT( finalSize, sizeAfterFirstProperty );

        std::string json = builder.toString();
        EXPECT_EQ( json.size(), finalSize );
    }

    TEST_F( BuilderTest, IsEmpty_InitialState )
    {
        Builder builder;
        EXPECT_TRUE( builder.isEmpty() );
        EXPECT_EQ( builder.size(), 0u );
    }

    TEST_F( BuilderTest, IsEmpty_AfterWriting )
    {
        Builder builder;
        builder.writeStartObject();
        EXPECT_FALSE( builder.isEmpty() );
        EXPECT_GT( builder.size(), 0u );

        builder.writeEndObject();
        EXPECT_FALSE( builder.isEmpty() );
    }

    TEST_F( BuilderTest, IsEmpty_AfterToString )
    {
        Builder builder;
        builder.writeStartObject();
        builder.writeEndObject();

        std::string json = builder.toString();
        EXPECT_FALSE( json.empty() );

        // toString() moves the buffer, so builder should be empty after
        EXPECT_TRUE( builder.isEmpty() );
        EXPECT_EQ( builder.size(), 0u );
    }

    TEST_F( BuilderTest, IsEmpty_AfterReset )
    {
        Builder builder;
        builder.writeStartObject();
        builder.write( "key", "value" );
        builder.writeEndObject();

        EXPECT_FALSE( builder.isEmpty() );

        builder.reset();
        EXPECT_TRUE( builder.isEmpty() );
        EXPECT_EQ( builder.size(), 0u );
    }

    TEST_F( BuilderTest, Reset_Chaining )
    {
        Builder builder;

        // reset() should return reference for chaining
        auto json = builder.reset().writeStartObject().write( "test", 123 ).writeEndObject().toString();

        EXPECT_EQ( json, R"({"test":123})" );
    }

    //=====================================================================
    // Buffer management tests
    //=====================================================================

    TEST_F( BuilderTest, Reserve_PreallocatesCapacity )
    {
        Builder builder;

        // Reserve should preallocate without changing size
        builder.reserve( 1024 );
        EXPECT_TRUE( builder.isEmpty() );
        EXPECT_EQ( builder.size(), 0u );

        // Should not reallocate during construction
        builder.writeStartObject();
        for( int i = 0; i < 100; ++i )
        {
            builder.write( "key" + std::to_string( i ), i );
        }
        builder.writeEndObject();

        std::string json = builder.toString();
        EXPECT_FALSE( json.empty() );
    }

    TEST_F( BuilderTest, Reserve_Chaining )
    {
        Builder builder;

        // reserve() should return reference for chaining
        auto json = builder.reserve( 512 ).writeStartObject().write( "test", true ).writeEndObject().toString();

        EXPECT_EQ( json, R"({"test":true})" );
    }

    //=====================================================================
    // Document integration tests
    //=====================================================================

    TEST_F( BuilderTest, WriteDocument_AsArrayElement )
    {
        Builder builder;

        // Create Documents
        Document doc1 = Document::object();
        doc1.set( "id", 1 );

        Document doc2 = Document::object();
        doc2.set( "id", 2 );

        // Write them in an array
        builder.writeStartArray();
        builder.write( doc1 );
        builder.write( doc2 );
        builder.writeEndArray();

        std::string json = builder.toString();

        // Parse and verify
        auto parsed = Document::fromString( json );
        ASSERT_TRUE( parsed.has_value() ) << "Failed to parse: " << json;
        EXPECT_EQ( parsed->type(), Type::Array );
        EXPECT_EQ( parsed->size(), 2u );
        EXPECT_EQ( parsed->get<int>( "0.id" ), 1 );
        EXPECT_EQ( parsed->get<int>( "1.id" ), 2 );
    }

    //=====================================================================
    // Additional utility methods tests
    //=====================================================================

    TEST_F( BuilderTest, ToStringView_NonDestructive )
    {
        Builder builder;
        builder.writeStartObject().write( "key", "value" ).writeEndObject();

        // Get view multiple times
        auto view1 = builder.toStringView();
        auto view2 = builder.toStringView();

        EXPECT_EQ( view1, view2 );
        EXPECT_EQ( view1, R"({"key":"value"})" );

        // Can still get string after
        std::string json = builder.toString();
        EXPECT_EQ( json, R"({"key":"value"})" );
    }

    TEST_F( BuilderTest, Capacity_ReturnsBufferCapacity )
    {
        Builder builder;
        builder.reserve( 1000 );

        EXPECT_GE( builder.capacity(), 1000 );
    }

    TEST_F( BuilderTest, Depth_TracksNesting )
    {
        Builder builder;

        EXPECT_EQ( builder.depth(), 0 );

        builder.writeStartObject();
        EXPECT_EQ( builder.depth(), 1 );

        builder.writeKey( "nested" );
        builder.writeStartArray();
        EXPECT_EQ( builder.depth(), 2 );

        builder.writeStartObject();
        EXPECT_EQ( builder.depth(), 3 );

        builder.writeEndObject();
        EXPECT_EQ( builder.depth(), 2 );

        builder.writeEndArray();
        EXPECT_EQ( builder.depth(), 1 );

        builder.writeEndObject();
        EXPECT_EQ( builder.depth(), 0 );
    }

    TEST_F( BuilderTest, IsValid_ChecksContextStack )
    {
        Builder builder;

        EXPECT_TRUE( builder.isValid() ); // Empty is valid

        builder.writeStartObject();
        EXPECT_FALSE( builder.isValid() ); // Unclosed object

        builder.writeEndObject();
        EXPECT_TRUE( builder.isValid() ); // Closed
    }

    TEST_F( BuilderTest, WriteArray_WithKey_IntVector )
    {
        Builder builder;
        std::vector<int> numbers = { 1, 2, 3, 4, 5 };

        builder.writeStartObject();
        builder.writeArray( "numbers", numbers );
        builder.writeEndObject();

        std::string json = builder.toString();
        EXPECT_EQ( json, R"({"numbers":[1,2,3,4,5]})" );
    }

    TEST_F( BuilderTest, WriteArray_WithKey_StringVector )
    {
        Builder builder;
        std::vector<std::string> words = { "hello", "world", "test" };

        builder.writeStartObject();
        builder.writeArray( "words", words );
        builder.writeEndObject();

        std::string json = builder.toString();
        EXPECT_EQ( json, R"({"words":["hello","world","test"]})" );
    }

    TEST_F( BuilderTest, WriteArray_Standalone )
    {
        Builder builder;
        std::vector<double> values = { 1.5, 2.5, 3.5 };

        builder.writeArray( values );

        std::string json = builder.toString();
        EXPECT_EQ( json, "[1.5,2.5,3.5]" );
    }

    TEST_F( BuilderTest, WriteArray_EmptyContainer )
    {
        Builder builder;
        std::vector<int> empty;

        builder.writeStartObject();
        builder.writeArray( "empty", empty );
        builder.writeEndObject();

        std::string json = builder.toString();
        EXPECT_EQ( json, R"({"empty":[]})" );
    }

    TEST_F( BuilderTest, WriteArray_NestedInArray )
    {
        Builder builder;
        std::vector<int> first = { 1, 2 };
        std::vector<int> second = { 3, 4 };

        builder.writeStartArray();
        builder.writeArray( first );
        builder.writeArray( second );
        builder.writeEndArray();

        std::string json = builder.toString();
        EXPECT_EQ( json, "[[1,2],[3,4]]" );
    }

    TEST_F( BuilderTest, WriteArray_DifferentContainerTypes )
    {
        Builder builder;

        // Test with std::list
        std::list<int> listData = { 1, 2, 3 };
        builder.writeStartObject();
        builder.writeArray( "list", listData );

        // Test with std::set (ordered)
        std::set<int> setData = { 3, 1, 2 }; // Will be ordered as 1,2,3
        builder.writeArray( "set", setData );

        // Test with std::deque
        std::deque<int> dequeData = { 4, 5, 6 };
        builder.writeArray( "deque", dequeData );

        builder.writeEndObject();

        std::string json = builder.toString();
        EXPECT_EQ( json, R"({"list":[1,2,3],"set":[1,2,3],"deque":[4,5,6]})" );
    }
} // namespace nfx::json::test
