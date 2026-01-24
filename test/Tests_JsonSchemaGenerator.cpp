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
 * @file Tests_JsonSchemaGenerator.cpp
 * @brief Unit tests for JSON Schema generation functionality
 * @details Tests covering SchemaGenerator class with type inference, format detection,
 *          multi-sample analysis, and schema output validation.
 */

#include <gtest/gtest.h>

#include <nfx/Json.h>

namespace nfx::json::test
{
    //=====================================================================
    // SchemaGenerator tests
    //=====================================================================

    class SchemaGeneratorTest : public ::testing::Test
    {
    protected:
    };

    //----------------------------------------------
    // Basic type inference tests
    //----------------------------------------------

    TEST_F( SchemaGeneratorTest, GenerateFromString )
    {
        auto docOpt = Document::fromString( R"({"name": "Alice", "age": 30})" );
        ASSERT_TRUE( docOpt.has_value() );

        SchemaGenerator gen( *docOpt );

        auto typeOpt = gen.schema().get<std::string>( "type" );
        ASSERT_TRUE( typeOpt.has_value() );
        EXPECT_EQ( *typeOpt, "object" );
    }

    TEST_F( SchemaGeneratorTest, GenerateFromInvalidString )
    {
        auto docOpt = Document::fromString( "{ invalid json }" );
        EXPECT_FALSE( docOpt.has_value() );
    }

    TEST_F( SchemaGeneratorTest, InferStringType )
    {
        auto doc = Document::fromString( R"("hello")" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );

        auto typeOpt = gen.schema().get<std::string>( "type" );
        ASSERT_TRUE( typeOpt.has_value() );
        EXPECT_EQ( *typeOpt, "string" );
    }

    TEST_F( SchemaGeneratorTest, InferIntegerType )
    {
        auto doc = Document::fromString( "42" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );

        auto typeOpt = gen.schema().get<std::string>( "type" );
        ASSERT_TRUE( typeOpt.has_value() );
        EXPECT_EQ( *typeOpt, "integer" );
    }

    TEST_F( SchemaGeneratorTest, InferNumberType )
    {
        auto doc = Document::fromString( "3.14" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );

        auto typeOpt = gen.schema().get<std::string>( "type" );
        ASSERT_TRUE( typeOpt.has_value() );
        EXPECT_EQ( *typeOpt, "number" );
    }

    TEST_F( SchemaGeneratorTest, InferBooleanType )
    {
        auto doc = Document::fromString( "true" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );

        auto typeOpt = gen.schema().get<std::string>( "type" );
        ASSERT_TRUE( typeOpt.has_value() );
        EXPECT_EQ( *typeOpt, "boolean" );
    }

    TEST_F( SchemaGeneratorTest, InferNullType )
    {
        auto doc = Document::fromString( "null" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );

        auto typeOpt = gen.schema().get<std::string>( "type" );
        ASSERT_TRUE( typeOpt.has_value() );
        EXPECT_EQ( *typeOpt, "null" );
    }

    TEST_F( SchemaGeneratorTest, InferArrayType )
    {
        auto doc = Document::fromString( R"([1, 2, 3])" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );

        auto typeOpt = gen.schema().get<std::string>( "type" );
        ASSERT_TRUE( typeOpt.has_value() );
        EXPECT_EQ( *typeOpt, "array" );
    }

    TEST_F( SchemaGeneratorTest, InferObjectType )
    {
        auto doc = Document::fromString( R"({"key": "value"})" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );

        auto typeOpt = gen.schema().get<std::string>( "type" );
        ASSERT_TRUE( typeOpt.has_value() );
        EXPECT_EQ( *typeOpt, "object" );
    }

    //----------------------------------------------
    // Object schema generation tests
    //----------------------------------------------

    TEST_F( SchemaGeneratorTest, GenerateObjectProperties )
    {
        auto doc = Document::fromString( R"({"name": "Alice", "age": 30})" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );
        const Document& schema = gen.schema();

        // Check properties exist
        EXPECT_TRUE( schema.contains( "properties" ) );
        EXPECT_TRUE( schema.contains( "properties.name" ) );
        EXPECT_TRUE( schema.contains( "properties.age" ) );

        // Check property types
        auto nameType = schema.get<std::string>( "properties.name.type" );
        ASSERT_TRUE( nameType.has_value() );
        EXPECT_EQ( *nameType, "string" );

        auto ageType = schema.get<std::string>( "properties.age.type" );
        ASSERT_TRUE( ageType.has_value() );
        EXPECT_EQ( *ageType, "integer" );
    }

