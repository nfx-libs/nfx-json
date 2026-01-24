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
 * @file Sample_JsonDocument.cpp
 * @brief Sample demonstrating JSON Document parsing and manipulation
 * @details Real-world examples covering JSON parsing, path-based access, array operations,
 *          nested object navigation, configuration files, and API response handling
 */

#include <nfx/Json.h>

#include <iostream>
#include <fstream>
#include <vector>

using namespace nfx::json;

int main()
{
    std::cout << "=== nfx-json Document Parsing and Manipulation ===\n\n";

    try
    {
        //=====================================================================
        // 1. Configuration File Management
        //=====================================================================
        {
            std::cout << "1. Configuration File Management\n";
            std::cout << "--------------------------------\n";

            Document config{};

            config.set<std::string>( "app.name", "MyApplication" );
            config.set<std::string>( "app.version", "1.2.3" );
            config.set<bool>( "app.debug", true );

            config.set<std::string>( "database.host", "localhost" );
            config.set<int64_t>( "database.port", 5432 );
            config.set<std::string>( "database.name", "myapp_db" );
            config.set<std::string>( "database.user", "admin" );

            config.set<Array>( "/server/allowedHosts" );
            config.set<std::string>( "/server/allowedHosts/0", "localhost" );
            config.set<std::string>( "/server/allowedHosts/1", "127.0.0.1" );
            config.set<std::string>( "/server/allowedHosts/2", "::1" );

            config.set<int64_t>( "/server/port", 8080 );
            config.set<int64_t>( "/server/maxConnections", 1000 );

            config.set<bool>( "features.enableCaching", true );
            config.set<bool>( "features.enableLogging", false );
            config.set<std::string>( "features.logLevel", "INFO" );

            std::cout << "Generated Configuration:\n";
            std::cout << config.toString( 2 ) << "\n\n";

            std::cout << "Reading configuration values:\n";
            std::cout << "App Name: " << config.get<std::string>( "app.name" ).value_or( "Unknown" ) << "\n";
            std::cout << "DB Port: " << config.get<int64_t>( "database.port" ).value_or( 0 ) << "\n";
            std::cout << "Debug Mode: " << ( config.get<bool>( "app.debug" ).value_or( false ) ? "ON" : "OFF" ) << "\n";
            std::cout << "Allowed Hosts: " << config.get<Array>( "/server/allowedHosts" ).value().size()
                      << " entries\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 2. API Response Processing
        //=====================================================================
        {
            std::cout << "2. API Response Processing\n";
            std::cout << "--------------------------\n";

            std::string apiResponse{ R"({
                "status": "success",
                "data": {
                    "user": {
                        "id": 12345,
                        "profile": {
                            "firstName": "Alice",
                            "lastName": "Johnson",
                            "email": "alice.johnson@example.com",
                            "age": 28,
                            "active": true
                        },
                        "preferences": {
                            "theme": "dark",
                            "notifications": true,
                            "language": "en-US"
                        },
                        "roles": ["user", "moderator"],
                        "lastLogin": "2025-10-02T10:30:00Z"
                    },
                    "permissions": [
                        {
                            "resource": "posts",
                            "actions": ["read", "write", "delete"]
                        },
                        {
                            "resource": "comments",
                            "actions": ["read", "write"]
                        }
                    ]
                }
            })" };

            auto maybeDoc{ Document::fromString( apiResponse ) };
            if( !maybeDoc.has_value() )
            {
                std::cout << "Failed to parse API response\n";
                return 1;
            }

            Document response{ maybeDoc.value() };

            std::cout << "API Response Status: " << response.get<std::string>( "status" ).value_or( "Unknown" ) << "\n";
            std::cout << "User ID: " << response.get<int64_t>( "data.user.id" ).value_or( 0 ) << "\n";
            std::cout << "Full Name: "
                      << response.get<std::string>( "data.user.profile.firstName" ).value_or( "Unknown" ) << " "
                      << response.get<std::string>( "data.user.profile.lastName" ).value_or( "Unknown" ) << "\n";
            std::cout << "Email: " << response.get<std::string>( "data.user.profile.email" ).value_or( "Unknown" )
                      << "\n";
            std::cout << "Theme: " << response.get<std::string>( "data.user.preferences.theme" ).value_or( "default" )
                      << "\n\n";

            std::cout << "Permissions:\n";
            auto permsOpt = response.get<Array>( "data.permissions" );
            if( permsOpt.has_value() )
            {
                auto& perms = permsOpt.value();
                for( size_t i{}; i < perms.size(); ++i )
                {
                    Document& perm = perms[i];
                    std::cout << "  Resource: " << perm.get<std::string>( "resource" ).value_or( "unknown" ) << "\n";
                    std::cout << "  Actions: " << perm.get<Array>( "actions" ).value().size() << " permissions\n";
                }
            }
        }

        std::cout << "\n";

        //=====================================================================
        // 3. Programmatic Document Construction
        //=====================================================================
        {
            std::cout << "3. Programmatic Document Construction\n";
            std::cout << "--------------------------------------\n";

            Document report{};

            report.set<std::string>( "reportId", "RPT-2025-001" );
            report.set<std::string>( "generatedAt", "2025-10-02T14:30:00Z" );
            report.set<std::string>( "title", "Monthly Sales Report" );

            report.set<double>( "summary.totalRevenue", 125478.50 );
            report.set<int64_t>( "summary.totalOrders", 1247 );
            report.set<double>( "summary.averageOrderValue", 100.62 );

            // Create sales array directly in the report document
            report.set<Array>( "/salesData" );

            for( int i{}; i < 5; ++i )
            {
                Document sale{};
                sale.set<std::string>( "date", "2025-09-" + std::to_string( 25 + i ) );
                sale.set<double>( "amount", 1250.00 + ( i * 50 ) );
                sale.set<int64_t>( "orderId", 1000 + i );

                // Access the array directly and push
                auto salesArrayOpt = report.get<Array>( "/salesData" );
                if( salesArrayOpt.has_value() )
                {
                    salesArrayOpt->push_back( sale );
                    // Update the array in the report
                    report.set<Array>( "/salesData", salesArrayOpt.value() );
                }
            }

            report.set<double>( "regions.north.revenue", 45000.00 );
            report.set<int64_t>( "regions.north.orders", 450 );
            report.set<double>( "regions.south.revenue", 38000.00 );
            report.set<int64_t>( "regions.south.orders", 380 );
            report.set<double>( "regions.east.revenue", 25000.00 );
            report.set<int64_t>( "regions.east.orders", 250 );
            report.set<double>( "regions.west.revenue", 17478.50 );
            report.set<int64_t>( "regions.west.orders", 167 );

            std::cout << "Generated Sales Report:\n";
            std::cout << report.toString( 2 ) << "\n\n";

            std::cout << "Report Summary:\n";
            std::cout << "Total Revenue: $" << report.get<double>( "summary.totalRevenue" ).value_or( 0.0 ) << "\n";
            std::cout << "Total Orders: " << report.get<int64_t>( "summary.totalOrders" ).value_or( 0 ) << "\n";
            std::cout << "Sales Records: " << report.get<Array>( "/salesData" ).value().size() << " entries\n";
            std::cout << "North Region: $" << report.get<double>( "regions.north.revenue" ).value_or( 0.0 ) << " ("
                      << report.get<int64_t>( "regions.north.orders" ).value_or( 0 ) << " orders)\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 4. Type Checking and Safe Access
        //=====================================================================
        {
            std::cout << "4. Type Checking and Safe Access\n";
            std::cout << "--------------------------------\n";

            Document doc{};

            doc.set<std::string>( "user.name", "Alice Johnson" );
            doc.set<int64_t>( "user.age", 28 );
            doc.set<double>( "user.height", 1.65 );
            doc.set<bool>( "user.active", true );
            doc.setNull( "user.spouse" );

            doc.set<std::string>( "user.address.city", "New York" );
            doc.set<std::string>( "user.address.country", "USA" );

            doc.set<Array>( "/user/hobbies" );
            doc.set<std::string>( "/user/hobbies/0", "reading" );
            doc.set<std::string>( "/user/hobbies/1", "gaming" );
            doc.set<std::string>( "/user/hobbies/2", "hiking" );

            std::cout << "Document structure:\n";
            std::cout << doc.toString( 2 ) << "\n\n";

            std::cout << "Type checking results:\n";
            std::cout << "user.name is string: " << ( doc.is<std::string>( "user.name" ) ? "YES" : "NO" ) << "\n";
            std::cout << "user.age is string: " << ( doc.is<std::string>( "user.age" ) ? "YES" : "NO" ) << "\n";
            std::cout << "user.age is integer: " << ( doc.is<int>( "user.age" ) ? "YES" : "NO" ) << "\n";
            std::cout << "user.height is integer: " << ( doc.is<int>( "user.height" ) ? "YES" : "NO" ) << "\n";
            std::cout << "user.height is double: " << ( doc.is<double>( "user.height" ) ? "YES" : "NO" ) << "\n";
            std::cout << "user.age is double: " << ( doc.is<double>( "user.age" ) ? "YES" : "NO" ) << "\n";
            std::cout << "user.active is boolean: " << ( doc.is<bool>( "user.active" ) ? "YES" : "NO" ) << "\n";
            std::cout << "user.name is boolean: " << ( doc.is<bool>( "user.name" ) ? "YES" : "NO" ) << "\n";
            std::cout << "user.spouse is null: " << ( doc.isNull( "user.spouse" ) ? "YES" : "NO" ) << "\n";
            std::cout << "user.name is null: " << ( doc.isNull( "user.name" ) ? "YES" : "NO" ) << "\n";
            std::cout << "user is object: " << ( doc.is<Object>( "user" ) ? "YES" : "NO" ) << "\n";
            std::cout << "user.address is object: " << ( doc.is<Object>( "user.address" ) ? "YES" : "NO" ) << "\n";
            std::cout << "user.name is object: " << ( doc.is<Object>( "user.name" ) ? "YES" : "NO" ) << "\n";
            std::cout << "user.hobbies is array: " << ( doc.is<Array>( "/user/hobbies" ) ? "YES" : "NO" ) << "\n";
            std::cout << "user.address is array: " << ( doc.is<Array>( "user.address" ) ? "YES" : "NO" ) << "\n\n";

            std::cout << "Safe data access using type checking:\n";

            if( auto name{ doc.get<std::string>( "user.name" ) } )
            {
                std::cout << "Name (safe): " << *name << "\n";
            }

            if( auto age{ doc.get<int64_t>( "user.age" ) } )
            {
                std::cout << "Age (safe): " << *age << " years\n";
            }

            if( auto height{ doc.get<double>( "user.height" ) } )
            {
                std::cout << "Height (safe): " << *height << "m\n";
            }

            if( auto active{ doc.get<bool>( "user.active" ) } )
            {
                std::cout << "Status (safe): " << ( *active ? "Active" : "Inactive" ) << "\n";
            }

            if( doc.isNull( "user.spouse" ) )
            {
                std::cout << "Spouse (safe): Not specified\n";
            }

            if( doc.is<Array>( "/user/hobbies" ) )
            {
                std::cout << "Hobbies (safe): " << doc.get<Array>( "/user/hobbies" ).value().size() << " items\n";
            }

            std::cout << "\nType checking with non-existent fields:\n";
            std::cout << "nonexistent.field is string: "
                      << ( doc.is<std::string>( "nonexistent.field" ) ? "YES" : "NO" ) << "\n";
            std::cout << "nonexistent.field is int: " << ( doc.is<int>( "nonexistent.field" ) ? "YES" : "NO" ) << "\n";
            std::cout << "nonexistent.field is object: " << ( doc.is<Object>( "nonexistent.field" ) ? "YES" : "NO" )
                      << "\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 5. Path Syntax Variations (Bracket Notation)
        //=====================================================================
        {
            std::cout << "5. Path Syntax Variations (Bracket Notation)\n";
            std::cout << "--------------------------------------------\n";

            Document doc{};

            // Create test data with arrays
            doc.set<Array>( "users" );
            doc.set<std::string>( "users[0].name", "Alice" );
            doc.set<int64_t>( "users[0].age", 30 );
            doc.set<std::string>( "users[1].name", "Bob" );
            doc.set<int64_t>( "users[1].age", 25 );
            doc.set<std::string>( "users[2].name", "Charlie" );
            doc.set<int64_t>( "users[2].age", 35 );

            doc.set<Array>( "matrix" );
            doc.set<Array>( "matrix[0]" );
            doc.set<int64_t>( "matrix[0][0]", 1 );
            doc.set<int64_t>( "matrix[0][1]", 2 );
            doc.set<int64_t>( "matrix[0][2]", 3 );
            doc.set<Array>( "matrix[1]" );
            doc.set<int64_t>( "matrix[1][0]", 4 );
            doc.set<int64_t>( "matrix[1][1]", 5 );
            doc.set<int64_t>( "matrix[1][2]", 6 );

            std::cout << "Document with bracket notation:\n";
            std::cout << doc.toString( 2 ) << "\n\n";

            std::cout << "Reading with bracket notation:\n";
            std::cout << "  users[0].name: " << doc.get<std::string>( "users[0].name" ).value_or( "N/A" ) << "\n";
            std::cout << "  users[1].age: " << doc.get<int64_t>( "users[1].age" ).value_or( 0 ) << "\n";
            std::cout << "  users[2].name: " << doc.get<std::string>( "users[2].name" ).value_or( "N/A" ) << "\n";
            std::cout << "  matrix[0][1]: " << doc.get<int64_t>( "matrix[0][1]" ).value_or( 0 ) << "\n";
            std::cout << "  matrix[1][2]: " << doc.get<int64_t>( "matrix[1][2]" ).value_or( 0 ) << "\n\n";

            std::cout << "Equivalent path syntaxes:\n";
            std::cout << "  Bracket:  users[0].name = " << doc.get<std::string>( "users[0].name" ).value_or( "" )
                      << "\n";
            std::cout << "  Dot:      users.0.name = " << doc.get<std::string>( "users.0.name" ).value_or( "" ) << "\n";
            std::cout << "  Pointer:  /users/0/name = " << doc.get<std::string>( "/users/0/name" ).value_or( "" )
                      << "\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 6. Error Handling and Validation
        //=====================================================================
        {
            std::cout << "6. Error Handling and Validation\n";
            std::cout << "--------------------------------\n";

            std::string invalidJson{ R"({
                "name": "Test",
                "value": 123,
                "missing":
            })" };

            auto maybeDoc{ Document::fromString( invalidJson ) };
            if( !maybeDoc.has_value() )
            {
                std::cout << "Successfully detected invalid JSON\n\n";
            }

            Document doc{};

            doc.set<std::string>( "config.database.host", "localhost" );
            doc.set<int64_t>( "config.database.port", 5432 );

            std::cout << "Validation Results:\n";
            std::cout << "  Has database config: " << ( doc.contains( "config.database.host" ) ? "YES" : "NO" ) << "\n";
            std::cout << "  Has missing field: " << ( doc.contains( "config.missing" ) ? "YES" : "NO" ) << "\n";
            std::cout << "  Document is valid: " << ( doc.isValid() ? "YES" : "NO" ) << "\n\n";

            std::cout << "Type Safety:\n";
            std::cout << "  String field as string: '"
                      << doc.get<std::string>( "config.database.host" ).value_or( "unknown" ) << "'\n";
            std::cout << "  String field as int: " << doc.get<int64_t>( "config.database.host" ).value_or( 0 )
                      << " (returns default 0)\n";
            std::cout << "  Int field as int: " << doc.get<int64_t>( "config.database.port" ).value_or( 0 ) << "\n";
            std::cout << "  Int field as string: '"
                      << doc.get<std::string>( "config.database.port" ).value_or( "unknown" ) << "' (returns empty)\n";
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
