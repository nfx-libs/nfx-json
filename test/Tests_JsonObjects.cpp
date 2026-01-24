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
 * @file Tests_JsonObjects.cpp
 * @brief Unit tests for JSON Object class functionality
 * @details Tests covering object construction, serialization methods, factory methods,
 *          validation, and integration with Document system
 */

#include <gtest/gtest.h>

#include <nfx/Json.h>

namespace nfx::json::test
{
    //=====================================================================
    // Object serialization and factory methods tests
    //=====================================================================

    class JSONObjectTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            // Create test document with object structure
            std::string jsonStr = R"({
                "user": {
                    "name": "Alice",
                    "age": 30,
                    "active": true,
                    "height": 1.65,
                    "spouse": null,
                    "preferences": {
                        "theme": "dark",
                        "notifications": true
                    },
                    "hobbies": ["reading", "gaming", "cooking"]
                },
                "settings": {
                    "volume": 0.8,
                    "language": "en",
                    "debug": false
                }
            })";

            auto doc = Document::fromString( jsonStr );
            ASSERT_TRUE( doc.has_value() );
            testDoc = std::move( doc.value() );
        }

        Document testDoc;
    };

    //----------------------------------------------
    // Object serialization methods (toJsonString/toJsonBytes)
    //----------------------------------------------

    TEST_F( JSONObjectTest, ToJsonStringEmpty )
    {
        Document emptyDoc;
        auto emptyObj = emptyDoc.get<Object>( "" );
        ASSERT_TRUE( emptyObj.has_value() );

        std::string jsonStr = emptyDoc.toString();
        EXPECT_EQ( jsonStr, "{}" );
    }

    TEST_F( JSONObjectTest, ToJsonBytesEmpty )
    {
        Document emptyDoc;
        auto emptyObj = emptyDoc.get<Object>( "" );
        ASSERT_TRUE( emptyObj.has_value() );

        std::vector<uint8_t> jsonBytes = emptyDoc.toBytes();
        std::string jsonStr( jsonBytes.begin(), jsonBytes.end() );
        EXPECT_EQ( jsonStr, "{}" );
    }

    //----------------------------------------------
    // Object validation methods
    //----------------------------------------------

    TEST_F( JSONObjectTest, IsValidForValidObject )
    {
        auto userObj = testDoc.get<Object>( "user" );
        ASSERT_TRUE( userObj.has_value() );

        EXPECT_TRUE( testDoc.isValid() );
    }

    TEST_F( JSONObjectTest, IsValidForEmptyObject )
    {
        Document emptyDoc;
        auto emptyObj = emptyDoc.get<Object>( "" );
        ASSERT_TRUE( emptyObj.has_value() );

        EXPECT_TRUE( emptyDoc.isValid() );
    }

    //----------------------------------------------
    // Nested object/array access via Object::get<Object/Array>
    //----------------------------------------------

    TEST_F( JSONObjectTest, GetNestedObjectFromObject )
    {
        auto theme = testDoc.get<std::string>( "/user/preferences/theme" );
        ASSERT_TRUE( theme.has_value() );
        EXPECT_EQ( theme.value(), "dark" );

        auto notifications = testDoc.get<bool>( "/user/preferences/notifications" );
        ASSERT_TRUE( notifications.has_value() );
        EXPECT_TRUE( notifications.value() );
    }

    TEST_F( JSONObjectTest, GetNestedArrayFromObject )
    {
        auto hobbiesArray = testDoc.get<Array>( "/user/hobbies" );
        ASSERT_TRUE( hobbiesArray.has_value() );

        EXPECT_EQ( hobbiesArray.value().size(), 3 );

        EXPECT_EQ( hobbiesArray.value()[0].get<std::string>( "" ).value_or( "" ), "reading" );
        EXPECT_EQ( hobbiesArray.value()[1].get<std::string>( "" ).value_or( "" ), "gaming" );
        EXPECT_EQ( hobbiesArray.value()[2].get<std::string>( "" ).value_or( "" ), "cooking" );
    }

    TEST_F( JSONObjectTest, GetNestedObjectAndModifyParent )
    {
        auto userName = testDoc.get<std::string>( "/user/name" );
        ASSERT_TRUE( userName.has_value() );
        EXPECT_EQ( userName.value(), "Alice" );

        auto theme = testDoc.get<std::string>( "/user/preferences/theme" );
        ASSERT_TRUE( theme.has_value() );
        EXPECT_EQ( theme.value(), "dark" );
    }
} // namespace nfx::json::test
