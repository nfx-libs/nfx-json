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
 * @file Tests_JsonObjectFieldIterator.cpp
 * @brief Unit tests for JSON Object iterator functionality
 * @details Tests covering object field iteration, key-value access, path validation,
 *          iterator state management, and range-for loop usage.
 */

#include <gtest/gtest.h>

#include <nfx/Json.h>

namespace nfx::json::test
{
    //=====================================================================
    // JSON Object iterator tests
    //=====================================================================

    //----------------------------------------------
    // Object iterator test fixture
    //----------------------------------------------

    class ObjectIteratorTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            // Create complex test JSON document with nested objects
            std::string jsonStr = R"({
                "user": {
                    "name": "Alice",
                    "age": 30,
                    "active": true,
                    "height": 1.65,
                    "spouse": null
                },
                "preferences": {
                    "theme": "dark",
                    "language": "en-US",
                    "notifications": true,
                    "fontSize": 12
                },
                "settings": {
                    "nested": {
                        "deep": "value"
                    }
                },
                "departments": {
                    "engineering": {"head": "Alice", "count": 10},
                    "marketing": {"head": "Bob", "count": 5},
                    "sales": {"head": "Charlie", "count": 8}
                },
                "empty": {},
                "mixed": {
                    "string": "test",
                    "number": 42,
                    "boolean": false,
                    "array": [1, 2, 3],
                    "object": {"key": "value"}
                }
            })";

            auto maybeDoc = Document::fromString( jsonStr );
            ASSERT_TRUE( maybeDoc.has_value() );
            document = std::move( maybeDoc.value() );
        }

        Document document;
    };

    //----------------------------------------------
    // Basic iteration
    //----------------------------------------------

    TEST_F( ObjectIteratorTest, RangeForLoop )
    {
        // Get user object
        auto userOpt = document.get<Object>( "user" );
        ASSERT_TRUE( userOpt.has_value() );
        Object user = userOpt.value();

        // Test range-for loop iteration
        std::vector<std::string> keys;
        for( const auto& [key, value] : user )
        {
            keys.push_back( key );
        }

        // Fields are in original insertion order: name, age, active, height, spouse
        EXPECT_EQ( keys.size(), 5 );
        EXPECT_EQ( keys[0], "name" );
        EXPECT_EQ( keys[1], "age" );
        EXPECT_EQ( keys[2], "active" );
        EXPECT_EQ( keys[3], "height" );
        EXPECT_EQ( keys[4], "spouse" );
    }

    TEST_F( ObjectIteratorTest, BeginEnditerators )
    {
        // Get preferences object
        auto prefsOpt = document.get<Object>( "preferences" );
        ASSERT_TRUE( prefsOpt.has_value() );
        Object prefs = prefsOpt.value();

        // Test begin/end
        auto it = prefs.begin();
        auto endIt = prefs.end();

        EXPECT_NE( it, endIt );

        // Count elements
        size_t count = 0;
        for( auto iter = prefs.begin(); iter != prefs.end(); ++iter )
        {
            ++count;
        }
        EXPECT_EQ( count, 4 ); // theme, language, notifications, fontSize
    }

    TEST_F( ObjectIteratorTest, iteratorValueAccess )
    {
        auto userOpt = document.get<Object>( "user" );
        ASSERT_TRUE( userOpt.has_value() );
        Object user = userOpt.value();

        // Iterate and verify values
        for( const auto& [key, valueDoc] : user )
        {
            if( key == "name" )
            {
                auto nameVal = valueDoc.get<std::string>( "" );
                ASSERT_TRUE( nameVal.has_value() );
                EXPECT_EQ( *nameVal, "Alice" );
            }
            else if( key == "age" )
            {
                auto ageVal = valueDoc.get<int64_t>( "" );
                ASSERT_TRUE( ageVal.has_value() );
                EXPECT_EQ( *ageVal, 30 );
            }
            else if( key == "active" )
            {
                auto activeVal = valueDoc.get<bool>( "" );
                ASSERT_TRUE( activeVal.has_value() );
                EXPECT_TRUE( *activeVal );
            }
            else if( key == "height" )
            {
                auto heightVal = valueDoc.get<double>( "" );
                ASSERT_TRUE( heightVal.has_value() );
                EXPECT_DOUBLE_EQ( *heightVal, 1.65 );
            }
            else if( key == "spouse" )
            {
                // Null value - typed accessors return nullopt
                EXPECT_FALSE( valueDoc.get<std::string>( "" ).has_value() );
            }
        }
    }

    //----------------------------------------------
    // Field access via iteration
    //----------------------------------------------

    TEST_F( ObjectIteratorTest, FieldKeyAndValueAccess )
    {
        auto userOpt = document.get<Object>( "user" );
        ASSERT_TRUE( userOpt.has_value() );
        Object user = userOpt.value();

        // Fields are in original insertion order: name, age, active, height, spouse
        auto it = user.begin();

        // First field: name
        auto [key1, val1] = *it;
        EXPECT_EQ( key1, "name" );
        auto nameValue = val1.get<std::string>( "" );
        ASSERT_TRUE( nameValue.has_value() );
        EXPECT_EQ( *nameValue, "Alice" );

        ++it;

        // Second field: age
        auto [key2, val2] = *it;
        EXPECT_EQ( key2, "age" );
        auto ageValue = val2.get<int64_t>( "" );
        ASSERT_TRUE( ageValue.has_value() );
        EXPECT_EQ( *ageValue, 30 );

        ++it;

        // Third field: active
        auto [key3, val3] = *it;
        EXPECT_EQ( key3, "active" );
        auto activeValue = val3.get<bool>( "" );
        ASSERT_TRUE( activeValue.has_value() );
        EXPECT_TRUE( *activeValue );

        ++it;

        // Fourth field: height
        auto [key4, val4] = *it;
        EXPECT_EQ( key4, "height" );
        auto heightValue = val4.get<double>( "" );
        ASSERT_TRUE( heightValue.has_value() );
        EXPECT_DOUBLE_EQ( *heightValue, 1.65 );

        ++it;

        // Fifth field: spouse (null)
        auto [key5, val5] = *it;
        EXPECT_EQ( key5, "spouse" );
        // Null values return nullopt for typed accessors
        EXPECT_FALSE( val5.get<std::string>( "" ).has_value() );
        EXPECT_FALSE( val5.get<int64_t>( "" ).has_value() );
    }

    TEST_F( ObjectIteratorTest, DocumentValueAccess )
    {
        auto mixedOpt = document.get<Object>( "mixed" );
        ASSERT_TRUE( mixedOpt.has_value() );
        Object mixed = mixedOpt.value();

        // Find and access object field
        for( const auto& [key, valueDoc] : mixed )
        {
            if( key == "object" )
            {
                // Get nested object's key field
                auto keyValue = valueDoc.get<std::string>( "key" );
                ASSERT_TRUE( keyValue.has_value() );
                EXPECT_EQ( *keyValue, "value" );
            }
        }
    }

    TEST_F( ObjectIteratorTest, WrongTypeAccess )
    {
        auto userOpt = document.get<Object>( "user" );
        ASSERT_TRUE( userOpt.has_value() );
        Object user = userOpt.value();

        // Find name field and try wrong type access
        for( const auto& [key, valueDoc] : user )
        {
            if( key == "name" )
            {
                // String field
                auto stringValue = valueDoc.get<std::string>( "" );
                EXPECT_TRUE( stringValue.has_value() );
                EXPECT_EQ( *stringValue, "Alice" );

                // Should return nullopt for wrong types
                auto intValue = valueDoc.get<int64_t>( "" );
                EXPECT_FALSE( intValue.has_value() );

                auto doubleValue = valueDoc.get<double>( "" );
                EXPECT_FALSE( doubleValue.has_value() );

                auto boolValue = valueDoc.get<bool>( "" );
                EXPECT_FALSE( boolValue.has_value() );
                break;
            }
        }
    }

    //----------------------------------------------
    // Traversal
    //----------------------------------------------

    TEST_F( ObjectIteratorTest, ForwardTraversal )
    {
        auto prefsOpt = document.get<Object>( "preferences" );
        ASSERT_TRUE( prefsOpt.has_value() );
        Object prefs = prefsOpt.value();

        std::vector<std::string> expectedKeys = { "theme", "language", "notifications", "fontSize" }; // insertion order
        std::vector<std::string> actualKeys;

        // Iterate through all fields
        for( const auto& [key, value] : prefs )
        {
            actualKeys.push_back( key );
        }

        EXPECT_EQ( actualKeys, expectedKeys );
    }

    TEST_F( ObjectIteratorTest, PostIncrementOperator )
    {
        auto prefsOpt = document.get<Object>( "preferences" );
        ASSERT_TRUE( prefsOpt.has_value() );
        Object prefs = prefsOpt.value();

        auto it = prefs.begin();

        // Post-increment returns copy of old iterator
        auto oldIt = it++;
        EXPECT_NE( it, oldIt );

        auto [oldKey, oldVal] = *oldIt;
        auto [newKey, newVal] = *it;
        EXPECT_EQ( oldKey, "theme" );    // first in insertion order
        EXPECT_EQ( newKey, "language" ); // second in insertion order
    }

    //----------------------------------------------
    // Edge cases
    //----------------------------------------------

    TEST_F( ObjectIteratorTest, EmptyObjectHandling )
    {
        auto emptyOpt = document.get<Object>( "empty" );
        ASSERT_TRUE( emptyOpt.has_value() );
        Object empty = emptyOpt.value();

        EXPECT_EQ( empty.size(), 0 );
        EXPECT_EQ( empty.begin(), empty.end() );

        // Range-for on empty should not iterate
        int count = 0;
        for( const auto& [key, value] : empty )
        {
            ++count;
        }
        EXPECT_EQ( count, 0 );
    }

    TEST_F( ObjectIteratorTest, MixedTypeFields )
    {
        auto mixedOpt = document.get<Object>( "mixed" );
        ASSERT_TRUE( mixedOpt.has_value() );
        Object mixed = mixedOpt.value();

        EXPECT_EQ( mixed.size(), 5 ); // array, boolean, number, object, string (sorted)

        std::map<std::string, bool> foundFields;

        for( const auto& [key, valueDoc] : mixed )
        {
            foundFields[key] = true;

            if( key == "string" )
            {
                auto strVal = valueDoc.get<std::string>( "" );
                EXPECT_TRUE( strVal.has_value() );
                EXPECT_EQ( *strVal, "test" );
            }
            else if( key == "number" )
            {
                auto intVal = valueDoc.get<int64_t>( "" );
                EXPECT_TRUE( intVal.has_value() );
                EXPECT_EQ( *intVal, 42 );
            }
            else if( key == "boolean" )
            {
                auto boolVal = valueDoc.get<bool>( "" );
                EXPECT_TRUE( boolVal.has_value() );
                EXPECT_FALSE( *boolVal );
            }
            else if( key == "array" )
            {
                EXPECT_TRUE( valueDoc.is<Array>( "" ) );
            }
            else if( key == "object" )
            {
                EXPECT_TRUE( valueDoc.is<Object>( "" ) );
            }
        }

        EXPECT_TRUE( foundFields["string"] );
        EXPECT_TRUE( foundFields["number"] );
        EXPECT_TRUE( foundFields["boolean"] );
        EXPECT_TRUE( foundFields["array"] );
        EXPECT_TRUE( foundFields["object"] );
    }

    //----------------------------------------------
    // Nested object iteration
    //----------------------------------------------

    TEST_F( ObjectIteratorTest, NestedObjectIteration )
    {
        // Test iterating over objects that contain objects
        auto deptsOpt = document.get<Object>( "departments" );
        ASSERT_TRUE( deptsOpt.has_value() );
        Object depts = deptsOpt.value();

        EXPECT_EQ( depts.size(), 3 );

        std::map<std::string, std::pair<std::string, int64_t>> result;

        for( const auto& [deptName, deptDoc] : depts )
        {
            auto deptObjOpt = deptDoc.get<Object>( "" );
            ASSERT_TRUE( deptObjOpt.has_value() );
            Object deptObj = deptObjOpt.value();

            std::string head;
            int64_t count = 0;

            for( const auto& [key, valueDoc] : deptObj )
            {
                if( key == "head" )
                {
                    auto headVal = valueDoc.get<std::string>( "" );
                    ASSERT_TRUE( headVal.has_value() );
                    head = *headVal;
                }
                else if( key == "count" )
                {
                    auto countVal = valueDoc.get<int64_t>( "" );
                    ASSERT_TRUE( countVal.has_value() );
                    count = *countVal;
                }
            }
            result[deptName] = { head, count };
        }

        EXPECT_EQ( result["engineering"].first, "Alice" );
        EXPECT_EQ( result["engineering"].second, 10 );
        EXPECT_EQ( result["marketing"].first, "Bob" );
        EXPECT_EQ( result["marketing"].second, 5 );
        EXPECT_EQ( result["sales"].first, "Charlie" );
        EXPECT_EQ( result["sales"].second, 8 );
    }

    //----------------------------------------------
    // JSON Pointer object access
    //----------------------------------------------

    TEST_F( ObjectIteratorTest, DeepPathObjectAccess )
    {
        // Test JSON Pointer navigation to nested object
        auto nestedOpt = document.get<Object>( "/settings/nested" );
        ASSERT_TRUE( nestedOpt.has_value() );
        Object nested = nestedOpt.value();

        EXPECT_EQ( nested.size(), 1 );

        for( const auto& [key, valueDoc] : nested )
        {
            EXPECT_EQ( key, "deep" );
            auto deepValue = valueDoc.get<std::string>( "" );
            ASSERT_TRUE( deepValue.has_value() );
            EXPECT_EQ( *deepValue, "value" );
        }
    }

    //----------------------------------------------
    // iterator comparison and state
    //----------------------------------------------

    TEST_F( ObjectIteratorTest, iteratorEquality )
    {
        auto userOpt = document.get<Object>( "user" );
        ASSERT_TRUE( userOpt.has_value() );
        Object user = userOpt.value();

        auto it1 = user.begin();
        auto it2 = user.begin();

        EXPECT_EQ( it1, it2 );

        ++it1;
        EXPECT_NE( it1, it2 );

        ++it2;
        EXPECT_EQ( it1, it2 );
    }

    TEST_F( ObjectIteratorTest, iteratorToEnd )
    {
        auto prefsOpt = document.get<Object>( "preferences" );
        ASSERT_TRUE( prefsOpt.has_value() );
        Object prefs = prefsOpt.value();

        auto it = prefs.begin();

        // Advance to end
        size_t count = 0;
        while( it != prefs.end() )
        {
            ++it;
            ++count;
        }

        EXPECT_EQ( count, 4 );
        EXPECT_EQ( it, prefs.end() );
    }
} // namespace nfx::json::test
