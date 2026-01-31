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
    // Serialization benchmarks - Document & Builder
    //----------------------------------------------

    static void SerializeSmallObject( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            Document doc = createSmallObject();

            (void)_;
            auto json = doc.toString( 0 );
            ::benchmark::DoNotOptimize( json );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeSmallObject_Builder( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;
            Builder builder{ { 0 } };
            builder.writeStartObject()
                .write( "name", "John Doe" )
                .write( "age", 30 )
                .write( "active", true )
                .writeEndObject();
            auto output = builder.toString();
            ::benchmark::DoNotOptimize( output );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeMediumObject( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            Document doc = createMediumObject();

            (void)_;
            auto json = doc.toString( 0 );
            ::benchmark::DoNotOptimize( json );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeMediumObject_Builder( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;
            Builder builder{ { 0 } };
            builder.writeStartObject()
                .write( "name", "Jane Smith" )
                .write( "age", 25 )
                .write( "email", "jane.smith@example.com" )
                .writeKey( "address" )
                .writeStartObject()
                .write( "street", "123 Main St" )
                .write( "city", "Springfield" )
                .write( "zip", "12345" )
                .writeEndObject()
                .writeKey( "preferences" )
                .writeStartObject()
                .write( "notifications", true )
                .write( "newsletter", false )
                .write( "theme", 1 )
                .writeEndObject()
                .writeEndObject();
            auto output = builder.toString();
            ::benchmark::DoNotOptimize( output );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeLargeObject( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            Document doc = createLargeObject();

            (void)_;
            auto json = doc.toString( 0 );
            ::benchmark::DoNotOptimize( json );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeLargeObject_Builder( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;
            Builder builder{ { 0 } };
            builder.writeStartObject();
            for( int i = 0; i < 100; ++i )
            {
                builder.write( "field" + std::to_string( i ), "value" + std::to_string( i ) );
            }
            builder.writeEndObject();
            auto output = builder.toString();
            ::benchmark::DoNotOptimize( output );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeSmallArray( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            Document doc = createSmallArray();

            (void)_;
            auto json = doc.toString( 0 );
            ::benchmark::DoNotOptimize( json );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeSmallArray_Builder( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;
            Builder builder{ { 0 } };
            builder.writeStartArray();
            for( int i = 0; i < 10; ++i )
            {
                builder.write( i );
            }
            builder.writeEndArray();
            auto output = builder.toString();
            ::benchmark::DoNotOptimize( output );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeLargeArray( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            Document doc = createLargeArray();

            (void)_;
            auto json = doc.toString( 0 );
            ::benchmark::DoNotOptimize( json );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeLargeArray_Builder( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;
            Builder builder{ { 0 } };
            builder.writeStartArray();
            for( int i = 0; i < 1000; ++i )
            {
                builder.write( i );
            }
            builder.writeEndArray();
            auto output = builder.toString();
            ::benchmark::DoNotOptimize( output );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeNested( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            Document doc = createNestedDocument();

            (void)_;
            auto json = doc.toString( 0 );
            ::benchmark::DoNotOptimize( json );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeNested_Builder( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;
            Builder builder{ { 0 } };
            builder.writeStartObject()
                .writeKey( "level1" )
                .writeStartObject()
                .write( "active", true )
                .writeKey( "level2" )
                .writeStartObject()
                .write( "count", 42 )
                .writeKey( "level3" )
                .writeStartObject()
                .write( "value", "deep" )
                .writeEndObject()
                .writeEndObject()
                .writeEndObject()
                .writeEndObject();
            auto output = builder.toString();
            ::benchmark::DoNotOptimize( output );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeStringHeavy( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            Document doc = createStringHeavyDocument();

            (void)_;
            auto json = doc.toString( 0 );
            ::benchmark::DoNotOptimize( json );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeStringHeavy_Builder( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;
            Builder builder{ { 0 } };
            builder.writeStartObject()
                .write( "text", "Line 1\nLine 2\tTabbed\r\nLine 3 with \"quotes\"" )
                .write( "code", "const char* str = \"hello\\nworld\";" )
                .writeEndObject();
            auto output = builder.toString();
            ::benchmark::DoNotOptimize( output );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    //----------------------------------------------
    // Serialization benchmarks - Pretty (indent=2)
    //----------------------------------------------

    static void SerializeSmallObject_Pretty( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            Document doc = createSmallObject();

            (void)_;
            auto json = doc.toString( 2 );
            ::benchmark::DoNotOptimize( json );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeSmallObject_Pretty_Builder( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            (void)_;
            Builder builder{ { 2 } }; // 2-space indent
            builder.writeStartObject()
                .write( "name", "John Doe" )
                .write( "age", 30 )
                .write( "active", true )
                .writeEndObject();
            auto output = builder.toString();
            ::benchmark::DoNotOptimize( output );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeMediumObject_Pretty( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            Document doc = createMediumObject();

            (void)_;
            auto json = doc.toString( 2 );
            ::benchmark::DoNotOptimize( json );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeMediumObject_Pretty_Builder( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            Builder builder{ { 2 } }; // 2-space indent
            builder.writeStartObject()
                .write( "id", 12345 )
                .write( "name", "Medium Test Object" )
                .write( "description", "This is a medium-sized JSON object for benchmarking" )
                .write( "active", true )
                .write( "count", 42 )
                .write( "ratio", 3.14159 )
                .writeKey( "tags" )
                .writeStartArray()
                .write( "test" )
                .write( "benchmark" )
                .write( "json" )
                .writeEndArray()
                .writeKey( "metadata" )
                .writeStartObject()
                .write( "version", "1.0" )
                .write( "author", "Benchmark Suite" )
                .writeEndObject()
                .writeEndObject();
            auto output = builder.toString();
            ::benchmark::DoNotOptimize( output );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeLargeObject_Pretty( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            Document doc = createLargeObject();

            (void)_;
            auto json = doc.toString( 2 );
            ::benchmark::DoNotOptimize( json );
        }
        state.SetItemsProcessed( state.iterations() );
    }

    static void SerializeLargeObject_Pretty_Builder( ::benchmark::State& state )
    {
        for( auto _ : state )
        {
            Builder builder{ { 2 } }; // 2-space indent
            builder.writeStartObject();
            for( int i = 0; i < 100; ++i )
            {
                builder.write( "field" + std::to_string( i ), "value" + std::to_string( i ) );
            }
            builder.writeEndObject();
            auto output = builder.toString();
            ::benchmark::DoNotOptimize( output );
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

    // Serialization
    BENCHMARK( SerializeSmallObject )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeSmallObject_Builder )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeMediumObject )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeMediumObject_Builder )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeLargeObject )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeLargeObject_Builder )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeSmallArray )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeSmallArray_Builder )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeLargeArray )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeLargeArray_Builder )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeNested )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeNested_Builder )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeStringHeavy )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeStringHeavy_Builder )->Unit( ::benchmark::kNanosecond );

    // Pretty printing
    BENCHMARK( SerializeSmallObject_Pretty )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeSmallObject_Pretty_Builder )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeMediumObject_Pretty )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeMediumObject_Pretty_Builder )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeLargeObject_Pretty )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( SerializeLargeObject_Pretty_Builder )->Unit( ::benchmark::kNanosecond );

    // Round-trip
    BENCHMARK( RoundTrip_SmallObject )->Unit( ::benchmark::kNanosecond );
    BENCHMARK( RoundTrip_LargeArray )->Unit( ::benchmark::kNanosecond );
} // namespace nfx::json::benchmark

BENCHMARK_MAIN();