    TEST_F( SchemaGeneratorTest, GenerateRequiredFields )
    {
        auto doc = Document::fromString( R"({"name": "Alice", "age": 30})" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );
        const Document& schema = gen.schema();

        // Check required array exists
        EXPECT_TRUE( schema.contains( "required" ) );

        auto requiredOpt = schema.get<Array>( "required" );
        ASSERT_TRUE( requiredOpt.has_value() );
        EXPECT_EQ( requiredOpt->size(), 2 );
    }

    TEST_F( SchemaGeneratorTest, GenerateNestedObject )
    {
        auto doc = Document::fromString( R"({"user": {"name": "Alice", "active": true}})" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );
        const Document& schema = gen.schema();

        // Check nested structure
        auto userType = schema.get<std::string>( "properties.user.type" );
        ASSERT_TRUE( userType.has_value() );
        EXPECT_EQ( *userType, "object" );

        auto nameType = schema.get<std::string>( "properties.user.properties.name.type" );
        ASSERT_TRUE( nameType.has_value() );
        EXPECT_EQ( *nameType, "string" );
    }

    //----------------------------------------------
    // Array schema generation tests
    //----------------------------------------------

    TEST_F( SchemaGeneratorTest, GenerateArrayItems )
    {
        auto doc = Document::fromString( R"({"items": [1, 2, 3]})" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );
        const Document& schema = gen.schema();

        auto itemsType = schema.get<std::string>( "properties.items.type" );
        ASSERT_TRUE( itemsType.has_value() );
        EXPECT_EQ( *itemsType, "array" );

        auto itemType = schema.get<std::string>( "properties.items.items.type" );
        ASSERT_TRUE( itemType.has_value() );
        EXPECT_EQ( *itemType, "integer" );
    }

    //----------------------------------------------
    // Format inference tests
    //----------------------------------------------

    TEST_F( SchemaGeneratorTest, InferEmailFormat )
    {
        auto doc = Document::fromString( R"({"email": "alice@example.com"})" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );

        auto format = gen.schema().get<std::string>( "properties.email.format" );
        ASSERT_TRUE( format.has_value() );
        EXPECT_EQ( *format, "email" );
    }

    TEST_F( SchemaGeneratorTest, InferIdnEmailFormat_Punycode )
    {
        // IDN email with Punycode domain (xn-- prefix)
        auto doc = Document::fromString( R"({"email": "user@xn--n3h.com"})" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );

        auto format = gen.schema().get<std::string>( "properties.email.format" );
        ASSERT_TRUE( format.has_value() );
        EXPECT_EQ( *format, "idn-email" );
    }

    TEST_F( SchemaGeneratorTest, InferHostnameFormat )
    {
        auto doc = Document::fromString( R"({"host": "example.com"})" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );

        auto format = gen.schema().get<std::string>( "properties.host.format" );
        ASSERT_TRUE( format.has_value() );
        EXPECT_EQ( *format, "hostname" );
    }

    TEST_F( SchemaGeneratorTest, InferIdnHostnameFormat_Punycode )
    {
        // IDN hostname with Punycode label (xn-- prefix)
        auto doc = Document::fromString( R"({"host": "xn--n3h.example.com"})" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );

        auto format = gen.schema().get<std::string>( "properties.host.format" );
        ASSERT_TRUE( format.has_value() );
        EXPECT_EQ( *format, "idn-hostname" );
    }

    TEST_F( SchemaGeneratorTest, InferDateTimeFormat )
    {
        auto doc = Document::fromString( R"({"timestamp": "2025-11-29T14:30:00Z"})" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );

        auto format = gen.schema().get<std::string>( "properties.timestamp.format" );
        ASSERT_TRUE( format.has_value() );
        EXPECT_EQ( *format, "date-time" );
    }

    TEST_F( SchemaGeneratorTest, InferDateFormat )
    {
        auto doc = Document::fromString( R"({"date": "2025-11-29"})" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );

        auto format = gen.schema().get<std::string>( "properties.date.format" );
        ASSERT_TRUE( format.has_value() );
        EXPECT_EQ( *format, "date" );
    }

