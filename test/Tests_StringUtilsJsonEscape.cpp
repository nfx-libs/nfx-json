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
 * @file Tests_StringUtilsJsonEscape.cpp
 * @brief Unit tests for nfx::string::jsonEscape() and jsonUnescape() functions
 */

#include <gtest/gtest.h>

#include <nfx/StringUtils.h>

using namespace nfx::string;

//==============================================================================
// jsonEscape Tests
//==============================================================================

TEST( JsonEscape, BasicEscapeSequences )
{
    // Test standard escape sequences
    EXPECT_EQ( jsonEscape( "\"" ), "\\\"" );
    EXPECT_EQ( jsonEscape( "\\" ), "\\\\" );
    EXPECT_EQ( jsonEscape( "/" ), "/" );
    EXPECT_EQ( jsonEscape( "\b" ), "\\b" );
    EXPECT_EQ( jsonEscape( "\f" ), "\\f" );
    EXPECT_EQ( jsonEscape( "\n" ), "\\n" );
    EXPECT_EQ( jsonEscape( "\r" ), "\\r" );
    EXPECT_EQ( jsonEscape( "\t" ), "\\t" );
}

TEST( JsonEscape, ControlCharacters )
{
    // Test control characters (0x00-0x1F)
    EXPECT_EQ( jsonEscape( std::string( 1, '\x00' ) ), "\\u0000" );
    EXPECT_EQ( jsonEscape( std::string( 1, '\x01' ) ), "\\u0001" );
    EXPECT_EQ( jsonEscape( std::string( 1, '\x0F' ) ), "\\u000F" );
    EXPECT_EQ( jsonEscape( std::string( 1, '\x1F' ) ), "\\u001F" );
}

TEST( JsonEscape, AsciiPassthrough )
{
    // ASCII characters should pass through unchanged (when escapeNonAscii=false)
    EXPECT_EQ( jsonEscape( "Hello World" ), "Hello World" );
    EXPECT_EQ( jsonEscape( "ABC123" ), "ABC123" );
    EXPECT_EQ( jsonEscape( "Test @#$%" ), "Test @#$%" );
}

TEST( JsonEscape, Utf8PassthroughWhenDisabled )
{
    // UTF-8 should pass through when escapeNonAscii=false (default)
    EXPECT_EQ( jsonEscape( "café" ), "café" );
    EXPECT_EQ( jsonEscape( "日本語" ), "日本語" );
    EXPECT_EQ( jsonEscape( "Привет" ), "Привет" );
    EXPECT_EQ( jsonEscape( "🎉" ), "🎉" );
}

TEST( JsonEscape, Utf8EscapingBasicMultilingualPlane )
{
    // Test escaping of BMP characters (U+0000 to U+FFFF)

    // Latin-1 Supplement: é (U+00E9)
    EXPECT_EQ( jsonEscape( "café", true ), "caf\\u00e9" );

    // Greek: α (U+03B1)
    EXPECT_EQ( jsonEscape( "α", true ), "\\u03b1" );

    // Cyrillic: Я (U+042F)
    EXPECT_EQ( jsonEscape( "Я", true ), "\\u042f" );

    // CJK: 中 (U+4E2D)
    EXPECT_EQ( jsonEscape( "中", true ), "\\u4e2d" );

    // Japanese: 日 (U+65E5)
    EXPECT_EQ( jsonEscape( "日本", true ), "\\u65e5\\u672c" );
}

TEST( JsonEscape, Utf8EscapingSupplementaryPlane )
{
    // Test escaping of supplementary plane characters with surrogate pairs

    // Emoji: 🎉 (U+1F389) -> \uD83C\uDF89
    EXPECT_EQ( jsonEscape( "🎉", true ), "\\ud83c\\udf89" );

    // Emoji: 😀 (U+1F600) -> \uD83D\uDE00
    EXPECT_EQ( jsonEscape( "😀", true ), "\\ud83d\\ude00" );

    // Musical symbol: 𝄞 (U+1D11E) -> \uD834\uDD1E
    EXPECT_EQ( jsonEscape( "𝄞", true ), "\\ud834\\udd1e" );
}

