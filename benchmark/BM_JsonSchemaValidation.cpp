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
 * @file JsonSchemaValidation.cpp
 * @brief Benchmarks for JSON Schema validation operations
 */

#include <benchmark/benchmark.h>

#include <nfx/Json.h>

namespace nfx::json::benchmark
{
    //----------------------------------------------
    // Test schemas
    //----------------------------------------------

    static Document createSimpleSchema()
    {
        Document schema;
        schema.set<std::string>( "$schema", "https://json-schema.org/draft/2020-12/schema" );
        schema.set<std::string>( "type", "object" );
        schema.set<std::string>( "properties.name.type", "string" );
        schema.set<std::string>( "properties.age.type", "integer" );
        schema.set<std::string>( "properties.active.type", "boolean" );
        return schema;
    }

    static Document createComplexSchema()
    {
        Document schema;
        schema.set<std::string>( "$schema", "https://json-schema.org/draft/2020-12/schema" );
        schema.set<std::string>( "type", "object" );
        schema.set<std::string>( "properties.name.type", "string" );
        schema.set<int>( "properties.name.minLength", 1 );
        schema.set<int>( "properties.name.maxLength", 100 );
        schema.set<std::string>( "properties.email.type", "string" );
        schema.set<std::string>( "properties.email.format", "email" );
        schema.set<std::string>( "properties.age.type", "integer" );
        schema.set<int>( "properties.age.minimum", 0 );
        schema.set<int>( "properties.age.maximum", 150 );
        schema.set<std::string>( "properties.address.type", "object" );
        schema.set<std::string>( "properties.address.properties.city.type", "string" );
        schema.set<std::string>( "properties.address.properties.country.type", "string" );
        return schema;
    }

    static Document createSchemaWithRef()
    {
        Document schema;
        schema.set<std::string>( "$schema", "https://json-schema.org/draft/2020-12/schema" );
        schema.set<std::string>( "type", "object" );
        schema.set<std::string>( "properties.user.$ref", "#/$defs/User" );
        schema.set<std::string>( "$defs.User.type", "object" );
        schema.set<std::string>( "$defs.User.properties.name.type", "string" );
        schema.set<std::string>( "$defs.User.properties.email.type", "string" );
        return schema;
    }

    static Document createSchemaWithFormats()
    {
        Document schema;
        schema.set<std::string>( "$schema", "https://json-schema.org/draft/2020-12/schema" );
        schema.set<std::string>( "type", "object" );
        schema.set<std::string>( "properties.email.type", "string" );
        schema.set<std::string>( "properties.email.format", "email" );
        schema.set<std::string>( "properties.uuid.type", "string" );
        schema.set<std::string>( "properties.uuid.format", "uuid" );
        schema.set<std::string>( "properties.date.type", "string" );
        schema.set<std::string>( "properties.date.format", "date" );
        schema.set<std::string>( "properties.uri.type", "string" );
        schema.set<std::string>( "properties.uri.format", "uri" );
        return schema;
    }

    //----------------------------------------------
    // Test documents
    //----------------------------------------------

    static Document createValidDocument()
    {
        Document doc;
        doc.set<std::string>( "name", "John Doe" );
        doc.set<int>( "age", 30 );
        doc.set<bool>( "active", true );
        return doc;
    }

    static Document createInvalidDocument()
    {
        Document doc;
        doc.set<int>( "name", 12345 );                 // Wrong type
        doc.set<std::string>( "age", "not a number" ); // Wrong type
        doc.set<std::string>( "active", "yes" );       // Wrong type
        return doc;
    }

    static Document createDocumentWithFormats()
    {
        Document doc;
        doc.set<std::string>( "email", "john@example.com" );
        doc.set<std::string>( "uuid", "550e8400-e29b-41d4-a716-446655440000" );
        doc.set<std::string>( "date", "2025-01-15" );
        doc.set<std::string>( "uri", "https://example.com/path" );
        return doc;
    }

