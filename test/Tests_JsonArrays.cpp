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
 * @file Tests_JsonArrays.cpp
 * @brief Unit tests for JSON Array class functionality
 * @details Tests covering array construction, element access, modification, insertion,
 *          type-safe operations, nested arrays/objects, and auto-detection features
 */

#include <gtest/gtest.h>

#include <nfx/Json.h>

namespace nfx::json::test
{
    //=====================================================================
    // Array construction and basic operations
    //=====================================================================

    class JSONArrayTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            // Create test document with various array structures
            std::string jsonStr = R"({
                "numbers": [1, 2, 3, 42, 100],
                "strings": ["hello", "world", "test"],
                "booleans": [true, false, true],
                "doubles": [3.14, 2.71, 1.41],
                "mixed": [1, "hello", true, 3.14],
                "nested_arrays": [[1, 2], [3, 4], ["a", "b"]],
                "nested_objects": [
                    {"name": "Alice", "age": 30},
                    {"name": "Bob", "age": 25}
                ],
                "empty_array": [],
                "single_char": ["a", "b", "X"]
            })";

            auto doc = Document::fromString( jsonStr );
            ASSERT_TRUE( doc.has_value() ) << "Failed to parse test JSON document";
            testDoc = doc.value();
        }

        Document testDoc;
    };

    //----------------------------------------------
    // Array construction
    //----------------------------------------------

    TEST_F( JSONArrayTest, DefaultConstructor )
    {
        Array emptyArray;
        EXPECT_EQ( emptyArray.size(), 0 );
    }

    TEST_F( JSONArrayTest, GetArrayFromDocument )
    {
        auto numbersArray = testDoc.get<Array>( "numbers" );
        ASSERT_TRUE( numbersArray.has_value() );
        EXPECT_EQ( numbersArray.value().size(), 5 );

        auto emptyArray = testDoc.get<Array>( "empty_array" );
        ASSERT_TRUE( emptyArray.has_value() );
        EXPECT_EQ( emptyArray.value().size(), 0 );
    }

    //----------------------------------------------
    // Element access (get<T>)
    //----------------------------------------------

    TEST_F( JSONArrayTest, GetStringElements )
    {
        auto stringsArrayOpt = testDoc.get<Array>( "strings" );
        ASSERT_TRUE( stringsArrayOpt.has_value() ) << "Failed to get 'strings' array from test document";
        auto& stringsArray = stringsArrayOpt.value();

        EXPECT_EQ( stringsArray[0].get<std::string>( "" ).value_or( "" ), "hello" );
        EXPECT_EQ( stringsArray[1].get<std::string>( "" ).value_or( "" ), "world" );
        EXPECT_EQ( stringsArray[2].get<std::string>( "" ).value_or( "" ), "test" );

        // Out of bounds - use Document API to test bounds checking
        EXPECT_FALSE( testDoc.get<std::string>( "/strings/10" ).has_value() );
    }

    TEST_F( JSONArrayTest, GetIntegerElements )
    {
        auto numbersArray = testDoc.get<Array>( "numbers" ).value();

        EXPECT_EQ( numbersArray[0].get<int64_t>( "" ).value_or( 0 ), 1 );
        EXPECT_EQ( numbersArray[1].get<int64_t>( "" ).value_or( 0 ), 2 );
        EXPECT_EQ( numbersArray[3].get<int64_t>( "" ).value_or( 0 ), 42 );

        // Test int32_t support
        EXPECT_EQ( numbersArray[0].get<int32_t>( "" ).value_or( 0 ), 1 );
        EXPECT_EQ( numbersArray[3].get<int32_t>( "" ).value_or( 0 ), 42 );
    }

    TEST_F( JSONArrayTest, GetDoubleElements )
    {
        auto doublesArray = testDoc.get<Array>( "doubles" ).value();

        EXPECT_DOUBLE_EQ( doublesArray[0].get<double>( "" ).value_or( 0.0 ), 3.14 );
        EXPECT_DOUBLE_EQ( doublesArray[1].get<double>( "" ).value_or( 0.0 ), 2.71 );
        EXPECT_DOUBLE_EQ( doublesArray[2].get<double>( "" ).value_or( 0.0 ), 1.41 );
    }

    TEST_F( JSONArrayTest, GetBooleanElements )
    {
        auto boolsArray = testDoc.get<Array>( "booleans" ).value();

        EXPECT_EQ( boolsArray[0].get<bool>( "" ).value_or( false ), true );
        EXPECT_EQ( boolsArray[1].get<bool>( "" ).value_or( true ), false );
        EXPECT_EQ( boolsArray[2].get<bool>( "" ).value_or( false ), true );
    }

    TEST_F( JSONArrayTest, GetCharacterElements )
    {
        auto charsArray = testDoc.get<Array>( "single_char" ).value();

        EXPECT_EQ( charsArray[0].get<char>( "" ).value_or( '\0' ), 'a' );
        EXPECT_EQ( charsArray[1].get<char>( "" ).value_or( '\0' ), 'b' );
        EXPECT_EQ( charsArray[2].get<char>( "" ).value_or( '\0' ), 'X' );
    }

    TEST_F( JSONArrayTest, GetDocumentElements )
    {
        auto mixedArray = testDoc.get<Array>( "mixed" ).value();

        // Any element can be retrieved as Document
        auto doc0 = mixedArray[0].get<Document>( "" );
        auto doc1 = mixedArray[1].get<Document>( "" );
        auto doc2 = mixedArray[2].get<Document>( "" );
        auto doc3 = mixedArray[3].get<Document>( "" );

        ASSERT_TRUE( doc0.has_value() );
        ASSERT_TRUE( doc1.has_value() );
        ASSERT_TRUE( doc2.has_value() );
        ASSERT_TRUE( doc3.has_value() );
    }

    TEST_F( JSONArrayTest, GetNestedArrayElements )
    {
        auto nestedArrays = testDoc.get<Array>( "nested_arrays" ).value();

        // Get nested array
        auto subArray0 = nestedArrays[0].get<Array>( "" );
        ASSERT_TRUE( subArray0.has_value() );
        EXPECT_EQ( subArray0.value().size(), 2 );
        EXPECT_EQ( subArray0.value()[0].get<int64_t>( "" ).value_or( 0 ), 1 );
        EXPECT_EQ( subArray0.value()[1].get<int64_t>( "" ).value_or( 0 ), 2 );

        auto subArray2 = nestedArrays[2].get<Array>( "" );
        ASSERT_TRUE( subArray2.has_value() );
        EXPECT_EQ( subArray2.value()[0].get<std::string>( "" ).value_or( "" ), "a" );
        EXPECT_EQ( subArray2.value()[1].get<std::string>( "" ).value_or( "" ), "b" );
    }

    TEST_F( JSONArrayTest, GetNestedObjectElements )
    {
        auto nestedObjects = testDoc.get<Array>( "nested_objects" ).value();

        // Get nested object
        EXPECT_EQ( nestedObjects[0].get<std::string>( "name" ).value_or( "" ), "Alice" );
        EXPECT_EQ( nestedObjects[0].get<int64_t>( "age" ).value_or( 0 ), 30 );

        EXPECT_EQ( nestedObjects[1].get<std::string>( "name" ).value_or( "" ), "Bob" );
        EXPECT_EQ( nestedObjects[1].get<int64_t>( "age" ).value_or( 0 ), 25 );
    }

    //----------------------------------------------
    // Type mismatch
    //----------------------------------------------

    TEST_F( JSONArrayTest, TypeMismatchReturnsNullopt )
    {
        auto stringsArray = testDoc.get<Array>( "strings" ).value();

        // Try to get string as number
        EXPECT_FALSE( stringsArray[0].get<int64_t>( "" ).has_value() );
        EXPECT_FALSE( stringsArray[0].get<double>( "" ).has_value() );
        EXPECT_FALSE( stringsArray[0].get<bool>( "" ).has_value() );

        auto numbersArray = testDoc.get<Array>( "numbers" ).value();

        // Try to get number as string
        EXPECT_FALSE( numbersArray[0].get<std::string>( "" ).has_value() );
    }

    //----------------------------------------------
    // Array element modification (set<T>)
    //----------------------------------------------

    TEST_F( JSONArrayTest, SetStringElements )
    {
        auto stringsArray = testDoc.get<Array>( "strings" ).value();

        stringsArray[1].set<std::string>( "", "modified" );
        EXPECT_EQ( stringsArray[1].get<std::string>( "" ).value_or( "" ), "modified" );
    }

    TEST_F( JSONArrayTest, SetIntegerElements )
    {
        auto numbersArray = testDoc.get<Array>( "numbers" ).value();

        numbersArray[0].set<int64_t>( "", 999 );
        EXPECT_EQ( numbersArray[0].get<int64_t>( "" ).value_or( 0 ), 999 );

        // Test int32_t
        numbersArray[1].set<int32_t>( "", 888 );
        EXPECT_EQ( numbersArray[1].get<int32_t>( "" ).value_or( 0 ), 888 );
    }

    TEST_F( JSONArrayTest, SetDoubleElements )
    {
        auto doublesArray = testDoc.get<Array>( "doubles" ).value();

        doublesArray[0].set<double>( "", 9.99 );
        EXPECT_DOUBLE_EQ( doublesArray[0].get<double>( "" ).value_or( 0.0 ), 9.99 );
    }

    TEST_F( JSONArrayTest, SetBooleanElements )
    {
        auto boolsArray = testDoc.get<Array>( "booleans" ).value();

        boolsArray[0].set<bool>( "", false );
        EXPECT_EQ( boolsArray[0].get<bool>( "" ).value_or( true ), false );
    }

    TEST_F( JSONArrayTest, SetCharacterElements )
    {
        auto charsArray = testDoc.get<Array>( "single_char" ).value();

        charsArray[0].set<char>( "", 'Z' );
        EXPECT_EQ( charsArray[0].get<char>( "" ).value_or( '\0' ), 'Z' );
    }

    TEST_F( JSONArrayTest, SetDocumentElements )
    {
        auto mixedArray = testDoc.get<Array>( "mixed" ).value();

        Document newDoc;

        newDoc.set<std::string>( "test", "value" );

        mixedArray[0].set<Document>( "", std::move( newDoc ) );
        auto retrievedDoc = mixedArray[0].get<Document>( "" );
        ASSERT_TRUE( retrievedDoc.has_value() );
        EXPECT_EQ( retrievedDoc.value().get<std::string>( "test" ).value_or( "" ), "value" );
    }

    TEST_F( JSONArrayTest, SetObjectElements )
    {
        auto nestedObjects = testDoc.get<Array>( "nested_objects" ).value();

        // Create new object and set it
        Document objDoc;
        objDoc.set<std::string>( "name", "Charlie" );
        objDoc.set<int64_t>( "age", 35 );

        // Set the object in the array using JSON Pointer path on the document
        testDoc.set<Document>( "/nested_objects/0", objDoc );

        // Verify using Document API - much cleaner!
        EXPECT_EQ( testDoc.get<std::string>( "/nested_objects/0/name" ).value_or( "" ), "Charlie" );
        EXPECT_EQ( testDoc.get<int64_t>( "/nested_objects/0/age" ).value_or( 0 ), 35 );
    }

    TEST_F( JSONArrayTest, SetArrayElements )
    {
        // Create new array and set it
        Document arrDoc;
        arrDoc.set<Array>( "" );

        arrDoc.set<int64_t>( "/0", 100 );
        arrDoc.set<int64_t>( "/1", 200 );

        // Set the new array in nested_arrays[0] using path
        testDoc.set<Document>( "/nested_arrays/0", arrDoc );

        auto retrievedArr = testDoc.get<Array>( "/nested_arrays/0" );
        ASSERT_TRUE( retrievedArr.has_value() );
        EXPECT_EQ( retrievedArr.value()[0].get<int64_t>( "" ).value_or( 0 ), 100 );
        EXPECT_EQ( retrievedArr.value()[1].get<int64_t>( "" ).value_or( 0 ), 200 );
    }

    //----------------------------------------------
    // Array element addition (append<T>)
    //----------------------------------------------

    TEST_F( JSONArrayTest, AddStringElements )
    {
        auto stringsArray = testDoc.get<Array>( "strings" ).value();
        size_t originalSize = stringsArray.size();

        // Add element using JSON Pointer on the document
        char path[64];
        std::snprintf( path, sizeof( path ), "/strings/%zu", originalSize );
        testDoc.set<std::string>( path, "new_string" );

        // Get updated array
        stringsArray = testDoc.get<Array>( "strings" ).value();
        EXPECT_EQ( stringsArray.size(), originalSize + 1 );
        EXPECT_EQ( stringsArray[originalSize].get<std::string>( "" ).value_or( "" ), "new_string" );
    }

    TEST_F( JSONArrayTest, AddIntegerElements )
    {
        auto numbersArray = testDoc.get<Array>( "numbers" ).value();
        size_t originalSize = numbersArray.size();

        // Add element using JSON Pointer
        char path[64];
        std::snprintf( path, sizeof( path ), "/numbers/%zu", originalSize );
        testDoc.set<int64_t>( path, 777 );

        numbersArray = testDoc.get<Array>( "numbers" ).value();
        EXPECT_EQ( numbersArray.size(), originalSize + 1 );
        EXPECT_EQ( numbersArray[originalSize].get<int64_t>( "" ).value_or( 0 ), 777 );

        // Test int32_t
        std::snprintf( path, sizeof( path ), "/numbers/%zu", originalSize + 1 );
        testDoc.set<int32_t>( path, 555 );
        numbersArray = testDoc.get<Array>( "numbers" ).value();
        EXPECT_EQ( numbersArray[originalSize + 1].get<int32_t>( "" ).value_or( 0 ), 555 );
    }

    TEST_F( JSONArrayTest, AddDoubleElements )
    {
        auto doublesArray = testDoc.get<Array>( "doubles" ).value();
        size_t originalSize = doublesArray.size();

        char path[64];
        std::snprintf( path, sizeof( path ), "/doubles/%zu", originalSize );
        testDoc.set<double>( path, 7.77 );

        doublesArray = testDoc.get<Array>( "doubles" ).value();
        EXPECT_EQ( doublesArray.size(), originalSize + 1 );
        EXPECT_DOUBLE_EQ( doublesArray[originalSize].get<double>( "" ).value_or( 0.0 ), 7.77 );
    }

    TEST_F( JSONArrayTest, AddBooleanElements )
    {
        auto boolsArray = testDoc.get<Array>( "booleans" ).value();
        size_t originalSize = boolsArray.size();

        char path[64];
        std::snprintf( path, sizeof( path ), "/booleans/%zu", originalSize );
        testDoc.set<bool>( path, false );

        boolsArray = testDoc.get<Array>( "booleans" ).value();
        EXPECT_EQ( boolsArray.size(), originalSize + 1 );
        EXPECT_EQ( boolsArray[originalSize].get<bool>( "" ).value_or( true ), false );
    }

    TEST_F( JSONArrayTest, AddCharacterElements )
    {
        auto charsArray = testDoc.get<Array>( "single_char" ).value();
        size_t originalSize = charsArray.size();

        char path[64];
        std::snprintf( path, sizeof( path ), "/single_char/%zu", originalSize );
        testDoc.set<char>( path, 'Y' );

        charsArray = testDoc.get<Array>( "single_char" ).value();
        EXPECT_EQ( charsArray.size(), originalSize + 1 );
        EXPECT_EQ( charsArray[originalSize].get<char>( "" ).value_or( '\0' ), 'Y' );
    }

    TEST_F( JSONArrayTest, AddDocumentElements )
    {
        auto mixedArray = testDoc.get<Array>( "mixed" ).value();
        size_t originalSize = mixedArray.size();

        Document newDoc;
        newDoc.set<std::string>( "added", "document" );

        char path[64];
        std::snprintf( path, sizeof( path ), "/mixed/%zu", originalSize );
        testDoc.set<Document>( path, newDoc );

        mixedArray = testDoc.get<Array>( "mixed" ).value();
        EXPECT_EQ( mixedArray.size(), originalSize + 1 );
        EXPECT_EQ( mixedArray[originalSize].get<std::string>( "added" ).value_or( "" ), "document" );
    }

    TEST_F( JSONArrayTest, AddObjectElements )
    {
        auto nestedObjects = testDoc.get<Array>( "nested_objects" ).value();
        size_t originalSize = nestedObjects.size();

        Document objDoc;
        objDoc.set<std::string>( "name", "David" );
        objDoc.set<int64_t>( "age", 40 );

        char path[64];
        std::snprintf( path, sizeof( path ), "/nested_objects/%zu", originalSize );
        testDoc.set<Document>( path, objDoc );

        nestedObjects = testDoc.get<Array>( "nested_objects" ).value();
        EXPECT_EQ( nestedObjects.size(), originalSize + 1 );
        EXPECT_EQ( nestedObjects[originalSize].get<std::string>( "name" ).value_or( "" ), "David" );
    }

    TEST_F( JSONArrayTest, AddArrayElements )
    {
        auto nestedArrays = testDoc.get<Array>( "nested_arrays" ).value();
        size_t originalSize = nestedArrays.size();

        Document arrDoc;
        arrDoc.set<Array>( "" );
        arrDoc.set<std::string>( "/0", "added" );
        arrDoc.set<std::string>( "/1", "array" );

        char path[64];
        std::snprintf( path, sizeof( path ), "/nested_arrays/%zu", originalSize );
        testDoc.set<Document>( path, arrDoc );

        nestedArrays = testDoc.get<Array>( "nested_arrays" ).value();
        EXPECT_EQ( nestedArrays.size(), originalSize + 1 );
        EXPECT_EQ( nestedArrays[originalSize][0].get<std::string>( "" ).value_or( "" ), "added" );
        EXPECT_EQ( nestedArrays[originalSize][1].get<std::string>( "" ).value_or( "" ), "array" );
    }

    TEST_F( JSONArrayTest, InsertStringElements )
    {
        auto stringsArray = testDoc.get<Array>( "strings" ).value();
        size_t originalSize = stringsArray.size();
        std::string originalSecond = stringsArray[1].get<std::string>( "" ).value_or( "" );

        // Insert using std::vector::insert, then set back to document
        stringsArray.insert( stringsArray.begin() + 1, Document{ "inserted" } );
        testDoc.set<Array>( "strings", stringsArray );

        stringsArray = testDoc.get<Array>( "strings" ).value();
        EXPECT_EQ( stringsArray.size(), originalSize + 1 );
        EXPECT_EQ( stringsArray[1].get<std::string>( "" ).value_or( "" ), "inserted" );
        EXPECT_EQ( stringsArray[2].get<std::string>( "" ).value_or( "" ), originalSecond ); // Shifted right
    }

    TEST_F( JSONArrayTest, InsertIntegerElements )
    {
        auto numbersArray = testDoc.get<Array>( "numbers" ).value();
        size_t originalSize = numbersArray.size();

        // Insert at beginning
        Document num999;
        num999.set<int64_t>( "", 999 );
        numbersArray.insert( numbersArray.begin(), num999 );
        testDoc.set<Array>( "numbers", numbersArray );

        numbersArray = testDoc.get<Array>( "numbers" ).value();
        EXPECT_EQ( numbersArray.size(), originalSize + 1 );
        EXPECT_EQ( numbersArray[0].get<int64_t>( "" ).value_or( 0 ), 999 );
        EXPECT_EQ( numbersArray[1].get<int64_t>( "" ).value_or( 0 ), 1 ); // Original first element shifted
    }

    TEST_F( JSONArrayTest, InsertAtEnd )
    {
        auto numbersArray = testDoc.get<Array>( "numbers" ).value();
        size_t originalSize = numbersArray.size();

        // Insert at end (same as add)
        Document num888;
        num888.set<int64_t>( "", 888 );
        numbersArray.insert( numbersArray.end(), num888 );
        testDoc.set<Array>( "numbers", numbersArray );

        numbersArray = testDoc.get<Array>( "numbers" ).value();
        EXPECT_EQ( numbersArray.size(), originalSize + 1 );
        EXPECT_EQ( numbersArray[originalSize].get<int64_t>( "" ).value_or( 0 ), 888 );
    }

    TEST_F( JSONArrayTest, InsertObjectElements )
    {
        auto nestedObjects = testDoc.get<Array>( "nested_objects" ).value();
        size_t originalSize = nestedObjects.size();

        Document objDoc;
        objDoc.set<std::string>( "name", "Inserted" );
        objDoc.set<int64_t>( "age", 99 );

        nestedObjects.insert( nestedObjects.begin() + 1, objDoc );
        testDoc.set<Array>( "nested_objects", nestedObjects );

        nestedObjects = testDoc.get<Array>( "nested_objects" ).value();
        EXPECT_EQ( nestedObjects.size(), originalSize + 1 );
        EXPECT_EQ( nestedObjects[1].get<std::string>( "name" ).value_or( "" ), "Inserted" );
    }

    //----------------------------------------------
    // Array utility methods
    //----------------------------------------------

    TEST_F( JSONArrayTest, HasElement )
    {
        auto stringsArray = testDoc.get<Array>( "strings" ).value();

        // std::vector doesn't have contains(), check with index bounds or Document API
        EXPECT_TRUE( 0 < stringsArray.size() );
        EXPECT_TRUE( 1 < stringsArray.size() );
        EXPECT_TRUE( 2 < stringsArray.size() );
        EXPECT_FALSE( 10 < stringsArray.size() );

        // Test JSON Pointer syntax using Document API
        EXPECT_TRUE( testDoc.get<std::string>( "/strings/0" ).has_value() );
        EXPECT_TRUE( testDoc.get<std::string>( "/strings/1" ).has_value() );
        EXPECT_FALSE( testDoc.get<std::string>( "/strings/10" ).has_value() );
    }

    TEST_F( JSONArrayTest, Size )
    {
        EXPECT_EQ( testDoc.get<Array>( "numbers" ).value().size(), 5 );
        EXPECT_EQ( testDoc.get<Array>( "strings" ).value().size(), 3 );
        EXPECT_EQ( testDoc.get<Array>( "empty_array" ).value().size(), 0 );
    }

    TEST_F( JSONArrayTest, Clear )
    {
        auto stringsArray = testDoc.get<Array>( "strings" ).value();
        EXPECT_GT( stringsArray.size(), 0 );

        stringsArray.clear();
        EXPECT_EQ( stringsArray.size(), 0 );
    }

    TEST_F( JSONArrayTest, Remove )
    {
        auto stringsArray = testDoc.get<Array>( "strings" ).value();
        size_t originalSize = stringsArray.size();
        std::string originalLast = stringsArray[2].get<std::string>( "" ).value_or( "" );

        // Use std::vector::erase instead of remove
        bool removed = true;
        if( 1 < stringsArray.size() )
        {
            stringsArray.erase( stringsArray.begin() + 1 ); // Remove middle element
        }
        else
        {
            removed = false;
        }

        EXPECT_TRUE( removed );
        EXPECT_EQ( stringsArray.size(), originalSize - 1 );
        EXPECT_EQ( stringsArray[1].get<std::string>( "" ).value_or( "" ), originalLast ); // Last element shifted left

        // Try to remove out of bounds
        bool removedOOB = 100 < stringsArray.size();
        EXPECT_FALSE( removedOOB );
    }

    //----------------------------------------------
    // Perfect forwarding
    //----------------------------------------------

    TEST_F( JSONArrayTest, PerfectForwardingMove )
    {
        auto stringsArray = testDoc.get<Array>( "strings" ).value();

        std::string movableString = "move_me";
        stringsArray[0].set<std::string>( "", std::move( movableString ) );
        EXPECT_EQ( stringsArray[0].get<std::string>( "" ).value_or( "" ), "move_me" );

        // Test with add
        std::string anotherMovable = "add_move";
        Document addDoc;
        addDoc.set<std::string>( "", std::move( anotherMovable ) );
        stringsArray.push_back( addDoc );
        testDoc.set<Array>( "strings", stringsArray );

        stringsArray = testDoc.get<Array>( "strings" ).value();
        EXPECT_EQ( stringsArray[stringsArray.size() - 1].get<std::string>( "" ).value_or( "" ), "add_move" );

        // Test with insert
        std::string insertMovable = "insert_move";
        Document insertDoc;
        insertDoc.set<std::string>( "", std::move( insertMovable ) );
        stringsArray.insert( stringsArray.begin() + 1, insertDoc );
        testDoc.set<Array>( "strings", stringsArray );

        stringsArray = testDoc.get<Array>( "strings" ).value();
        EXPECT_EQ( stringsArray[1].get<std::string>( "" ).value_or( "" ), "insert_move" );
    }

    //----------------------------------------------
    // Auto-detection and path support
    //----------------------------------------------

    TEST_F( JSONArrayTest, AutoDetectionWithJsonPointer )
    {
        // Test that Array methods work with both dot notation and JSON Pointer paths internally
        auto nestedArrays = testDoc.get<Array>( "nested_arrays" ).value();

        // The auto-detection should work in nested path construction
        auto subArray = nestedArrays[0].get<Array>( "" );
        ASSERT_TRUE( subArray.has_value() );

        // Verify the sub-array has correct elements
        EXPECT_EQ( subArray.value()[0].get<int64_t>( "" ).value_or( 0 ), 1 );
        EXPECT_EQ( subArray.value()[1].get<int64_t>( "" ).value_or( 0 ), 2 );
    }

    //----------------------------------------------
    // Edge cases and error Handling
    //----------------------------------------------

    TEST_F( JSONArrayTest, OutOfBoundsAccess )
    {
        // Test using Document API which handles bounds checking properly
        EXPECT_FALSE( testDoc.get<std::string>( "/strings/100" ).has_value() );
        EXPECT_FALSE( testDoc.get<int64_t>( "/strings/100" ).has_value() );
        EXPECT_FALSE( testDoc.get<Document>( "/strings/100" ).has_value() );
    }

    TEST_F( JSONArrayTest, SetBeyondBounds )
    {
        auto emptyArray = testDoc.get<Array>( "empty_array" ).value();
        EXPECT_EQ( emptyArray.size(), 0 );

        // std::vector doesn't auto-expand on operator[], need to resize or use Document API
        // Use Document API with JSON Pointer
        testDoc.set<std::string>( "/empty_array/5", "expanded" );

        emptyArray = testDoc.get<Array>( "empty_array" ).value();
        EXPECT_GE( emptyArray.size(), 6 ); // Array should have at least 6 elements now
        EXPECT_EQ( emptyArray[5].get<std::string>( "" ).value_or( "" ), "expanded" );
    }

    TEST_F( JSONArrayTest, InsertBeyondBounds )
    {
        auto smallArray = testDoc.get<Array>( "strings" ).value();
        size_t originalSize = smallArray.size();

        // std::vector::insert with iterator beyond end is undefined, just append
        Document appendDoc;
        appendDoc.set<std::string>( "", "appended" );
        smallArray.push_back( appendDoc );
        testDoc.set<Array>( "strings", smallArray );

        smallArray = testDoc.get<Array>( "strings" ).value();
        EXPECT_EQ( smallArray.size(), originalSize + 1 );
        EXPECT_EQ( smallArray[originalSize].get<std::string>( "" ).value_or( "" ), "appended" );
    }

    //----------------------------------------------
    // Complex nested operations
    //----------------------------------------------

    TEST_F( JSONArrayTest, DeepNestedOperations )
    {
        // Create complex nested structure using Document API
        Document complexDoc;

        // Use JSON Pointer to create deep structures
        complexDoc.set<std::string>( "/level1/0/0/deep_value", "found_it" );

        // Verify deep access works
        auto retrieved = complexDoc.get<std::string>( "/level1/0/0/deep_value" );
        EXPECT_TRUE( retrieved.has_value() );
        EXPECT_EQ( retrieved.value_or( "" ), "found_it" );
    }

    TEST_F( JSONArrayTest, MixedTypeOperations )
    {
        auto mixedArray = testDoc.get<Array>( "mixed" ).value();

        // Verify we can handle mixed types correctly
        EXPECT_TRUE( mixedArray[0].get<int64_t>( "" ).has_value() );     // number
        EXPECT_TRUE( mixedArray[1].get<std::string>( "" ).has_value() ); // string
        EXPECT_TRUE( mixedArray[2].get<bool>( "" ).has_value() );        // boolean
        EXPECT_TRUE( mixedArray[3].get<double>( "" ).has_value() );      // double

        // Verify type mismatches return nullopt
        EXPECT_FALSE( mixedArray[0].get<std::string>( "" ).has_value() ); // number as string
        EXPECT_FALSE( mixedArray[1].get<int64_t>( "" ).has_value() );     // string as number
    }

    //----------------------------------------------
    // Nested access features
    //----------------------------------------------

    TEST_F( JSONArrayTest, NestedPathAccess )
    {
        // Test nested get with JSON Pointer through Document
        auto name1 = testDoc.get<std::string>( "/nested_objects/0/name" );
        ASSERT_TRUE( name1.has_value() );
        EXPECT_EQ( name1.value(), "Alice" );

        auto age1 = testDoc.get<int64_t>( "/nested_objects/0/age" );
        ASSERT_TRUE( age1.has_value() );
        EXPECT_EQ( age1.value(), 30 );

        auto name2 = testDoc.get<std::string>( "/nested_objects/1/name" );
        ASSERT_TRUE( name2.has_value() );
        EXPECT_EQ( name2.value(), "Bob" );

        auto age2 = testDoc.get<int64_t>( "/nested_objects/1/age" );
        ASSERT_TRUE( age2.has_value() );
        EXPECT_EQ( age2.value(), 25 );
    }

    TEST_F( JSONArrayTest, NestedPathAccessWithJsonPointer )
    {
        // Already using JSON Pointer syntax - same as above test
        auto name1 = testDoc.get<std::string>( "/nested_objects/0/name" );
        ASSERT_TRUE( name1.has_value() );
        EXPECT_EQ( name1.value(), "Alice" );

        auto age2 = testDoc.get<int64_t>( "/nested_objects/1/age" );
        ASSERT_TRUE( age2.has_value() );
        EXPECT_EQ( age2.value(), 25 );
    }

    TEST_F( JSONArrayTest, NestedPathAccessInvalidPaths )
    {
        // Test invalid index
        auto invalid1 = testDoc.get<std::string>( "/nested_objects/5/name" );
        EXPECT_FALSE( invalid1.has_value() );

        // Test invalid field
        auto invalid2 = testDoc.get<std::string>( "/nested_objects/0/nonexistent" );
        EXPECT_FALSE( invalid2.has_value() );

        // Test empty path
        auto invalid3 = testDoc.get<std::string>( "" );
        EXPECT_FALSE( invalid3.has_value() );

        // Test type mismatch
        auto invalid4 = testDoc.get<int64_t>( "/nested_objects/0/name" );
        EXPECT_FALSE( invalid4.has_value() );
    }

    TEST_F( JSONArrayTest, NestedPathModification )
    {
        // Test nested set with JSON Pointer through Document
        testDoc.set<std::string>( "/nested_objects/0/name", "Modified Alice" );
        auto modifiedName = testDoc.get<std::string>( "/nested_objects/0/name" );
        ASSERT_TRUE( modifiedName.has_value() );
        EXPECT_EQ( modifiedName.value(), "Modified Alice" );

        // Test modifying age
        testDoc.set<int64_t>( "/nested_objects/1/age", 99 );
        auto modifiedAge = testDoc.get<int64_t>( "/nested_objects/1/age" );
        ASSERT_TRUE( modifiedAge.has_value() );
        EXPECT_EQ( modifiedAge.value(), 99 );
    }

    TEST_F( JSONArrayTest, NestedPathModificationWithJsonPointer )
    {
        // Test with JSON Pointer syntax
        testDoc.set<std::string>( "/nested_objects/0/name", "JSON Pointer Alice" );
        auto modifiedName = testDoc.get<std::string>( "/nested_objects/0/name" );
        ASSERT_TRUE( modifiedName.has_value() );
        EXPECT_EQ( modifiedName.value(), "JSON Pointer Alice" );
    }

    TEST_F( JSONArrayTest, NestedPathCreation )
    {
        testDoc.set<std::string>( "/nested_objects/0/address/city", "New York" );
        testDoc.set<std::string>( "/nested_objects/0/address/country", "USA" );

        // Verify the nested fields were created
        auto city = testDoc.get<std::string>( "/nested_objects/0/address/city" );
        ASSERT_TRUE( city.has_value() );
        EXPECT_EQ( city.value(), "New York" );

        auto country = testDoc.get<std::string>( "/nested_objects/0/address/country" );
        ASSERT_TRUE( country.has_value() );
        EXPECT_EQ( country.value(), "USA" );
    }

    TEST_F( JSONArrayTest, NestedArrayAccess )
    {
        // Test accessing elements within nested arrays using Document API
        auto firstElement = testDoc.get<int64_t>( "/nested_arrays/0/0" );
        ASSERT_TRUE( firstElement.has_value() );
        EXPECT_EQ( firstElement.value(), 1 );

        auto secondElement = testDoc.get<int64_t>( "/nested_arrays/0/1" );
        ASSERT_TRUE( secondElement.has_value() );
        EXPECT_EQ( secondElement.value(), 2 );

        // Test string elements in nested array
        auto stringElement = testDoc.get<std::string>( "/nested_arrays/2/0" );
        ASSERT_TRUE( stringElement.has_value() );
        EXPECT_EQ( stringElement.value(), "a" );
    }

    TEST_F( JSONArrayTest, NestedArrayModification )
    {
        // Test modifying elements within nested arrays using Document API
        testDoc.set<int64_t>( "/nested_arrays/0/0", 999 );
        auto modifiedElement = testDoc.get<int64_t>( "/nested_arrays/0/0" );
        ASSERT_TRUE( modifiedElement.has_value() );
        EXPECT_EQ( modifiedElement.value(), 999 );

        // Test setting string in nested array
        testDoc.set<std::string>( "/nested_arrays/2/1", "modified" );
        auto modifiedString = testDoc.get<std::string>( "/nested_arrays/2/1" );
        ASSERT_TRUE( modifiedString.has_value() );
        EXPECT_EQ( modifiedString.value(), "modified" );
    }

    TEST_F( JSONArrayTest, DeepNestedPathAccess )
    {
        // Create a complex nested structure for testing
        Document complexDoc;
        complexDoc.set<Array>( "" );

        complexDoc.set<Object>( "/0" );
        complexDoc.set<Array>( "/0/level1" );
        complexDoc.set<Object>( "/0/level1/0" );
        complexDoc.set<std::string>( "/0/level1/0/deep_field", "deep_value" );

        // Test deep nested access using Document API
        auto deepValue = complexDoc.get<std::string>( "/0/level1/0/deep_field" );
        ASSERT_TRUE( deepValue.has_value() );
        EXPECT_EQ( deepValue.value(), "deep_value" );

        // Test deep nested modification
        complexDoc.set<std::string>( "/0/level1/0/deep_field", "modified_deep_value" );
        auto modifiedDeepValue = complexDoc.get<std::string>( "/0/level1/0/deep_field" );
        ASSERT_TRUE( modifiedDeepValue.has_value() );
        EXPECT_EQ( modifiedDeepValue.value(), "modified_deep_value" );
    }

    TEST_F( JSONArrayTest, NestedPathPerfectForwarding )
    {
        // Test perfect forwarding with move semantics using Document API
        std::string movableValue = "moved_value";
        testDoc.set<std::string>( "/nested_objects/0/moved_field", std::move( movableValue ) );

        auto retrievedValue = testDoc.get<std::string>( "/nested_objects/0/moved_field" );
        ASSERT_TRUE( retrievedValue.has_value() );
        EXPECT_EQ( retrievedValue.value(), "moved_value" );
    }

    //----------------------------------------------
    // Array serialization methods (toJsonString/toJsonBytes)
    //----------------------------------------------

    TEST_F( JSONArrayTest, ToJsonStringEmpty )
    {
        auto emptyArray = testDoc.get<Array>( "empty_array" );
        ASSERT_TRUE( emptyArray.has_value() );

        // Array (std::vector) doesn't have toString(), must create a Document
        Document arrayDoc( emptyArray.value() );
        std::string jsonStr = arrayDoc.toString();
        EXPECT_EQ( jsonStr, "[]" );
    }

    TEST_F( JSONArrayTest, ToJsonBytesEmpty )
    {
        auto emptyArray = testDoc.get<Array>( "empty_array" );
        ASSERT_TRUE( emptyArray.has_value() );

        // Array (std::vector) doesn't have toBytes(), must create a Document
        Document arrayDoc( emptyArray.value() );
        std::vector<uint8_t> jsonBytes = arrayDoc.toBytes();
        std::string jsonStr( jsonBytes.begin(), jsonBytes.end() );
        EXPECT_EQ( jsonStr, "[]" );
    }

    //----------------------------------------------
    // Array validation methods
    //----------------------------------------------

    TEST_F( JSONArrayTest, IsValidForValidArray )
    {
        auto numbersArray = testDoc.get<Array>( "numbers" );
        ASSERT_TRUE( numbersArray.has_value() );

        // Array (std::vector) doesn't have isValid(), check Document instead
        EXPECT_TRUE( testDoc.isValid() );
    }

    TEST_F( JSONArrayTest, IsValidForEmptyArray )
    {
        auto emptyArray = testDoc.get<Array>( "empty_array" );
        ASSERT_TRUE( emptyArray.has_value() );

        // Array (std::vector) doesn't have isValid(), check Document instead
        EXPECT_TRUE( testDoc.isValid() );
    }

    //----------------------------------------------
    // Nested object/array access via Array::get<Object/Array>(path)
    //----------------------------------------------

    TEST_F( JSONArrayTest, GetNestedObjectFromArrayByPath )
    {
        // Get the nested_objects array
        auto objArray = testDoc.get<Array>( "nested_objects" );
        ASSERT_TRUE( objArray.has_value() );

        // Access element 0 directly from array
        ASSERT_GT( objArray.value().size(), 0 );
        auto& firstElem = objArray.value()[0];

        // Access fields from the nested object using Document API
        auto name = firstElem.get<std::string>( "name" );
        ASSERT_TRUE( name.has_value() );
        EXPECT_EQ( name.value(), "Alice" );

        auto age = firstElem.get<int32_t>( "age" );
        ASSERT_TRUE( age.has_value() );
        EXPECT_EQ( age.value(), 30 );

        // Get second object
        ASSERT_GT( objArray.value().size(), 1 );
        auto& secondElem = objArray.value()[1];

        auto name2 = secondElem.get<std::string>( "name" );
        ASSERT_TRUE( name2.has_value() );
        EXPECT_EQ( name2.value(), "Bob" );
    }

    TEST_F( JSONArrayTest, GetNestedArrayFromArrayByPath )
    {
        // Get the nested_arrays array
        auto arrayOfArrays = testDoc.get<Array>( "nested_arrays" );
        ASSERT_TRUE( arrayOfArrays.has_value() );

        // Access element 0 directly from array
        ASSERT_GT( arrayOfArrays.value().size(), 0 );
        auto firstNestedArray = arrayOfArrays.value()[0].get<Array>( "" );
        ASSERT_TRUE( firstNestedArray.has_value() );

        // Access elements from the nested array to ensure it's still valid
        EXPECT_EQ( firstNestedArray.value().size(), 2 );

        auto elem0 = firstNestedArray.value()[0].get<int32_t>( "" );
        ASSERT_TRUE( elem0.has_value() );
        EXPECT_EQ( elem0.value(), 1 );

        auto elem1 = firstNestedArray.value()[1].get<int32_t>( "" );
        ASSERT_TRUE( elem1.has_value() );
        EXPECT_EQ( elem1.value(), 2 );
    }

    TEST_F( JSONArrayTest, GetNestedObjectAndModifyParentArray )
    {
        // This test ensures the returned nested object remains valid
        // even if we continue working with the parent array
        auto objArray = testDoc.get<Array>( "nested_objects" );
        ASSERT_TRUE( objArray.has_value() );

        // Access first object via Document [index]
        ASSERT_GT( objArray.value().size(), 0 );
        auto& firstElem = objArray.value()[0];

        // Access parent array after getting nested object
        EXPECT_EQ( objArray.value().size(), 2 );

        // Get another element from parent
        ASSERT_GT( objArray.value().size(), 1 );
        auto& secondElem = objArray.value()[1];

        // First nested object should still be valid
        auto name = firstElem.get<std::string>( "name" );
        ASSERT_TRUE( name.has_value() );
        EXPECT_EQ( name.value(), "Alice" );

        // Second nested object should also be valid
        auto name2 = secondElem.get<std::string>( "name" );
        ASSERT_TRUE( name2.has_value() );
        EXPECT_EQ( name2.value(), "Bob" );
    }

    TEST_F( JSONArrayTest, GetNestedObjectWithJsonPointerPath )
    {
        // Use Document API with full JSON Pointer path
        auto name = testDoc.get<std::string>( "/nested_objects/0/name" );
        ASSERT_TRUE( name.has_value() );
        EXPECT_EQ( name.value(), "Alice" );
    }

    //=====================================================================
    // Array at() with bounds checking
    //=====================================================================

    TEST( JSONArrayAtTest, AtValidIndex )
    {
        Document arr = Document::array();
        arr.push_back( int64_t{ 10 } );
        arr.push_back( int64_t{ 20 } );
        arr.push_back( int64_t{ 30 } );

        EXPECT_EQ( arr.at( 0 ).get<int64_t>( "" ), 10 );
        EXPECT_EQ( arr.at( 1 ).get<int64_t>( "" ), 20 );
        EXPECT_EQ( arr.at( 2 ).get<int64_t>( "" ), 30 );
    }

    TEST( JSONArrayAtTest, AtValidIndexConst )
    {
        Document arr = Document::array();
        arr.push_back( Document{ std::string{ "hello" } } );
        const Document& constArr = arr;

        EXPECT_EQ( constArr.at( 0 ).get<std::string>( "" ), "hello" );
    }

    TEST( JSONArrayAtTest, AtOutOfBounds )
    {
        Document arr = Document::array();
        arr.push_back( int64_t{ 42 } );

        EXPECT_THROW( { (void)arr.at( 1 ); }, std::out_of_range );
        EXPECT_THROW( { (void)arr.at( 10 ); }, std::out_of_range );
    }

    TEST( JSONArrayAtTest, AtOutOfBoundsConst )
    {
        Document arr = Document::array();
        const Document& constArr = arr;

        EXPECT_THROW( { (void)constArr.at( 0 ); }, std::out_of_range );
    }

    TEST( JSONArrayAtTest, AtNotArray )
    {
        Document obj = Document::object();
        EXPECT_THROW( { (void)obj.at( 0 ); }, std::runtime_error );

        Document str{ std::string( "test" ) };
        EXPECT_THROW( { (void)str.at( 0 ); }, std::runtime_error );

        Document num{ int64_t{ 42 } };
        EXPECT_THROW( { (void)num.at( 0 ); }, std::runtime_error );
    }

    TEST( JSONArrayAtTest, AtModifyElement )
    {
        Document arr = Document::array();
        arr.push_back( int64_t{ 100 } );

        arr.at( 0 ) = int64_t{ 200 };
        EXPECT_EQ( arr[0].get<int64_t>( "" ), 200 );
    }
} // namespace nfx::json::test
