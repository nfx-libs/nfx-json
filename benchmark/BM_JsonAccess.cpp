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
 * @file JsonAccess.cpp
 * @brief Benchmarks for JSON Document value access operations
 */

#include <benchmark/benchmark.h>

#include <nfx/json/Document.h>

namespace nfx::json::benchmark
{
    //----------------------------------------------
    // Test data setup
    //----------------------------------------------

    static Document createTestDocument()
    {
        Document doc;
        doc.set<std::string>( "name", "John Doe" );
        doc.set<int>( "age", 30 );
        doc.set<bool>( "active", true );
        doc.set<double>( "balance", 1234.56 );
        doc.set<std::string>( "email", "john@example.com" );
        doc.set<std::string>( "address.street", "123 Main St" );
        doc.set<std::string>( "address.city", "Springfield" );
        doc.set<std::string>( "address.country", "USA" );
        doc.set<int>( "address.zip", 12345 );
        doc.set<std::string>( "company.name", "Acme Corp" );
        doc.set<std::string>( "company.department.name", "Engineering" );
        doc.set<int>( "company.department.floor", 5 );
        doc.set<std::string>( "company.department.manager.name", "Jane Smith" );
        doc.set<std::string>( "company.department.manager.email", "jane@acme.com" );
        return doc;
    }

    //----------------------------------------------
    // Get by Key benchmarks
    //----------------------------------------------

    static void GetByKey_TopLevel( ::benchmark::State& state )
    {
        Document doc = createTestDocument();

        for( auto _ : state )
        {
            (void)_;

            auto value = doc["name"].rootRef<std::string>();
            ::benchmark::DoNotOptimize( value );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void GetByKey_Nested( ::benchmark::State& state )
    {
        Document doc = createTestDocument();

        for( auto _ : state )
        {
            (void)_;

            auto value = doc["address"]["city"].rootRef<std::string>();
            ::benchmark::DoNotOptimize( value );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    //----------------------------------------------
    // Get by JSON Pointer benchmarks
    //----------------------------------------------

    static void GetByPointer_TopLevel( ::benchmark::State& state )
    {
        Document doc = createTestDocument();

        for( auto _ : state )
        {
            (void)_;

            auto value = doc.get<std::string>( "/name" );
            ::benchmark::DoNotOptimize( value );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void GetByPointer_Nested( ::benchmark::State& state )
    {
        Document doc = createTestDocument();

        for( auto _ : state )
        {
            (void)_;

            auto value = doc.get<std::string>( "/address/city" );
            ::benchmark::DoNotOptimize( value );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    //----------------------------------------------
    // Get Deep Nested Value benchmarks
    //----------------------------------------------

    static void GetDeepNested_3Levels( ::benchmark::State& state )
    {
        Document doc = createTestDocument();

        for( auto _ : state )
        {
            (void)_;

            auto value = doc["company"]["department"]["name"].rootRef<std::string>();
            ::benchmark::DoNotOptimize( value );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void GetDeepNested_4Levels( ::benchmark::State& state )
    {
        Document doc = createTestDocument();

        for( auto _ : state )
        {
            (void)_;

            auto value = doc["company"]["department"]["manager"]["name"].rootRef<std::string>();
            ::benchmark::DoNotOptimize( value );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    //----------------------------------------------
    // Check Field Existence benchmarks
    //----------------------------------------------

    static void HasField_Exists( ::benchmark::State& state )
    {
        Document doc = createTestDocument();

        for( auto _ : state )
        {
            (void)_;

            bool exists = doc.contains( "/name" );
            ::benchmark::DoNotOptimize( exists );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void HasField_NotExists( ::benchmark::State& state )
    {
        Document doc = createTestDocument();

        for( auto _ : state )
        {
            (void)_;

            bool exists = doc.contains( "/nonexistent" );
            ::benchmark::DoNotOptimize( exists );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void HasField_NestedExists( ::benchmark::State& state )
    {
        Document doc = createTestDocument();

        for( auto _ : state )
        {
            (void)_;

            bool exists = doc.contains( "/company/department/manager/email" );
            ::benchmark::DoNotOptimize( exists );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    //=====================================================================
    // Benchmark Registration
    //=====================================================================

    BENCHMARK( GetByKey_TopLevel )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( GetByKey_Nested )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( GetByPointer_TopLevel )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( GetByPointer_Nested )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( GetDeepNested_3Levels )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( GetDeepNested_4Levels )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( HasField_Exists )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( HasField_NotExists )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( HasField_NestedExists )->Unit( ::benchmark::kNanosecond );
} // namespace nfx::json::benchmark

BENCHMARK_MAIN();
