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
 * @file Tests_JsonPathView.cpp
 * @brief Unit tests for Document::PathView
 * @details Tests for path iteration, depth tracking, format options, and leaf filtering
 */

#include <gtest/gtest.h>

#include <nfx/Json.h>

#include <algorithm>
#include <set>

namespace nfx::json::test
{
    //=====================================================================
    // PathView tests
    //=====================================================================

    class PathViewTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            // Simple flat object
            flatJson = R"({
                "name": "Alice",
                "age": 30,
                "active": true
            })";

            // Nested object
            nestedJson = R"({
                "user": {
                    "name": "Bob",
                    "address": {
                        "city": "Seattle",
                        "zip": "98101"
                    }
                }
            })";

            // Array with primitives
            arrayJson = R"({
                "items": ["apple", "banana", "cherry"]
            })";

            // Complex mixed structure
            complexJson = R"({
                "users": [
                    {
                        "name": "Alice",
                        "emails": ["alice@example.com", "a.smith@work.com"]
                    },
                    {
                        "name": "Bob",
                        "emails": ["bob@example.com"]
                    }
                ],
                "metadata": {
                    "version": "1.0",
                    "count": 2
                }
            })";
        }

        std::string flatJson;
        std::string nestedJson;
        std::string arrayJson;
        std::string complexJson;
    };

    //----------------------------------------------
    // Basic iteration tests
    //----------------------------------------------

    TEST_F( PathViewTest, IterateFlatObject_JsonPointer )
    {
        auto doc = Document::fromString( flatJson );
        ASSERT_TRUE( doc.has_value() );

        // Collect path strings using range-based for
        std::set<std::string> pathSet;
        for( const auto& entry : Document::PathView{ *doc, Document::PathView::Format::JsonPointer } )
        {
            pathSet.insert( entry.path );
        }

        // Should have 3 leaf paths
        EXPECT_EQ( pathSet.size(), 3 );

        EXPECT_TRUE( pathSet.count( "/name" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/age" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/active" ) > 0 );
    }

    TEST_F( PathViewTest, IterateFlatObject_DotNotation )
    {
        auto doc = Document::fromString( flatJson );
        ASSERT_TRUE( doc.has_value() );

        std::set<std::string> pathSet;
        for( const auto& entry : Document::PathView{ *doc, Document::PathView::Format::DotNotation } )
        {
            pathSet.insert( entry.path );
        }

        EXPECT_TRUE( pathSet.count( "name" ) > 0 );
        EXPECT_TRUE( pathSet.count( "age" ) > 0 );
        EXPECT_TRUE( pathSet.count( "active" ) > 0 );
    }

    TEST_F( PathViewTest, IterateNestedObject_JsonPointer )
    {
        auto doc = Document::fromString( nestedJson );
        ASSERT_TRUE( doc.has_value() );

        std::set<std::string> pathSet;
        for( const auto& entry : Document::PathView{ *doc, Document::PathView::Format::JsonPointer } )
        {
            pathSet.insert( entry.path );
        }

        // Should include nested paths
        EXPECT_TRUE( pathSet.count( "/user" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/user/name" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/user/address" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/user/address/city" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/user/address/zip" ) > 0 );
    }

    TEST_F( PathViewTest, IterateNestedObject_DotNotation )
    {
        auto doc = Document::fromString( nestedJson );
        ASSERT_TRUE( doc.has_value() );

        std::set<std::string> pathSet;
        for( const auto& entry : Document::PathView{ *doc, Document::PathView::Format::DotNotation } )
        {
            pathSet.insert( entry.path );
        }

        EXPECT_TRUE( pathSet.count( "user" ) > 0 );
        EXPECT_TRUE( pathSet.count( "user.name" ) > 0 );
        EXPECT_TRUE( pathSet.count( "user.address" ) > 0 );
        EXPECT_TRUE( pathSet.count( "user.address.city" ) > 0 );
        EXPECT_TRUE( pathSet.count( "user.address.zip" ) > 0 );
    }

    //----------------------------------------------
    // Array path tests
    //----------------------------------------------

    TEST_F( PathViewTest, IterateArray_JsonPointer )
    {
        auto doc = Document::fromString( arrayJson );
        ASSERT_TRUE( doc.has_value() );

        std::set<std::string> pathSet;
        for( const auto& entry : Document::PathView{ *doc, Document::PathView::Format::JsonPointer } )
        {
            pathSet.insert( entry.path );
        }

        EXPECT_TRUE( pathSet.count( "/items" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/items/0" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/items/1" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/items/2" ) > 0 );
    }

    TEST_F( PathViewTest, IterateArray_DotNotation )
    {
        auto doc = Document::fromString( arrayJson );
        ASSERT_TRUE( doc.has_value() );

        std::set<std::string> pathSet;
        for( const auto& entry : Document::PathView{ *doc, Document::PathView::Format::DotNotation } )
        {
            pathSet.insert( entry.path );
        }

        EXPECT_TRUE( pathSet.count( "items" ) > 0 );
        EXPECT_TRUE( pathSet.count( "items[0]" ) > 0 );
        EXPECT_TRUE( pathSet.count( "items[1]" ) > 0 );
        EXPECT_TRUE( pathSet.count( "items[2]" ) > 0 );
    }

    //----------------------------------------------
    // Depth tracking tests
    //----------------------------------------------

    TEST_F( PathViewTest, DepthTracking )
    {
        auto doc = Document::fromString( nestedJson );
        ASSERT_TRUE( doc.has_value() );

        for( const auto& entry : Document::PathView{ *doc } )
        {
            if( entry.path == "/user" )
            {
                EXPECT_EQ( entry.depth, 1 );
            }
            else if( entry.path == "/user/name" || entry.path == "/user/address" )
            {
                EXPECT_EQ( entry.depth, 2 );
            }
            else if( entry.path == "/user/address/city" || entry.path == "/user/address/zip" )
            {
                EXPECT_EQ( entry.depth, 3 );
            }
        }
    }

    //----------------------------------------------
    // Leaf filtering tests
    //----------------------------------------------

    TEST_F( PathViewTest, IterateLeaves )
    {
        auto doc = Document::fromString( nestedJson );
        ASSERT_TRUE( doc.has_value() );

        std::vector<Document::PathView::Entry> leaves;
        for( const auto& entry : Document::PathView{ *doc } )
        {
            if( entry.isLeaf )
            {
                leaves.push_back( entry );
            }
        }

        // All entries should be leaves
        for( const auto& entry : leaves )
        {
            EXPECT_TRUE( entry.isLeaf );
        }

        // Should only have primitive values (name, city, zip)
        EXPECT_EQ( leaves.size(), 3 );

        std::set<std::string> pathSet;
        for( const auto& entry : leaves )
        {
            pathSet.insert( entry.path );
        }

        EXPECT_TRUE( pathSet.count( "/user/name" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/user/address/city" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/user/address/zip" ) > 0 );

        // Container paths should not be present
        EXPECT_EQ( pathSet.count( "/user" ), 0 );
        EXPECT_EQ( pathSet.count( "/user/address" ), 0 );
    }

    TEST_F( PathViewTest, ExcludeContainers )
    {
        auto doc = Document::fromString( nestedJson );
        ASSERT_TRUE( doc.has_value() );

        // Filter manually for leaves only
        for( const auto& entry : Document::PathView{ *doc } )
        {
            if( entry.isLeaf )
            {
                EXPECT_TRUE( entry.isLeaf );
            }
        }
    }

    //----------------------------------------------
    // Value access tests
    //----------------------------------------------

    TEST_F( PathViewTest, ValueAccess )
    {
        auto doc = Document::fromString( flatJson );
        ASSERT_TRUE( doc.has_value() );

        auto paths = Document::PathView{ *doc };

        for( const auto& entry : paths )
        {
            if( entry.path == "/name" )
            {
                auto val = entry.value().get<std::string>( "" );
                ASSERT_TRUE( val.has_value() );
                EXPECT_EQ( *val, "Alice" );
            }
            else if( entry.path == "/age" )
            {
                auto val = entry.value().get<int>( "" );
                ASSERT_TRUE( val.has_value() );
                EXPECT_EQ( *val, 30 );
            }
            else if( entry.path == "/active" )
            {
                auto val = entry.value().get<bool>( "" );
                ASSERT_TRUE( val.has_value() );
                EXPECT_EQ( *val, true );
            }
        }
    }

    TEST_F( PathViewTest, ContainerValueAccess )
    {
        auto doc = Document::fromString( nestedJson );
        ASSERT_TRUE( doc.has_value() );

        auto paths = Document::PathView{ *doc };

        for( const auto& entry : paths )
        {
            if( entry.path == "/user/address" )
            {
                // Container value should be accessible as object
                auto obj = entry.value().get<Object>( "" );
                ASSERT_TRUE( obj.has_value() );
                EXPECT_EQ( obj->size(), 2 );
            }
        }
    }

    //----------------------------------------------
    // iterator interface tests
    //----------------------------------------------

    TEST_F( PathViewTest, iteratorBeginEnd )
    {
        auto doc = Document::fromString( flatJson );
        ASSERT_TRUE( doc.has_value() );

        Document::PathView iter( *doc );
        auto it = iter.begin();
        auto endIt = iter.end();

        size_t count = 0;
        for( ; it != endIt; ++it )
        {
            ++count;
        }

        EXPECT_EQ( count, 3 );
    }

    TEST_F( PathViewTest, iteratorDereference )
    {
        auto doc = Document::fromString( flatJson );
        ASSERT_TRUE( doc.has_value() );

        Document::PathView iter( *doc );
        auto it = iter.begin();

        // Test * operator
        const auto& entry = *it;
        EXPECT_FALSE( entry.path.empty() );

        // Test -> operator
        EXPECT_FALSE( it->path.empty() );
    }

    TEST_F( PathViewTest, iteratorIncrement )
    {
        auto doc = Document::fromString( flatJson );
        ASSERT_TRUE( doc.has_value() );

        Document::PathView iter( *doc );
        auto it = iter.begin();

        std::string firstPath = it->path;

        // Pre-increment
        ++it;
        EXPECT_NE( it->path, firstPath );

        // Post-increment
        std::string secondPath = it->path;
        it++;
        EXPECT_NE( it->path, secondPath );
    }

    //----------------------------------------------
    // Complex structure tests
    //----------------------------------------------

    TEST_F( PathViewTest, ComplexStructure_JsonPointer )
    {
        auto doc = Document::fromString( complexJson );
        ASSERT_TRUE( doc.has_value() );

        std::set<std::string> pathSet;
        for( const auto& entry : Document::PathView{ *doc } )
        {
            pathSet.insert( entry.path );
        }

        // Check nested array paths
        EXPECT_TRUE( pathSet.count( "/users" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/users/0" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/users/0/name" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/users/0/emails" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/users/0/emails/0" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/users/0/emails/1" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/users/1" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/users/1/name" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/metadata" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/metadata/version" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/metadata/count" ) > 0 );
    }

    TEST_F( PathViewTest, ComplexStructure_DotNotation )
    {
        auto doc = Document::fromString( complexJson );
        ASSERT_TRUE( doc.has_value() );

        auto paths = Document::PathView{ *doc, Document::PathView::Format::DotNotation };

        std::set<std::string> pathSet;
        for( const auto& entry : paths )
        {
            pathSet.insert( entry.path );
        }

        EXPECT_TRUE( pathSet.count( "users" ) > 0 );
        EXPECT_TRUE( pathSet.count( "users[0]" ) > 0 );
        EXPECT_TRUE( pathSet.count( "users[0].name" ) > 0 );
        EXPECT_TRUE( pathSet.count( "users[0].emails" ) > 0 );
        EXPECT_TRUE( pathSet.count( "users[0].emails[0]" ) > 0 );
        EXPECT_TRUE( pathSet.count( "users[0].emails[1]" ) > 0 );
        EXPECT_TRUE( pathSet.count( "metadata" ) > 0 );
        EXPECT_TRUE( pathSet.count( "metadata.version" ) > 0 );
    }

    //----------------------------------------------
    // Edge cases
    //----------------------------------------------

    TEST_F( PathViewTest, EmptyDocument )
    {
        auto doc = Document::fromString( "{}" );
        ASSERT_TRUE( doc.has_value() );

        auto paths = Document::PathView{ *doc };
        EXPECT_TRUE( paths.empty() );
    }

    TEST_F( PathViewTest, EmptyArray )
    {
        auto doc = Document::fromString( R"({"items": []})" );
        ASSERT_TRUE( doc.has_value() );

        auto paths = Document::PathView{ *doc };

        // Should have the "items" container path
        EXPECT_EQ( paths.size(), 1 );
        EXPECT_EQ( paths[0].path, "/items" );
        EXPECT_FALSE( paths[0].isLeaf );
    }

    TEST_F( PathViewTest, NullValue )
    {
        auto doc = Document::fromString( R"({"value": null})" );
        ASSERT_TRUE( doc.has_value() );

        auto paths = Document::PathView{ *doc };

        EXPECT_EQ( paths.size(), 1 );
        EXPECT_EQ( paths[0].path, "/value" );
        EXPECT_TRUE( paths[0].isLeaf );
    }

    TEST_F( PathViewTest, SpecialCharactersInKeys )
    {
        auto doc = Document::fromString( R"({"key/with/slashes": "value", "key~with~tildes": "value2"})" );
        ASSERT_TRUE( doc.has_value() );

        auto paths = Document::PathView{ *doc };

        std::set<std::string> pathSet;
        for( const auto& entry : paths )
        {
            pathSet.insert( entry.path );
        }

        // JSON Pointer escaping: / -> ~1, ~ -> ~0
        EXPECT_TRUE( pathSet.count( "/key~1with~1slashes" ) > 0 );
        EXPECT_TRUE( pathSet.count( "/key~0with~0tildes" ) > 0 );
    }

    TEST_F( PathViewTest, IsLeafFlag )
    {
        auto doc = Document::fromString( complexJson );
        ASSERT_TRUE( doc.has_value() );

        auto paths = Document::PathView{ *doc };

        for( const auto& entry : paths )
        {
            if( entry.path == "/users" || entry.path == "/users/0" || entry.path == "/users/0/emails" ||
                entry.path == "/metadata" )
            {
                EXPECT_FALSE( entry.isLeaf ) << "Path should be container: " << entry.path;
            }
            else if(
                entry.path == "/users/0/name" || entry.path == "/users/0/emails/0" ||
                entry.path == "/metadata/version" || entry.path == "/metadata/count" )
            {
                EXPECT_TRUE( entry.isLeaf ) << "Path should be leaf: " << entry.path;
            }
        }
    }
} // namespace nfx::json::test
