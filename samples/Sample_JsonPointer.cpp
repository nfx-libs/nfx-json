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
 * @file Sample_JsonPointer.cpp
 * @brief Sample demonstrating JSON Pointer (RFC 6901) functionality
 * @details Real-world examples showcasing JSON Pointer syntax for document navigation,
 *          array element access, nested object manipulation, and escaped character handling
 */

#include <nfx/Json.h>

#include <iostream>
#include <string>
#include <vector>

using namespace nfx::json;

int main()
{
    std::cout << "=== nfx-json JSON Pointer (RFC 6901) Samples ===\n\n";

    try
    {
        //=====================================================================
        // 1. Basic JSON Pointer Operations
        //=====================================================================
        {
            std::cout << "1. Basic JSON Pointer Operations\n";
            std::cout << "---------------------------------\n";

            Document doc{};

            doc.set<std::string>( "/name", "Alice Johnson" );
            doc.set<int64_t>( "/age", 30 );
            doc.set<double>( "/height", 1.75 );
            doc.set<bool>( "/active", true );
            doc.setNull( "/spouse" );

            std::cout << "Created document using JSON Pointers:\n";
            std::cout << doc.toString( 2 ) << "\n\n";

            std::cout << "Reading values with JSON Pointers:\n";
            std::cout << "Name: " << doc.get<std::string>( "/name" ).value_or( "Unknown" ) << "\n";
            std::cout << "Age: " << doc.get<int64_t>( "/age" ).value_or( 0 ) << "\n";
            std::cout << "Height: " << doc.get<double>( "/height" ).value_or( 0.0 ) << "m\n";
            std::cout << "Active: " << ( doc.get<bool>( "/active" ).value_or( false ) ? "Yes" : "No" ) << "\n\n";

            std::cout << "Field existence checks:\n";
            std::cout << "Has name: " << ( doc.contains( "/name" ) ? "Yes" : "No" ) << "\n";
            std::cout << "Has spouse: " << ( doc.contains( "/spouse" ) ? "Yes" : "No" ) << "\n";
            std::cout << "Has nonexistent: " << ( doc.contains( "/nonexistent" ) ? "Yes" : "No" ) << "\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 2. Nested Object Navigation
        //=====================================================================
        {
            std::cout << "2. Nested Object Navigation\n";
            std::cout << "---------------------------\n";

            Document doc{};

            doc.set<std::string>( "/user/profile/firstName", "John" );
            doc.set<std::string>( "/user/profile/lastName", "Doe" );
            doc.set<std::string>( "/user/profile/email", "john.doe@example.com" );
            doc.set<int64_t>( "/user/profile/age", 28 );

            doc.set<std::string>( "/user/address/street", "123 Main St" );
            doc.set<std::string>( "/user/address/city", "New York" );
            doc.set<std::string>( "/user/address/country", "USA" );
            doc.set<std::string>( "/user/address/zipCode", "10001" );

            doc.set<bool>( "/user/settings/theme/dark", true );
            doc.set<bool>( "/user/settings/notifications/email", false );
            doc.set<bool>( "/user/settings/notifications/push", true );

            std::cout << "Nested document structure:\n";
            std::cout << doc.toString( 2 ) << "\n\n";

            std::cout << "Accessing nested data:\n";
            std::cout << "Full name: " << doc.get<std::string>( "/user/profile/firstName" ).value_or( "" ) << " "
                      << doc.get<std::string>( "/user/profile/lastName" ).value_or( "" ) << "\n";
            std::cout << "Email: " << doc.get<std::string>( "/user/profile/email" ).value_or( "N/A" ) << "\n";
            std::cout << "Address: " << doc.get<std::string>( "/user/address/street" ).value_or( "" ) << ", "
                      << doc.get<std::string>( "/user/address/city" ).value_or( "" ) << ", "
                      << doc.get<std::string>( "/user/address/country" ).value_or( "" ) << "\n";
            std::cout << "Dark theme: "
                      << ( doc.get<bool>( "/user/settings/theme/dark" ).value_or( false ) ? "Yes" : "No" ) << "\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 3. Array Operations with JSON Pointers
        //=====================================================================
        {
            std::cout << "3. Array Operations with JSON Pointers\n";
            std::cout << "---------------------------------------\n";

            Document doc{};

            doc.set<std::string>( "/users/0/name", "Alice" );
            doc.set<int64_t>( "/users/0/age", 25 );
            doc.set<std::string>( "/users/0/role", "Developer" );

            doc.set<std::string>( "/users/1/name", "Bob" );
            doc.set<int64_t>( "/users/1/age", 30 );
            doc.set<std::string>( "/users/1/role", "Manager" );

            doc.set<std::string>( "/users/2/name", "Charlie" );
            doc.set<int64_t>( "/users/2/age", 35 );
            doc.set<std::string>( "/users/2/role", "Architect" );

            doc.set<double>( "/scores/0", 95.5 );
            doc.set<double>( "/scores/1", 87.3 );
            doc.set<double>( "/scores/2", 92.8 );
            doc.set<double>( "/scores/3", 88.1 );

            std::cout << "Document with arrays:\n";
            std::cout << doc.toString( 2 ) << "\n\n";

            std::cout << "Accessing array elements:\n";
            for( int i{}; i < 3; ++i )
            {
                std::string namePath{ "/users/" + std::to_string( i ) + "/name" };
                std::string agePath{ "/users/" + std::to_string( i ) + "/age" };
                std::string rolePath{ "/users/" + std::to_string( i ) + "/role" };

                auto name{ doc.get<std::string>( namePath ) };
                auto age{ doc.get<int64_t>( agePath ) };
                auto role{ doc.get<std::string>( rolePath ) };

                if( name && age && role )
                {
                    std::cout << "User " << i << ": " << *name << " (age " << *age << ", " << *role << ")\n";
                }
            }

            std::cout << "\nScores array:\n";
            for( int i{}; i < 4; ++i )
            {
                std::string scorePath{ "/scores/" + std::to_string( i ) };
                auto score{ doc.get<double>( scorePath ) };
                if( score )
                {
                    std::cout << "Score " << i << ": " << *score << "\n";
                }
            }
        }

        std::cout << "\n";

        //=====================================================================
        // 4. Escaped Characters in JSON Pointers
        //=====================================================================
        {
            std::cout << "4. Escaped Characters in JSON Pointers\n";
            std::cout << "---------------------------------------\n";

            Document doc{};

            doc.set<std::string>( "/field~1with~0tilde", "Contains / and ~ characters" );
            doc.set<std::string>( "/path~1to~1data", "Deep path with slashes" );
            doc.set<int64_t>( "/config~1api~1port", 8080 );

            std::cout << "Document with escaped characters:\n";
            std::cout << doc.toString( 2 ) << "\n\n";

            std::cout << "Reading escaped fields:\n";
            std::cout << "Field with / and ~: " << doc.get<std::string>( "/field~1with~0tilde" ).value_or( "N/A" )
                      << "\n";
            std::cout << "Path data: " << doc.get<std::string>( "/path~1to~1data" ).value_or( "N/A" ) << "\n";
            std::cout << "API Port: " << doc.get<int64_t>( "/config~1api~1port" ).value_or( 0 ) << "\n";

            std::cout << "\nNote: RFC 6901 escape sequences:\n";
            std::cout << "  ~0 represents ~ (tilde)\n";
            std::cout << "  ~1 represents / (forward slash)\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 5. JSON Pointer vs Dot Notation Compatibility
        //=====================================================================
        {
            std::cout << "5. JSON Pointer vs Dot Notation Compatibility\n";
            std::cout << "----------------------------------------------\n";

            Document doc{};

            doc.set<std::string>( "app.name", "MyApplication" );
            doc.set<int64_t>( "app.version.major", 2 );
            doc.set<int64_t>( "app.version.minor", 1 );

            std::cout << "Values set with dot notation, accessed with JSON Pointers:\n";
            std::cout << "App name: " << doc.get<std::string>( "/app/name" ).value_or( "Unknown" ) << "\n";
            std::cout << "Major version: " << doc.get<int64_t>( "/app/version/major" ).value_or( 0 ) << "\n";
            std::cout << "Minor version: " << doc.get<int64_t>( "/app/version/minor" ).value_or( 0 ) << "\n\n";

            doc.set<std::string>( "/database/host", "localhost" );
            doc.set<int64_t>( "/database/port", 5432 );
            doc.set<std::string>( "/database/name", "myapp_db" );

            std::cout << "Values set with JSON Pointers, accessed with dot notation:\n";
            std::cout << "DB host: " << doc.get<std::string>( "database.host" ).value_or( "Unknown" ) << "\n";
            std::cout << "DB port: " << doc.get<int64_t>( "database.port" ).value_or( 0 ) << "\n";
            std::cout << "DB name: " << doc.get<std::string>( "database.name" ).value_or( "Unknown" ) << "\n\n";

            std::cout << "Complete document:\n";
            std::cout << doc.toString( 2 ) << "\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 6. Error Handling and Validation
        //=====================================================================
        {
            std::cout << "6. Error Handling and Validation\n";
            std::cout << "--------------------------------\n";

            Document doc{};

            doc.set<std::string>( "/existing/field", "value" );
            doc.set<int64_t>( "/numbers/0", 10 );
            doc.set<int64_t>( "/numbers/1", 20 );

            std::cout << "Test document:\n";
            std::cout << doc.toString( 2 ) << "\n\n";

            std::cout << "Error handling tests:\n";

            std::cout << "Invalid pointer 'no-leading-slash': "
                      << ( doc.contains( "no-leading-slash" ) ? "Found" : "Not found" ) << "\n";

            std::cout << "Non-existent field '/missing/field': "
                      << ( doc.contains( "/missing/field" ) ? "Found" : "Not found" ) << "\n";

            std::cout << "Out of bounds '/numbers/5': " << ( doc.contains( "/numbers/5" ) ? "Found" : "Not found" )
                      << "\n";

            auto wrongType{ doc.get<int64_t>( "/existing/field" ) };
            std::cout << "String field accessed as int: "
                      << ( wrongType.has_value() ? std::to_string( *wrongType ) : "No value (correct)" ) << "\n";

            std::cout << "Invalid array index '/numbers/01': "
                      << ( doc.contains( "/numbers/01" ) ? "Found" : "Not found (correct)" ) << "\n";
            std::cout << "Non-numeric array index '/numbers/abc': "
                      << ( doc.contains( "/numbers/abc" ) ? "Found" : "Not found (correct)" ) << "\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 7. Real-World API Response Processing
        //=====================================================================
        {
            std::cout << "7. Real-World API Response Processing\n";
            std::cout << "--------------------------------------\n";

            Document response{};

            response.set<std::string>( "/status", "success" );
            response.set<int64_t>( "/code", 200 );
            response.set<std::string>( "/timestamp", "2025-10-03T14:30:00Z" );

            response.set<std::string>( "/data/user/id", "usr_12345" );
            response.set<std::string>( "/data/user/email", "john.doe@example.com" );
            response.set<std::string>( "/data/user/profile/firstName", "John" );
            response.set<std::string>( "/data/user/profile/lastName", "Doe" );
            response.set<bool>( "/data/user/profile/verified", true );

            response.set<std::string>( "/data/user/permissions/0/resource", "users" );
            response.set<std::string>( "/data/user/permissions/0/actions/0", "read" );
            response.set<std::string>( "/data/user/permissions/0/actions/1", "write" );

            response.set<std::string>( "/data/user/permissions/1/resource", "posts" );
            response.set<std::string>( "/data/user/permissions/1/actions/0", "read" );
            response.set<std::string>( "/data/user/permissions/1/actions/1", "write" );
            response.set<std::string>( "/data/user/permissions/1/actions/2", "delete" );

            response.set<double>( "/metadata/version", 2.1 );
            response.set<std::string>( "/metadata/server", "api-server-01" );
            response.set<int64_t>( "/metadata/processingTime", 45 );

            std::cout << "API Response:\n";
            std::cout << response.toString( 2 ) << "\n\n";

            std::cout << "Processing API response:\n";

            auto status{ response.get<std::string>( "/status" ) };
            auto code{ response.get<int64_t>( "/code" ) };

            if( status && code && *status == "success" && *code == 200 )
            {
                std::cout << "Request successful\n\n";

                auto userId{ response.get<std::string>( "/data/user/id" ) };
                auto firstName{ response.get<std::string>( "/data/user/profile/firstName" ) };
                auto lastName{ response.get<std::string>( "/data/user/profile/lastName" ) };
                auto verified{ response.get<bool>( "/data/user/profile/verified" ) };

                if( userId && firstName && lastName )
                {
                    std::cout << "User: " << *firstName << " " << *lastName << " (ID: " << *userId << ")\n";
                    std::cout << "Verified: " << ( verified.value_or( false ) ? "Yes" : "No" ) << "\n";
                }

                std::cout << "\nPermissions:\n";
                for( int i{}; i < 10; ++i )
                {
                    std::string resourcePath{ "/data/user/permissions/" + std::to_string( i ) + "/resource" };
                    auto resource{ response.get<std::string>( resourcePath ) };

                    if( !resource )
                        break;

                    std::cout << "- " << *resource << ": ";

                    int actionCount{};
                    for( int j{}; j < 10; ++j )
                    {
                        std::string actionPath{ "/data/user/permissions/" + std::to_string( i ) + "/actions/" +
                                                std::to_string( j ) };
                        if( response.contains( actionPath ) )
                        {
                            actionCount++;
                        }
                        else
                        {
                            break;
                        }
                    }
                    std::cout << actionCount << " action(s)\n";
                }

                auto version{ response.get<double>( "/metadata/version" ) };
                auto server{ response.get<std::string>( "/metadata/server" ) };
                auto processingTime{ response.get<int64_t>( "/metadata/processingTime" ) };

                std::cout << "\nMetadata:\n";
                if( version )
                    std::cout << "API Version: " << *version << "\n";
                if( server )
                    std::cout << "Server: " << *server << "\n";
                if( processingTime )
                    std::cout << "Processing Time: " << *processingTime << "ms\n";
            }
            else
            {
                std::cout << "Request failed\n";
            }
        }

        std::cout << "\n";
    }
    catch( const std::exception& e )
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
