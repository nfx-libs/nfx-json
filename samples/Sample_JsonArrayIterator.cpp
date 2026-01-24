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
 * @file Sample_JsonArrayIterator.cpp
 * @brief Demonstrates Array::iterator for JSON array traversal
 * @details Shows efficient array iteration with range-for loops, element access,
 *          and type-safe value extraction for structured data processing
 */

#include <nfx/Json.h>

#include <iostream>
#include <string>

int main()
{
    using namespace nfx::json;

    std::cout << "=== Array iterator Sample ===\n\n";

    // Create test JSON document with arrays
    std::string jsonStr = R"({
        "users": [
            {"name": "Alice", "age": 30, "active": true},
            {"name": "Bob", "age": 25, "active": false},
            {"name": "Charlie", "age": 35, "active": true}
        ],
        "scores": [100, 95, 87, 92, 78],
        "tags": ["important", "urgent", "review"]
    })";

    auto doc = Document::fromString( jsonStr );
    if( !doc.has_value() )
    {
        std::cerr << "Failed to parse JSON\n";
        return 1;
    }

    //=====================================================================
    // 1. Range-for loop over array elements
    //=====================================================================
    {
        std::cout << "1. Range-for loop over array elements\n";
        std::cout << "--------------------------------------\n";

        auto usersOpt = doc->get<Array>( "users" );
        if( usersOpt.has_value() )
        {
            Array& users = usersOpt.value();
            std::cout << "Array size: " << users.size() << "\n";

            size_t index = 0;
            for( const auto& user : users )
            {
                auto name = user.get<std::string>( "name" );
                auto age = user.get<int64_t>( "age" );
                auto active = user.get<bool>( "active" );

                std::cout << "User " << index << ": " << ( name ? *name : "N/A" ) << ", "
                          << "age " << ( age ? std::to_string( *age ) : "N/A" ) << ", "
                          << ( active && *active ? "active" : "inactive" ) << "\n";
                ++index;
            }
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 2. Iterating over primitive arrays
    //=====================================================================
    {
        std::cout << "2. Iterating over primitive arrays\n";
        std::cout << "-----------------------------------\n";

        auto scoresOpt = doc->get<Array>( "/scores" );
        if( scoresOpt.has_value() )
        {
            Array& scores = scoresOpt.value();
            std::cout << "Scores array size: " << scores.size() << "\n";

            size_t index = 0;
            for( const auto& scoreDoc : scores )
            {
                auto score = scoreDoc.get<int>( "" );
                std::cout << "Score[" << index << "]: " << ( score ? std::to_string( *score ) : "N/A" ) << "\n";
                ++index;
            }
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 3. Index-based access with size()
    //=====================================================================
    {
        std::cout << "3. Index-based access with size()\n";
        std::cout << "----------------------------------\n";

        auto tagsOpt = doc->get<Array>( "tags" );
        if( tagsOpt.has_value() )
        {
            Array& tags = tagsOpt.value();
            std::cout << "Tags array size: " << tags.size() << "\n";

            // Access specific indices
            if( auto tag = tags[1].get<std::string>( "" ) )
            {
                std::cout << "Tag at index 1: " << *tag << "\n";
            }

            if( auto tag = tags[2].get<std::string>( "" ) )
            {
                std::cout << "Tag at index 2: " << *tag << "\n";
            }

            if( auto tag = tags[0].get<std::string>( "" ) )
            {
                std::cout << "First tag: " << *tag << "\n";
            }
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 4. Using iterators directly
    //=====================================================================
    {
        std::cout << "4. Using iterators directly\n";
        std::cout << "----------------------------\n";

        auto tagsOpt = doc->get<Array>( "tags" );
        if( tagsOpt.has_value() )
        {
            Array& tags = tagsOpt.value();

            std::cout << "Forward iteration: ";
            for( auto it = tags.begin(); it != tags.end(); ++it )
            {
                auto tag = ( *it ).get<std::string>( "" );
                if( tag )
                {
                    std::cout << *tag << " ";
                }
            }
            std::cout << "\n";
        }

        std::cout << "\n";
    }

    return 0;
}