    TEST_F( SchemaGeneratorTest, InferUuidFormat )
    {
        auto doc = Document::fromString( R"({"id": "550e8400-e29b-41d4-a716-446655440000"})" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );

        auto format = gen.schema().get<std::string>( "properties.id.format" );
        ASSERT_TRUE( format.has_value() );
        EXPECT_EQ( *format, "uuid" );
    }

    TEST_F( SchemaGeneratorTest, InferUriFormat )
    {
        auto doc = Document::fromString( R"({"url": "https://example.com/path"})" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );

        auto format = gen.schema().get<std::string>( "properties.url.format" );
        ASSERT_TRUE( format.has_value() );
        EXPECT_EQ( *format, "uri" );
    }

    TEST_F( SchemaGeneratorTest, InferIriFormat_Unicode )
    {
        // IRI with Unicode path - note: using raw UTF-8 bytes
        auto doc = Document::fromString( "{\"url\": \"https://example.com/\xE8\xB7\xAF\xE5\xBE\x84\"}" ); // 路径
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );

        auto format = gen.schema().get<std::string>( "properties.url.format" );
        ASSERT_TRUE( format.has_value() );
        EXPECT_EQ( *format, "iri" );
    }

    TEST_F( SchemaGeneratorTest, InferIpv4Format )
    {
        auto doc = Document::fromString( R"({"ip": "192.168.1.1"})" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );

        auto format = gen.schema().get<std::string>( "properties.ip.format" );
        ASSERT_TRUE( format.has_value() );
        EXPECT_EQ( *format, "ipv4" );
    }