    static Document createDeeplyNestedDocument()
    {
        Document doc;
        doc.set<std::string>( "l1.l2.l3.l4.l5.l6.l7.l8.l9.l10.value", "deep" );
        return doc;
    }

    //----------------------------------------------
    // Validate Simple Schema benchmarks
    //----------------------------------------------

    static void ValidateSimpleSchema_Valid( ::benchmark::State& state )
    {
        Document schema = createSimpleSchema();
        Document doc = createValidDocument();
        SchemaValidator validator( schema );

        for( auto _ : state )
        {
            auto result = validator.validate( doc );
            ::benchmark::DoNotOptimize( result );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    //----------------------------------------------
    // Validate Complex Schema benchmarks
    //----------------------------------------------

    static void ValidateComplexSchema( ::benchmark::State& state )
    {
        Document schema = createComplexSchema();
        Document doc;
        doc.set<std::string>( "name", "John Doe" );
        doc.set<std::string>( "email", "john@example.com" );
        doc.set<int>( "age", 30 );
        doc.set<std::string>( "address.city", "NYC" );
        doc.set<std::string>( "address.country", "USA" );

        SchemaValidator validator( schema );

        for( auto _ : state )
        {
            auto result = validator.validate( doc );
            ::benchmark::DoNotOptimize( result );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    //----------------------------------------------
    // Validate with $ref benchmarks
    //----------------------------------------------

    static void ValidateWithRef( ::benchmark::State& state )
    {
        Document schema = createSchemaWithRef();
        Document doc;
        doc.set<std::string>( "user.name", "John" );
        doc.set<std::string>( "user.email", "john@example.com" );

        SchemaValidator validator( schema );

        for( auto _ : state )
        {
            auto result = validator.validate( doc );
            ::benchmark::DoNotOptimize( result );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    //----------------------------------------------
    // Validate String Formats benchmarks
    //----------------------------------------------

    static void ValidateStringFormats( ::benchmark::State& state )
    {
        Document schema = createSchemaWithFormats();
        Document doc = createDocumentWithFormats();
        SchemaValidator validator( schema );

        for( auto _ : state )
        {
            auto result = validator.validate( doc );
            ::benchmark::DoNotOptimize( result );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    //----------------------------------------------
    // Validate with Errors benchmarks
    //----------------------------------------------

    static void ValidateWithErrors( ::benchmark::State& state )
    {
        Document schema = createSimpleSchema();
        Document doc = createInvalidDocument();
        SchemaValidator validator( schema );

        for( auto _ : state )
        {
            auto result = validator.validate( doc );
            ::benchmark::DoNotOptimize( result );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    //----------------------------------------------
    // Validate Deeply Nested benchmarks
    //----------------------------------------------

    static void ValidateDeeplyNested( ::benchmark::State& state )
    {
        Document schema;
        schema.set<std::string>( "$schema", "https://json-schema.org/draft/2020-12/schema" );
        schema.set<std::string>( "type", "object" );

        Document doc = createDeeplyNestedDocument();
        SchemaValidator validator( schema );

        for( auto _ : state )
        {
            auto result = validator.validate( doc );
            ::benchmark::DoNotOptimize( result );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    //=====================================================================
    // Benchmark Registration
    //=====================================================================

    BENCHMARK( ValidateSimpleSchema_Valid )->Unit( ::benchmark::kMicrosecond );
    BENCHMARK( ValidateComplexSchema )->Unit( ::benchmark::kMicrosecond );
    BENCHMARK( ValidateWithRef )->Unit( ::benchmark::kMicrosecond );
    BENCHMARK( ValidateStringFormats )->Unit( ::benchmark::kMicrosecond );
    BENCHMARK( ValidateWithErrors )->Unit( ::benchmark::kMicrosecond );
    BENCHMARK( ValidateDeeplyNested )->Unit( ::benchmark::kMicrosecond );
} // namespace nfx::json::benchmark

BENCHMARK_MAIN();
