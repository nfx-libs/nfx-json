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
 * @file Tests_JsonFileLoading.cpp
 * @brief Tests for JSON data loading functionality
 * @details Tests covering loading JSON documents from embedded testdata,
 *          validating data parsing and real-world JSON handling.
 */

#include <gtest/gtest.h>

#include <nfx/Json.h>

#include <testdata.h>

namespace nfx::json::test
{
    //=====================================================================
    // JSON Data Loading tests
    //=====================================================================

    //----------------------------------------------
    // Data Loading test fixture
    //----------------------------------------------

    class FileLoadingTest : public ::testing::Test
    {
    protected:
        std::optional<Document> loadJsonFile( const std::string& filename ) const
        {
            auto* resource = nfx::json::test::testdata::find( filename );
            if( !resource || resource->empty() )
            {
                return std::nullopt;
            }

            try
            {
                return Document::fromString( resource->str() );
            }
            catch( ... )
            {
                return std::nullopt;
            }
        }
    };

    //----------------------------------------------
    // Basic data loading
    //----------------------------------------------

    TEST_F( FileLoadingTest, LoadSmallObject )
    {
        auto docOpt = loadJsonFile( "small_object.json" );
        ASSERT_TRUE( docOpt.has_value() ) << "Failed to load small_object.json";

        Document& doc = docOpt.value();

        auto name = doc.get<std::string>( "name" );
        ASSERT_TRUE( name.has_value() );
        EXPECT_EQ( *name, "Alice Johnson" );

        auto age = doc.get<int64_t>( "age" );
        ASSERT_TRUE( age.has_value() );
        EXPECT_EQ( *age, 30 );

        auto active = doc.get<bool>( "active" );
        ASSERT_TRUE( active.has_value() );
        EXPECT_TRUE( *active );

        auto email = doc.get<std::string>( "email" );
        ASSERT_TRUE( email.has_value() );
        EXPECT_EQ( *email, "alice@example.com" );
    }

    TEST_F( FileLoadingTest, LoadMediumObject )
    {
        auto docOpt = loadJsonFile( "medium_object.json" );
        ASSERT_TRUE( docOpt.has_value() ) << "Failed to load medium_object.json";

        Document& doc = docOpt.value();

        // Test nested access
        auto firstName = doc.get<std::string>( "/user/profile/firstName" );
        ASSERT_TRUE( firstName.has_value() );
        EXPECT_EQ( *firstName, "Alice" );

        auto city = doc.get<std::string>( "/user/address/city" );
        ASSERT_TRUE( city.has_value() );
        EXPECT_EQ( *city, "New York" );

        auto theme = doc.get<std::string>( "/user/preferences/theme" );
        ASSERT_TRUE( theme.has_value() );
        EXPECT_EQ( *theme, "dark" );

        auto accountType = doc.get<std::string>( "/account/type" );
        ASSERT_TRUE( accountType.has_value() );
        EXPECT_EQ( *accountType, "premium" );
    }

    TEST_F( FileLoadingTest, LoadLargeObject )
    {
        auto docOpt = loadJsonFile( "large_object.json" );
        ASSERT_TRUE( docOpt.has_value() ) << "Failed to load large_object.json";

        Document& doc = docOpt.value();

        // Test company info
        auto companyName = doc.get<std::string>( "/company/name" );
        ASSERT_TRUE( companyName.has_value() );
        EXPECT_EQ( *companyName, "TechCorp International" );

        // Test nested array access
        auto depts = doc.get<Array>( "departments" );
        ASSERT_TRUE( depts.has_value() );
        EXPECT_GE( depts->size(), 1 );

        // Test financial data
        auto totalRevenue = doc.get<double>( "/financials/revenue/total" );
        ASSERT_TRUE( totalRevenue.has_value() );
        EXPECT_GT( *totalRevenue, 0 );
    }

    //----------------------------------------------
    // Array file loading
    //----------------------------------------------

    TEST_F( FileLoadingTest, LoadSmallArray )
    {
        auto docOpt = loadJsonFile( "small_array.json" );
        ASSERT_TRUE( docOpt.has_value() ) << "Failed to load small_array.json";

        Document& doc = docOpt.value();

        auto arr = doc.get<Array>( "" );
        ASSERT_TRUE( arr.has_value() );
        EXPECT_EQ( arr->size(), 10 );

        // Verify first and last elements
        auto first = ( *arr )[0].get<int64_t>( "" );
        ASSERT_TRUE( first.has_value() );
        EXPECT_EQ( *first, 1 );

        auto last = ( *arr )[9].get<int64_t>( "" );
        ASSERT_TRUE( last.has_value() );
        EXPECT_EQ( *last, 10 );
    }

    TEST_F( FileLoadingTest, LoadLargeArray )
    {
        auto docOpt = loadJsonFile( "large_array.json" );
        ASSERT_TRUE( docOpt.has_value() ) << "Failed to load large_array.json";

        Document& doc = docOpt.value();

        auto arr = doc.get<Array>( "" );
        ASSERT_TRUE( arr.has_value() );
        EXPECT_EQ( arr->size(), 50 );

        // Verify structure of first element
        auto it = arr->begin();
        Document firstItem = *it;

        auto id = firstItem.get<int64_t>( "id" );
        ASSERT_TRUE( id.has_value() );
        EXPECT_EQ( *id, 1 );

        auto name = firstItem.get<std::string>( "name" );
        ASSERT_TRUE( name.has_value() );
        EXPECT_EQ( *name, "Item 001" );
    }

