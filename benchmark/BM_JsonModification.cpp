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
 * @file BM_JsonModification.cpp
 * @brief Benchmarks for JSON Document modification operations
 */

#include <benchmark/benchmark.h>

#include <nfx/Json.h>

namespace nfx::json::benchmark
{
    //----------------------------------------------
    // Set Primitive Value benchmarks
    //----------------------------------------------

    static void SetPrimitiveValue_String( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;

            Document doc;
            doc.set<std::string>( "name", "John Doe" );
            ::benchmark::DoNotOptimize( doc );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SetPrimitiveValue_Int( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;

            Document doc;
            doc.set<int>( "age", 30 );
            ::benchmark::DoNotOptimize( doc );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SetPrimitiveValue_Bool( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;

            Document doc;
            doc.set<bool>( "active", true );
            ::benchmark::DoNotOptimize( doc );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SetPrimitiveValue_Double( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;

            Document doc;
            doc.set<double>( "balance", 1234.56 );
            ::benchmark::DoNotOptimize( doc );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    //----------------------------------------------
    // Set Nested Value benchmarks
    //----------------------------------------------

    static void SetNestedValue_2Levels( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;

            Document doc;
            doc.set<std::string>( "address.city", "Springfield" );
            ::benchmark::DoNotOptimize( doc );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SetNestedValue_3Levels( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;

            Document doc;
            doc.set<std::string>( "company.address.city", "Springfield" );
            ::benchmark::DoNotOptimize( doc );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SetNestedValue_4Levels( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;

            Document doc;
            doc.set<std::string>( "company.department.manager.name", "Jane Smith" );
            ::benchmark::DoNotOptimize( doc );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    //----------------------------------------------
    // Add Object Field benchmarks
    //----------------------------------------------

    static void AddObjectField_Sequential( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;

            Document doc;
            doc.set<std::string>( "field1", "value1" );
            doc.set<std::string>( "field2", "value2" );
            doc.set<std::string>( "field3", "value3" );
            doc.set<std::string>( "field4", "value4" );
            doc.set<std::string>( "field5", "value5" );
            ::benchmark::DoNotOptimize( doc );
        }
        state.SetItemsProcessed( state.iterations() * 5 );
    }

    //----------------------------------------------
    // Add Array Element benchmarks
    //----------------------------------------------

    static void AddArrayElement_PushBack( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;

            Document doc;
            doc.set<Array>( "" );
            for( int64_t i = 0; i < 10; ++i )
            {
                doc.set<int64_t>( "/" + std::to_string( i ), i );
            }
            ::benchmark::DoNotOptimize( doc );
        }
        state.SetItemsProcessed( state.iterations() * 10 );
    }

    static void AddArrayElement_LargeArray( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;

            Document doc;
            doc.set<Array>( "" );
            for( int64_t i = 0; i < 100; ++i )
            {
                doc.set<int64_t>( "/" + std::to_string( i ), i );
            }
            ::benchmark::DoNotOptimize( doc );
        }
        state.SetItemsProcessed( state.iterations() * 100 );
    }

    //=====================================================================
    // Benchmark Registration
    //=====================================================================

    BENCHMARK( SetPrimitiveValue_String )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SetPrimitiveValue_Int )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SetPrimitiveValue_Bool )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SetPrimitiveValue_Double )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SetNestedValue_2Levels )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SetNestedValue_3Levels )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SetNestedValue_4Levels )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( AddObjectField_Sequential )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( AddArrayElement_PushBack )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( AddArrayElement_LargeArray )->Unit( ::benchmark::kNanosecond );
} // namespace nfx::json::benchmark

BENCHMARK_MAIN();
