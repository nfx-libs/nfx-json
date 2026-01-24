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
 * @file BM_JsonSerialization.cpp
 * @brief Benchmarks for JSON serialization (writing) operations
 */

#include <benchmark/benchmark.h>

#include <nfx/Json.h>

namespace nfx::json::benchmark
{
    //----------------------------------------------
    // Test data setup
    //----------------------------------------------

    static Document createSmallObject()
    {
        Document doc;
        doc.set<std::string>( "name", "John Doe" );
        doc.set<int64_t>( "age", 30 );
        doc.set<bool>( "active", true );
        return doc;
    }

    static Document createMediumObject()
    {
        Document doc;
        doc.set<std::string>( "name", "Jane Smith" );
        doc.set<int64_t>( "age", 25 );
        doc.set<std::string>( "email", "jane.smith@example.com" );
        doc.set<std::string>( "address/street", "123 Main St" );
        doc.set<std::string>( "address/city", "Springfield" );
        doc.set<std::string>( "address/zip", "12345" );
        doc.set<bool>( "preferences/notifications", true );
        doc.set<bool>( "preferences/newsletter", false );
        doc.set<int64_t>( "preferences/theme", 1 );
        return doc;
    }

    static Document createLargeObject()
    {
        Document doc;
        for( int i = 0; i < 100; ++i )
        {
            doc.set<std::string>( "field" + std::to_string( i ), "value" + std::to_string( i ) );
        }
        return doc;
    }

    static Document createSmallArray()
    {
        Document doc;
        doc.set<Array>( "" );
        for( int i = 0; i < 10; ++i )
        {
            doc.set<int64_t>( "/" + std::to_string( i ), i );
        }
        return doc;
    }

    static Document createLargeArray()
    {
        Document doc;
        doc.set<Array>( "" );
        for( int i = 0; i < 1000; ++i )
        {
            doc.set<int64_t>( "/" + std::to_string( i ), i );
        }
        return doc;
    }

    static Document createNestedDocument()
    {
        Document doc;
        doc.set<std::string>( "level1/level2/level3/value", "deep" );
        doc.set<int64_t>( "level1/level2/count", 42 );
        doc.set<bool>( "level1/active", true );
        return doc;
    }

    static Document createStringHeavyDocument()
    {
        Document doc;
        doc.set<std::string>( "simple", "Hello World" );
        doc.set<std::string>( "with_quotes", "She said \"Hello\"" );
        doc.set<std::string>( "with_newlines", "Line1\nLine2\nLine3" );
        doc.set<std::string>( "with_tabs", "Col1\tCol2\tCol3" );
        doc.set<std::string>( "unicode", "Emoji: 🚀 Unicode: \u00A0" );
        doc.set<std::string>( "long_text", std::string( 500, 'x' ) );
        return doc;
    }

    //----------------------------------------------
    // Serialization benchmarks - Compact
    //----------------------------------------------

    static void SerializeSmallObject_Compact( ::benchmark::State& state )
    {
        Document doc = createSmallObject();

        for( auto _ : state )
        {
            (void)_;
            auto json = doc.toString( 0 );
            ::benchmark::DoNotOptimize( json );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeMediumObject_Compact( ::benchmark::State& state )
    {
        Document doc = createMediumObject();

        for( auto _ : state )
        {
            (void)_;
            auto json = doc.toString( 0 );
            ::benchmark::DoNotOptimize( json );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeLargeObject_Compact( ::benchmark::State& state )
    {
        Document doc = createLargeObject();

        for( auto _ : state )
        {
            (void)_;
            auto json = doc.toString( 0 );
            ::benchmark::DoNotOptimize( json );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeSmallArray_Compact( ::benchmark::State& state )
    {
        Document doc = createSmallArray();

        for( auto _ : state )
        {
            (void)_;
            auto json = doc.toString( 0 );
            ::benchmark::DoNotOptimize( json );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeLargeArray_Compact( ::benchmark::State& state )
    {
        Document doc = createLargeArray();

        for( auto _ : state )
        {
            (void)_;
            auto json = doc.toString( 0 );
            ::benchmark::DoNotOptimize( json );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeNested_Compact( ::benchmark::State& state )
    {
        Document doc = createNestedDocument();

        for( auto _ : state )
        {
            (void)_;
            auto json = doc.toString( 0 );
            ::benchmark::DoNotOptimize( json );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeStringHeavy_Compact( ::benchmark::State& state )
    {
        Document doc = createStringHeavyDocument();

        for( auto _ : state )
        {
            (void)_;
            auto json = doc.toString( 0 );
            ::benchmark::DoNotOptimize( json );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    //----------------------------------------------
    // Serialization benchmarks - Pretty (indent=2)
    //----------------------------------------------

    static void SerializeSmallObject_Pretty( ::benchmark::State& state )
    {
        Document doc = createSmallObject();

        for( auto _ : state )
        {
            (void)_;
            auto json = doc.toString( 2 );
            ::benchmark::DoNotOptimize( json );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeMediumObject_Pretty( ::benchmark::State& state )
    {
        Document doc = createMediumObject();

        for( auto _ : state )
        {
            (void)_;
            auto json = doc.toString( 2 );
            ::benchmark::DoNotOptimize( json );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeLargeObject_Pretty( ::benchmark::State& state )
    {
        Document doc = createLargeObject();

        for( auto _ : state )
        {
            (void)_;
            auto json = doc.toString( 2 );
            ::benchmark::DoNotOptimize( json );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    //----------------------------------------------
    // Round-trip benchmarks
    //----------------------------------------------

    static void RoundTrip_SmallObject( ::benchmark::State& state )
    {
        const char* json = R"({"name":"John Doe","age":30,"active":true})";

        for( auto _ : state )
        {
            (void)_;
            auto docOpt = Document::fromString( json );
            if( docOpt )
            {
                auto output = docOpt->toString( 0 );
                ::benchmark::DoNotOptimize( output );
            }
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void RoundTrip_LargeArray( ::benchmark::State& state )
    {
        Document original = createLargeArray();
        std::string json = original.toString( 0 );

        for( auto _ : state )
        {
            (void)_;
            auto docOpt = Document::fromString( json );
            if( docOpt )
            {
                auto output = docOpt->toString( 0 );
                ::benchmark::DoNotOptimize( output );
            }
        }
        state.SetItemsProcessed( state.iterations() );
    }

    //=====================================================================
    // Benchmark Registration
    //=====================================================================

    // Compact serialization
    BENCHMARK( SerializeSmallObject_Compact )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeMediumObject_Compact )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeLargeObject_Compact )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeSmallArray_Compact )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeLargeArray_Compact )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeNested_Compact )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeStringHeavy_Compact )->Unit( ::benchmark::kNanosecond );

    // Pretty printing
    BENCHMARK( SerializeSmallObject_Pretty )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeMediumObject_Pretty )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeLargeObject_Pretty )->Unit( ::benchmark::kNanosecond );

    // Round-trip
    BENCHMARK( RoundTrip_SmallObject )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( RoundTrip_LargeArray )->Unit( ::benchmark::kNanosecond );

} // namespace nfx::json::benchmark

BENCHMARK_MAIN();
