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
 * @file JsonSchemaGeneration.cpp
 * @brief Benchmarks for JSON Schema generation from documents
 */

#include <benchmark/benchmark.h>

#include <nfx/Json.h>

namespace nfx::json::benchmark
{
    //----------------------------------------------
    // Test data setup
    //----------------------------------------------

    static Document createSmallDocument()
    {
        Document doc;
        doc.set<std::string>( "name", "John" );
        doc.set<int>( "age", 30 );
        doc.set<bool>( "active", true );
        return doc;
    }

    static Document createLargeDocument()
    {
        Document doc;
        doc.set<std::string>( "id", "550e8400-e29b-41d4-a716-446655440000" );
        doc.set<std::string>( "name", "John Doe" );
        doc.set<std::string>( "email", "john.doe@example.com" );
        doc.set<int>( "age", 30 );
        doc.set<bool>( "active", true );
        doc.set<double>( "balance", 12345.67 );
        doc.set<std::string>( "created", "2025-01-15T10:30:00Z" );
        doc.set<std::string>( "address.street", "123 Main Street" );
        doc.set<std::string>( "address.city", "Springfield" );
        doc.set<std::string>( "address.country", "USA" );
        doc.set<std::string>( "company.name", "Acme Corporation" );
        doc.set<int>( "company.employees", 5000 );
        return doc;
    }

    static std::vector<Document> createMultipleDocuments( int count )
    {
        std::vector<Document> docs;
        docs.reserve( count );
        for( int i = 0; i < count; ++i )
        {
            Document doc;
            doc.set<std::string>( "name", "User " + std::to_string( i ) );
            doc.set<int>( "id", i );
            doc.set<std::string>( "email", "user" + std::to_string( i ) + "@example.com" );
            doc.set<bool>( "active", i % 2 == 0 );
            docs.push_back( std::move( doc ) );
        }
        return docs;
    }

    //----------------------------------------------
    // Generate from Single Document benchmarks
    //----------------------------------------------

    static void GenerateFromSmallDoc( ::benchmark::State& state )
    {
        Document doc = createSmallDocument();

        for( auto _ : state )
        {
            SchemaGenerator gen( doc );
            auto schema = gen.schema();
            ::benchmark::DoNotOptimize( schema );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void GenerateFromLargeDoc( ::benchmark::State& state )
    {
        Document doc = createLargeDocument();

        for( auto _ : state )
        {
            SchemaGenerator gen( doc );
            auto schema = gen.schema();
            ::benchmark::DoNotOptimize( schema );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    //----------------------------------------------
    // Generate from Multiple Documents benchmarks
    //----------------------------------------------

    static void GenerateFromMultipleDocs_10( ::benchmark::State& state )
    {
        std::vector<Document> docs = createMultipleDocuments( 10 );

        for( auto _ : state )
        {
            SchemaGenerator gen( docs );
            auto schema = gen.schema();
            ::benchmark::DoNotOptimize( schema );
        }
        state.SetItemsProcessed( state.iterations() * 10 );
    }

    static void GenerateFromMultipleDocs_100( ::benchmark::State& state )
    {
        std::vector<Document> docs = createMultipleDocuments( 100 );

        for( auto _ : state )
        {
            SchemaGenerator gen( docs );
            auto schema = gen.schema();
            ::benchmark::DoNotOptimize( schema );
        }
        state.SetItemsProcessed( state.iterations() * 100 );
    }

    //----------------------------------------------
    // Format Inference benchmarks
    //----------------------------------------------

    static void FormatInference( ::benchmark::State& state )
    {
        Document doc;
        doc.set<std::string>( "email", "john@example.com" );
        doc.set<std::string>( "uuid", "550e8400-e29b-41d4-a716-446655440000" );
        doc.set<std::string>( "date", "2025-01-15" );
        doc.set<std::string>( "datetime", "2025-01-15T10:30:00Z" );
        doc.set<std::string>( "uri", "https://example.com/path" );
        doc.set<std::string>( "ipv4", "192.168.1.1" );

        SchemaGenerator::Options options;
        options.inferFormats = true;

        for( auto _ : state )
        {
            SchemaGenerator gen( doc, options );
            auto schema = gen.schema();
            ::benchmark::DoNotOptimize( schema );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    //----------------------------------------------
    // Constraint Inference benchmarks
    //----------------------------------------------

    static void ConstraintInference( ::benchmark::State& state )
    {
        std::vector<Document> docs = createMultipleDocuments( 50 );

        SchemaGenerator::Options options;
        options.inferConstraints = true;

        for( auto _ : state )
        {
            SchemaGenerator gen( docs, options );
            auto schema = gen.schema();
            ::benchmark::DoNotOptimize( schema );
        }
        state.SetItemsProcessed( state.iterations() * 50 );
    }

    //=====================================================================
    // Benchmark Registration
    //=====================================================================

    BENCHMARK( GenerateFromSmallDoc )->Unit( ::benchmark::kMicrosecond );
    BENCHMARK( GenerateFromLargeDoc )->Unit( ::benchmark::kMicrosecond );
    BENCHMARK( GenerateFromMultipleDocs_10 )->Unit( ::benchmark::kMicrosecond );
    BENCHMARK( GenerateFromMultipleDocs_100 )->Unit( ::benchmark::kMicrosecond );
    BENCHMARK( FormatInference )->Unit( ::benchmark::kMicrosecond );
    BENCHMARK( ConstraintInference )->Unit( ::benchmark::kMicrosecond );
} // namespace nfx::json::benchmark

BENCHMARK_MAIN();
