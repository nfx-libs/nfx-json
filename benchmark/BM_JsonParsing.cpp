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
 * @file JsonParsing.cpp
 * @brief Benchmarks for JSON parsing operations
 */

#include <benchmark/benchmark.h>

#include <nfx/Json.h>

#include <cstring>

namespace nfx::json::benchmark
{
    //----------------------------------------------
    // Test JSON strings
    //----------------------------------------------

    static constexpr const char* SMALL_OBJECT = R"({"name":"John","age":30,"active":true})";

    static constexpr const char* MEDIUM_OBJECT = R"({
        "name": "John Doe",
        "age": 30,
        "email": "john@example.com",
        "active": true,
        "balance": 1234.56,
        "address": {
            "street": "123 Main St",
            "city": "Springfield",
            "country": "USA",
            "zip": 12345
        },
        "tags": ["developer", "senior", "remote"]
    })";

    static constexpr const char* LARGE_OBJECT = R"({
        "id": "550e8400-e29b-41d4-a716-446655440000",
        "name": "John Doe",
        "email": "john.doe@example.com",
        "age": 30,
        "active": true,
        "balance": 12345.67,
        "created": "2025-01-15T10:30:00Z",
        "address": {
            "street": "123 Main Street",
            "city": "Springfield",
            "state": "IL",
            "country": "USA",
            "zip": "62701"
        },
        "company": {
            "name": "Acme Corporation",
            "industry": "Technology",
            "employees": 5000,
            "founded": 1985
        },
        "skills": ["C++", "Python", "JavaScript", "Rust", "Go"],
        "projects": [
            {"name": "Alpha", "status": "completed", "budget": 50000},
            {"name": "Beta", "status": "in-progress", "budget": 75000},
            {"name": "Gamma", "status": "planned", "budget": 100000}
        ],
        "metadata": {
            "source": "api",
            "version": "2.0",
            "checksum": "abc123def456"
        }
    })";

    static constexpr const char* NESTED_OBJECT = R"({
        "level1": {
            "level2": {
                "level3": {
                    "level4": {
                        "level5": {
                            "value": "deep nested value"
                        }
                    }
                }
            }
        }
    })";

    static constexpr const char* SMALL_ARRAY = R"([1, 2, 3, 4, 5])";

    static constexpr const char* LARGE_ARRAY = R"([
        {"id": 1, "name": "Item 1", "value": 100},
        {"id": 2, "name": "Item 2", "value": 200},
        {"id": 3, "name": "Item 3", "value": 300},
        {"id": 4, "name": "Item 4", "value": 400},
        {"id": 5, "name": "Item 5", "value": 500},
        {"id": 6, "name": "Item 6", "value": 600},
        {"id": 7, "name": "Item 7", "value": 700},
        {"id": 8, "name": "Item 8", "value": 800},
        {"id": 9, "name": "Item 9", "value": 900},
        {"id": 10, "name": "Item 10", "value": 1000}
    ])";

    static constexpr const char* MIXED_TYPES = R"({
        "string": "hello world",
        "integer": 42,
        "float": 3.14159,
        "boolean": true,
        "null": null,
        "array": [1, "two", 3.0, false, null],
        "object": {"nested": "value"}
    })";

    //----------------------------------------------
    // Parse benchmarks
    //----------------------------------------------

    static void ParseSmallObject( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;

            auto doc = Document::fromString( SMALL_OBJECT );
            ::benchmark::DoNotOptimize( doc );
        }
        state.SetBytesProcessed( state.iterations() * strlen( SMALL_OBJECT ) );
    }

    static void ParseMediumObject( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;

            auto doc = Document::fromString( MEDIUM_OBJECT );
            ::benchmark::DoNotOptimize( doc );
        }
        state.SetBytesProcessed( state.iterations() * strlen( MEDIUM_OBJECT ) );
    }

    static void ParseLargeObject( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;

            auto doc = Document::fromString( LARGE_OBJECT );
            ::benchmark::DoNotOptimize( doc );
        }
        state.SetBytesProcessed( state.iterations() * strlen( LARGE_OBJECT ) );
    }

    static void ParseNestedObjects( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;

            auto doc = Document::fromString( NESTED_OBJECT );
            ::benchmark::DoNotOptimize( doc );
        }
        state.SetBytesProcessed( state.iterations() * strlen( NESTED_OBJECT ) );
    }

    static void ParseSmallArray( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;

            auto doc = Document::fromString( SMALL_ARRAY );
            ::benchmark::DoNotOptimize( doc );
        }
        state.SetBytesProcessed( state.iterations() * strlen( SMALL_ARRAY ) );
    }

    static void ParseLargeArray( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;

            auto doc = Document::fromString( LARGE_ARRAY );
            ::benchmark::DoNotOptimize( doc );
        }
        state.SetBytesProcessed( state.iterations() * strlen( LARGE_ARRAY ) );
    }

    static void ParseMixedTypes( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;

            auto doc = Document::fromString( MIXED_TYPES );
            ::benchmark::DoNotOptimize( doc );
        }
        state.SetBytesProcessed( state.iterations() * strlen( MIXED_TYPES ) );
    }

    //=====================================================================
    // Benchmark Registration
    //=====================================================================

    BENCHMARK( ParseSmallObject )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( ParseMediumObject )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( ParseLargeObject )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( ParseNestedObjects )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( ParseSmallArray )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( ParseLargeArray )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( ParseMixedTypes )->Unit( ::benchmark::kNanosecond );
} // namespace nfx::json::benchmark

BENCHMARK_MAIN();