TEST( JsonEscape, MixedContent )
{
    // Test mixed ASCII + UTF-8
    EXPECT_EQ( jsonEscape( "Hello café!", true ), "Hello caf\\u00e9!" );
    EXPECT_EQ( jsonEscape( "Test 🎉 emoji", true ), "Test \\ud83c\\udf89 emoji" );

    // Mixed with escape sequences
    EXPECT_EQ( jsonEscape( "Line1\nCafé", true ), "Line1\\nCaf\\u00e9" );
    EXPECT_EQ( jsonEscape( "\"Hello\" 中文", true ), "\\\"Hello\\\" \\u4e2d\\u6587" );
}

TEST( JsonEscape, EmptyString )
{
    EXPECT_EQ( jsonEscape( "" ), "" );
    EXPECT_EQ( jsonEscape( "", true ), "" );
}

TEST( JsonEscape, MultiByteSequences )
{
    // 2-byte UTF-8: € (U+20AC)
    EXPECT_EQ( jsonEscape( "€", true ), "\\u20ac" );

    // 3-byte UTF-8: ￥ (U+FFE5)
    EXPECT_EQ( jsonEscape( "￥", true ), "\\uffe5" );

    // 4-byte UTF-8: 𐍈 (U+10348)
    EXPECT_EQ( jsonEscape( "𐍈", true ), "\\ud800\\udf48" );
}

//==============================================================================
// jsonUnescape Tests
//==============================================================================

TEST( JsonUnescape, BasicEscapeSequences )
{
    EXPECT_EQ( jsonUnescape( "\\\"" ), "\"" );
    EXPECT_EQ( jsonUnescape( "\\\\" ), "\\" );
    EXPECT_EQ( jsonUnescape( "\\/" ), "/" );
    EXPECT_EQ( jsonUnescape( "\\b" ), "\b" );
    EXPECT_EQ( jsonUnescape( "\\f" ), "\f" );
    EXPECT_EQ( jsonUnescape( "\\n" ), "\n" );
    EXPECT_EQ( jsonUnescape( "\\r" ), "\r" );
    EXPECT_EQ( jsonUnescape( "\\t" ), "\t" );
}

TEST( JsonUnescape, UnicodeEscapesBMP )
{
    // Basic Multilingual Plane
    EXPECT_EQ( jsonUnescape( "\\u0041" ), "A" );
    EXPECT_EQ( jsonUnescape( "\\u00e9" ), "é" );
    EXPECT_EQ( jsonUnescape( "\\u4e2d" ), "中" );
    EXPECT_EQ( jsonUnescape( "\\u65e5\\u672c" ), "日本" );
}

TEST( JsonUnescape, UnicodeEscapesSurrogatePairs )
{
    // Supplementary plane with surrogate pairs

    // 🎉 (U+1F389)
    EXPECT_EQ( jsonUnescape( "\\ud83c\\udf89" ), "🎉" );

    // 😀 (U+1F600)
    EXPECT_EQ( jsonUnescape( "\\ud83d\\ude00" ), "😀" );

    // 𝄞 (U+1D11E)
    EXPECT_EQ( jsonUnescape( "\\ud834\\udd1e" ), "𝄞" );
}

TEST( JsonUnescape, MixedContent )
{
    EXPECT_EQ( jsonUnescape( "Hello \\u00e9" ), "Hello é" );
    EXPECT_EQ( jsonUnescape( "Test \\ud83c\\udf89 emoji" ), "Test 🎉 emoji" );
    EXPECT_EQ( jsonUnescape( "Line1\\nCaf\\u00e9" ), "Line1\nCafé" );
}

TEST( JsonUnescape, PlainText )
{
    EXPECT_EQ( jsonUnescape( "Hello World" ), "Hello World" );
    EXPECT_EQ( jsonUnescape( "No escapes here" ), "No escapes here" );
}

TEST( JsonUnescape, EmptyString )
{
    EXPECT_EQ( jsonUnescape( "" ), "" );
}

TEST( JsonUnescape, InvalidEscapes )
{
    // Invalid escape sequences should return empty string
    EXPECT_EQ( jsonUnescape( "\\x" ), "" );     // Invalid escape
    EXPECT_EQ( jsonUnescape( "\\u" ), "" );     // Incomplete
    EXPECT_EQ( jsonUnescape( "\\u123" ), "" );  // Incomplete hex
    EXPECT_EQ( jsonUnescape( "\\uGGGG" ), "" ); // Invalid hex
    EXPECT_EQ( jsonUnescape( "test\\" ), "" );  // Backslash at end
}

