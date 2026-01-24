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
 * @file JsonUniqueItems.cpp
 * @brief Benchmarks for JSON Schema uniqueItems validation with hash-based optimization
 * @details Measures the performance improvement from O(n²) to O(n) using FastHashSet
 */

#include <benchmark/benchmark.h>

#include <nfx/Json.h>

namespace nfx::json::benchmark
{
    //----------------------------------------------
    // Test schema for uniqueItems validation
    //----------------------------------------------

    static Document createUniqueItemsSchema()
    {
        Document schema;
        schema.set<std::string>( "$schema", "https://json-schema.org/draft/2020-12/schema" );
        schema.set<std::string>( "type", "array" );
        schema.set<bool>( "uniqueItems", true );
        return schema;
    }

    //----------------------------------------------
    // Test data generators
    //----------------------------------------------

    static Document createUniqueIntegerArray( int size )
    {
        Document doc;
        doc.set<Array>( "" );
        for( int i = 0; i < size; ++i )
        {
            doc.set<int64_t>( "/" + std::to_string( i ), i );
        }
        return doc;
    }

    static Document createUniqueStringArray( int size )
    {
        Document doc;
        doc.set<Array>( "" );
        for( int i = 0; i < size; ++i )
        {
            doc.set<std::string>( "/" + std::to_string( i ), "item_" + std::to_string( i ) );
        }
        return doc;
    }

    static Document createUniqueObjectArray( int size )
    {
        Document doc;
        doc.set<Array>( "" );
        for( int i = 0; i < size; ++i )
        {
            std::string base = "/" + std::to_string( i );
            doc.set<int64_t>( base + "/id", i );
            doc.set<std::string>( base + "/name", "User " + std::to_string( i ) );
            doc.set<std::string>( base + "/email", "user" + std::to_string( i ) + "@example.com" );
        }
        return doc;
    }

    static Document createUniqueMixedArray( int size )
    {
        Document doc;
        doc.set<Array>( "" );
        for( int i = 0; i < size; ++i )
        {
            std::string path = "/" + std::to_string( i );
            switch( i % 5 )
            {
                case 0:
                    doc.set<int64_t>( path, i );
                    break;
                case 1:
                    doc.set<std::string>( path, "string_" + std::to_string( i ) );
                    break;
                case 2:
                    doc.set<double>( path, i * 1.5 );
                    break;
                case 3:
                    doc.set<bool>( path, i % 2 == 0 );
                    break;
                case 4:
                    doc.set<std::string>( path + "/key", "value_" + std::to_string( i ) );
                    break;
            }
        }
        return doc;
    }

    static Document createArrayWithDuplicateAtEnd( int size )
    {
        Document doc;
        doc.set<Array>( "" );
        for( int i = 0; i < size - 1; ++i )
        {
            doc.set<int64_t>( "/" + std::to_string( i ), i );
        }
        // Duplicate of first element at the end (worst case for O(n²))
        doc.set<int64_t>( "/" + std::to_string( size - 1 ), 0 );
        return doc;
    }

    //----------------------------------------------
    // Benchmark: Unique Integers (all valid)
    //----------------------------------------------

    static void UniqueItems_Integers_10( ::benchmark::State& state )
    {
        Document schema = createUniqueItemsSchema();
        Document doc = createUniqueIntegerArray( 10 );
        SchemaValidator validator( schema );

        for( auto _ : state )
        {
            auto result = validator.validate( doc );
            ::benchmark::DoNotOptimize( result );
        }
        state.SetItemsProcessed( state.iterations() * 10 );
    }

    static void UniqueItems_Integers_100( ::benchmark::State& state )
    {
        Document schema = createUniqueItemsSchema();
        Document doc = createUniqueIntegerArray( 100 );
        SchemaValidator validator( schema );

        for( auto _ : state )
        {
            auto result = validator.validate( doc );
            ::benchmark::DoNotOptimize( result );
        }
        state.SetItemsProcessed( state.iterations() * 100 );
    }

    static void UniqueItems_Integers_500( ::benchmark::State& state )
    {
        Document schema = createUniqueItemsSchema();
        Document doc = createUniqueIntegerArray( 500 );
        SchemaValidator validator( schema );

        for( auto _ : state )
        {
            auto result = validator.validate( doc );
            ::benchmark::DoNotOptimize( result );
        }
        state.SetItemsProcessed( state.iterations() * 500 );
    }

    static void UniqueItems_Integers_1000( ::benchmark::State& state )
    {
        Document schema = createUniqueItemsSchema();
        Document doc = createUniqueIntegerArray( 1000 );
        SchemaValidator validator( schema );

        for( auto _ : state )
        {
            auto result = validator.validate( doc );
            ::benchmark::DoNotOptimize( result );
        }
        state.SetItemsProcessed( state.iterations() * 1000 );
    }

    static void UniqueItems_Integers_5000( ::benchmark::State& state )
    {
        Document schema = createUniqueItemsSchema();
        Document doc = createUniqueIntegerArray( 5000 );
        SchemaValidator validator( schema );

        for( auto _ : state )
        {
            auto result = validator.validate( doc );
            ::benchmark::DoNotOptimize( result );
        }
        state.SetItemsProcessed( state.iterations() * 5000 );
    }

    //----------------------------------------------
    // Benchmark: Unique Strings
    //----------------------------------------------

    static void UniqueItems_Strings_100( ::benchmark::State& state )
    {
        Document schema = createUniqueItemsSchema();
        Document doc = createUniqueStringArray( 100 );
        SchemaValidator validator( schema );

        for( auto _ : state )
        {
            auto result = validator.validate( doc );
            ::benchmark::DoNotOptimize( result );
        }
        state.SetItemsProcessed( state.iterations() * 100 );
    }

