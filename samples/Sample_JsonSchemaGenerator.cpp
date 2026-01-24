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
 * @file Sample_JsonSchemaGenerator.cpp
 * @brief Sample demonstrating JSON Schema generation functionality
 * @details Real-world examples showcasing JSON Schema generation from JSON data including
 *          type inference, format detection, and multi-sample analysis
 */

#include <nfx/Json.h>

#include <iostream>
#include <string>
#include <vector>

using namespace nfx::json;

int main()
{
    std::cout << "=== nfx-json JSON Schema Generator Samples ===\n\n";

    try
    {
        //=====================================================================
        // 1. Basic schema generation
        //=====================================================================
        {
            std::cout << "1. Basic schema generation\n";
            std::cout << "---------------------------\n";

            // Create a sample JSON document
            Document user;
            user.set<std::string>( "name", "Alice Johnson" );
            user.set<int64_t>( "age", 30 );
            user.set<bool>( "active", true );

            std::cout << "Input JSON:\n" << user.toString( 2 ) << "\n\n";

            // Generate schema - constructor does the work
            SchemaGenerator gen( user );

            std::cout << "Generated Schema:\n" << gen.schema().toString( 2 ) << "\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 2. Format detection
        //=====================================================================
        {
            std::cout << "2. Format detection\n";
            std::cout << "-------------------\n";

            Document data;
            data.set<std::string>( "email", "alice@example.com" );
            data.set<std::string>( "created", "2025-11-29T14:30:00Z" );
            data.set<std::string>( "website", "https://example.com" );
            data.set<std::string>( "id", "550e8400-e29b-41d4-a716-446655440000" );

            std::cout << "Input JSON:\n" << data.toString( 2 ) << "\n\n";

            SchemaGenerator::Options opts;
            opts.inferFormats = true;

            SchemaGenerator gen( data, opts );

            std::cout << "Generated Schema (with format detection):\n" << gen.schema().toString( 2 ) << "\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 3. Nested objects and arrays
        //=====================================================================
        {
            std::cout << "3. Nested objects and arrays\n";
            std::cout << "----------------------------\n";

            auto docOpt = Document::fromString( R"({
                "user": {
                    "name": "Bob",
                    "address": {
                        "city": "Seattle",
                        "zip": "98101"
                    }
                },
                "tags": ["developer", "admin"]
            })" );

            if( docOpt.has_value() )
            {
                std::cout << "Input JSON:\n" << docOpt->toString( 2 ) << "\n\n";

                SchemaGenerator gen( *docOpt );

                std::cout << "Generated Schema:\n" << gen.schema().toString( 2 ) << "\n";
            }
        }

        std::cout << "\n";

        //=====================================================================
        // 4. Schema with metadata
        //=====================================================================
        {
            std::cout << "4. Schema with metadata\n";
            std::cout << "-----------------------\n";

            Document config;
            config.set<std::string>( "host", "localhost" );
            config.set<int64_t>( "port", 8080 );
            config.set<bool>( "ssl", true );

            SchemaGenerator::Options opts;
            opts.title = "Server Configuration";
            opts.description = "Schema for server configuration settings";
            opts.id = "https://example.com/schemas/config.json";

            SchemaGenerator gen( config, opts );

            std::cout << "Generated Schema:\n" << gen.schema().toString( 2 ) << "\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 5. Generate from JSON string
        //=====================================================================
        {
            std::cout << "5. Generate from JSON string\n";
            std::cout << "----------------------------\n";

            std::string jsonInput = R"({"temperature": 23.5, "humidity": 65, "timestamp": "2025-11-29"})";

            std::cout << "Input JSON: " << jsonInput << "\n\n";

            auto docOpt = Document::fromString( jsonInput );
            if( docOpt.has_value() )
            {
                SchemaGenerator gen( *docOpt );
                std::cout << "Generated Schema:\n" << gen.schema().toString( 2 ) << "\n";
            }
            else
            {
                std::cout << "Failed to generate schema (invalid JSON)\n";
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
