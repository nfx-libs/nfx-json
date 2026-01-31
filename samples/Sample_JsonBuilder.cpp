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
 * @file Sample_JsonBuilder.cpp
 * @brief Demonstrates JSON Builder API for high-performance JSON construction
 * @details Real-world examples showing fluent API, method chaining, compact vs pretty printing,
 *          complex nested structures, and performance-optimized JSON generation
 */

#include <nfx/Json.h>

#include <iostream>
#include <chrono>
#include <iomanip>
#include <string>
#include <vector>

using namespace nfx::json;

// Simple timing utility
class Timer
{
public:
    Timer( const std::string& name )
        : m_name{ name },
          m_start{ std::chrono::high_resolution_clock::now() }
    {
    }

    ~Timer()
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( end - m_start );
        std::cout << "  " << m_name << ": " << duration.count() << " μs\n";
    }

private:
    std::string m_name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
};

int main()
{
    std::cout << "=== nfx-json Builder API ===\n\n";

    try
    {
        //=====================================================================
        // 1. Basic Object Construction
        //=====================================================================
        {
            std::cout << "1. Basic Object Construction\n";
            std::cout << "----------------------------\n";

            Builder builder;
            builder.writeStartObject()
                .write( "name", "Alice Johnson" )
                .write( "age", 28 )
                .write( "email", "alice@example.com" )
                .write( "active", true )
                .write( "balance", 1234.56 )
                .writeEndObject();

            std::string json = builder.toString();

            std::cout << "Compact JSON:\n" << json << "\n\n";

            // Verify by parsing
            auto doc = Document::fromString( json );
            if( doc.has_value() )
            {
                std::cout << "Verification:\n";
                std::cout << "  Name: " << doc->get<std::string>( "name" ).value_or( "N/A" ) << "\n";
                std::cout << "  Age: " << doc->get<int64_t>( "age" ).value_or( 0 ) << "\n";
                std::cout << "  Active: " << ( doc->get<bool>( "active" ).value_or( false ) ? "Yes" : "No" ) << "\n";
            }
        }

        std::cout << "\n";

        //=====================================================================
        // 2. Pretty Printing with Indentation
        //=====================================================================
        {
            std::cout << "2. Pretty Printing with Indentation\n";
            std::cout << "------------------------------------\n";

            Builder builder{ { 2 } }; // 2-space indentation

            builder.writeStartObject()
                .writeKey( "server" )
                .writeStartObject()
                .write( "host", "localhost" )
                .write( "port", 8080 )
                .write( "ssl", false )
                .writeEndObject()
                .writeKey( "database" )
                .writeStartObject()
                .write( "host", "db.example.com" )
                .write( "port", 5432 )
                .write( "name", "myapp_db" )
                .write( "poolSize", 50 )
                .writeEndObject()
                .writeKey( "features" )
                .writeStartObject()
                .write( "enableCache", true )
                .write( "enableMetrics", true )
                .writeEndObject()
                .writeEndObject();

            std::string json = builder.toString();

            std::cout << "Pretty-printed JSON (2-space indent):\n";
            std::cout << json << "\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 3. Building Arrays
        //=====================================================================
        {
            std::cout << "3. Building Arrays\n";
            std::cout << "------------------\n";

            // Simple array
            std::cout << "Simple number array:\n";
            Builder arrayBuilder;
            arrayBuilder.writeStartArray().write( 10 ).write( 20 ).write( 30 ).write( 40 ).write( 50 ).writeEndArray();

            std::cout << arrayBuilder.toString() << "\n\n";

            // Array of strings
            std::cout << "String array:\n";
            Builder stringArrayBuilder;
            stringArrayBuilder.writeStartArray()
                .write( "developer" )
                .write( "designer" )
                .write( "manager" )
                .writeEndArray();

            std::cout << stringArrayBuilder.toString() << "\n\n";

            // Array of objects
            std::cout << "Array of objects:\n";
            Builder objectArrayBuilder{ { 2 } }; // 2-space indentation
            objectArrayBuilder.writeStartArray()
                .writeStartObject()
                .write( "id", "usr_001" )
                .write( "name", "Alice" )
                .write( "role", "admin" )
                .writeEndObject()
                .writeStartObject()
                .write( "id", "usr_002" )
                .write( "name", "Bob" )
                .write( "role", "user" )
                .writeEndObject()
                .writeStartObject()
                .write( "id", "usr_003" )
                .write( "name", "Charlie" )
                .write( "role", "moderator" )
                .writeEndObject()
                .writeEndArray();

            std::cout << objectArrayBuilder.toString() << "\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 4. Complex Nested Structures
        //=====================================================================
        {
            std::cout << "4. Complex Nested Structures\n";
            std::cout << "-----------------------------\n";

            Builder builder{ { 2 } }; // 2-space indentation

            builder.writeStartObject()
                .writeKey( "user" )
                .writeStartObject()
                .write( "id", 12345 )
                .writeKey( "profile" )
                .writeStartObject()
                .write( "firstName", "Alice" )
                .write( "lastName", "Johnson" )
                .write( "email", "alice.johnson@example.com" )
                .write( "age", 28 )
                .writeEndObject()
                .writeKey( "preferences" )
                .writeStartObject()
                .write( "theme", "dark" )
                .write( "notifications", true )
                .write( "language", "en-US" )
                .writeEndObject()
                .writeKey( "roles" )
                .writeStartArray()
                .write( "user" )
                .write( "moderator" )
                .writeEndArray()
                .writeEndObject()
                .writeKey( "permissions" )
                .writeStartArray()
                .writeStartObject()
                .write( "resource", "posts" )
                .writeKey( "actions" )
                .writeStartArray()
                .write( "read" )
                .write( "write" )
                .write( "delete" )
                .writeEndArray()
                .writeEndObject()
                .writeStartObject()
                .write( "resource", "comments" )
                .writeKey( "actions" )
                .writeStartArray()
                .write( "read" )
                .write( "write" )
                .writeEndArray()
                .writeEndObject()
                .writeEndArray()
                .writeEndObject();

            std::string json = builder.toString();

            std::cout << "Complex nested structure:\n";
            std::cout << json << "\n\n";

            // Verify structure
            auto doc = Document::fromString( json );
            if( doc.has_value() )
            {
                std::cout << "Verification:\n";
                std::cout << "  User ID: " << doc->get<int64_t>( "user.id" ).value_or( 0 ) << "\n";
                std::cout << "  First name: " << doc->get<std::string>( "user.profile.firstName" ).value_or( "N/A" )
                          << "\n";
                std::cout << "  Theme: " << doc->get<std::string>( "user.preferences.theme" ).value_or( "N/A" ) << "\n";
                std::cout << "  Roles count: " << doc->get<Array>( "user.roles" ).value_or( Array{} ).size() << "\n";
                std::cout << "  Permissions count: " << doc->get<Array>( "permissions" ).value_or( Array{} ).size()
                          << "\n";
            }
        }

        std::cout << "\n";

        //=====================================================================
        // 5. Dynamic Content Generation
        //=====================================================================
        {
            std::cout << "5. Dynamic Content Generation\n";
            std::cout << "------------------------------\n";

            std::cout << "Generating sales report for 5 days...\n";

            Builder builder{ { 2 } }; // 2-space indentation

            builder.writeStartObject()
                .write( "reportId", "RPT-2025-001" )
                .write( "generatedAt", "2025-01-29T14:30:00Z" )
                .write( "title", "Weekly Sales Report" )
                .writeKey( "salesData" )
                .writeStartArray();

            // Generate sales data dynamically
            for( int day = 1; day <= 5; ++day )
            {
                builder.writeStartObject()
                    .write( "date", "2025-01-" + std::to_string( 24 + day ) )
                    .write( "revenue", 1000.0 + ( day * 150.5 ) )
                    .write( "orders", 10 + ( day * 3 ) )
                    .write( "avgOrderValue", 95.50 + ( day * 2.5 ) )
                    .writeEndObject();
            }

            builder.writeEndArray()
                .writeKey( "summary" )
                .writeStartObject()
                .write( "totalRevenue", 6377.50 )
                .write( "totalOrders", 80 )
                .write( "averageOrderValue", 102.75 )
                .writeEndObject()
                .writeEndObject();

            std::string json = builder.toString();

            std::cout << "\nGenerated report:\n";
            std::cout << json << "\n\n";

            // Verify
            auto doc = Document::fromString( json );
            if( doc.has_value() )
            {
                std::cout << "Report verification:\n";
                std::cout << "  Report ID: " << doc->get<std::string>( "reportId" ).value_or( "N/A" ) << "\n";
                std::cout << "  Sales records: " << doc->get<Array>( "salesData" ).value_or( Array{} ).size()
                          << " days\n";
                std::cout << "  Total revenue: $" << doc->get<double>( "summary.totalRevenue" ).value_or( 0.0 ) << "\n";
            }
        }

        std::cout << "\n";

        //=====================================================================
        // 6. String Escaping and Special Characters
        //=====================================================================
        {
            std::cout << "6. String Escaping and Special Characters\n";
            std::cout << "------------------------------------------\n";

            Builder builder{ { 2 } }; // 2-space indentation

            builder.writeStartObject()
                .write( "quotes", "He said \"Hello World\"" )
                .write( "backslash", "Path: C:\\Users\\Alice\\Documents" )
                .write( "newline", "Line 1\nLine 2\nLine 3" )
                .write( "tab", "Column1\tColumn2\tColumn3" )
                .write( "unicode", "Emoji: 🚀 Unicode: \u00E9" )
                .write( "mixed", "Mix: \"quotes\"\n\\backslash\\\t<tab>" )
                .writeEndObject();

            std::string json = builder.toString();

            std::cout << "JSON with escaped special characters:\n";
            std::cout << json << "\n\n";

            // Verify proper escaping by round-trip
            auto doc = Document::fromString( json );
            if( doc.has_value() )
            {
                std::cout << "Round-trip verification:\n";
                std::cout << "  Quotes: " << doc->get<std::string>( "quotes" ).value_or( "N/A" ) << "\n";
                std::cout << "  Backslash: " << doc->get<std::string>( "backslash" ).value_or( "N/A" ) << "\n";
                std::cout << "  [OK] All special characters properly escaped and restored\n";
            }
        }

        std::cout << "\n";

        //=====================================================================
        // 7. Null Values and Type Variety
        //=====================================================================
        {
            std::cout << "7. Null Values and Type Variety\n";
            std::cout << "--------------------------------\n";

            Builder builder{ { 2 } }; // 2-space indentation

            builder.writeStartObject()
                .write( "stringValue", "text" )
                .write( "intValue", 42 )
                .write( "uintValue", 100u )
                .write( "int64Value", 9223372036854775807LL )
                .write( "uint64Value", 18446744073709551615ULL )
                .write( "floatValue", 3.14f )
                .write( "doubleValue", 2.718281828 )
                .write( "trueValue", true )
                .write( "falseValue", false )
                .write( "nullValue", nullptr )
                .writeKey( "emptyArray" )
                .writeStartArray()
                .writeEndArray()
                .writeKey( "emptyObject" )
                .writeStartObject()
                .writeEndObject()
                .writeEndObject();

            std::string json = builder.toString();

            std::cout << "JSON with all value types:\n";
            std::cout << json << "\n\n";

            // Type verification
            auto doc = Document::fromString( json );
            if( doc.has_value() )
            {
                std::cout << "Type checks:\n";
                std::cout << "  stringValue is string: " << ( doc->is<std::string>( "stringValue" ) ? "YES" : "NO" )
                          << "\n";
                std::cout << "  intValue is int: " << ( doc->is<int>( "intValue" ) ? "YES" : "NO" ) << "\n";
                std::cout << "  doubleValue is double: " << ( doc->is<double>( "doubleValue" ) ? "YES" : "NO" ) << "\n";
                std::cout << "  trueValue is bool: " << ( doc->is<bool>( "trueValue" ) ? "YES" : "NO" ) << "\n";
                std::cout << "  nullValue is null: " << ( doc->isNull( "nullValue" ) ? "YES" : "NO" ) << "\n";
                std::cout << "  emptyArray is array: " << ( doc->is<Array>( "emptyArray" ) ? "YES" : "NO" ) << "\n";
                std::cout << "  emptyObject is object: " << ( doc->is<Object>( "emptyObject" ) ? "YES" : "NO" ) << "\n";
            }
        }

        std::cout << "\n";

        //=====================================================================
        // 8. Performance: Builder vs Document API
        //=====================================================================
        {
            std::cout << "8. Performance: Builder vs Document API\n";
            std::cout << "----------------------------------------\n";

            const int iterations = 1000;

            std::cout << "Generating medium-sized JSON " << iterations << " times...\n\n";

            // Benchmark Builder API
            std::string builderResult;
            {
                Timer timer( "Builder API" );

                for( int i = 0; i < iterations; ++i )
                {
                    Builder builder;
                    builder.writeStartObject()
                        .write( "id", i )
                        .write( "name", "Product " + std::to_string( i ) )
                        .write( "price", 99.99 + i )
                        .write( "inStock", i % 2 == 0 )
                        .writeKey( "tags" )
                        .writeStartArray()
                        .write( "electronics" )
                        .write( "featured" )
                        .writeEndArray()
                        .writeKey( "specifications" )
                        .writeStartObject()
                        .write( "weight", 1.5 + ( i * 0.1 ) )
                        .write( "dimensions", "10x20x5" )
                        .writeEndObject()
                        .writeEndObject();

                    builderResult = builder.toString();
                }
            }

            // Benchmark Document API
            std::string documentResult;
            {
                Timer timer( "Document API" );

                for( int i = 0; i < iterations; ++i )
                {
                    Document doc;
                    doc.set<int>( "id", i );
                    doc.set<std::string>( "name", "Product " + std::to_string( i ) );
                    doc.set<double>( "price", 99.99 + i );
                    doc.set<bool>( "inStock", i % 2 == 0 );

                    doc.set<Array>( "tags" );
                    doc.set<std::string>( "tags/0", "electronics" );
                    doc.set<std::string>( "tags/1", "featured" );

                    doc.set<double>( "specifications.weight", 1.5 + ( i * 0.1 ) );
                    doc.set<std::string>( "specifications.dimensions", "10x20x5" );

                    documentResult = doc.toString( 0 );
                }
            }

            std::cout << "\nLast generated JSON (Builder):\n";
            std::cout << builderResult.substr( 0, 150 ) << "...\n\n";

            std::cout << "[OK] Builder API provides faster JSON construction for streaming use cases\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 9. API Response Generation
        //=====================================================================
        {
            std::cout << "9. API Response Generation\n";
            std::cout << "--------------------------\n";

            Builder builder{ { 2 } }; // 2-space indentation

            builder.writeStartObject()
                .write( "status", "success" )
                .write( "code", 200 )
                .write( "timestamp", "2025-01-29T14:45:00Z" )
                .writeKey( "data" )
                .writeStartObject()
                .writeKey( "users" )
                .writeStartArray()
                .writeStartObject()
                .write( "id", 1 )
                .write( "username", "alice_dev" )
                .write( "email", "alice@example.com" )
                .write( "verified", true )
                .writeEndObject()
                .writeStartObject()
                .write( "id", 2 )
                .write( "username", "bob_admin" )
                .write( "email", "bob@example.com" )
                .write( "verified", true )
                .writeEndObject()
                .writeEndArray()
                .writeKey( "pagination" )
                .writeStartObject()
                .write( "page", 1 )
                .write( "perPage", 10 )
                .write( "total", 42 )
                .write( "hasNext", true )
                .writeEndObject()
                .writeEndObject()
                .writeKey( "meta" )
                .writeStartObject()
                .write( "requestId", "req_abc123xyz" )
                .write( "processingTime", 45.7 )
                .writeEndObject()
                .writeEndObject();

            std::string json = builder.toString();

            std::cout << "Generated API response:\n";
            std::cout << json << "\n\n";

            // Verify structure
            auto doc = Document::fromString( json );
            if( doc.has_value() )
            {
                std::cout << "Response verification:\n";
                std::cout << "  Status: " << doc->get<std::string>( "status" ).value_or( "N/A" ) << "\n";
                std::cout << "  Code: " << doc->get<int>( "code" ).value_or( 0 ) << "\n";
                std::cout << "  Users returned: " << doc->get<Array>( "data.users" ).value_or( Array{} ).size() << "\n";
                std::cout << "  Current page: " << doc->get<int>( "data.pagination.page" ).value_or( 0 ) << "\n";
                std::cout << "  Request ID: " << doc->get<std::string>( "meta.requestId" ).value_or( "N/A" ) << "\n";
            }
        }

        std::cout << "\n";

        //=====================================================================
        // 10. Configuration File Generation
        //=====================================================================
        {
            std::cout << "10. Configuration File Generation\n";
            std::cout << "----------------------------------\n";

            Builder builder{ { 2 } }; // 2-space indentation

            builder.writeStartObject()
                .writeKey( "app" )
                .writeStartObject()
                .write( "name", "MyApplication" )
                .write( "version", "2.1.0" )
                .write( "environment", "production" )
                .write( "debug", false )
                .writeEndObject()
                .writeKey( "server" )
                .writeStartObject()
                .write( "host", "0.0.0.0" )
                .write( "port", 8080 )
                .write( "ssl", true )
                .writeKey( "allowedHosts" )
                .writeStartArray()
                .write( "example.com" )
                .write( "*.example.com" )
                .write( "api.example.com" )
                .writeEndArray()
                .write( "maxConnections", 1000 )
                .write( "timeout", 30 )
                .writeEndObject()
                .writeKey( "database" )
                .writeStartObject()
                .write( "host", "prod-db.example.com" )
                .write( "port", 5432 )
                .write( "name", "myapp_prod" )
                .write( "user", "app_user" )
                .write( "poolSize", 50 )
                .write( "ssl", true )
                .writeEndObject()
                .writeKey( "logging" )
                .writeStartObject()
                .write( "level", "INFO" )
                .write( "format", "json" )
                .writeKey( "outputs" )
                .writeStartArray()
                .write( "stdout" )
                .write( "/var/log/app/app.log" )
                .writeEndArray()
                .write( "colorOutput", false )
                .writeEndObject()
                .writeKey( "features" )
                .writeStartObject()
                .write( "enableCache", true )
                .write( "enableMetrics", true )
                .write( "enableRateLimiting", true )
                .write( "cacheExpiry", 3600 )
                .writeEndObject()
                .writeEndObject();

            std::string json = builder.toString();

            std::cout << "Generated configuration file:\n";
            std::cout << json << "\n\n";

            // Verify
            auto doc = Document::fromString( json );
            if( doc.has_value() )
            {
                std::cout << "Configuration verification:\n";
                std::cout << "  App name: " << doc->get<std::string>( "app.name" ).value_or( "N/A" ) << "\n";
                std::cout << "  Environment: " << doc->get<std::string>( "app.environment" ).value_or( "N/A" ) << "\n";
                std::cout << "  Server port: " << doc->get<int>( "server.port" ).value_or( 0 ) << "\n";
                std::cout << "  Database: " << doc->get<std::string>( "database.host" ).value_or( "N/A" ) << ":"
                          << doc->get<int>( "database.port" ).value_or( 0 ) << "\n";
                std::cout << "  Allowed hosts: " << doc->get<Array>( "server.allowedHosts" ).value_or( Array{} ).size()
                          << "\n";
                std::cout << "  Cache enabled: "
                          << ( doc->get<bool>( "features.enableCache" ).value_or( false ) ? "Yes" : "No" ) << "\n";
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