    //----------------------------------------------
    // Special cases
    //----------------------------------------------

    TEST_F( FileLoadingTest, LoadDeeplyNested )
    {
        auto docOpt = loadJsonFile( "deeply_nested.json" );
        ASSERT_TRUE( docOpt.has_value() ) << "Failed to load deeply_nested.json";

        Document& doc = docOpt.value();

        // Access deeply nested value using JSON Pointer
        auto deepValue =
            doc.get<std::string>( "/level1/level2/level3/level4/level5/level6/level7/level8/level9/level10/value" );
        ASSERT_TRUE( deepValue.has_value() );
        EXPECT_EQ( *deepValue, "deeply nested value" );

        auto deepNumber =
            doc.get<int64_t>( "/level1/level2/level3/level4/level5/level6/level7/level8/level9/level10/number" );
        ASSERT_TRUE( deepNumber.has_value() );
        EXPECT_EQ( *deepNumber, 42 );
    }

    TEST_F( FileLoadingTest, LoadMixedTypes )
    {
        auto docOpt = loadJsonFile( "mixed_types.json" );
        ASSERT_TRUE( docOpt.has_value() ) << "Failed to load mixed_types.json";

        Document& doc = docOpt.value();

        // Test various types
        auto stringVal = doc.get<std::string>( "string" );
        ASSERT_TRUE( stringVal.has_value() );
        EXPECT_EQ( *stringVal, "Hello, World!" );

        auto intVal = doc.get<int64_t>( "integer" );
        ASSERT_TRUE( intVal.has_value() );
        EXPECT_EQ( *intVal, 42 );

        auto negVal = doc.get<int64_t>( "negative" );
        ASSERT_TRUE( negVal.has_value() );
        EXPECT_EQ( *negVal, -123 );

        auto floatVal = doc.get<double>( "float" );
        ASSERT_TRUE( floatVal.has_value() );
        EXPECT_NEAR( *floatVal, 3.14159265359, 0.0000001 );

        auto boolTrue = doc.get<bool>( "boolean_true" );
        ASSERT_TRUE( boolTrue.has_value() );
        EXPECT_TRUE( *boolTrue );

        auto boolFalse = doc.get<bool>( "boolean_false" );
        ASSERT_TRUE( boolFalse.has_value() );
        EXPECT_FALSE( *boolFalse );

        // Null value should return nullopt for typed accessors
        EXPECT_FALSE( doc.get<std::string>( "null_value" ).has_value() );

        // Empty containers
        auto emptyObj = doc.get<Object>( "empty_object" );
        ASSERT_TRUE( emptyObj.has_value() );
        EXPECT_EQ( emptyObj->size(), 0 );

        auto emptyArr = doc.get<Array>( "empty_array" );
        ASSERT_TRUE( emptyArr.has_value() );
        EXPECT_EQ( emptyArr->size(), 0 );

        // Unicode
        auto unicode = doc.get<std::string>( "unicode" );
        ASSERT_TRUE( unicode.has_value() );
        EXPECT_FALSE( unicode->empty() );
    }

    //----------------------------------------------
    // Error handling
    //----------------------------------------------

    TEST_F( FileLoadingTest, LoadNonExistentFile )
    {
        auto docOpt = loadJsonFile( "nonexistent.json" );
        EXPECT_FALSE( docOpt.has_value() ) << "Should fail to load nonexistent file";
    }

    TEST_F( FileLoadingTest, LoadInvalidJsonFile )
    {
        auto docOpt = loadJsonFile( "invalid.json" );
        EXPECT_FALSE( docOpt.has_value() ) << "Should fail to load invalid JSON file";
    }

    //----------------------------------------------
    // Iteration over loaded files
    //----------------------------------------------

    TEST_F( FileLoadingTest, IterateLargeArrayFromFile )
    {
        auto docOpt = loadJsonFile( "large_array.json" );
        ASSERT_TRUE( docOpt.has_value() );

        auto arrOpt = docOpt->get<Array>( "" );
        ASSERT_TRUE( arrOpt.has_value() );

        size_t count = 0;
        double totalValue = 0.0;

        for( const auto& item : arrOpt.value() )
        {
            auto value = item.get<double>( "value" );
            if( value.has_value() )
            {
                totalValue += *value;
            }
            ++count;
        }

        EXPECT_EQ( count, 50 );
        EXPECT_GT( totalValue, 0 );
    }

    TEST_F( FileLoadingTest, IterateLargeObjectFromFile )
    {
        auto docOpt = loadJsonFile( "large_object.json" );
        ASSERT_TRUE( docOpt.has_value() );

        // Iterate departments
        auto deptsOpt = docOpt->get<Array>( "departments" );
        ASSERT_TRUE( deptsOpt.has_value() );

        std::vector<std::string> deptNames;
        for( const auto& dept : deptsOpt.value() )
        {
            auto name = dept.get<std::string>( "name" );
            if( name.has_value() )
            {
                deptNames.push_back( *name );
            }
        }

        EXPECT_GE( deptNames.size(), 1 );

        // Iterate locations
        auto locsOpt = docOpt->get<Array>( "locations" );
        ASSERT_TRUE( locsOpt.has_value() );

        int totalEmployees = 0;
        for( const auto& loc : locsOpt.value() )
        {
            auto employees = loc.get<int64_t>( "employees" );
            if( employees.has_value() )
            {
                totalEmployees += static_cast<int>( *employees );
            }
        }

        EXPECT_GT( totalEmployees, 0 );
    }
} // namespace nfx::json::test
