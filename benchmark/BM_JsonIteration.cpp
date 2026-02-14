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
 * @file JsonIteration.cpp
 * @brief Benchmarks for JSON container iteration operations
 */

#include <benchmark/benchmark.h>

#include <nfx/Json.h>

namespace nfx::json::benchmark
{
    //----------------------------------------------
    // Test data setup
    //----------------------------------------------

    static Document createObjectDocument()
    {
        Document doc;
        doc.set<std::string>( "/field1", "value1" );
        doc.set<std::string>( "/field2", "value2" );
        doc.set<std::string>( "/field3", "value3" );
        doc.set<std::string>( "/field4", "value4" );
        doc.set<std::string>( "/field5", "value5" );
        doc.set<int>( "/field6", 100 );
        doc.set<int>( "/field7", 200 );
        doc.set<int>( "/field8", 300 );
        doc.set<bool>( "/field9", true );
        doc.set<bool>( "/field10", false );
        return doc;
    }

    static Document createArrayDocument()
    {
        Document doc;
        doc.set<Array>( "" ); // Create root as array
        for( int64_t i = 0; i < 100; ++i )
        {
            doc.set<int64_t>( "/" + std::to_string( i ), i );
        }
        return doc;
    }

    static Document createNestedObjectDocument()
    {
        Document doc;
        doc.set<std::string>( "/user/name", "John" );
        doc.set<int>( "/user/age", 30 );
        doc.set<std::string>( "/user/address/city", "NYC" );
        doc.set<std::string>( "/user/address/country", "USA" );
        doc.set<std::string>( "/company/name", "Acme" );
        doc.set<int>( "/company/size", 500 );
        doc.set<std::string>( "/company/location/city", "LA" );
        doc.set<std::string>( "/company/location/country", "USA" );
        return doc;
    }

    //----------------------------------------------
    // Object Field Iteration benchmarks
    //----------------------------------------------

    static void ObjectFieldIteration( ::benchmark::State& state )
    {
        Document doc = createObjectDocument();
        auto objOpt = doc.rootRef<Object>();
        const auto& obj = objOpt.value().get();

        for( auto _ : state )
        {
            (void)_;

            int count = 0;
            for( const auto& [key, value] : obj )
            {
                auto k = key;
                auto v = value;
                ::benchmark::DoNotOptimize( k );
                ::benchmark::DoNotOptimize( v );
                ++count;
            }
            ::benchmark::DoNotOptimize( count );
        }
        state.SetItemsProcessed( state.iterations() * 10 );
    }

    //----------------------------------------------
    // Array Element Iteration benchmarks
    //----------------------------------------------

    static void ArrayElementIteration( ::benchmark::State& state )
    {
        Document doc = createArrayDocument();
        auto arrOpt = doc.rootRef<Array>();
        const auto& arr = arrOpt.value().get();

        for( auto _ : state )
        {
            (void)_;

            int64_t sum = 0;
            for( const auto& elem : arr )
            {
                sum += elem.rootRef<int64_t>().value().get();
            }
            ::benchmark::DoNotOptimize( sum );
        }
        state.SetItemsProcessed( state.iterations() * 100 );
    }

    //----------------------------------------------
    // Nested Iteration benchmarks
    //----------------------------------------------

    static void NestedObjectIteration( ::benchmark::State& state )
    {
        Document doc = createNestedObjectDocument();

        for( auto _ : state )
        {
            int count = 0;
            for( const auto& entry : Document::PathView( doc ) )
            {
                auto path = entry.path;
                auto value = entry.value();
                ::benchmark::DoNotOptimize( path );
                ::benchmark::DoNotOptimize( value );
                ++count;
            }
            ::benchmark::DoNotOptimize( count );
        }
        state.SetItemsProcessed( state.iterations() * 8 );
    }

    //=====================================================================
    // Benchmark Registration
    //=====================================================================

    BENCHMARK( ObjectFieldIteration )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( ArrayElementIteration )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( NestedObjectIteration )->Unit( ::benchmark::kNanosecond );
} // namespace nfx::json::benchmark

BENCHMARK_MAIN();
