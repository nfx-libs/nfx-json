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
 * @file Sample_JsonSchemaValidator.cpp
 * @brief Sample demonstrating JSON Schema validation functionality
 * @details Real-world examples showcasing JSON Schema validation including
 *          type validation, constraint checking, required field validation, and error reporting
 */

#include <nfx/Json.h>

#include <iostream>
#include <string>

using namespace nfx::json;

int main()
{
    std::cout << "=== nfx-json JSON Schema Validation Samples ===\n\n";

    try
    {
        //=====================================================================
        // 1. Basic Schema Validation
        //=====================================================================
        {
            std::cout << "1. Basic Schema Validation\n";
            std::cout << "--------------------------\n";

            std::string userSchemaJson{ R"({
                "type": "object",
                "properties": {
                    "name": {
                        "type": "string",
                        "minLength": 1,
                        "maxLength": 100
                    },
                    "age": {
                        "type": "integer",
                        "minimum": 0,
                        "maximum": 150
                    },
                    "email": {
                        "type": "string"
                    }
                },
                "required": ["name", "age"]
            })" };

            SchemaValidator validator{};
            bool schemaLoaded{ validator.load( userSchemaJson ) };

            std::cout << "Schema loaded: " << ( schemaLoaded ? "SUCCESS" : "FAILED" ) << "\n\n";

            std::cout << "Test Case 1: Valid User Data\n";

            Document validUser{};
            validUser.set<std::string>( "name", "Alice Johnson" );
            validUser.set<int64_t>( "age", 30 );
            validUser.set<std::string>( "email", "alice@example.com" );

            std::cout << "User data: " << validUser.toString( 2 ) << "\n";

            ValidationResult result1{ validator.validate( validUser ) };
            if( result1.isValid() )
            {
                std::cout << "[PASS] Validation PASSED - User data is valid\n";
            }
            else
            {
                std::cout << "[FAIL] Validation FAILED:\n";
                std::cout << result1.errorSummary() << "\n";
            }

            std::cout << "\nTest Case 2: Missing Required Field\n";

            Document invalidUser{};
            invalidUser.set<std::string>( "email", "bob@example.com" );

            std::cout << "User data: " << invalidUser.toString( 2 ) << "\n";

            ValidationResult result2{ validator.validate( invalidUser ) };
            if( result2.isValid() )
            {
                std::cout << "[PASS] Validation PASSED\n";
            }
            else
            {
                std::cout << "[FAIL] Validation FAILED (Expected):\n";
                std::cout << result2.errorSummary() << "\n";
            }

            std::cout << "\nTest Case 3: Type Mismatch\n";

            Document typeError{};
            typeError.set<std::string>( "name", "Charlie" );
            typeError.set<std::string>( "age", "thirty" );
            typeError.set<std::string>( "email", "charlie@example.com" );

            std::cout << "User data: " << typeError.toString( 2 ) << "\n";

            ValidationResult result3{ validator.validate( typeError ) };
            if( result3.isValid() )
            {
                std::cout << "[PASS] Validation PASSED\n";
            }
            else
            {
                std::cout << "[FAIL] Validation FAILED (Expected):\n";
                std::cout << result3.errorSummary() << "\n";
            }
        }

        std::cout << "\n";

        //=====================================================================
        // 2. Error Handling and Schema Information
        //=====================================================================
        {
            std::cout << "2. Error Handling and Schema Information\n";
            std::cout << "----------------------------------------\n";

            std::cout << "Validation Without Schema\n";

            SchemaValidator emptyValidator{};
            Document testDoc{};
            testDoc.set<std::string>( "test", "data" );

            try
            {
                ValidationResult result{ emptyValidator.validate( testDoc ) };
                std::cout << "Validation result without schema:\n";
                std::cout << "  Is valid: " << ( result.isValid() ? "Yes" : "No" ) << "\n";
                std::cout << "  Error count: " << result.errorCount() << "\n";
                if( !result.isValid() )
                {
                    std::cout << "  Error summary:\n";
                    std::cout << result.errorSummary() << "\n";
                }
                else
                {
                    std::cout << "  Unexpected: validation succeeded without schema\n";
                }
            }
            catch( const std::exception& e )
            {
                std::cout << "[PASS] Correctly caught exception: " << e.what() << "\n";
            }

            std::cout << "\nInvalid Schema Loading\n";

            std::string invalidSchema{ "{ invalid json }" };
            bool loaded{ emptyValidator.load( invalidSchema ) };
            std::cout << "Invalid schema load result: " << ( loaded ? "Loaded (unexpected)" : "Failed (correct)" )
                      << "\n";

            std::cout << "\nSchema Information\n";

            std::string infoSchemaJson{ R"({
                "$schema": "https://json-schema.org/draft/2020-12/schema",
                "title": "Demo Schema",
                "description": "A demonstration schema for testing",
                "type": "object",
                "properties": {
                    "name": { "type": "string" }
                }
            })" };

            SchemaValidator infoValidator{};
            infoValidator.load( infoSchemaJson );

            std::cout << "Schema version: '" << infoValidator.version() << "'\n";
            std::cout << "Schema title: '" << infoValidator.title() << "'\n";
            std::cout << "Schema description: '" << infoValidator.description() << "'\n";
            std::cout << "Has valid schema: " << ( infoValidator.hasSchema() ? "Yes" : "No" ) << "\n";
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
