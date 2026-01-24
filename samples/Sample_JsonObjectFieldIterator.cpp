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
 * @file Sample_JsonObjectFieldIterator.cpp
 * @brief Demonstrates Object::iterator for JSON object field traversal
 * @details Shows efficient object field iteration with range-for loops, key-value access,
 *          and type-safe value extraction for configuration and structured data processing
 */

#include <nfx/Json.h>

#include <iostream>
#include <string>

int main()
{
    using namespace nfx::json;

    std::cout << "=== Object iterator Sample ===\n\n";

    // Create test JSON document with nested objects
    std::string jsonStr = R"({
        "user": {
            "name": "Alice Johnson",
            "age": 28,
            "active": true,
            "height": 1.65,
            "department": "Engineering"
        },
        "preferences": {
            "theme": "dark",
            "language": "en-US",
            "notifications": true,
            "fontSize": 14
        },
        "config": {
            "server": {
                "host": "localhost",
                "port": 8080,
                "ssl": false
            }
        },
        "metadata": {
            "created": "2024-01-15T10:30:00Z",
            "version": "1.2.3",
            "tags": ["production", "web", "api"]
        }
    })";

    auto doc = Document::fromString( jsonStr );
    if( !doc.has_value() )
    {
        std::cerr << "Failed to parse JSON\n";
        return 1;
    }

    //=====================================================================
    // 1. Range-for loop over object fields
    //=====================================================================
    {
        std::cout << "1. Range-for loop over object fields\n";
        std::cout << "--------------------------------------\n";

        auto userOpt = doc->get<Object>( "user" );
        if( userOpt.has_value() )
        {
            Object& user = userOpt.value();
            std::cout << "Object field count: " << user.size() << "\n";

            for( const auto& [key, value] : user )
            {
                std::cout << "Field '" << key << "': ";

                if( auto stringVal = value.get<std::string>( "" ) )
                {
                    std::cout << *stringVal << "\n";
                }
                else if( auto intVal = value.get<int>( "" ) )
                {
                    std::cout << *intVal << "\n";
                }
                else if( auto doubleVal = value.get<double>( "" ) )
                {
                    std::cout << *doubleVal << "\n";
                }
                else if( auto boolVal = value.get<bool>( "" ) )
                {
                    std::cout << ( *boolVal ? "true" : "false" ) << "\n";
                }
            }
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 2. JSON Pointer navigation
    //=====================================================================
    {
        std::cout << "2. JSON Pointer navigation\n";
        std::cout << "--------------------------\n";

        auto prefsOpt = doc->get<Object>( "/preferences" );
        if( prefsOpt.has_value() )
        {
            Object& prefs = prefsOpt.value();
            std::cout << "Preferences field count: " << prefs.size() << "\n";

            for( const auto& [key, value] : prefs )
            {
                std::cout << "Preference '" << key << "': ";

                if( auto stringVal = value.get<std::string>( "" ) )
                {
                    std::cout << "\"" << *stringVal << "\"\n";
                }
                else if( auto boolVal = value.get<bool>( "" ) )
                {
                    std::cout << ( *boolVal ? "true" : "false" ) << "\n";
                }
                else if( auto intVal = value.get<int>( "" ) )
                {
                    std::cout << *intVal << "\n";
                }
            }
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 3. Direct field access with contains() and get()
    //=====================================================================
    {
        std::cout << "3. Direct field access with contains() and get()\n";
        std::cout << "-------------------------------------------------\n";

        auto userOpt = doc->get<Object>( "user" );
        if( userOpt.has_value() )
        {
            Object& user = userOpt.value();

            // Access using std::find_if since Object is std::vector<std::pair<string, Document>>
            auto nameIt = std::find_if( user.begin(), user.end(), []( const auto& kv ) { return kv.first == "name"; } );
            if( nameIt != user.end() )
            {
                std::cout << "Direct access to 'name': " << *nameIt->second.get<std::string>( "" ) << "\n";
            }

            auto ageIt = std::find_if( user.begin(), user.end(), []( const auto& kv ) { return kv.first == "age"; } );
            if( ageIt != user.end() )
            {
                std::cout << "Direct access to 'age': " << *ageIt->second.get<int>( "" ) << "\n";
            }

            auto activeIt =
                std::find_if( user.begin(), user.end(), []( const auto& kv ) { return kv.first == "active"; } );
            if( activeIt != user.end() )
            {
                std::cout << "Direct access to 'active': " << ( *activeIt->second.get<bool>( "" ) ? "true" : "false" )
                          << "\n";
            }
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 4. Nested object traversal
    //=====================================================================
    {
        std::cout << "4. Nested object traversal\n";
        std::cout << "--------------------------\n";

        auto serverOpt = doc->get<Object>( "/config/server" );
        if( serverOpt.has_value() )
        {
            std::cout << "Server configuration fields:\n";

            for( const auto& [key, value] : serverOpt.value() )
            {
                if( auto stringVal = value.get<std::string>( "" ) )
                {
                    std::cout << "  " << key << ": \"" << *stringVal << "\"\n";
                }
                else if( auto intVal = value.get<int>( "" ) )
                {
                    std::cout << "  " << key << ": " << *intVal << "\n";
                }
                else if( auto boolVal = value.get<bool>( "" ) )
                {
                    std::cout << "  " << key << ": " << ( *boolVal ? "true" : "false" ) << "\n";
                }
            }
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 5. Root object iteration
    //=====================================================================
    {
        std::cout << "5. Root object iteration\n";
        std::cout << "------------------------\n";

        auto rootOpt = doc->get<Object>( "" );
        if( rootOpt.has_value() )
        {
            Object& root = rootOpt.value();
            std::cout << "Root object has " << root.size() << " top-level fields:\n";

            for( const auto& [key, value] : root )
            {
                std::cout << "  - " << key << "\n";
            }
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 6. Using iterators directly
    //=====================================================================
    {
        std::cout << "6. Using iterators directly\n";
        std::cout << "----------------------------\n";

        auto prefsOpt = doc->get<Object>( "preferences" );
        if( prefsOpt.has_value() )
        {
            Object& prefs = prefsOpt.value();

            std::cout << "Fields: ";
            for( auto it = prefs.begin(); it != prefs.end(); ++it )
            {
                auto [key, value] = *it;
                std::cout << key << " ";
            }
            std::cout << "\n";
        }

        std::cout << "\n";
    }

    return 0;
}
