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
 * @file Tests_JsonArrayIterator.cpp
 * @brief Unit tests for JSON Array iterator functionality
 * @details Tests covering array iteration, element access, path validation,
 *          iterator state management, and range-for loop usage.
 */

#include <gtest/gtest.h>

#include <nfx/Json.h>

namespace nfx::json::test
{
    //=====================================================================
    // JSON Array iterator tests
    //=====================================================================

    //----------------------------------------------
    // Array iterator test fixture
    //----------------------------------------------

    class ArrayiteratorTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            // Create test JSON with various array types
            std::string testJson = R"({
                "users": [
                    {"name": "Alice", "age": 30, "active": true},
                    {"name": "Bob", "age": 25, "active": false},
                    {"name": "Charlie", "age": 35, "active": true}
                ],
                "scores": [100, 95, 87, 92, 78],
                "tags": ["important", "urgent", "review", "critical"],
                "empty": [],
                "mixed": [1, "text", true, null, {"nested": "object"}],
                "nested": {
                    "levels": {
                        "deep": {
                            "array": ["deep1", "deep2", "deep3"]
                        }
                    }
                },
                "matrix": [
                    [1, 2, 3],
                    [4, 5, 6],
                    [7, 8, 9]
                ]
            })";

            auto docOpt = Document::fromString( testJson );
            ASSERT_TRUE( docOpt.has_value() );
            document = std::move( docOpt.value() );
        }

        Document document;
    };

    //----------------------------------------------
    // Basic iteration
    //----------------------------------------------

    TEST_F( ArrayiteratorTest, RangeForLoop )
    {
        // Get scores array
        auto scoresOpt = document.get<Array>( "scores" );
        ASSERT_TRUE( scoresOpt.has_value() );
        Array scores = scoresOpt.value();

        // Test range-for loop iteration
        std::vector<int64_t> values;
        for( const auto& element : scores )
        {
            auto val = element.get<int64_t>( "" );
            ASSERT_TRUE( val.has_value() );
            values.push_back( *val );
        }

        std::vector<int64_t> expected = { 100, 95, 87, 92, 78 };
        EXPECT_EQ( values, expected );
    }

    TEST_F( ArrayiteratorTest, BeginEnditerators )
    {
        // Get tags array
        auto tagsOpt = document.get<Array>( "tags" );
        ASSERT_TRUE( tagsOpt.has_value() );
        Array tags = tagsOpt.value();

        // Test begin/end
        auto it = tags.begin();
        auto endIt = tags.end();

        EXPECT_NE( it, endIt );

        // Count elements
        size_t count = 0;
        for( auto iter = tags.begin(); iter != tags.end(); ++iter )
        {
            ++count;
        }
        EXPECT_EQ( count, 4 ); // important, urgent, review, critical
    }

    TEST_F( ArrayiteratorTest, iteratorValueAccess )
    {
        auto tagsOpt = document.get<Array>( "tags" );
        ASSERT_TRUE( tagsOpt.has_value() );
        Array tags = tagsOpt.value();

        std::vector<std::string> expected = { "important", "urgent", "review", "critical" };
        size_t index = 0;

        for( const auto& element : tags )
        {
            auto val = element.get<std::string>( "" );
            ASSERT_TRUE( val.has_value() );
            EXPECT_EQ( *val, expected[index] );
            ++index;
        }
    }

    //----------------------------------------------
    // Element access
    //----------------------------------------------

    TEST_F( ArrayiteratorTest, PrimitiveElementAccess )
    {
        // Test integer array access
        auto scoresOpt = document.get<Array>( "scores" );
        ASSERT_TRUE( scoresOpt.has_value() );
        Array scores = scoresOpt.value();

        auto it = scores.begin();

        auto score0 = ( *it ).get<int64_t>( "" );
        ASSERT_TRUE( score0.has_value() );
        EXPECT_EQ( *score0, 100 );

        ++it;
        auto score1 = ( *it ).get<int64_t>( "" );
        ASSERT_TRUE( score1.has_value() );
        EXPECT_EQ( *score1, 95 );

        // Test string array access
        auto tagsOpt = document.get<Array>( "tags" );
        ASSERT_TRUE( tagsOpt.has_value() );
        Array tags = tagsOpt.value();

        auto tagIt = tags.begin();
        auto tag0 = ( *tagIt ).get<std::string>( "" );
        ASSERT_TRUE( tag0.has_value() );
        EXPECT_EQ( *tag0, "important" );

        ++tagIt;
        auto tag1 = ( *tagIt ).get<std::string>( "" );
        ASSERT_TRUE( tag1.has_value() );
        EXPECT_EQ( *tag1, "urgent" );
    }

    TEST_F( ArrayiteratorTest, ObjectElementAccess )
    {
        // Test object array access
        auto usersOpt = document.get<Array>( "users" );
        ASSERT_TRUE( usersOpt.has_value() );
        Array users = usersOpt.value();

        auto it = users.begin();

        // Get first user
        Document user0 = *it;
        auto name = user0.get<std::string>( "name" );
        auto age = user0.get<int64_t>( "age" );
        auto active = user0.get<bool>( "active" );

        ASSERT_TRUE( name.has_value() );
        EXPECT_EQ( *name, "Alice" );
        ASSERT_TRUE( age.has_value() );
        EXPECT_EQ( *age, 30 );
        ASSERT_TRUE( active.has_value() );
        EXPECT_TRUE( *active );

        // Move to next user
        ++it;
        Document user1 = *it;
        name = user1.get<std::string>( "name" );
        age = user1.get<int64_t>( "age" );
        active = user1.get<bool>( "active" );

        ASSERT_TRUE( name.has_value() );
        EXPECT_EQ( *name, "Bob" );
        ASSERT_TRUE( age.has_value() );
        EXPECT_EQ( *age, 25 );
        ASSERT_TRUE( active.has_value() );
        EXPECT_FALSE( *active );
    }

    TEST_F( ArrayiteratorTest, WrongTypeAccess )
    {
        // Test accessing string array as int
        auto tagsOpt = document.get<Array>( "tags" );
        ASSERT_TRUE( tagsOpt.has_value() );
        Array tags = tagsOpt.value();

        auto it = tags.begin();
        Document element = *it;

        auto intValue = element.get<int64_t>( "" );
        EXPECT_FALSE( intValue.has_value() );

        auto stringValue = element.get<std::string>( "" );
        EXPECT_TRUE( stringValue.has_value() );
        EXPECT_EQ( *stringValue, "important" );

        // Test accessing int array as string
        auto scoresOpt = document.get<Array>( "scores" );
        ASSERT_TRUE( scoresOpt.has_value() );
        Array scores = scoresOpt.value();

        auto scoreIt = scores.begin();
        Document scoreElement = *scoreIt;

        auto stringFromInt = scoreElement.get<std::string>( "" );
        EXPECT_FALSE( stringFromInt.has_value() );

        auto intFromInt = scoreElement.get<int64_t>( "" );
        EXPECT_TRUE( intFromInt.has_value() );
        EXPECT_EQ( *intFromInt, 100 );
    }

    //----------------------------------------------
    // Traversal
    //----------------------------------------------

    TEST_F( ArrayiteratorTest, ForwardTraversal )
    {
        auto scoresOpt = document.get<Array>( "scores" );
        ASSERT_TRUE( scoresOpt.has_value() );
        Array scores = scoresOpt.value();

        std::vector<int64_t> expectedScores = { 100, 95, 87, 92, 78 };
        std::vector<int64_t> actualScores;

        // Iterate through all elements
        for( const auto& element : scores )
        {
            auto score = element.get<int64_t>( "" );
            ASSERT_TRUE( score.has_value() );
            actualScores.push_back( *score );
        }

        EXPECT_EQ( actualScores, expectedScores );
    }

    TEST_F( ArrayiteratorTest, PostIncrementOperator )
    {
        auto tagsOpt = document.get<Array>( "tags" );
        ASSERT_TRUE( tagsOpt.has_value() );
        Array tags = tagsOpt.value();

        auto it = tags.begin();

        // Post-increment returns copy of old iterator
        auto oldIt = it++;
        EXPECT_NE( it, oldIt );

        auto oldVal = ( *oldIt ).get<std::string>( "" );
        auto newVal = ( *it ).get<std::string>( "" );
        ASSERT_TRUE( oldVal.has_value() );
        ASSERT_TRUE( newVal.has_value() );
        EXPECT_EQ( *oldVal, "important" );
        EXPECT_EQ( *newVal, "urgent" );
    }

    //----------------------------------------------
    // Edge cases
    //----------------------------------------------

    TEST_F( ArrayiteratorTest, EmptyArrayHandling )
    {
        auto emptyOpt = document.get<Array>( "empty" );
        ASSERT_TRUE( emptyOpt.has_value() );
        Array empty = emptyOpt.value();

        EXPECT_EQ( empty.size(), 0 );
        EXPECT_EQ( empty.begin(), empty.end() );

        // Range-for on empty should not iterate
        int count = 0;
        for( const auto& element : empty )
        {
            (void)element;
            ++count;
        }
        EXPECT_EQ( count, 0 );
    }

    TEST_F( ArrayiteratorTest, MixedTypeArray )
    {
        auto mixedOpt = document.get<Array>( "mixed" );
        ASSERT_TRUE( mixedOpt.has_value() );
        Array mixed = mixedOpt.value();

        EXPECT_EQ( mixed.size(), 5 );

        auto it = mixed.begin();

        // Element 0: integer
        auto intVal = ( *it ).get<int64_t>( "" );
        EXPECT_TRUE( intVal.has_value() );
        EXPECT_EQ( *intVal, 1 );

        // Element 1: string
        ++it;
        auto strVal = ( *it ).get<std::string>( "" );
        EXPECT_TRUE( strVal.has_value() );
        EXPECT_EQ( *strVal, "text" );

        // Element 2: boolean
        ++it;
        auto boolVal = ( *it ).get<bool>( "" );
        EXPECT_TRUE( boolVal.has_value() );
        EXPECT_TRUE( *boolVal );

        // Element 3: null (should not be accessible as typed values)
        ++it;
        EXPECT_FALSE( ( *it ).get<std::string>( "" ).has_value() );
        EXPECT_FALSE( ( *it ).get<int64_t>( "" ).has_value() );
        EXPECT_FALSE( ( *it ).get<bool>( "" ).has_value() );

        // Element 4: object
        ++it;
        Document objDoc = *it;
        auto nestedVal = objDoc.get<std::string>( "nested" );
        EXPECT_TRUE( nestedVal.has_value() );
        EXPECT_EQ( *nestedVal, "object" );
    }

    //----------------------------------------------
    // JSON Pointer array access
    //----------------------------------------------

    TEST_F( ArrayiteratorTest, DeepPathArrayAccess )
    {
        // Test deeply nested array access with JSON Pointer
        auto deepArrayOpt = document.get<Array>( "/nested/levels/deep/array" );
        ASSERT_TRUE( deepArrayOpt.has_value() );
        Array deepArray = deepArrayOpt.value();

        EXPECT_EQ( deepArray.size(), 3 );

        auto it = deepArray.begin();

        auto deep1 = ( *it ).get<std::string>( "" );
        ASSERT_TRUE( deep1.has_value() );
        EXPECT_EQ( *deep1, "deep1" );

        ++it;
        auto deep2 = ( *it ).get<std::string>( "" );
        ASSERT_TRUE( deep2.has_value() );
        EXPECT_EQ( *deep2, "deep2" );
    }

    //----------------------------------------------
    // Nested array iteration
    //----------------------------------------------

    TEST_F( ArrayiteratorTest, NestedArrayIteration )
    {
        // Test iterating over arrays within arrays (matrix)
        auto matrixOpt = document.get<Array>( "matrix" );
        ASSERT_TRUE( matrixOpt.has_value() );
        Array matrix = matrixOpt.value();

        EXPECT_EQ( matrix.size(), 3 );

        std::vector<std::vector<int64_t>> result;

        for( const auto& rowDoc : matrix )
        {
            auto rowOpt = rowDoc.get<Array>( "" );
            ASSERT_TRUE( rowOpt.has_value() );
            Array row = rowOpt.value();

            std::vector<int64_t> rowValues;
            for( const auto& cellDoc : row )
            {
                auto cell = cellDoc.get<int64_t>( "" );
                ASSERT_TRUE( cell.has_value() );
                rowValues.push_back( *cell );
            }
            result.push_back( rowValues );
        }

        std::vector<std::vector<int64_t>> expected = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };
        EXPECT_EQ( result, expected );
    }

    //----------------------------------------------
    // iterator comparison and state
    //----------------------------------------------

    TEST_F( ArrayiteratorTest, iteratorEquality )
    {
        auto scoresOpt = document.get<Array>( "scores" );
        ASSERT_TRUE( scoresOpt.has_value() );
        Array scores = scoresOpt.value();

        auto it1 = scores.begin();
        auto it2 = scores.begin();

        EXPECT_EQ( it1, it2 );

        ++it1;
        EXPECT_NE( it1, it2 );

        ++it2;
        EXPECT_EQ( it1, it2 );
    }

    TEST_F( ArrayiteratorTest, iteratorToEnd )
    {
        auto tagsOpt = document.get<Array>( "tags" );
        ASSERT_TRUE( tagsOpt.has_value() );
        Array tags = tagsOpt.value();

        auto it = tags.begin();

        // Advance to end
        size_t count = 0;
        while( it != tags.end() )
        {
            ++it;
            ++count;
        }

        EXPECT_EQ( count, 4 );
        EXPECT_EQ( it, tags.end() );
    }

    //----------------------------------------------
    // Complex object iteration
    //----------------------------------------------

    TEST_F( ArrayiteratorTest, UsersArrayIteration )
    {
        auto usersOpt = document.get<Array>( "users" );
        ASSERT_TRUE( usersOpt.has_value() );
        Array users = usersOpt.value();

        EXPECT_EQ( users.size(), 3 );

        std::vector<std::string> names;
        for( const auto& userDoc : users )
        {
            auto name = userDoc.get<std::string>( "name" );
            ASSERT_TRUE( name.has_value() );
            names.push_back( *name );
        }

        std::vector<std::string> expected = { "Alice", "Bob", "Charlie" };
        EXPECT_EQ( names, expected );
    }
} // namespace nfx::json::test