    static void UniqueItems_Strings_1000( ::benchmark::State& state )
    {
        Document schema = createUniqueItemsSchema();
        Document doc = createUniqueStringArray( 1000 );
        SchemaValidator validator( schema );

        for( auto _ : state )
        {
            auto result = validator.validate( doc );
            ::benchmark::DoNotOptimize( result );
        }
        state.SetItemsProcessed( state.iterations() * 1000 );
    }

    //----------------------------------------------
    // Benchmark: Unique Objects
    //----------------------------------------------

    static void UniqueItems_Objects_100( ::benchmark::State& state )
    {
        Document schema = createUniqueItemsSchema();
        Document doc = createUniqueObjectArray( 100 );
        SchemaValidator validator( schema );

        for( auto _ : state )
        {
            auto result = validator.validate( doc );
            ::benchmark::DoNotOptimize( result );
        }
        state.SetItemsProcessed( state.iterations() * 100 );
    }

    static void UniqueItems_Objects_1000( ::benchmark::State& state )
    {
        Document schema = createUniqueItemsSchema();
        Document doc = createUniqueObjectArray( 1000 );
        SchemaValidator validator( schema );

        for( auto _ : state )
        {
            auto result = validator.validate( doc );
            ::benchmark::DoNotOptimize( result );
        }
        state.SetItemsProcessed( state.iterations() * 1000 );
    }

    //----------------------------------------------
    // Benchmark: Mixed Types
    //----------------------------------------------

    static void UniqueItems_Mixed_100( ::benchmark::State& state )
    {
        Document schema = createUniqueItemsSchema();
        Document doc = createUniqueMixedArray( 100 );
        SchemaValidator validator( schema );

        for( auto _ : state )
        {
            auto result = validator.validate( doc );
            ::benchmark::DoNotOptimize( result );
        }
        state.SetItemsProcessed( state.iterations() * 100 );
    }

    static void UniqueItems_Mixed_1000( ::benchmark::State& state )
    {
        Document schema = createUniqueItemsSchema();
        Document doc = createUniqueMixedArray( 1000 );
        SchemaValidator validator( schema );

        for( auto _ : state )
        {
            auto result = validator.validate( doc );
            ::benchmark::DoNotOptimize( result );
        }
        state.SetItemsProcessed( state.iterations() * 1000 );
    }

    //----------------------------------------------
    // Benchmark: Duplicate Detection (worst case)
    //----------------------------------------------

    static void UniqueItems_DuplicateAtEnd_100( ::benchmark::State& state )
    {
        Document schema = createUniqueItemsSchema();
        Document doc = createArrayWithDuplicateAtEnd( 100 );
        SchemaValidator validator( schema );

        for( auto _ : state )
        {
            auto result = validator.validate( doc );
            ::benchmark::DoNotOptimize( result );
        }
        state.SetItemsProcessed( state.iterations() * 100 );
    }

    static void UniqueItems_DuplicateAtEnd_1000( ::benchmark::State& state )
    {
        Document schema = createUniqueItemsSchema();
        Document doc = createArrayWithDuplicateAtEnd( 1000 );
        SchemaValidator validator( schema );

        for( auto _ : state )
        {
            auto result = validator.validate( doc );
            ::benchmark::DoNotOptimize( result );
        }
        state.SetItemsProcessed( state.iterations() * 1000 );
    }

    static void UniqueItems_DuplicateAtEnd_5000( ::benchmark::State& state )
    {
        Document schema = createUniqueItemsSchema();
        Document doc = createArrayWithDuplicateAtEnd( 5000 );
        SchemaValidator validator( schema );

        for( auto _ : state )
        {
            auto result = validator.validate( doc );
            ::benchmark::DoNotOptimize( result );
        }
        state.SetItemsProcessed( state.iterations() * 5000 );
    }

    //=====================================================================
    // Benchmark Registration
    //=====================================================================

    // Unique Integers - scaling test
    BENCHMARK( UniqueItems_Integers_10 )->Unit( ::benchmark::kMicrosecond );
    BENCHMARK( UniqueItems_Integers_100 )->Unit( ::benchmark::kMicrosecond );
    BENCHMARK( UniqueItems_Integers_500 )->Unit( ::benchmark::kMicrosecond );
    BENCHMARK( UniqueItems_Integers_1000 )->Unit( ::benchmark::kMicrosecond );
    BENCHMARK( UniqueItems_Integers_5000 )->Unit( ::benchmark::kMicrosecond );

    // Unique Strings
    BENCHMARK( UniqueItems_Strings_100 )->Unit( ::benchmark::kMicrosecond );
    BENCHMARK( UniqueItems_Strings_1000 )->Unit( ::benchmark::kMicrosecond );

    // Unique Objects
    BENCHMARK( UniqueItems_Objects_100 )->Unit( ::benchmark::kMicrosecond );
    BENCHMARK( UniqueItems_Objects_1000 )->Unit( ::benchmark::kMicrosecond );

    // Mixed Types
    BENCHMARK( UniqueItems_Mixed_100 )->Unit( ::benchmark::kMicrosecond );
    BENCHMARK( UniqueItems_Mixed_1000 )->Unit( ::benchmark::kMicrosecond );

    // Worst case: duplicate at end (O(n²) would scan everything)
    BENCHMARK( UniqueItems_DuplicateAtEnd_100 )->Unit( ::benchmark::kMicrosecond );
    BENCHMARK( UniqueItems_DuplicateAtEnd_1000 )->Unit( ::benchmark::kMicrosecond );
    BENCHMARK( UniqueItems_DuplicateAtEnd_5000 )->Unit( ::benchmark::kMicrosecond );
} // namespace nfx::json::benchmark

BENCHMARK_MAIN();