    TEST_F( SchemaGeneratorTest, DisableFormatInference )
    {
        auto doc = Document::fromString( R"({"email": "alice@example.com"})" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator::Options opts;
        opts.inferFormats = false;

        SchemaGenerator gen( *doc, opts );

        EXPECT_FALSE( gen.schema().contains( "properties.email.format" ) );
    }

    //----------------------------------------------
    // Metadata tests
    //----------------------------------------------

    TEST_F( SchemaGeneratorTest, AddSchemaMetadata )
    {
        auto doc = Document::fromString( R"({"name": "test"})" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator gen( *doc );

        // Should have $schema
        auto schemaUri = gen.schema().get<std::string>( "$schema" );
        ASSERT_TRUE( schemaUri.has_value() );
        EXPECT_EQ( *schemaUri, "https://json-schema.org/draft/2020-12/schema" );
    }

    TEST_F( SchemaGeneratorTest, AddCustomMetadata )
    {
        auto doc = Document::fromString( R"({"name": "test"})" );
        ASSERT_TRUE( doc.has_value() );

        SchemaGenerator::Options opts;
        opts.title = "Test Schema";
        opts.description = "A test schema";
        opts.id = "https://example.com/test.schema.json";

        SchemaGenerator gen( *doc, opts );
        const Document& schema = gen.schema();

        auto title = schema.get<std::string>( "title" );
        ASSERT_TRUE( title.has_value() );
        EXPECT_EQ( *title, "Test Schema" );

        auto desc = schema.get<std::string>( "description" );
        ASSERT_TRUE( desc.has_value() );
        EXPECT_EQ( *desc, "A test schema" );

        auto id = schema.get<std::string>( "$id" );
        ASSERT_TRUE( id.has_value() );
        EXPECT_EQ( *id, "https://example.com/test.schema.json" );
    }

    //----------------------------------------------
    // Multi-sample tests
    //----------------------------------------------

    TEST_F( SchemaGeneratorTest, MultiSampleRequiredFields )
    {
        auto doc1 = Document::fromString( R"({"name": "Alice", "age": 30, "email": "alice@example.com"})" );
        auto doc2 = Document::fromString( R"({"name": "Bob", "age": 25})" );
        ASSERT_TRUE( doc1.has_value() );
        ASSERT_TRUE( doc2.has_value() );

        std::vector<Document> samples = { *doc1, *doc2 };
        SchemaGenerator gen( samples );
        const Document& schema = gen.schema();

        // Only "name" and "age" should be required (present in both)
        auto requiredOpt = schema.get<Array>( "required" );
        ASSERT_TRUE( requiredOpt.has_value() );

        // Check that we have 2 required fields
        EXPECT_EQ( requiredOpt->size(), 2 );
    }

    TEST_F( SchemaGeneratorTest, EmptySamplesVector )
    {
        std::vector<Document> samples;
        SchemaGenerator gen( samples );

        // Should still have $schema
        EXPECT_TRUE( gen.schema().contains( "$schema" ) );
    }

    //----------------------------------------------
    // Multi-sample with non-object root tests
    //----------------------------------------------

    TEST_F( SchemaGeneratorTest, MultiSampleArrayRoot )
    {
        auto doc1 = Document::fromString( R"([1, 2, 3])" );
        auto doc2 = Document::fromString( R"([4, 5, 6])" );
        ASSERT_TRUE( doc1.has_value() );
        ASSERT_TRUE( doc2.has_value() );

        std::vector<Document> documents = { *doc1, *doc2 };
        SchemaGenerator gen( documents );

        auto typeOpt = gen.schema().get<std::string>( "type" );
        ASSERT_TRUE( typeOpt.has_value() );
        EXPECT_EQ( *typeOpt, "array" );

        auto itemsOpt = gen.schema().get<Document>( "items" );
        ASSERT_TRUE( itemsOpt.has_value() );
        auto itemTypeOpt = itemsOpt->get<std::string>( "type" );
        ASSERT_TRUE( itemTypeOpt.has_value() );
        EXPECT_EQ( *itemTypeOpt, "integer" );
    }

    TEST_F( SchemaGeneratorTest, MultiSampleStringRoot )
    {
        auto doc1 = Document::fromString( R"("hello@example.com")" );
        auto doc2 = Document::fromString( R"("world@example.com")" );
        ASSERT_TRUE( doc1.has_value() );
        ASSERT_TRUE( doc2.has_value() );

        std::vector<Document> documents = { *doc1, *doc2 };
        SchemaGenerator gen( documents );

        auto typeOpt = gen.schema().get<std::string>( "type" );
        ASSERT_TRUE( typeOpt.has_value() );
        EXPECT_EQ( *typeOpt, "string" );

        auto formatOpt = gen.schema().get<std::string>( "format" );
        ASSERT_TRUE( formatOpt.has_value() );
        EXPECT_EQ( *formatOpt, "email" );
    }

    TEST_F( SchemaGeneratorTest, MultiSampleIntegerRoot )
    {
        auto doc1 = Document::fromString( R"(42)" );
        auto doc2 = Document::fromString( R"(100)" );
        ASSERT_TRUE( doc1.has_value() );
        ASSERT_TRUE( doc2.has_value() );

        std::vector<Document> documents = { *doc1, *doc2 };
        SchemaGenerator gen( documents );

        auto typeOpt = gen.schema().get<std::string>( "type" );
        ASSERT_TRUE( typeOpt.has_value() );
        EXPECT_EQ( *typeOpt, "integer" );
    }

    TEST_F( SchemaGeneratorTest, MultiSampleMixedTypes )
    {
        auto doc1 = Document::fromString( R"(42)" );
        auto doc2 = Document::fromString( R"("hello")" );
        ASSERT_TRUE( doc1.has_value() );
        ASSERT_TRUE( doc2.has_value() );

        std::vector<Document> documents = { *doc1, *doc2 };
        SchemaGenerator gen( documents );

        // Should use first schema
        auto typeOpt = gen.schema().get<std::string>( "type" );
        ASSERT_TRUE( typeOpt.has_value() );
        EXPECT_EQ( *typeOpt, "integer" );
    }

    //----------------------------------------------
    // Round-trip validation tests
    //----------------------------------------------

    TEST_F( SchemaGeneratorTest, GeneratedSchemaValidatesOriginal )
    {
        auto doc = Document::fromString( R"({
            "name": "Alice",
            "age": 30,
            "active": true,
            "tags": ["developer", "admin"]
        })" );
        ASSERT_TRUE( doc.has_value() );

        // Generate schema
        SchemaGenerator gen( *doc );

        // Validate original document against generated schema
        SchemaValidator validator( gen.schema() );
        ValidationResult result = validator.validate( *doc );

        EXPECT_TRUE( result.isValid() ) << result.errorSummary();
    }
} // namespace nfx::json::test
