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
 * @file Tests_JsonSchemaValidator.cpp
 * @brief Unit tests for JSON Schema validation functionality
 * @details Tests covering ValidationError, ValidationResult, and SchemaValidator classes
 *          with complete API coverage, error handling, and validation scenarios.
 */

#include <gtest/gtest.h>

#include <nfx/Json.h>

namespace nfx::json::test
{
    //=====================================================================
    // ValidationError tests
    //=====================================================================

    //----------------------------------------------
    // ValidationError test fixture
    //----------------------------------------------

    class ValidationErrorTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            // Create test error data
            errorEntry = { "user.email", "Invalid email format", "format", "valid email", "invalid@" };
        }

        ValidationError::ErrorEntry errorEntry;
    };

    TEST_F( ValidationErrorTest, ConstructorFromErrorEntry )
    {
        ValidationError error( errorEntry );

        EXPECT_EQ( error.path(), "user.email" );
        EXPECT_EQ( error.message(), "Invalid email format" );
        EXPECT_EQ( error.constraint(), "format" );
        EXPECT_EQ( error.expectedValue(), "valid email" );
        EXPECT_EQ( error.actualValue(), "invalid@" );
    }

    TEST_F( ValidationErrorTest, ConstructorWithParameters )
    {
        ValidationError error( "user.age", "Value too low", "minimum", "18", "16" );

        EXPECT_EQ( error.path(), "user.age" );
        EXPECT_EQ( error.message(), "Value too low" );
        EXPECT_EQ( error.constraint(), "minimum" );
        EXPECT_EQ( error.expectedValue(), "18" );
        EXPECT_EQ( error.actualValue(), "16" );
    }

    TEST_F( ValidationErrorTest, ConstructorWithOptionalParameters )
    {
        ValidationError error( "user.name", "Required field missing", "required" );

        EXPECT_EQ( error.path(), "user.name" );
        EXPECT_EQ( error.message(), "Required field missing" );
        EXPECT_EQ( error.constraint(), "required" );
        EXPECT_EQ( error.expectedValue(), "" );
        EXPECT_EQ( error.actualValue(), "" );
    }

    TEST_F( ValidationErrorTest, ToStringFormatting )
    {
        ValidationError error( "user.email", "Invalid format", "format", "email", "not-email" );
        std::string errorStr = error.toString();

        EXPECT_NE( errorStr.find( "user.email" ), std::string::npos );
        EXPECT_NE( errorStr.find( "Invalid format" ), std::string::npos );
        EXPECT_NE( errorStr.find( "format" ), std::string::npos );
        EXPECT_NE( errorStr.find( "email" ), std::string::npos );
        EXPECT_NE( errorStr.find( "not-email" ), std::string::npos );
    }

    TEST_F( ValidationErrorTest, ToStringWithoutOptionalValues )
    {
        ValidationError error( "user.name", "Field missing", "required" );
        std::string errorStr = error.toString();

        EXPECT_NE( errorStr.find( "user.name" ), std::string::npos );
        EXPECT_NE( errorStr.find( "Field missing" ), std::string::npos );
        EXPECT_NE( errorStr.find( "required" ), std::string::npos );
    }

    //=====================================================================
    // ValidationResult tests
    //=====================================================================

    //----------------------------------------------
    // ValidationResult test fixture
    //----------------------------------------------

    class ValidationResultTest : public ::testing::Test
    {
    protected:
        ValidationError error1{ "user.name", "Required field missing", "required", "name", "undefined" };
        ValidationError error2{ "user.age", "Type mismatch", "type", "integer", "string" };
        ValidationError error3{ "user.email", "Invalid format", "format", "email", "invalid" };
    };

    TEST_F( ValidationResultTest, DefaultConstructorCreatesValidResult )
    {
        ValidationResult result;

        EXPECT_TRUE( result.isValid() );
        EXPECT_FALSE( result.hasErrors() );
        EXPECT_EQ( result.errorCount(), 0 );
        EXPECT_TRUE( result.errors().empty() );
    }

    TEST_F( ValidationResultTest, ConstructorWithErrors )
    {
        std::vector<ValidationError> errors = { error1, error2 };
        ValidationResult result( errors );

        EXPECT_FALSE( result.isValid() );
        EXPECT_TRUE( result.hasErrors() );
        EXPECT_EQ( result.errorCount(), 2 );
        EXPECT_EQ( result.errors().size(), 2 );
    }

    TEST_F( ValidationResultTest, AddErrorFromErrorEntry )
    {
        ValidationResult result;
        ValidationError::ErrorEntry entry = { "test.path", "Test message", "test", "expected", "actual" };

        result.addError( entry );

        EXPECT_FALSE( result.isValid() );
        EXPECT_EQ( result.errorCount(), 1 );
        EXPECT_EQ( result.error( 0 ).path(), "test.path" );
    }

    TEST_F( ValidationResultTest, AddErrorFromValidationError )
    {
        ValidationResult result;

        result.addError( error1 );

        EXPECT_FALSE( result.isValid() );
        EXPECT_EQ( result.errorCount(), 1 );
        EXPECT_EQ( result.error( 0 ).path(), "user.name" );
    }

    TEST_F( ValidationResultTest, AddErrorWithParameters )
    {
        ValidationResult result;

        result.addError( "user.score", "Value out of range", "maximum", "100", "150" );

        EXPECT_FALSE( result.isValid() );
        EXPECT_EQ( result.errorCount(), 1 );
        EXPECT_EQ( result.error( 0 ).path(), "user.score" );
        EXPECT_EQ( result.error( 0 ).message(), "Value out of range" );
    }

    TEST_F( ValidationResultTest, MultipleErrors )
    {
        ValidationResult result;

        result.addError( error1 );
        result.addError( error2 );
        result.addError( error3 );

        EXPECT_FALSE( result.isValid() );
        EXPECT_TRUE( result.hasErrors() );
        EXPECT_EQ( result.errorCount(), 3 );

        EXPECT_EQ( result.error( 0 ).path(), "user.name" );
        EXPECT_EQ( result.error( 1 ).path(), "user.age" );
        EXPECT_EQ( result.error( 2 ).path(), "user.email" );
    }

    TEST_F( ValidationResultTest, ErrorAccessOutOfRange )
    {
        ValidationResult result;
        result.addError( error1 );

        EXPECT_THROW( result.error( 1 ), std::out_of_range );
        EXPECT_THROW( result.error( 10 ), std::out_of_range );
    }

    TEST_F( ValidationResultTest, ErrorSummaryFormatting )
    {
        ValidationResult result;
        result.addError( error1 );
        result.addError( error2 );

        std::string summary = result.errorSummary();

        EXPECT_NE( summary.find( "2 error" ), std::string::npos );
        EXPECT_NE( summary.find( "user.name" ), std::string::npos );
        EXPECT_NE( summary.find( "user.age" ), std::string::npos );
        EXPECT_NE( summary.find( "1." ), std::string::npos );
        EXPECT_NE( summary.find( "2." ), std::string::npos );
    }

    TEST_F( ValidationResultTest, ErrorSummaryForValidResult )
    {
        ValidationResult result;
        std::string summary = result.errorSummary();

        EXPECT_NE( summary.find( "No validation errors" ), std::string::npos );
    }

    //=====================================================================
    // SchemaValidator tests
    //=====================================================================

    //----------------------------------------------
    // SchemaValidator test fixture
    //----------------------------------------------

    class SchemaValidatorTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            // Create basic test schema
            basicSchemaJson = R"({
                "$schema": "https://json-schema.org/draft/2020-12/schema",
                "title": "User Schema",
                "description": "A schema for user data validation",
                "type": "object",
                "properties": {
                    "name": {
                        "type": "string",
                        "minLength": 2,
                        "maxLength": 50
                    },
                    "age": {
                        "type": "integer",
                        "minimum": 0,
                        "maximum": 150
                    },
                    "email": {
                        "type": "string",
                        "format": "date-time"
                    },
                    "active": {
                        "type": "boolean"
                    }
                },
                "required": ["name", "age"],
                "additionalProperties": false
            })";

            auto maybeSchema = Document::fromString( basicSchemaJson );
            ASSERT_TRUE( maybeSchema.has_value() );
            basicSchema = std::move( maybeSchema.value() );

            // Create schema with references
            schemaWithRefsJson = R"({
                "$schema": "https://json-schema.org/draft/2020-12/schema",
                "title": "Organization Schema",
                "type": "object",
                "properties": {
                    "company": {
                        "$ref": "#/definitions/Company"
                    },
                    "employees": {
                        "type": "array",
                        "items": {
                            "$ref": "#/definitions/Employee"
                        }
                    }
                },
                "definitions": {
                    "Company": {
                        "type": "object",
                        "properties": {
                            "name": { "type": "string" },
                            "founded": { "type": "integer" }
                        },
                        "required": ["name"]
                    },
                    "Employee": {
                        "type": "object",
                        "properties": {
                            "id": { "type": "integer" },
                            "name": { "type": "string" },
                            "department": { "type": "string" }
                        },
                        "required": ["id", "name"]
                    }
                }
            })";

            auto maybeRefSchema = Document::fromString( schemaWithRefsJson );
            ASSERT_TRUE( maybeRefSchema.has_value() );
            schemaWithRefs = std::move( maybeRefSchema.value() );

            // Create valid test document
            validDocumentJson = R"({
                "name": "John Doe",
                "age": 30,
                "email": "2023-10-03T14:30:00Z",
                "active": true
            })";

            auto maybeValidDoc = Document::fromString( validDocumentJson );
            ASSERT_TRUE( maybeValidDoc.has_value() );
            validDocument = std::move( maybeValidDoc.value() );

            // Create invalid test document
            invalidDocumentJson = R"({
                "name": "A",
                "age": "not-a-number",
                "email": "invalid-date",
                "active": "not-boolean",
                "extra": "property"
            })";

            auto maybeInvalidDoc = Document::fromString( invalidDocumentJson );
            ASSERT_TRUE( maybeInvalidDoc.has_value() );
            invalidDocument = std::move( maybeInvalidDoc.value() );
        }

        std::string basicSchemaJson;
        std::string schemaWithRefsJson;
        std::string validDocumentJson;
        std::string invalidDocumentJson;

        Document basicSchema;
        Document schemaWithRefs;
        Document validDocument;
        Document invalidDocument;
    };

    //----------------------------------------------
    // Construction
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, DefaultConstructor )
    {
        SchemaValidator validator;

        EXPECT_FALSE( validator.hasSchema() );
        EXPECT_EQ( validator.version(), "" );
        EXPECT_EQ( validator.title(), "" );
        EXPECT_EQ( validator.description(), "" );
        EXPECT_FALSE( validator.options().strictMode );
        EXPECT_EQ( validator.options().maxDepth, 64 ); // Default depth limit for circular $ref protection
    }

    TEST_F( SchemaValidatorTest, ConstructorWithSchema )
    {
        SchemaValidator validator( basicSchema );

        EXPECT_TRUE( validator.hasSchema() );
        EXPECT_EQ( validator.title(), "User Schema" );
        EXPECT_EQ( validator.description(), "A schema for user data validation" );
    }

    TEST_F( SchemaValidatorTest, ConstructorWithOptions )
    {
        SchemaValidator::Options opts;
        opts.strictMode = true;
        opts.maxDepth = 10;

        SchemaValidator validator( basicSchema, opts );

        EXPECT_TRUE( validator.hasSchema() );
        EXPECT_EQ( validator.title(), "User Schema" );
        EXPECT_TRUE( validator.options().strictMode );
        EXPECT_EQ( validator.options().maxDepth, 10 );
    }

    TEST_F( SchemaValidatorTest, CopyConstructor )
    {
        SchemaValidator original( basicSchema, { .strictMode = true, .maxDepth = 10 } );

        SchemaValidator copy( original );

        EXPECT_TRUE( copy.hasSchema() );
        EXPECT_EQ( copy.title(), original.title() );
        EXPECT_EQ( copy.options().strictMode, original.options().strictMode );
        EXPECT_EQ( copy.options().maxDepth, original.options().maxDepth );
    }

    TEST_F( SchemaValidatorTest, MoveConstructor )
    {
        SchemaValidator original( basicSchema );
        std::string expectedTitle = original.title();

        SchemaValidator moved( std::move( original ) );

        EXPECT_TRUE( moved.hasSchema() );
        EXPECT_EQ( moved.title(), expectedTitle );
        EXPECT_FALSE( original.hasSchema() ); // Original should be moved from
    }

    TEST_F( SchemaValidatorTest, CopyAssignment )
    {
        SchemaValidator validator1( basicSchema );
        SchemaValidator validator2;

        validator2 = validator1;

        EXPECT_TRUE( validator2.hasSchema() );
        EXPECT_EQ( validator2.title(), validator1.title() );
    }

    TEST_F( SchemaValidatorTest, MoveAssignment )
    {
        SchemaValidator validator1( basicSchema );
        std::string expectedTitle = validator1.title();
        SchemaValidator validator2;

        validator2 = std::move( validator1 );

        EXPECT_TRUE( validator2.hasSchema() );
        EXPECT_EQ( validator2.title(), expectedTitle );
        EXPECT_FALSE( validator1.hasSchema() ); // Original should be moved from
    }

    //----------------------------------------------
    // Schema management
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, LoadSchemaFromDocument )
    {
        SchemaValidator validator;

        EXPECT_TRUE( validator.load( basicSchema ) );
        EXPECT_TRUE( validator.hasSchema() );
        EXPECT_EQ( validator.title(), "User Schema" );
    }

    TEST_F( SchemaValidatorTest, LoadSchemaFromString )
    {
        SchemaValidator validator;

        EXPECT_TRUE( validator.load( basicSchemaJson ) );
        EXPECT_TRUE( validator.hasSchema() );
        EXPECT_EQ( validator.title(), "User Schema" );
    }

    TEST_F( SchemaValidatorTest, LoadSchemaFromInvalidString )
    {
        SchemaValidator validator;
        std::string invalidJson = "{ invalid json }";

        EXPECT_FALSE( validator.load( invalidJson ) );
        EXPECT_FALSE( validator.hasSchema() );
    }

    TEST_F( SchemaValidatorTest, LoadInvalidSchema )
    {
        SchemaValidator validator;

        // Test with a non-object schema (JSON Schema must be an object)
        auto arraySchemaOpt = Document::fromString( "[]" );
        ASSERT_TRUE( arraySchemaOpt.has_value() );
        Document arraySchema = arraySchemaOpt.value();

        EXPECT_FALSE( validator.load( arraySchema ) );
        EXPECT_FALSE( validator.hasSchema() );

        // Test with a string schema (JSON Schema must be an object)
        auto stringSchemaOpt = Document::fromString( "\"not a schema\"" );
        ASSERT_TRUE( stringSchemaOpt.has_value() );
        Document stringSchema = stringSchemaOpt.value();

        EXPECT_FALSE( validator.load( stringSchema ) );
        EXPECT_FALSE( validator.hasSchema() );

        // Test with number schema (JSON Schema must be an object)
        auto numberSchemaOpt = Document::fromString( "42" );
        ASSERT_TRUE( numberSchemaOpt.has_value() );
        Document numberSchema = numberSchemaOpt.value();

        EXPECT_FALSE( validator.load( numberSchema ) );
        EXPECT_FALSE( validator.hasSchema() );
    }

    TEST_F( SchemaValidatorTest, LoadValidEmptySchema )
    {
        SchemaValidator validator;

        // Empty object is a valid JSON Schema (matches anything)
        Document emptySchema; // Default-constructed = {}

        EXPECT_TRUE( validator.load( emptySchema ) );
        EXPECT_TRUE( validator.hasSchema() );

        // Also test with explicit empty object
        auto explicitEmptyOpt = Document::fromString( "{}" );
        ASSERT_TRUE( explicitEmptyOpt.has_value() );
        Document explicitEmpty = explicitEmptyOpt.value();

        validator.clear();
        EXPECT_TRUE( validator.load( explicitEmpty ) );
        EXPECT_TRUE( validator.hasSchema() );
    }

    TEST_F( SchemaValidatorTest, LoadSchemaFromInvalidJsonString )
    {
        SchemaValidator validator;

        // Test with empty string (not valid JSON)
        EXPECT_FALSE( validator.load( std::string_view{ "" } ) );
        EXPECT_FALSE( validator.hasSchema() );

        // Test with whitespace-only string (not valid JSON)
        EXPECT_FALSE( validator.load( std::string_view{ "   \n\t  " } ) );
        EXPECT_FALSE( validator.hasSchema() );

        // Test with malformed JSON
        EXPECT_FALSE( validator.load( std::string_view{ "{invalid json}" } ) );
        EXPECT_FALSE( validator.hasSchema() );

        // Test with valid JSON but non-object (should fail schema loading)
        EXPECT_FALSE( validator.load( std::string_view{ "\"string\"" } ) );
        EXPECT_FALSE( validator.hasSchema() );

        EXPECT_FALSE( validator.load( std::string_view{ "123" } ) );
        EXPECT_FALSE( validator.hasSchema() );

        EXPECT_FALSE( validator.load( std::string_view{ "[]" } ) );
        EXPECT_FALSE( validator.hasSchema() );
    }

    TEST_F( SchemaValidatorTest, ClearSchema )
    {
        SchemaValidator validator( basicSchema );
        EXPECT_TRUE( validator.hasSchema() );

        validator.clear();

        EXPECT_FALSE( validator.hasSchema() );
        EXPECT_EQ( validator.title(), "" );
        EXPECT_EQ( validator.description(), "" );
    }

    TEST_F( SchemaValidatorTest, GetSchema )
    {
        SchemaValidator validator( basicSchema );

        Document retrievedSchema = validator.schema();
        auto title = retrievedSchema.get<std::string>( "title" );
        EXPECT_TRUE( title.has_value() );
        EXPECT_EQ( title.value(), "User Schema" );
    }

    //----------------------------------------------
    // Schema information
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, SchemaVersion )
    {
        SchemaValidator validator( basicSchema );
        EXPECT_EQ( validator.version(), "https://json-schema.org/draft/2020-12/schema" );
    }

    TEST_F( SchemaValidatorTest, SchemaDraft )
    {
        SchemaValidator validator( basicSchema );
        EXPECT_EQ( validator.draft(), SchemaDraft::Draft202012 );
        EXPECT_EQ( validator.draftString(), "2020-12" );
    }

    TEST_F( SchemaValidatorTest, SchemaDraftUnknown )
    {
        auto schemaNoVersionOpt = Document::fromString( R"({"type": "object"})" );
        ASSERT_TRUE( schemaNoVersionOpt.has_value() );
        Document schemaNoVersion = schemaNoVersionOpt.value();

        SchemaValidator validator( schemaNoVersion );
        EXPECT_EQ( validator.draft(), SchemaDraft::Unknown );
        EXPECT_EQ( validator.draftString(), "" );
    }

    TEST_F( SchemaValidatorTest, SchemaTitle )
    {
        SchemaValidator validator( basicSchema );
        EXPECT_EQ( validator.title(), "User Schema" );
    }

    TEST_F( SchemaValidatorTest, SchemaDescription )
    {
        SchemaValidator validator( basicSchema );
        EXPECT_EQ( validator.description(), "A schema for user data validation" );
    }

    TEST_F( SchemaValidatorTest, SchemaInfoWithoutSchema )
    {
        SchemaValidator validator;

        EXPECT_EQ( validator.version(), "" );
        EXPECT_EQ( validator.title(), "" );
        EXPECT_EQ( validator.description(), "" );
    }

    //----------------------------------------------
    // Validation configuration
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, StrictModeConfiguration )
    {
        SchemaValidator defaultValidator;
        EXPECT_FALSE( defaultValidator.options().strictMode );

        SchemaValidator strictValidator( Document{}, { .strictMode = true } );
        EXPECT_TRUE( strictValidator.options().strictMode );
    }

    TEST_F( SchemaValidatorTest, MaxDepthConfiguration )
    {
        SchemaValidator defaultValidator;
        EXPECT_EQ( defaultValidator.options().maxDepth, 64 ); // Default depth limit

        SchemaValidator customValidator( Document{}, { .maxDepth = 10 } );
        EXPECT_EQ( customValidator.options().maxDepth, 10 );

        SchemaValidator unlimitedValidator( Document{}, { .maxDepth = 0 } );
        EXPECT_EQ( unlimitedValidator.options().maxDepth, 0 ); // 0 = unlimited
    }

    TEST_F( SchemaValidatorTest, MaxDepthExceeded )
    {
        // Schema with self-referencing definition (simulated deep nesting)
        Document circularSchema;
        circularSchema.set<std::string>( "type", "object" );
        circularSchema.set<std::string>( "properties.child.$ref", "#/definitions/Node" );
        circularSchema.set<std::string>( "definitions.Node.type", "object" );
        circularSchema.set<std::string>( "definitions.Node.properties.child.$ref", "#/definitions/Node" );

        SchemaValidator validator( circularSchema, { .maxDepth = 3 } ); // Very low limit to trigger quickly

        // Document with nested structure
        Document nestedDoc;
        nestedDoc.set<std::string>( "child.child.child.child.name", "too deep" );

        ValidationResult result = validator.validate( nestedDoc );

        // Should fail due to max depth exceeded
        EXPECT_FALSE( result.isValid() );
        EXPECT_GT( result.errorCount(), 0 );

        // Check that the error mentions max depth
        bool hasMaxDepthError = false;
        for( size_t i = 0; i < result.errorCount(); ++i )
        {
            if( result.error( i ).constraint() == "maxDepth" )
            {
                hasMaxDepthError = true;
                break;
            }
        }
        EXPECT_TRUE( hasMaxDepthError );
    }

    //----------------------------------------------
    // Basic validation
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, ValidateValidDocument )
    {
        SchemaValidator validator( basicSchema );

        ValidationResult result = validator.validate( validDocument );

        EXPECT_TRUE( result.isValid() );
        EXPECT_EQ( result.errorCount(), 0 );
    }

    TEST_F( SchemaValidatorTest, ValidateInvalidDocument )
    {
        SchemaValidator validator( basicSchema );

        ValidationResult result = validator.validate( invalidDocument );

        EXPECT_FALSE( result.isValid() );
        EXPECT_GT( result.errorCount(), 0 );
    }

    TEST_F( SchemaValidatorTest, ValidateWithoutSchema )
    {
        SchemaValidator validator;

        EXPECT_THROW( validator.validate( validDocument ), std::runtime_error );
    }

    TEST_F( SchemaValidatorTest, ValidateInvalidJsonDocument )
    {
        SchemaValidator validator( basicSchema );
        Document emptyDoc; // Default-constructed document = {} (valid JSON but invalid schema)

        ValidationResult result = validator.validate( emptyDoc );

        EXPECT_FALSE( result.isValid() );
        EXPECT_GT( result.errorCount(), 0 );
        EXPECT_TRUE(
            result.error( 0 ).message().find( "required" ) != std::string::npos ||
            result.error( 0 ).constraint() == "required" );
    }

    TEST_F( SchemaValidatorTest, ValidateReturnsResult )
    {
        SchemaValidator validator( basicSchema );

        EXPECT_TRUE( validator.validate( validDocument ).isValid() );
        EXPECT_FALSE( validator.validate( invalidDocument ).isValid() );
    }

    //----------------------------------------------
    // Path-specific validation
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, ValidateAtPathBasic )
    {
        SchemaValidator validator( schemaWithRefs );

        std::string docJson = R"({
            "company": {
                "name": "Acme Corp",
                "founded": 1990
            },
            "employees": [
                {
                    "id": 1,
                    "name": "John Doe",
                    "department": "Engineering"
                }
            ]
        })";

        auto maybeDoc = Document::fromString( docJson );
        ASSERT_TRUE( maybeDoc.has_value() );
        Document document = maybeDoc.value();

        // Validate entire document
        ValidationResult fullResult = validator.validateAtPath( document );
        EXPECT_TRUE( fullResult.isValid() );

        // Validate just company
        ValidationResult companyResult = validator.validateAtPath( document, "company", "#/definitions/Company" );
        EXPECT_TRUE( companyResult.isValid() ) << companyResult.errorSummary();

        // Validate specific employee
        ValidationResult employeeResult =
            validator.validateAtPath( document, "employees[0]", "#/definitions/Employee" );
        EXPECT_TRUE( employeeResult.isValid() ) << employeeResult.errorSummary();
    }

    TEST_F( SchemaValidatorTest, ValidateAtPathInvalidDocumentPath )
    {
        SchemaValidator validator( basicSchema );

        ValidationResult result = validator.validateAtPath( validDocument, "nonexistent.path" );

        EXPECT_FALSE( result.isValid() );
        EXPECT_GT( result.errorCount(), 0 );
        EXPECT_NE( result.error( 0 ).message().find( "Document path not found" ), std::string::npos );
    }

    TEST_F( SchemaValidatorTest, ValidateAtPathInvalidSchemaPath )
    {
        SchemaValidator validator( basicSchema );

        ValidationResult result = validator.validateAtPath( validDocument, "name", "#/definitions/NonExistent" );

        EXPECT_FALSE( result.isValid() );
        EXPECT_GT( result.errorCount(), 0 );
        EXPECT_NE( result.error( 0 ).message().find( "Schema path not found:" ), std::string::npos );
    }

    TEST_F( SchemaValidatorTest, ValidateAtPathWithoutSchema )
    {
        SchemaValidator validator;

        EXPECT_THROW( validator.validateAtPath( validDocument ), std::runtime_error );
    }

    //----------------------------------------------
    // Validation constraint
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, TypeValidationErrors )
    {
        SchemaValidator validator( basicSchema );

        std::string wrongTypeJson = R"({
            "name": 123,
            "age": "not-a-number",
            "active": "not-boolean"
        })";

        auto maybeDoc = Document::fromString( wrongTypeJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        ValidationResult result = validator.validate( maybeDoc.value() );

        EXPECT_FALSE( result.isValid() );
        EXPECT_GE( result.errorCount(), 3 );

        // Check for type mismatch errors
        bool foundNameError = false, foundAgeError = false, foundActiveError = false;
        for( const auto& error : result.errors() )
        {
            if( error.path() == "name" && error.constraint() == "type" )
            {
                foundNameError = true;
            }
            if( error.path() == "age" && error.constraint() == "type" )
            {
                foundAgeError = true;
            }
            if( error.path() == "active" && error.constraint() == "type" )
            {
                foundActiveError = true;
            }
        }

        EXPECT_TRUE( foundNameError );
        EXPECT_TRUE( foundAgeError );
        EXPECT_TRUE( foundActiveError );
    }

    TEST_F( SchemaValidatorTest, RequiredFieldValidation )
    {
        SchemaValidator validator( basicSchema );

        std::string missingFieldsJson = R"({
            "email": "2023-10-03T14:30:00Z",
            "active": true
        })";

        auto maybeDoc = Document::fromString( missingFieldsJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        ValidationResult result = validator.validate( maybeDoc.value() );

        EXPECT_FALSE( result.isValid() );
        EXPECT_GE( result.errorCount(), 2 );

        // Check for required field errors
        bool foundNameRequired = false, foundAgeRequired = false;
        for( const auto& error : result.errors() )
        {
            if( error.path() == "name" && error.constraint() == "required" )
            {
                foundNameRequired = true;
            }
            if( error.path() == "age" && error.constraint() == "required" )
            {
                foundAgeRequired = true;
            }
        }

        EXPECT_TRUE( foundNameRequired );
        EXPECT_TRUE( foundAgeRequired );
    }

    TEST_F( SchemaValidatorTest, StringConstraintValidation )
    {
        SchemaValidator validator( basicSchema );

        std::string constraintViolationJson = R"({
            "name": "A",
            "age": 25
        })";

        auto maybeDoc = Document::fromString( constraintViolationJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        ValidationResult result = validator.validate( maybeDoc.value() );

        EXPECT_FALSE( result.isValid() );
        EXPECT_GE( result.errorCount(), 1 );

        // Check for minLength error
        bool foundMinLengthError = false;
        for( const auto& error : result.errors() )
        {
            if( error.path() == "name" && error.constraint() == "minLength" )
            {
                foundMinLengthError = true;
            }
        }

        EXPECT_TRUE( foundMinLengthError );
    }

    TEST_F( SchemaValidatorTest, NumericConstraintValidation )
    {
        SchemaValidator validator( basicSchema );

        std::string constraintViolationJson = R"({
            "name": "John Doe",
            "age": -5
        })";

        auto maybeDoc = Document::fromString( constraintViolationJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        ValidationResult result = validator.validate( maybeDoc.value() );

        EXPECT_FALSE( result.isValid() );
        EXPECT_GE( result.errorCount(), 1 );

        // Check for minimum error
        bool foundMinimumError = false;
        for( const auto& error : result.errors() )
        {
            if( error.path() == "age" && error.constraint() == "minimum" )
            {
                foundMinimumError = true;
            }
        }

        EXPECT_TRUE( foundMinimumError );
    }

    TEST_F( SchemaValidatorTest, AdditionalPropertiesValidation )
    {
        SchemaValidator validator( basicSchema );

        std::string extraPropertyJson = R"({
            "name": "John Doe",
            "age": 30,
            "extraProperty": "not allowed"
        })";

        auto maybeDoc = Document::fromString( extraPropertyJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        ValidationResult result = validator.validate( maybeDoc.value() );

        EXPECT_FALSE( result.isValid() );
        EXPECT_GE( result.errorCount(), 1 );

        // Check for additional property error
        bool foundAdditionalPropError = false;
        for( const auto& error : result.errors() )
        {
            if( error.path() == "extraProperty" && error.constraint() == "additionalProperties" )
            {
                foundAdditionalPropError = true;
            }
        }

        EXPECT_TRUE( foundAdditionalPropError );
    }

    //----------------------------------------------
    // Array validation
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, ArrayValidationWithReferences )
    {
        SchemaValidator validator( schemaWithRefs );

        std::string arrayDocJson = R"({
            "company": {
                "name": "Test Company",
                "founded": 2000
            },
            "employees": [
                {
                    "id": 1,
                    "name": "Alice Smith",
                    "department": "HR"
                },
                {
                    "id": 2,
                    "name": "Bob Jones",
                    "department": "IT"
                }
            ]
        })";

        auto maybeDoc = Document::fromString( arrayDocJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        ValidationResult result = validator.validate( maybeDoc.value() );

        EXPECT_TRUE( result.isValid() );
    }

    TEST_F( SchemaValidatorTest, ArrayValidationWithInvalidItems )
    {
        SchemaValidator validator( schemaWithRefs );

        std::string invalidArrayJson = R"({
            "company": {
                "name": "Test Company"
            },
            "employees": [
                {
                    "id": "not-a-number",
                    "name": "Alice Smith"
                },
                {
                    "id": 2
                }
            ]
        })";

        auto maybeDoc = Document::fromString( invalidArrayJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        ValidationResult result = validator.validate( maybeDoc.value() );

        EXPECT_FALSE( result.isValid() );
        EXPECT_GE( result.errorCount(), 2 );

        // Check for array item validation errors
        bool foundIdTypeError = false, foundNameRequiredError = false;
        for( const auto& error : result.errors() )
        {
            if( error.path().find( "employees[0].id" ) != std::string::npos && error.constraint() == "type" )
                foundIdTypeError = true;
            if( error.path().find( "employees[1].name" ) != std::string::npos && error.constraint() == "required" )
            {
                foundNameRequiredError = true;
            }
        }

        EXPECT_TRUE( foundIdTypeError );
        EXPECT_TRUE( foundNameRequiredError );
    }

    //----------------------------------------------
    // Reference resolution
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, ValidateWithSchemaReferences )
    {
        SchemaValidator validator( schemaWithRefs );

        std::string validRefDocJson = R"({
            "company": {
                "name": "Acme Corp",
                "founded": 1990
            },
            "employees": []
        })";

        auto maybeDoc = Document::fromString( validRefDocJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        ValidationResult result = validator.validate( maybeDoc.value() );

        EXPECT_TRUE( result.isValid() );
    }

    TEST_F( SchemaValidatorTest, ValidateWithInvalidReferences )
    {
        SchemaValidator validator( schemaWithRefs );

        std::string invalidRefDocJson = R"({
            "company": {
                "founded": 1990
            },
            "employees": []
        })";

        auto maybeDoc = Document::fromString( invalidRefDocJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        ValidationResult result = validator.validate( maybeDoc.value() );

        EXPECT_FALSE( result.isValid() );
        EXPECT_GE( result.errorCount(), 1 );

        // Check for missing required field in referenced object
        bool foundCompanyNameRequired = false;
        for( const auto& error : result.errors() )
        {
            if( error.path() == "company.name" && error.constraint() == "required" )
            {
                foundCompanyNameRequired = true;
            }
        }

        EXPECT_TRUE( foundCompanyNameRequired );
    }

    //=====================================================================
    // Applicator Keywords: allOf, anyOf, oneOf, not
    //=====================================================================

    TEST_F( SchemaValidatorTest, AllOfValidation_AllMatch )
    {
        std::string schemaJson = R"({
            "allOf": [
                { "type": "object" },
                { "properties": { "name": { "type": "string" } }, "required": ["name"] },
                { "properties": { "age": { "type": "integer" } }, "required": ["age"] }
            ]
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::string validDocJson = R"({ "name": "John", "age": 30 })";
        auto maybeDoc = Document::fromString( validDocJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        ValidationResult result = validator.validate( maybeDoc.value() );
        EXPECT_TRUE( result.isValid() ) << result.errorSummary();
    }

    TEST_F( SchemaValidatorTest, AllOfValidation_OneFails )
    {
        std::string schemaJson = R"({
            "allOf": [
                { "properties": { "name": { "type": "string" } }, "required": ["name"] },
                { "properties": { "age": { "type": "integer", "minimum": 18 } }, "required": ["age"] }
            ]
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // Age is below minimum - fails second schema
        std::string invalidDocJson = R"({ "name": "John", "age": 10 })";
        auto maybeDoc = Document::fromString( invalidDocJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        ValidationResult result = validator.validate( maybeDoc.value() );
        EXPECT_FALSE( result.isValid() );
        EXPECT_GE( result.errorCount(), 1 );

        bool foundMinimumError = false;
        for( const auto& error : result.errors() )
        {
            if( error.constraint() == "minimum" )
                foundMinimumError = true;
        }
        EXPECT_TRUE( foundMinimumError );
    }

    TEST_F( SchemaValidatorTest, AllOfValidation_MissingRequired )
    {
        std::string schemaJson = R"({
            "allOf": [
                { "properties": { "a": { "type": "string" } }, "required": ["a"] },
                { "properties": { "b": { "type": "string" } }, "required": ["b"] }
            ]
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // Missing "b" - fails second schema
        std::string invalidDocJson = R"({ "a": "value" })";
        auto maybeDoc = Document::fromString( invalidDocJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        ValidationResult result = validator.validate( maybeDoc.value() );
        EXPECT_FALSE( result.isValid() );
    }

    TEST_F( SchemaValidatorTest, AnyOfValidation_FirstMatches )
    {
        std::string schemaJson = R"({
            "anyOf": [
                { "type": "string" },
                { "type": "integer" }
            ]
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::string stringDocJson = R"("hello")";
        auto maybeDoc = Document::fromString( stringDocJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        ValidationResult result = validator.validate( maybeDoc.value() );
        EXPECT_TRUE( result.isValid() ) << result.errorSummary();
    }

    TEST_F( SchemaValidatorTest, AnyOfValidation_SecondMatches )
    {
        std::string schemaJson = R"({
            "anyOf": [
                { "type": "string" },
                { "type": "integer" }
            ]
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::string intDocJson = R"(42)";
        auto maybeDoc = Document::fromString( intDocJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        ValidationResult result = validator.validate( maybeDoc.value() );
        EXPECT_TRUE( result.isValid() ) << result.errorSummary();
    }

    TEST_F( SchemaValidatorTest, AnyOfValidation_NoneMatch )
    {
        std::string schemaJson = R"({
            "anyOf": [
                { "type": "string" },
                { "type": "integer" }
            ]
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::string boolDocJson = R"(true)";
        auto maybeDoc = Document::fromString( boolDocJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        ValidationResult result = validator.validate( maybeDoc.value() );
        EXPECT_FALSE( result.isValid() );

        bool foundAnyOfError = false;
        for( const auto& error : result.errors() )
        {
            if( error.constraint() == "anyOf" )
                foundAnyOfError = true;
        }
        EXPECT_TRUE( foundAnyOfError );
    }

    TEST_F( SchemaValidatorTest, AnyOfValidation_ComplexSchemas )
    {
        std::string schemaJson = R"({
            "anyOf": [
                {
                    "type": "object",
                    "properties": { "type": { "const": "A" }, "valueA": { "type": "string" } },
                    "required": ["type", "valueA"]
                },
                {
                    "type": "object",
                    "properties": { "type": { "const": "B" }, "valueB": { "type": "integer" } },
                    "required": ["type", "valueB"]
                }
            ]
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // Matches first schema
        std::string docAJson = R"({ "type": "A", "valueA": "hello" })";
        auto maybeDocA = Document::fromString( docAJson );
        ASSERT_TRUE( maybeDocA.has_value() );

        ValidationResult resultA = validator.validate( maybeDocA.value() );
        EXPECT_TRUE( resultA.isValid() ) << resultA.errorSummary();

        // Matches second schema
        std::string docBJson = R"({ "type": "B", "valueB": 42 })";
        auto maybeDocB = Document::fromString( docBJson );
        ASSERT_TRUE( maybeDocB.has_value() );

        ValidationResult resultB = validator.validate( maybeDocB.value() );
        EXPECT_TRUE( resultB.isValid() ) << resultB.errorSummary();
    }

    TEST_F( SchemaValidatorTest, OneOfValidation_ExactlyOneMatches )
    {
        std::string schemaJson = R"({
            "oneOf": [
                { "type": "integer", "minimum": 0, "maximum": 10 },
                { "type": "integer", "minimum": 100, "maximum": 200 }
            ]
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // Matches first range only
        std::string doc1Json = R"(5)";
        auto maybeDoc1 = Document::fromString( doc1Json );
        ASSERT_TRUE( maybeDoc1.has_value() );

        ValidationResult result1 = validator.validate( maybeDoc1.value() );
        EXPECT_TRUE( result1.isValid() ) << result1.errorSummary();

        // Matches second range only
        std::string doc2Json = R"(150)";
        auto maybeDoc2 = Document::fromString( doc2Json );
        ASSERT_TRUE( maybeDoc2.has_value() );

        ValidationResult result2 = validator.validate( maybeDoc2.value() );
        EXPECT_TRUE( result2.isValid() ) << result2.errorSummary();
    }

    TEST_F( SchemaValidatorTest, OneOfValidation_NoneMatch )
    {
        std::string schemaJson = R"({
            "oneOf": [
                { "type": "integer", "minimum": 0, "maximum": 10 },
                { "type": "integer", "minimum": 100, "maximum": 200 }
            ]
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // Matches neither range
        std::string docJson = R"(50)";
        auto maybeDoc = Document::fromString( docJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        ValidationResult result = validator.validate( maybeDoc.value() );
        EXPECT_FALSE( result.isValid() );

        bool foundOneOfError = false;
        for( const auto& error : result.errors() )
        {
            if( error.constraint() == "oneOf" )
                foundOneOfError = true;
        }
        EXPECT_TRUE( foundOneOfError );
    }

    TEST_F( SchemaValidatorTest, OneOfValidation_MultipleMatch )
    {
        std::string schemaJson = R"({
            "oneOf": [
                { "type": "integer" },
                { "type": "integer", "minimum": 0 }
            ]
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // Matches BOTH schemas (positive integer satisfies both)
        std::string docJson = R"(5)";
        auto maybeDoc = Document::fromString( docJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        ValidationResult result = validator.validate( maybeDoc.value() );
        EXPECT_FALSE( result.isValid() );

        bool foundOneOfError = false;
        for( const auto& error : result.errors() )
        {
            if( error.constraint() == "oneOf" && error.message().find( "multiple" ) != std::string::npos )
                foundOneOfError = true;
        }
        EXPECT_TRUE( foundOneOfError );
    }

    TEST_F( SchemaValidatorTest, NotValidation_Valid )
    {
        std::string schemaJson = R"({
            "not": { "type": "string" }
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // Integer is NOT a string - valid
        std::string docJson = R"(42)";
        auto maybeDoc = Document::fromString( docJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        ValidationResult result = validator.validate( maybeDoc.value() );
        EXPECT_TRUE( result.isValid() ) << result.errorSummary();
    }

    TEST_F( SchemaValidatorTest, NotValidation_Invalid )
    {
        std::string schemaJson = R"({
            "not": { "type": "string" }
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // String IS a string - invalid (matches "not" schema)
        std::string docJson = R"("hello")";
        auto maybeDoc = Document::fromString( docJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        ValidationResult result = validator.validate( maybeDoc.value() );
        EXPECT_FALSE( result.isValid() );

        bool foundNotError = false;
        for( const auto& error : result.errors() )
        {
            if( error.constraint() == "not" )
                foundNotError = true;
        }
        EXPECT_TRUE( foundNotError );
    }

    TEST_F( SchemaValidatorTest, NotValidation_ComplexSchema )
    {
        std::string schemaJson = R"({
            "type": "object",
            "not": {
                "properties": { "status": { "const": "deleted" } },
                "required": ["status"]
            }
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // Object without status - valid (doesn't match "not" schema)
        std::string doc1Json = R"({ "name": "item" })";
        auto maybeDoc1 = Document::fromString( doc1Json );
        ASSERT_TRUE( maybeDoc1.has_value() );

        ValidationResult result1 = validator.validate( maybeDoc1.value() );
        EXPECT_TRUE( result1.isValid() ) << result1.errorSummary();

        // Object with status "active" - valid (doesn't match "not" schema due to const)
        std::string doc2Json = R"({ "status": "active" })";
        auto maybeDoc2 = Document::fromString( doc2Json );
        ASSERT_TRUE( maybeDoc2.has_value() );

        ValidationResult result2 = validator.validate( maybeDoc2.value() );
        EXPECT_TRUE( result2.isValid() ) << result2.errorSummary();

        // Object with status "deleted" - invalid (matches "not" schema)
        std::string doc3Json = R"({ "status": "deleted" })";
        auto maybeDoc3 = Document::fromString( doc3Json );
        ASSERT_TRUE( maybeDoc3.has_value() );

        ValidationResult result3 = validator.validate( maybeDoc3.value() );
        EXPECT_FALSE( result3.isValid() );
    }

    TEST_F( SchemaValidatorTest, CombinedApplicators )
    {
        // Combine allOf with not
        std::string schemaJson = R"({
            "allOf": [
                { "type": "object" },
                { "properties": { "name": { "type": "string" } }, "required": ["name"] }
            ],
            "not": {
                "properties": { "name": { "const": "forbidden" } }
            }
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // Valid: has name and it's not "forbidden"
        std::string validDocJson = R"({ "name": "allowed" })";
        auto maybeValidDoc = Document::fromString( validDocJson );
        ASSERT_TRUE( maybeValidDoc.has_value() );

        ValidationResult validResult = validator.validate( maybeValidDoc.value() );
        EXPECT_TRUE( validResult.isValid() ) << validResult.errorSummary();

        // Invalid: name is "forbidden"
        std::string invalidDocJson = R"({ "name": "forbidden" })";
        auto maybeInvalidDoc = Document::fromString( invalidDocJson );
        ASSERT_TRUE( maybeInvalidDoc.has_value() );

        ValidationResult invalidResult = validator.validate( maybeInvalidDoc.value() );
        EXPECT_FALSE( invalidResult.isValid() );
    }

    //=====================================================================
    // Conditional Validation: if/then/else
    //=====================================================================

    TEST_F( SchemaValidatorTest, IfThenElse_IfMatchesThenApplied )
    {
        // If country is USA, postal code must be 5 digits
        std::string schemaJson = R"({
            "type": "object",
            "properties": {
                "country": { "type": "string" },
                "postalCode": { "type": "string" }
            },
            "if": {
                "properties": { "country": { "const": "USA" } },
                "required": ["country"]
            },
            "then": {
                "properties": { "postalCode": { "minLength": 5, "maxLength": 5 } }
            }
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // Valid: USA with 5-digit postal code
        std::string validDocJson = R"({ "country": "USA", "postalCode": "12345" })";
        auto maybeValidDoc = Document::fromString( validDocJson );
        ASSERT_TRUE( maybeValidDoc.has_value() );

        ValidationResult validResult = validator.validate( maybeValidDoc.value() );
        EXPECT_TRUE( validResult.isValid() ) << validResult.errorSummary();

        // Invalid: USA with wrong postal code length
        std::string invalidDocJson = R"({ "country": "USA", "postalCode": "1234" })";
        auto maybeInvalidDoc = Document::fromString( invalidDocJson );
        ASSERT_TRUE( maybeInvalidDoc.has_value() );

        ValidationResult invalidResult = validator.validate( maybeInvalidDoc.value() );
        EXPECT_FALSE( invalidResult.isValid() );

        bool foundMinLengthError = false;
        for( const auto& error : invalidResult.errors() )
        {
            if( error.constraint() == "minLength" )
                foundMinLengthError = true;
        }
        EXPECT_TRUE( foundMinLengthError );
    }

    TEST_F( SchemaValidatorTest, IfThenElse_IfNotMatchedThenSkipped )
    {
        // If country is USA, postal code must be 5 digits - but for other countries, any length is OK
        std::string schemaJson = R"({
            "type": "object",
            "properties": {
                "country": { "type": "string" },
                "postalCode": { "type": "string" }
            },
            "if": {
                "properties": { "country": { "const": "USA" } },
                "required": ["country"]
            },
            "then": {
                "properties": { "postalCode": { "minLength": 5, "maxLength": 5 } }
            }
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // Valid: Canada with any postal code (if doesn't match, then not applied)
        std::string validDocJson = R"({ "country": "Canada", "postalCode": "A1B 2C3" })";
        auto maybeValidDoc = Document::fromString( validDocJson );
        ASSERT_TRUE( maybeValidDoc.has_value() );

        ValidationResult validResult = validator.validate( maybeValidDoc.value() );
        EXPECT_TRUE( validResult.isValid() ) << validResult.errorSummary();
    }

    TEST_F( SchemaValidatorTest, IfThenElse_ElseApplied )
    {
        // If type is "business", require tax_id; else require personal_id
        std::string schemaJson = R"({
            "type": "object",
            "if": {
                "properties": { "type": { "const": "business" } },
                "required": ["type"]
            },
            "then": {
                "properties": { "tax_id": { "type": "string" } },
                "required": ["tax_id"]
            },
            "else": {
                "properties": { "personal_id": { "type": "string" } },
                "required": ["personal_id"]
            }
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // Valid business with tax_id
        std::string businessDocJson = R"({ "type": "business", "tax_id": "123-45-6789" })";
        auto maybeBusinessDoc = Document::fromString( businessDocJson );
        ASSERT_TRUE( maybeBusinessDoc.has_value() );

        ValidationResult businessResult = validator.validate( maybeBusinessDoc.value() );
        EXPECT_TRUE( businessResult.isValid() ) << businessResult.errorSummary();

        // Valid personal with personal_id
        std::string personalDocJson = R"({ "type": "personal", "personal_id": "P12345" })";
        auto maybePersonalDoc = Document::fromString( personalDocJson );
        ASSERT_TRUE( maybePersonalDoc.has_value() );

        ValidationResult personalResult = validator.validate( maybePersonalDoc.value() );
        EXPECT_TRUE( personalResult.isValid() ) << personalResult.errorSummary();

        // Invalid: business without tax_id
        std::string invalidBusinessJson = R"({ "type": "business" })";
        auto maybeInvalidBusiness = Document::fromString( invalidBusinessJson );
        ASSERT_TRUE( maybeInvalidBusiness.has_value() );

        ValidationResult invalidBusinessResult = validator.validate( maybeInvalidBusiness.value() );
        EXPECT_FALSE( invalidBusinessResult.isValid() );

        // Invalid: personal without personal_id
        std::string invalidPersonalJson = R"({ "type": "personal" })";
        auto maybeInvalidPersonal = Document::fromString( invalidPersonalJson );
        ASSERT_TRUE( maybeInvalidPersonal.has_value() );

        ValidationResult invalidPersonalResult = validator.validate( maybeInvalidPersonal.value() );
        EXPECT_FALSE( invalidPersonalResult.isValid() );
    }

    TEST_F( SchemaValidatorTest, IfThenElse_NestedConditions )
    {
        // Complex: if age >= 18, require license; else require guardian
        std::string schemaJson = R"({
            "type": "object",
            "properties": {
                "age": { "type": "integer" },
                "name": { "type": "string" }
            },
            "required": ["age", "name"],
            "if": {
                "properties": { "age": { "minimum": 18 } }
            },
            "then": {
                "properties": { "license": { "type": "string" } },
                "required": ["license"]
            },
            "else": {
                "properties": { "guardian": { "type": "string" } },
                "required": ["guardian"]
            }
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // Valid adult with license
        std::string adultDocJson = R"({ "age": 25, "name": "John", "license": "DL123" })";
        auto maybeAdultDoc = Document::fromString( adultDocJson );
        ASSERT_TRUE( maybeAdultDoc.has_value() );

        ValidationResult adultResult = validator.validate( maybeAdultDoc.value() );
        EXPECT_TRUE( adultResult.isValid() ) << adultResult.errorSummary();

        // Valid minor with guardian
        std::string minorDocJson = R"({ "age": 15, "name": "Jane", "guardian": "Parent Name" })";
        auto maybeMinorDoc = Document::fromString( minorDocJson );
        ASSERT_TRUE( maybeMinorDoc.has_value() );

        ValidationResult minorResult = validator.validate( maybeMinorDoc.value() );
        EXPECT_TRUE( minorResult.isValid() ) << minorResult.errorSummary();

        // Invalid: adult without license
        std::string invalidAdultJson = R"({ "age": 30, "name": "Bob" })";
        auto maybeInvalidAdult = Document::fromString( invalidAdultJson );
        ASSERT_TRUE( maybeInvalidAdult.has_value() );

        ValidationResult invalidAdultResult = validator.validate( maybeInvalidAdult.value() );
        EXPECT_FALSE( invalidAdultResult.isValid() );
    }

    TEST_F( SchemaValidatorTest, IfThenElse_WithoutElse )
    {
        // Only "then", no "else"
        std::string schemaJson = R"({
            "type": "object",
            "if": {
                "properties": { "premium": { "const": true } }
            },
            "then": {
                "properties": { "features": { "type": "array", "minItems": 1 } },
                "required": ["features"]
            }
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // Valid: premium with features
        std::string premiumDocJson = R"({ "premium": true, "features": ["feature1"] })";
        auto maybePremiumDoc = Document::fromString( premiumDocJson );
        ASSERT_TRUE( maybePremiumDoc.has_value() );

        ValidationResult premiumResult = validator.validate( maybePremiumDoc.value() );
        EXPECT_TRUE( premiumResult.isValid() ) << premiumResult.errorSummary();

        // Valid: non-premium without features (else not required)
        std::string basicDocJson = R"({ "premium": false })";
        auto maybeBasicDoc = Document::fromString( basicDocJson );
        ASSERT_TRUE( maybeBasicDoc.has_value() );

        ValidationResult basicResult = validator.validate( maybeBasicDoc.value() );
        EXPECT_TRUE( basicResult.isValid() ) << basicResult.errorSummary();

        // Invalid: premium without features
        std::string invalidDocJson = R"({ "premium": true })";
        auto maybeInvalidDoc = Document::fromString( invalidDocJson );
        ASSERT_TRUE( maybeInvalidDoc.has_value() );

        ValidationResult invalidResult = validator.validate( maybeInvalidDoc.value() );
        EXPECT_FALSE( invalidResult.isValid() );
    }

    //=====================================================================
    // $anchor Reference Validation
    //=====================================================================

    TEST_F( SchemaValidatorTest, AnchorReferenceValidation )
    {
        // Schema using $anchor for named definitions
        std::string anchorSchemaJson = R"({
            "$schema": "https://json-schema.org/draft/2020-12/schema",
            "type": "object",
            "properties": {
                "address": { "$ref": "#addressSchema" },
                "billingAddress": { "$ref": "#addressSchema" }
            },
            "$defs": {
                "Address": {
                    "$anchor": "addressSchema",
                    "type": "object",
                    "properties": {
                        "street": { "type": "string" },
                        "city": { "type": "string" },
                        "zipCode": { "type": "string" }
                    },
                    "required": ["street", "city"]
                }
            }
        })";

        auto maybeSchema = Document::fromString( anchorSchemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // Valid document - both addresses satisfy the addressSchema anchor
        std::string validDocJson = R"({
            "address": {
                "street": "123 Main St",
                "city": "Springfield",
                "zipCode": "12345"
            },
            "billingAddress": {
                "street": "456 Oak Ave",
                "city": "Shelbyville"
            }
        })";

        auto maybeValidDoc = Document::fromString( validDocJson );
        ASSERT_TRUE( maybeValidDoc.has_value() );

        ValidationResult validResult = validator.validate( maybeValidDoc.value() );
        EXPECT_TRUE( validResult.isValid() ) << validResult.errorSummary();

        // Invalid document - missing required "city" field
        std::string invalidDocJson = R"({
            "address": {
                "street": "123 Main St"
            },
            "billingAddress": {
                "street": "456 Oak Ave",
                "city": "Shelbyville"
            }
        })";

        auto maybeInvalidDoc = Document::fromString( invalidDocJson );
        ASSERT_TRUE( maybeInvalidDoc.has_value() );

        ValidationResult invalidResult = validator.validate( maybeInvalidDoc.value() );
        EXPECT_FALSE( invalidResult.isValid() );
        EXPECT_GE( invalidResult.errorCount(), 1 );
    }

    TEST_F( SchemaValidatorTest, MultipleAnchorsInSchema )
    {
        // Schema with multiple anchors
        std::string multiAnchorSchemaJson = R"({
            "type": "object",
            "properties": {
                "name": { "$ref": "#stringType" },
                "age": { "$ref": "#positiveInteger" }
            },
            "$defs": {
                "StringDef": {
                    "$anchor": "stringType",
                    "type": "string",
                    "minLength": 1
                },
                "PositiveIntDef": {
                    "$anchor": "positiveInteger",
                    "type": "integer",
                    "minimum": 0
                }
            }
        })";

        auto maybeSchema = Document::fromString( multiAnchorSchemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // Valid document
        std::string validDocJson = R"({
            "name": "John",
            "age": 30
        })";

        auto maybeValidDoc = Document::fromString( validDocJson );
        ASSERT_TRUE( maybeValidDoc.has_value() );

        ValidationResult validResult = validator.validate( maybeValidDoc.value() );
        EXPECT_TRUE( validResult.isValid() ) << validResult.errorSummary();

        // Invalid document - empty name and negative age
        std::string invalidDocJson = R"({
            "name": "",
            "age": -5
        })";

        auto maybeInvalidDoc = Document::fromString( invalidDocJson );
        ASSERT_TRUE( maybeInvalidDoc.has_value() );

        ValidationResult invalidResult = validator.validate( maybeInvalidDoc.value() );
        EXPECT_FALSE( invalidResult.isValid() );
        EXPECT_GE( invalidResult.errorCount(), 2 ); // At least 2 errors
    }

    TEST_F( SchemaValidatorTest, NestedAnchorReference )
    {
        // Schema with anchor in nested location
        std::string nestedAnchorSchemaJson = R"({
            "type": "object",
            "properties": {
                "items": {
                    "type": "array",
                    "items": { "$ref": "#itemSchema" }
                }
            },
            "$defs": {
                "Item": {
                    "$anchor": "itemSchema",
                    "type": "object",
                    "properties": {
                        "id": { "type": "integer" },
                        "name": { "type": "string" }
                    },
                    "required": ["id"]
                }
            }
        })";

        auto maybeSchema = Document::fromString( nestedAnchorSchemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // Valid document
        std::string validDocJson = R"({
            "items": [
                { "id": 1, "name": "Item A" },
                { "id": 2, "name": "Item B" }
            ]
        })";

        auto maybeValidDoc = Document::fromString( validDocJson );
        ASSERT_TRUE( maybeValidDoc.has_value() );

        ValidationResult validResult = validator.validate( maybeValidDoc.value() );
        EXPECT_TRUE( validResult.isValid() ) << validResult.errorSummary();

        // Invalid document - missing required "id" in one item
        std::string invalidDocJson = R"({
            "items": [
                { "id": 1, "name": "Item A" },
                { "name": "Item B" }
            ]
        })";

        auto maybeInvalidDoc = Document::fromString( invalidDocJson );
        ASSERT_TRUE( maybeInvalidDoc.has_value() );

        ValidationResult invalidResult = validator.validate( maybeInvalidDoc.value() );
        EXPECT_FALSE( invalidResult.isValid() );
    }

    //=====================================================================
    // Edge cases and error scenarios
    //=====================================================================

    TEST_F( SchemaValidatorTest, EmptyDocumentValidation )
    {
        SchemaValidator validator( basicSchema );

        Document emptyDoc;

        ValidationResult result = validator.validate( emptyDoc );

        EXPECT_FALSE( result.isValid() );
        EXPECT_GE( result.errorCount(), 2 ); // Missing required fields
    }

    TEST_F( SchemaValidatorTest, NullValueHandling )
    {
        std::string nullSchemaJson = R"({
            "type": "object",
            "properties": {
                "nullableField": {
                    "type": ["string", "null"]
                },
                "nonNullableField": {
                    "type": "string"
                }
            }
        })";

        auto maybeSchema = Document::fromString( nullSchemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::string nullDocJson = R"({
            "nullableField": null,
            "nonNullableField": null
        })";

        auto maybeDoc = Document::fromString( nullDocJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        ValidationResult result = validator.validate( maybeDoc.value() );

        // Should fail only for nonNullableField
        EXPECT_FALSE( result.isValid() );
    }

    //=====================================================================
    // Format Validation Tests (JSON Schema Draft 2020-12 Section 7.3)
    //=====================================================================

    //----------------------------------------------
    // DateTime format (RFC 3339)
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, FormatDateTime_Valid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "date-time"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // Valid RFC 3339 date-times
        std::vector<std::string> validValues = { R"("2025-11-29T14:30:00Z")",       R"("2025-01-01T00:00:00Z")",
                                                 R"("2025-12-31T23:59:59Z")",       R"("2025-06-15T12:30:45+02:00")",
                                                 R"("2025-06-15T12:30:45-05:00")",  R"("2025-06-15T12:30:45.123Z")",
                                                 R"("2025-06-15T12:30:45.123456Z")" };

        for( const auto& value : validValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() ) << "Failed to parse: " << value;

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_TRUE( result.isValid() ) << "Expected valid: " << value << "\n" << result.errorSummary();
        }
    }

    TEST_F( SchemaValidatorTest, FormatDateTime_Invalid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "date-time"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // Invalid date-times
        std::vector<std::string> invalidValues = {
            R"("2025-11-29")",           // Missing time
            R"("14:30:00Z")",            // Missing date
            R"("2025-11-29 14:30:00Z")", // Space instead of T
            R"("2025-13-29T14:30:00Z")", // Invalid month
            R"("2025-11-32T14:30:00Z")", // Invalid day
            R"("2025-11-29T25:30:00Z")", // Invalid hour
            R"("2025-11-29T14:60:00Z")", // Invalid minute
            R"("not-a-date-time")"       // Completely invalid
        };

        for( const auto& value : invalidValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() ) << "Failed to parse: " << value;

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_FALSE( result.isValid() ) << "Expected invalid: " << value;
        }
    }

    //----------------------------------------------
    // Date format (RFC 3339 full-date)
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, FormatDate_Valid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "date"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> validValues = {
            R"("2025-11-29")",
            R"("2025-01-01")",
            R"("2025-12-31")",
            R"("2000-02-29")" // Leap year
        };

        for( const auto& value : validValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_TRUE( result.isValid() ) << "Expected valid: " << value << "\n" << result.errorSummary();
        }
    }

    TEST_F( SchemaValidatorTest, FormatDate_Invalid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "date"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> invalidValues = { R"("2025-13-01")", // Invalid month
                                                   R"("2025-00-01")", // Zero month
                                                   R"("2025-11-32")", // Invalid day
                                                   R"("2025-11-00")", // Zero day
                                                   R"("25-11-29")",   // Short year
                                                   R"("2025/11/29")", // Wrong separator
                                                   R"("not-a-date")" };

        for( const auto& value : invalidValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_FALSE( result.isValid() ) << "Expected invalid: " << value;
        }
    }

    //----------------------------------------------
    // Time format (RFC 3339 full-time)
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, FormatTime_Valid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "time"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> validValues = { R"("14:30:00Z")",      R"("00:00:00Z")",      R"("23:59:59Z")",
                                                 R"("12:30:45+02:00")", R"("12:30:45-05:00")", R"("12:30:45.123Z")" };

        for( const auto& value : validValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_TRUE( result.isValid() ) << "Expected valid: " << value << "\n" << result.errorSummary();
        }
    }

    TEST_F( SchemaValidatorTest, FormatTime_Invalid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "time"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        // Note: For time-only format, :60 seconds is allowed at 23:59 (leap second context)
        // but without date context, the validator accepts :60 at any time
        std::vector<std::string> invalidValues = { R"("25:00:00Z")", // Invalid hour
                                                   R"("14:60:00Z")", // Invalid minute
                                                   R"("14:30:00")",  // Missing timezone
                                                   R"("not-a-time")" };

        for( const auto& value : invalidValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_FALSE( result.isValid() ) << "Expected invalid: " << value;
        }
    }

    //----------------------------------------------
    // Duration format (ISO 8601)
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, FormatDuration_Valid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "duration"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> validValues = {
            R"("P3Y6M4DT12H30M5S")", // Full duration
            R"("P1Y")",              // Just years
            R"("P1M")",              // Just months
            R"("P1D")",              // Just days
            R"("PT1H")",             // Just hours
            R"("PT1M")",             // Just minutes
            R"("PT1S")",             // Just seconds
            R"("P1W")",              // Weeks
            R"("PT0.5S")"            // Fractional seconds
        };

        for( const auto& value : validValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_TRUE( result.isValid() ) << "Expected valid: " << value << "\n" << result.errorSummary();
        }
    }

    TEST_F( SchemaValidatorTest, FormatDuration_Invalid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "duration"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> invalidValues = { R"("P")",  // Just P, no value
                                                   R"("1Y")", // Missing P prefix
                                                   R"("not-a-duration")" };

        for( const auto& value : invalidValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_FALSE( result.isValid() ) << "Expected invalid: " << value;
        }
    }

    //----------------------------------------------
    // Email format (RFC 5321)
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, FormatEmail_Valid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "email"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> validValues = { R"("user@example.com")",
                                                 R"("john.doe@example.org")",
                                                 R"("test+label@example.co.uk")",
                                                 R"("user123@subdomain.example.com")" };

        for( const auto& value : validValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_TRUE( result.isValid() ) << "Expected valid: " << value << "\n" << result.errorSummary();
        }
    }

    TEST_F( SchemaValidatorTest, FormatEmail_Invalid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "email"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> invalidValues = {
            R"("not-an-email")",
            R"("missing@domain")",
            R"("@example.com")",
            R"("user@")",
            R"("user name@example.com")" // Space in local part
        };

        for( const auto& value : invalidValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_FALSE( result.isValid() ) << "Expected invalid: " << value;
        }
    }

    //----------------------------------------------
    // Hostname format (RFC 1123)
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, FormatHostname_Valid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "hostname"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> validValues = {
            R"("example.com")", R"("sub.example.com")", R"("example-site.org")", R"("localhost")", R"("a.b.c.d.e")"
        };

        for( const auto& value : validValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_TRUE( result.isValid() ) << "Expected valid: " << value << "\n" << result.errorSummary();
        }
    }

    TEST_F( SchemaValidatorTest, FormatHostname_Invalid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "hostname"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> invalidValues = {
            R"("-example.com")", // Starts with hyphen
            R"("example-.com")", // Ends with hyphen
            R"("example..com")", // Double dot
            R"("")"              // Empty string
        };

        for( const auto& value : invalidValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_FALSE( result.isValid() ) << "Expected invalid: " << value;
        }
    }

    //----------------------------------------------
    // IPv4 format (RFC 2673)
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, FormatIPv4_Valid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "ipv4"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> validValues = {
            R"("192.168.1.1")", R"("0.0.0.0")", R"("255.255.255.255")", R"("10.0.0.1")", R"("127.0.0.1")"
        };

        for( const auto& value : validValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_TRUE( result.isValid() ) << "Expected valid: " << value << "\n" << result.errorSummary();
        }
    }

    TEST_F( SchemaValidatorTest, FormatIPv4_Invalid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "ipv4"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> invalidValues = { R"("256.0.0.1")",     // Octet > 255
                                                   R"("192.168.1")",     // Only 3 octets
                                                   R"("192.168.1.1.1")", // 5 octets
                                                   R"("192.168.1.a")",   // Non-numeric
                                                   R"("not-an-ip")" };

        for( const auto& value : invalidValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_FALSE( result.isValid() ) << "Expected invalid: " << value;
        }
    }

    //----------------------------------------------
    // IPv6 format (RFC 4291)
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, FormatIPv6_Valid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "ipv6"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> validValues = {
            R"("2001:0db8:85a3:0000:0000:8a2e:0370:7334")", // Full format
            R"("2001:db8:85a3:0:0:8a2e:370:7334")",         // Leading zeros omitted
            R"("::")",                                      // All zeros
            R"("::1")",                                     // Loopback
            R"("fe80::1")"                                  // Link-local
        };

        for( const auto& value : validValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_TRUE( result.isValid() ) << "Expected valid: " << value << "\n" << result.errorSummary();
        }
    }

    TEST_F( SchemaValidatorTest, FormatIPv6_Invalid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "ipv6"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> invalidValues = {
            R"("not-an-ipv6")",
            R"("192.168.1.1")",                          // IPv4 format
            R"("2001:db8:85a3:0:0:8a2e:370:7334:extra")" // Too many groups
        };

        for( const auto& value : invalidValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_FALSE( result.isValid() ) << "Expected invalid: " << value;
        }
    }

    //----------------------------------------------
    // UUID format (RFC 4122)
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, FormatUUID_Valid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "uuid"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> validValues = {
            R"("550e8400-e29b-41d4-a716-446655440000")",
            R"("123e4567-e89b-12d3-a456-426614174000")",
            R"("00000000-0000-0000-0000-000000000000")",
            R"("FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF")", // Uppercase
            R"("ffffffff-ffff-ffff-ffff-ffffffffffff")"  // Lowercase
        };

        for( const auto& value : validValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_TRUE( result.isValid() ) << "Expected valid: " << value << "\n" << result.errorSummary();
        }
    }

    TEST_F( SchemaValidatorTest, FormatUUID_Invalid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "uuid"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> invalidValues = {
            R"("not-a-uuid")",
            R"("550e8400-e29b-41d4-a716")",                    // Too short
            R"("550e8400-e29b-41d4-a716-446655440000-extra")", // Too long
            R"("550e8400e29b41d4a716446655440000")",           // Missing dashes
            R"("g50e8400-e29b-41d4-a716-446655440000")"        // Invalid character 'g'
        };

        for( const auto& value : invalidValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_FALSE( result.isValid() ) << "Expected invalid: " << value;
        }
    }

    //----------------------------------------------
    // URI format (RFC 3986)
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, FormatURI_Valid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "uri"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> validValues = {
            R"("https://example.com")",     R"("http://example.com/path")", R"("ftp://files.example.com/file.txt")",
            R"("mailto:user@example.com")", R"("urn:isbn:0451450523")",     R"("file:///path/to/file")"
        };

        for( const auto& value : validValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_TRUE( result.isValid() ) << "Expected valid: " << value << "\n" << result.errorSummary();
        }
    }

    TEST_F( SchemaValidatorTest, FormatURI_Invalid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "uri"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> invalidValues = {
            R"("not-a-uri")",        // No scheme
            R"("/relative/path")",   // Relative reference
            R"("://missing-scheme")" // Missing scheme name
        };

        for( const auto& value : invalidValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_FALSE( result.isValid() ) << "Expected invalid: " << value;
        }
    }

    //----------------------------------------------
    // URI-Reference format (RFC 3986)
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, FormatURIReference_Valid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "uri-reference"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> validValues = {
            R"("https://example.com")", // Absolute URI
            R"("/relative/path")",      // Relative reference
            R"("../parent/file.txt")",  // Relative path
            R"("#fragment")",           // Fragment only
            R"("?query=value")"         // Query only
        };

        for( const auto& value : validValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_TRUE( result.isValid() ) << "Expected valid: " << value << "\n" << result.errorSummary();
        }
    }

    //----------------------------------------------
    // JSON Pointer format (RFC 6901)
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, FormatJSONPointer_Valid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "json-pointer"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> validValues = {
            R"("")", // Empty string (root)
            R"("/foo")",   R"("/foo/bar")",
            R"("/foo/0")", // Array index
            R"("/a~1b")",  // Escaped /
            R"("/m~0n")"   // Escaped ~
        };

        for( const auto& value : validValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_TRUE( result.isValid() ) << "Expected valid: " << value << "\n" << result.errorSummary();
        }
    }

    TEST_F( SchemaValidatorTest, FormatJSONPointer_Invalid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "json-pointer"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> invalidValues = {
            R"("foo")",   // Missing leading /
            R"("/foo~")", // Incomplete escape
            R"("/foo~2")" // Invalid escape (only ~0 and ~1 are valid)
        };

        for( const auto& value : invalidValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_FALSE( result.isValid() ) << "Expected invalid: " << value;
        }
    }

    //----------------------------------------------
    // Relative JSON Pointer format
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, FormatRelativeJSONPointer_Valid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "relative-json-pointer"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> validValues = {
            R"("0")",
            R"("1")",
            R"("0/foo")",
            R"("1/foo/bar")",
            R"("2#")" // Key access
        };

        for( const auto& value : validValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_TRUE( result.isValid() ) << "Expected valid: " << value << "\n" << result.errorSummary();
        }
    }

    TEST_F( SchemaValidatorTest, FormatRelativeJSONPointer_Invalid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "relative-json-pointer"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> invalidValues = {
            R"("/foo")",  // Absolute pointer, not relative
            R"("abc")",   // Non-numeric prefix
            R"("-1/foo")" // Negative number
        };

        for( const auto& value : invalidValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_FALSE( result.isValid() ) << "Expected invalid: " << value;
        }
    }

    //----------------------------------------------
    // Regex format (ECMA-262)
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, FormatRegex_Valid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "regex"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> validValues = {
            R"("^[a-z]+$")", R"("\\d{3}-\\d{4}")", R"(".*")", R"json("(foo|bar)")json", R"("[A-Za-z0-9_]+")"
        };

        for( const auto& value : validValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_TRUE( result.isValid() ) << "Expected valid: " << value << "\n" << result.errorSummary();
        }
    }

    TEST_F( SchemaValidatorTest, FormatRegex_Invalid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "format": "regex"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> invalidValues = {
            R"("[unclosed")",         // Unclosed bracket
            R"json("(unclosed")json", // Unclosed parenthesis
            R"("*invalid")",          // Quantifier without target
            R"("[z-a]")"              // Invalid range
        };

        for( const auto& value : invalidValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_FALSE( result.isValid() ) << "Expected invalid: " << value;
        }
    }

    //----------------------------------------------
    // Pattern validation
    //----------------------------------------------

    TEST_F( SchemaValidatorTest, PatternValidation_Valid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "pattern": "^[A-Z]{2}-\\d{4}$"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> validValues = { R"("AB-1234")", R"("XY-0000")", R"("ZZ-9999")" };

        for( const auto& value : validValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_TRUE( result.isValid() ) << "Expected valid: " << value << "\n" << result.errorSummary();
        }
    }

    TEST_F( SchemaValidatorTest, PatternValidation_Invalid )
    {
        std::string schemaJson = R"({
            "type": "string",
            "pattern": "^[A-Z]{2}-\\d{4}$"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::vector<std::string> invalidValues = { R"("ab-1234")",  // Lowercase letters
                                                   R"("ABC-1234")", // Three letters
                                                   R"("AB-123")",   // Only 3 digits
                                                   R"("AB1234")",   // Missing dash
                                                   R"("not-matching")" };

        for( const auto& value : invalidValues )
        {
            auto maybeDoc = Document::fromString( value );
            ASSERT_TRUE( maybeDoc.has_value() );

            ValidationResult result = validator.validate( maybeDoc.value() );
            EXPECT_FALSE( result.isValid() ) << "Expected invalid: " << value;
        }
    }

    TEST_F( SchemaValidatorTest, PatternValidation_InvalidRegex )
    {
        // Schema with invalid regex pattern - should skip validation gracefully
        std::string schemaJson = R"({
            "type": "string",
            "pattern": "[invalid"
        })";

        auto maybeSchema = Document::fromString( schemaJson );
        ASSERT_TRUE( maybeSchema.has_value() );

        SchemaValidator validator( maybeSchema.value() );

        std::string docJson = R"("any-value")";
        auto maybeDoc = Document::fromString( docJson );
        ASSERT_TRUE( maybeDoc.has_value() );

        // Invalid regex pattern in schema is reported as a validation error
        ValidationResult result = validator.validate( maybeDoc.value() );
        EXPECT_FALSE( result.isValid() );
        ASSERT_EQ( result.errorCount(), 1 );
        EXPECT_TRUE( result.error( 0 ).message().find( "Invalid regex pattern" ) != std::string::npos );
    }
} // namespace nfx::json::test