TEST( JsonUnescape, InvalidSurrogatePairs )
{
    // High surrogate without low surrogate
    EXPECT_EQ( jsonUnescape( "\\ud83c" ), "" );
    EXPECT_EQ( jsonUnescape( "\\ud83cX" ), "" );

    // High surrogate with invalid low surrogate
    EXPECT_EQ( jsonUnescape( "\\ud83c\\u0041" ), "" );

    // Lone low surrogate
    EXPECT_EQ( jsonUnescape( "\\udf89" ), "" );
}

TEST( JsonUnescape, ControlCharacters )
{
    EXPECT_EQ( jsonUnescape( "\\u0000" ), std::string( 1, '\x00' ) );
    EXPECT_EQ( jsonUnescape( "\\u0001" ), std::string( 1, '\x01' ) );
    EXPECT_EQ( jsonUnescape( "\\u001f" ), std::string( 1, '\x1F' ) );
}

//==============================================================================
// Round-trip Tests
//==============================================================================

TEST( JsonRoundTrip, BasicStrings )
{
    auto testRoundTrip = []( const std::string& str ) {
        auto escaped = jsonEscape( str );
        auto unescaped = jsonUnescape( escaped );
        EXPECT_EQ( unescaped, str );
    };

    testRoundTrip( "Hello World" );
    testRoundTrip( "Test\nNewline" );
    testRoundTrip( "Quote: \"test\"" );
    testRoundTrip( "Backslash: \\" );
}

TEST( JsonRoundTrip, Utf8WithEscaping )
{
    auto testRoundTrip = []( const std::string& str ) {
        auto escaped = jsonEscape( str, true );
        auto unescaped = jsonUnescape( escaped );
        EXPECT_EQ( unescaped, str );
    };

    testRoundTrip( "café" );
    testRoundTrip( "日本語" );
    testRoundTrip( "Привет мир" );
    testRoundTrip( "🎉😀🎊" );
    testRoundTrip( "Mixed: Hello 世界 🌍" );
}

TEST( JsonRoundTrip, ComplexStrings )
{
    auto testRoundTrip = []( const std::string& str ) {
        auto escaped = jsonEscape( str, true );
        auto unescaped = jsonUnescape( escaped );
        EXPECT_EQ( unescaped, str );
    };

    testRoundTrip( "Line1\nLine2\tTab" );
    testRoundTrip( "\"Quote\" and \\backslash\\" );
    testRoundTrip( "Control: \x01\x02\x1F" );
    testRoundTrip( "Everything: \"\\/ \b\f\n\r\t café 🎉" );
}

//==============================================================================
// Edge Cases
//==============================================================================

TEST( JsonEdgeCases, LongStrings )
{
    // Test with long strings
    std::string longStr( 10000, 'a' );
    EXPECT_EQ( jsonEscape( longStr ), longStr );
    EXPECT_EQ( jsonUnescape( longStr ), longStr );
}

TEST( JsonEdgeCases, RepeatedEscapes )
{
    EXPECT_EQ( jsonEscape( "\n\n\n\n\n" ), "\\n\\n\\n\\n\\n" );
    EXPECT_EQ( jsonUnescape( "\\n\\n\\n\\n\\n" ), "\n\n\n\n\n" );
}

TEST( JsonEdgeCases, AllControlCharacters )
{
    // Test all control characters
    for( int i = 0; i < 32; ++i )
    {
        std::string input( 1, static_cast<char>( i ) );
        auto escaped = jsonEscape( input );

        // Should contain a backslash (escaped)
        EXPECT_NE( escaped.find( '\\' ), std::string::npos ) << "Control char " << i << " should be escaped";

        // Round trip should work
        auto unescaped = jsonUnescape( escaped );
        EXPECT_EQ( unescaped, input ) << "Round trip failed for control char " << i;
    }
}

TEST( JsonEdgeCases, BoundaryCodepoints )
{
    // Test boundary codepoints
    auto testRoundTrip = []( const std::string& str ) {
        auto escaped = jsonEscape( str, true );
        auto unescaped = jsonUnescape( escaped );
        EXPECT_EQ( unescaped, str );
    };

    testRoundTrip( "\x7F" ); // Max ASCII
    testRoundTrip( "€" );    // U+20AC (2-byte UTF-8)
    testRoundTrip( "ￜ" );    // U+FFDC (3-byte UTF-8)
    testRoundTrip( "🀀" );    // U+1F000 (4-byte UTF-8)
}
