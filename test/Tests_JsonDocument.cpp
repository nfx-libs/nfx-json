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
 * @file Tests_JsonDocument.cpp
 * @brief Unit tests for JSON Document serialization and manipulation
 * @details Tests covering JSON parsing, path-based access, array operations, validation,
 *          nested object navigation, and enterprise-grade JSON document handling
 */

#include <gtest/gtest.h>

#include <nfx/Json.h>

namespace nfx::json::test
{
    //=====================================================================
    // JSON Document tests
    //=====================================================================

    //----------------------------------------------
    // JSON Document construction
    //----------------------------------------------

    TEST( DocumentTest, DefaultConstruction )
    {
        Document doc;
        EXPECT_TRUE( doc.isValid() );
        EXPECT_EQ( doc.toString(), "{}" );
    }

    TEST( DocumentTest, FactoryMethods )
    {
        Document obj;
        EXPECT_TRUE( obj.isValid() );
        EXPECT_EQ( obj.toString(), "{}" );

        Document arr;
        arr.set<Array>( "" );
        EXPECT_TRUE( arr.isValid() );
        EXPECT_EQ( arr.toString(), "[]" );
    }

    TEST( DocumentTest, CopyAndMove )
    {
        Document original;
        original.set<std::string>( "test", "value" );

        // Copy constructor
        Document copied( original );
        EXPECT_EQ( copied.get<std::string>( "test" ), "value" );

        // Move constructor
        Document moved( std::move( copied ) );
        EXPECT_EQ( moved.get<std::string>( "test" ), "value" );

        // Copy assignment
        Document assigned;
        assigned = original;
        EXPECT_EQ( assigned.get<std::string>( "test" ), "value" );
    }

    //----------------------------------------------
    // JSON parsing
    //----------------------------------------------

    TEST( DocumentTest, ParseValidJson )
    {
        std::string jsonStr = R"({"name": "John", "age": 30, "active": true})";
        auto maybeDoc = Document::fromString( jsonStr );

        ASSERT_TRUE( maybeDoc.has_value() );
        Document doc = maybeDoc.value();

        EXPECT_EQ( doc.get<std::string>( "name" ), "John" );
        EXPECT_EQ( doc.get<int64_t>( "age" ), 30 );
        EXPECT_EQ( doc.get<bool>( "active" ), true );
    }

    TEST( DocumentTest, ParseInvalidJson )
    {
        std::string invalidJson = R"({"name": "John", "age":})";
        auto maybeDoc = Document::fromString( invalidJson );

        EXPECT_FALSE( maybeDoc.has_value() );
    }

    TEST( DocumentTest, ParseEmptyAndWhitespaceStrings )
    {
        // Test empty string - not valid JSON
        auto emptyResult = Document::fromString( "" );
        EXPECT_FALSE( emptyResult.has_value() );

        // Test whitespace-only strings - not valid JSON
        auto spacesResult = Document::fromString( "   " );
        EXPECT_FALSE( spacesResult.has_value() );

        auto tabsResult = Document::fromString( "\t\t" );
        EXPECT_FALSE( tabsResult.has_value() );

        auto newlinesResult = Document::fromString( "\n\n" );
        EXPECT_FALSE( newlinesResult.has_value() );

        auto mixedWhitespaceResult = Document::fromString( " \t\n\r " );
        EXPECT_FALSE( mixedWhitespaceResult.has_value() );

        // Test valid JSON with surrounding whitespace (should work)
        auto validWithWhitespaceResult = Document::fromString( "  {\"test\": \"value\"}  " );
        EXPECT_TRUE( validWithWhitespaceResult.has_value() );
        if( validWithWhitespaceResult.has_value() )
        {
            EXPECT_EQ( validWithWhitespaceResult->get<std::string>( "test" ), "value" );
        }
    }

    TEST( DocumentTest, ParseMinimalValidJson )
    {
        // Test minimal valid JSON values
        auto nullResult = Document::fromString( "null" );
        EXPECT_TRUE( nullResult.has_value() );

        auto trueResult = Document::fromString( "true" );
        EXPECT_TRUE( trueResult.has_value() );

        auto falseResult = Document::fromString( "false" );
        EXPECT_TRUE( falseResult.has_value() );

        auto numberResult = Document::fromString( "42" );
        EXPECT_TRUE( numberResult.has_value() );

        auto stringResult = Document::fromString( "\"hello\"" );
        EXPECT_TRUE( stringResult.has_value() );

        auto emptyArrayResult = Document::fromString( "[]" );
        EXPECT_TRUE( emptyArrayResult.has_value() );

        auto emptyObjectResult = Document::fromString( "{}" );
        EXPECT_TRUE( emptyObjectResult.has_value() );
    }

    TEST( DocumentTest, ParseJsonStringEdgeCases )
    {
        // Test various invalid JSON edge cases
        auto singleCharResult = Document::fromString( "{" );
        EXPECT_FALSE( singleCharResult.has_value() );

        auto unclosedStringResult = Document::fromString( "\"unclosed" );
        EXPECT_FALSE( unclosedStringResult.has_value() );

        auto invalidEscapeResult = Document::fromString( "\"invalid\\xescape\"" );
        EXPECT_FALSE( invalidEscapeResult.has_value() );

        auto trailingCommaResult = Document::fromString( "{\"key\": \"value\",}" );
        EXPECT_FALSE( trailingCommaResult.has_value() );

        // Test that single quote strings are invalid (JSON requires double quotes)
        auto singleQuoteResult = Document::fromString( "{'key': 'value'}" );
        EXPECT_FALSE( singleQuoteResult.has_value() );
    }

    TEST( DocumentTest, ParseNestedJson )
    {
        std::string jsonStr = R"({
            "user": {
                "profile": {
                    "name": "Alice",
                    "settings": {
                        "theme": "dark"
                    }
                }
            }
        })";

        auto maybeDoc = Document::fromString( jsonStr );
        ASSERT_TRUE( maybeDoc.has_value() );
        Document doc = maybeDoc.value();

        EXPECT_EQ( doc.get<std::string>( "user.profile.name" ), "Alice" );
        EXPECT_EQ( doc.get<std::string>( "user.profile.settings.theme" ), "dark" );
    }

    //----------------------------------------------
    // Unicode and non-ASCII character tests
    //----------------------------------------------

    TEST( DocumentTest, ParseUnicodeBasicMultilingualPlane )
    {
        // Test characters in the Basic Multilingual Plane (BMP)
        // Latin accented characters
        std::string latinJson = R"({"text": "café"})";
        auto latinDoc = Document::fromString( latinJson );
        ASSERT_TRUE( latinDoc.has_value() );
        EXPECT_EQ( latinDoc->get<std::string>( "text" ), "café" );

        // Greek characters
        std::string greekJson = R"({"text": "Ελληνικά"})";
        auto greekDoc = Document::fromString( greekJson );
        ASSERT_TRUE( greekDoc.has_value() );
        EXPECT_EQ( greekDoc->get<std::string>( "text" ), "Ελληνικά" );

        // Cyrillic characters
        std::string cyrillicJson = R"({"text": "Русский"})";
        auto cyrillicDoc = Document::fromString( cyrillicJson );
        ASSERT_TRUE( cyrillicDoc.has_value() );
        EXPECT_EQ( cyrillicDoc->get<std::string>( "text" ), "Русский" );

        // Arabic characters
        std::string arabicJson = R"({"text": "العربية"})";
        auto arabicDoc = Document::fromString( arabicJson );
        ASSERT_TRUE( arabicDoc.has_value() );
        EXPECT_EQ( arabicDoc->get<std::string>( "text" ), "العربية" );

        // Hebrew characters
        std::string hebrewJson = R"({"text": "עברית"})";
        auto hebrewDoc = Document::fromString( hebrewJson );
        ASSERT_TRUE( hebrewDoc.has_value() );
        EXPECT_EQ( hebrewDoc->get<std::string>( "text" ), "עברית" );
    }

    TEST( DocumentTest, ParseUnicodeCJKCharacters )
    {
        // Chinese characters
        std::string chineseJson = R"({"text": "中文"})";
        auto chineseDoc = Document::fromString( chineseJson );
        ASSERT_TRUE( chineseDoc.has_value() );
        EXPECT_EQ( chineseDoc->get<std::string>( "text" ), "中文" );

        // Japanese characters (Hiragana, Katakana, Kanji)
        std::string japaneseJson = R"({"text": "日本語ひらがなカタカナ"})";
        auto japaneseDoc = Document::fromString( japaneseJson );
        ASSERT_TRUE( japaneseDoc.has_value() );
        EXPECT_EQ( japaneseDoc->get<std::string>( "text" ), "日本語ひらがなカタカナ" );

        // Korean characters (Hangul)
        std::string koreanJson = R"({"text": "한국어"})";
        auto koreanDoc = Document::fromString( koreanJson );
        ASSERT_TRUE( koreanDoc.has_value() );
        EXPECT_EQ( koreanDoc->get<std::string>( "text" ), "한국어" );
    }

    TEST( DocumentTest, ParseUnicodeEmojiWithSurrogatePairs )
    {
        // Emoji require UTF-16 surrogate pairs (beyond BMP, U+10000 and above)
        // Grinning face emoji: U+1F600 = \uD83D\uDE00
        std::string emojiJson = R"({"emoji": "\uD83D\uDE00"})";
        auto emojiDoc = Document::fromString( emojiJson );
        ASSERT_TRUE( emojiDoc.has_value() );
        EXPECT_EQ( emojiDoc->get<std::string>( "emoji" ), "😀" );

        // Heart emoji: U+2764 (BMP, no surrogate needed)
        std::string heartJson = R"({"emoji": "\u2764"})";
        auto heartDoc = Document::fromString( heartJson );
        ASSERT_TRUE( heartDoc.has_value() );
        EXPECT_EQ( heartDoc->get<std::string>( "emoji" ), "❤" );

        // Thumbs up: U+1F44D = \uD83D\uDC4D
        std::string thumbsJson = R"({"emoji": "\uD83D\uDC4D"})";
        auto thumbsDoc = Document::fromString( thumbsJson );
        ASSERT_TRUE( thumbsDoc.has_value() );
        EXPECT_EQ( thumbsDoc->get<std::string>( "emoji" ), "👍" );

        // Fire emoji: U+1F525 = \uD83D\uDD25
        std::string fireJson = R"({"emoji": "\uD83D\uDD25"})";
        auto fireDoc = Document::fromString( fireJson );
        ASSERT_TRUE( fireDoc.has_value() );
        EXPECT_EQ( fireDoc->get<std::string>( "emoji" ), "🔥" );

        // Rocket emoji: U+1F680 = \uD83D\uDE80
        std::string rocketJson = R"({"emoji": "\uD83D\uDE80"})";
        auto rocketDoc = Document::fromString( rocketJson );
        ASSERT_TRUE( rocketDoc.has_value() );
        EXPECT_EQ( rocketDoc->get<std::string>( "emoji" ), "🚀" );
    }

    TEST( DocumentTest, ParseUnicodeComplexEmoji )
    {
        // Multiple emoji in one string
        std::string multiEmojiJson = R"({"text": "\uD83D\uDE00\uD83D\uDE0D\uD83D\uDE02"})";
        auto multiEmojiDoc = Document::fromString( multiEmojiJson );
        ASSERT_TRUE( multiEmojiDoc.has_value() );
        EXPECT_EQ( multiEmojiDoc->get<std::string>( "text" ), "😀😍😂" );

        // Mix of emoji and text
        std::string mixedJson = R"({"text": "Hello \uD83C\uDF0D World!"})";
        auto mixedDoc = Document::fromString( mixedJson );
        ASSERT_TRUE( mixedDoc.has_value() );
        EXPECT_EQ( mixedDoc->get<std::string>( "text" ), "Hello 🌍 World!" );

        // Musical notes: U+1F3B5 = \uD83C\uDFB5
        std::string musicJson = R"({"emoji": "\uD83C\uDFB5"})";
        auto musicDoc = Document::fromString( musicJson );
        ASSERT_TRUE( musicDoc.has_value() );
        EXPECT_EQ( musicDoc->get<std::string>( "emoji" ), "🎵" );
    }

    TEST( DocumentTest, ParseUnicodeSpecialCharacters )
    {
        // Mathematical symbols
        std::string mathJson = R"({"text": "∑∫∂∇"})";
        auto mathDoc = Document::fromString( mathJson );
        ASSERT_TRUE( mathDoc.has_value() );
        EXPECT_EQ( mathDoc->get<std::string>( "text" ), "∑∫∂∇" );

        // Currency symbols
        std::string currencyJson = R"({"text": "€£¥₹"})";
        auto currencyDoc = Document::fromString( currencyJson );
        ASSERT_TRUE( currencyDoc.has_value() );
        EXPECT_EQ( currencyDoc->get<std::string>( "text" ), "€£¥₹" );

        // Arrows and symbols
        std::string symbolsJson = R"({"text": "→←↑↓"})";
        auto symbolsDoc = Document::fromString( symbolsJson );
        ASSERT_TRUE( symbolsDoc.has_value() );
        EXPECT_EQ( symbolsDoc->get<std::string>( "text" ), "→←↑↓" );

        // Box drawing characters
        std::string boxJson = R"({"text": "┌─┐│└┘"})";
        auto boxDoc = Document::fromString( boxJson );
        ASSERT_TRUE( boxDoc.has_value() );
        EXPECT_EQ( boxDoc->get<std::string>( "text" ), "┌─┐│└┘" );
    }

    TEST( DocumentTest, ParseUnicodeEscapeSequences )
    {
        // Test \uXXXX escape sequences
        // Latin small letter e with acute: é = U+00E9
        std::string escapeJson = R"({"text": "caf\u00E9"})";
        auto escapeDoc = Document::fromString( escapeJson );
        ASSERT_TRUE( escapeDoc.has_value() );
        EXPECT_EQ( escapeDoc->get<std::string>( "text" ), "café" );

        // Copyright symbol: © = U+00A9
        std::string copyrightJson = R"({"text": "\u00A9 2025"})";
        auto copyrightDoc = Document::fromString( copyrightJson );
        ASSERT_TRUE( copyrightDoc.has_value() );
        EXPECT_EQ( copyrightDoc->get<std::string>( "text" ), "© 2025" );

        // Degree symbol: ° = U+00B0
        std::string degreeJson = R"({"temp": "25\u00B0C"})";
        auto degreeDoc = Document::fromString( degreeJson );
        ASSERT_TRUE( degreeDoc.has_value() );
        EXPECT_EQ( degreeDoc->get<std::string>( "temp" ), "25°C" );

        // Yen sign: ¥ = U+00A5
        std::string yenJson = R"({"price": "\u00A5100"})";
        auto yenDoc = Document::fromString( yenJson );
        ASSERT_TRUE( yenDoc.has_value() );
        EXPECT_EQ( yenDoc->get<std::string>( "price" ), "¥100" );
    }

    TEST( DocumentTest, ParseInvalidSurrogatePairs )
    {
        // High surrogate without low surrogate
        std::string invalidHighJson = R"({"text": "\uD83D"})";
        auto invalidHighDoc = Document::fromString( invalidHighJson );
        EXPECT_FALSE( invalidHighDoc.has_value() );

        // Low surrogate without high surrogate
        std::string invalidLowJson = R"({"text": "\uDE00"})";
        auto invalidLowDoc = Document::fromString( invalidLowJson );
        EXPECT_FALSE( invalidLowDoc.has_value() );

        // High surrogate followed by non-surrogate
        std::string invalidFollowJson = R"({"text": "\uD83D\u0041"})";
        auto invalidFollowDoc = Document::fromString( invalidFollowJson );
        EXPECT_FALSE( invalidFollowDoc.has_value() );

        // High surrogate followed by invalid low surrogate range
        std::string invalidRangeJson = R"({"text": "\uD83D\u1234"})";
        auto invalidRangeDoc = Document::fromString( invalidRangeJson );
        EXPECT_FALSE( invalidRangeDoc.has_value() );
    }

    TEST( DocumentTest, ParseInvalidUnicodeEscapeSequences )
    {
        // Invalid hex digits - non-hexadecimal characters
        EXPECT_FALSE( Document::fromString( R"({"text": "\uGGGG"})" ).has_value() );
        EXPECT_FALSE( Document::fromString( R"({"text": "\u12XY"})" ).has_value() );
        EXPECT_FALSE( Document::fromString( R"({"text": "\uZZZZ"})" ).has_value() );

        // Incomplete escape sequences at end of string
        EXPECT_FALSE( Document::fromString( R"({"text": "\u"})" ).has_value() );
        EXPECT_FALSE( Document::fromString( R"({"text": "\u1"})" ).has_value() );
        EXPECT_FALSE( Document::fromString( R"({"text": "\u12"})" ).has_value() );
        EXPECT_FALSE( Document::fromString( R"({"text": "\u123"})" ).has_value() );

        // Incomplete escape in middle of string
        EXPECT_FALSE( Document::fromString( R"({"text": "Hello\u12World"})" ).has_value() );

        // Mixed valid and invalid escapes
        EXPECT_FALSE( Document::fromString( R"({"text": "\u0041\uGGGG\u0042"})" ).has_value() );
    }

    TEST( DocumentTest, SerializeUnicodeCharacters )
    {
        // Test that unicode characters round-trip correctly
        Document doc;
        doc.set<std::string>( "emoji", "😀🎉🔥" );
        doc.set<std::string>( "chinese", "你好世界" );
        doc.set<std::string>( "arabic", "مرحبا" );
        doc.set<std::string>( "mixed", "Hello 世界 🌍!" );

        std::string serialized = doc.toString();
        auto deserialized = Document::fromString( serialized );

        ASSERT_TRUE( deserialized.has_value() );
        EXPECT_EQ( deserialized->get<std::string>( "emoji" ), "😀🎉🔥" );
        EXPECT_EQ( deserialized->get<std::string>( "chinese" ), "你好世界" );
        EXPECT_EQ( deserialized->get<std::string>( "arabic" ), "مرحبا" );
        EXPECT_EQ( deserialized->get<std::string>( "mixed" ), "Hello 世界 🌍!" );
    }

    TEST( DocumentTest, UnicodeInObjectKeys )
    {
        // Test unicode characters in object keys
        std::string unicodeKeyJson = R"({"名前": "太郎", "🔑": "value"})";
        auto doc = Document::fromString( unicodeKeyJson );
        ASSERT_TRUE( doc.has_value() );
        EXPECT_EQ( doc->get<std::string>( "名前" ), "太郎" );
        EXPECT_EQ( doc->get<std::string>( "🔑" ), "value" );
    }

    TEST( DocumentTest, UnicodeEdgeCases )
    {
        // Null character (U+0000)
        std::string nullCharJson = R"({"text": "\u0000"})";
        auto nullCharDoc = Document::fromString( nullCharJson );
        ASSERT_TRUE( nullCharDoc.has_value() );
        auto text = nullCharDoc->get<std::string>( "text" );
        ASSERT_TRUE( text.has_value() );
        EXPECT_EQ( text->length(), 1 );
        EXPECT_EQ( ( *text )[0], '\0' );

        // Space character (U+0020)
        std::string spaceJson = R"({"text": "\u0020"})";
        auto spaceDoc = Document::fromString( spaceJson );
        ASSERT_TRUE( spaceDoc.has_value() );
        EXPECT_EQ( spaceDoc->get<std::string>( "text" ), " " );

        // Non-breaking space (U+00A0) - UTF-8: 0xC2 0xA0
        std::string nbspJson = R"({"text": "\u00A0"})";
        auto nbspDoc = Document::fromString( nbspJson );
        ASSERT_TRUE( nbspDoc.has_value() );
        EXPECT_EQ( nbspDoc->get<std::string>( "text" ), "\xC2\xA0" );

        // Zero-width space (U+200B) - UTF-8: 0xE2 0x80 0x8B
        std::string zwspJson = R"({"text": "\u200B"})";
        auto zwspDoc = Document::fromString( zwspJson );
        ASSERT_TRUE( zwspDoc.has_value() );
        EXPECT_EQ( zwspDoc->get<std::string>( "text" ), "\xE2\x80\x8B" );
    }

    //----------------------------------------------
    // Value access
    //----------------------------------------------

    TEST( DocumentTest, BasicValueAccess )
    {
        Document doc;
        doc.set<std::string>( "name", "Bob" );
        doc.set<int64_t>( "age", 25 );
        doc.set<double>( "height", 1.75 );
        doc.set<bool>( "married", false );
        doc.setNull( "spouse" );

        EXPECT_EQ( doc.get<std::string>( "name" ), "Bob" );
        EXPECT_EQ( doc.get<int64_t>( "age" ), 25 );
        EXPECT_EQ( doc.get<double>( "height" ), 1.75 );
        EXPECT_EQ( doc.get<bool>( "married" ), false );
        EXPECT_TRUE( doc.contains( "spouse" ) );
    }

    TEST( DocumentTest, PathBasedAccess )
    {
        Document doc;
        doc.set<std::string>( "user.profile.firstName", "Charlie" );
        doc.set<std::string>( "user.profile.lastName", "Brown" );
        doc.set<int64_t>( "user.settings.notifications", 1 );

        EXPECT_EQ( doc.get<std::string>( "user.profile.firstName" ), "Charlie" );
        EXPECT_EQ( doc.get<std::string>( "user.profile.lastName" ), "Brown" );
        EXPECT_EQ( doc.get<int64_t>( "user.settings.notifications" ), 1 );
    }

    TEST( DocumentTest, NonExistentFields )
    {
        Document doc;

        // Test that non-existent fields return empty optionals
        EXPECT_FALSE( doc.contains( "nonexistent" ) );
        EXPECT_FALSE( doc.get<std::string>( "nonexistent" ).has_value() );
        EXPECT_FALSE( doc.get<int64_t>( "nonexistent" ).has_value() );
        EXPECT_FALSE( doc.get<double>( "nonexistent" ).has_value() );
        EXPECT_FALSE( doc.get<bool>( "nonexistent" ).has_value() );
    }

    //----------------------------------------------
    // Array operations
    //----------------------------------------------

    TEST( DocumentTest, BasicArrayOperations )
    {
        Document doc;

        // Use Document API with JSON Pointer to add array elements
        doc.set<int64_t>( "/numbers/0", 1 );
        doc.set<int64_t>( "/numbers/1", 2 );
        doc.set<int64_t>( "/numbers/2", 3 );

        EXPECT_TRUE( doc.is<Array>( "numbers" ) );
        EXPECT_EQ( doc.get<Array>( "numbers" ).value().size(), 3 );
    }

    TEST( DocumentTest, ArrayWithDifferentTypes )
    {
        Document doc;

        // Use Document API with JSON Pointer to add mixed type elements
        doc.set<std::string>( "/mixed/0", "hello" );
        doc.set<int64_t>( "/mixed/1", 42 );
        doc.set<double>( "/mixed/2", 3.14 );

        EXPECT_EQ( doc.get<Array>( "mixed" ).value().size(), 3 );
    }

    TEST( DocumentTest, ArrayElementAccess )
    {
        std::string jsonStr = R"({"items": [{"name": "item1"}, {"name": "item2"}]})";
        auto maybeDoc = Document::fromString( jsonStr );
        ASSERT_TRUE( maybeDoc.has_value() );
        Document doc = maybeDoc.value();

        auto array = doc.get<Array>( "items" );
        ASSERT_TRUE( array.has_value() );

        // Access elements directly via array index
        ASSERT_GE( array->size(), 2 );
        auto& firstItem = ( *array )[0];
        auto& secondItem = ( *array )[1];

        EXPECT_EQ( firstItem.get<std::string>( "name" ).value_or( "" ), "item1" );
        EXPECT_EQ( secondItem.get<std::string>( "name" ).value_or( "" ), "item2" );
    }

    TEST( DocumentTest, ClearArray )
    {
        Document doc;

        // Use Document API to add array elements
        doc.set<int64_t>( "/numbers/0", 1 );
        doc.set<int64_t>( "/numbers/1", 2 );
        doc.set<int64_t>( "/numbers/2", 3 );

        EXPECT_EQ( doc.get<Array>( "numbers" ).value().size(), 3 );

        // Clear the array using rootRef for direct modification
        if( auto arrRef = doc.at( "numbers" ).rootRef<Array>() )
        {
            arrRef->get().clear();
        }
        EXPECT_EQ( doc.get<Array>( "numbers" ).value().size(), 0 );
        EXPECT_TRUE( doc.is<Array>( "numbers" ) ); // Should still be an array, just empty

        // Test clearing non-existent array (should not crash)
        auto nonexistentArray = doc.get<Array>( "nonexistent" );
        if( nonexistentArray.has_value() )
        {
            nonexistentArray.value().clear();
        }

        // Test clearing non-array field (should not crash)
        doc.set<std::string>( "notAnArray", "value" );
        auto notAnArray = doc.get<Array>( "notAnArray" );
        if( notAnArray.has_value() )
        {
            notAnArray.value().clear();
        }
        EXPECT_EQ( doc.get<std::string>( "notAnArray" ), "value" ); // Should remain unchanged
    }

    TEST( DocumentTest, ArrayElementPrimitiveAccess )
    {
        Document doc;

        doc.set<std::string>( "/strings/0", "hello" );
        doc.set<std::string>( "/strings/1", "world" );
        doc.set<std::string>( "/strings/2", "test" );

        doc.set<int64_t>( "/numbers/0", 10 );
        doc.set<int64_t>( "/numbers/1", 20 );
        doc.set<int64_t>( "/numbers/2", 30 );

        doc.set<double>( "/doubles/0", 1.5 );
        doc.set<double>( "/doubles/1", 2.5 );
        doc.set<double>( "/doubles/2", 3.5 );

        doc.set<bool>( "/bools/0", true );
        doc.set<bool>( "/bools/1", false );
        doc.set<bool>( "/bools/2", true );

        // Test string array element access
        auto stringArray = doc.get<Array>( "strings" ).value();
        EXPECT_EQ( stringArray[0].get<std::string>( "" ).value_or( "" ), "hello" );
        EXPECT_EQ( stringArray[1].get<std::string>( "" ).value_or( "" ), "world" );
        EXPECT_EQ( stringArray[2].get<std::string>( "" ).value_or( "" ), "test" );
        EXPECT_FALSE(
            ( stringArray.size() > 10 && stringArray[10].get<std::string>( "" ).has_value() ) ); // Out of bounds

        // Test integer array element access
        auto numberArray = doc.get<Array>( "numbers" ).value();
        EXPECT_EQ( numberArray[0].get<int64_t>( "" ).value_or( 0 ), 10 );
        EXPECT_EQ( numberArray[1].get<int64_t>( "" ).value_or( 0 ), 20 );
        EXPECT_EQ( numberArray[2].get<int64_t>( "" ).value_or( 0 ), 30 );
        EXPECT_FALSE( ( numberArray.size() > 10 && numberArray[10].get<int64_t>( "" ).has_value() ) ); // Out of bounds

        // Test double array element access
        auto doubleArray = doc.get<Array>( "doubles" ).value();
        EXPECT_EQ( doubleArray[0].get<double>( "" ).value_or( 0.0 ), 1.5 );
        EXPECT_EQ( doubleArray[1].get<double>( "" ).value_or( 0.0 ), 2.5 );
        EXPECT_EQ( doubleArray[2].get<double>( "" ).value_or( 0.0 ), 3.5 );
        EXPECT_FALSE( ( doubleArray.size() > 10 && doubleArray[10].get<double>( "" ).has_value() ) ); // Out of bounds

        // Test boolean array element access
        auto boolArray = doc.get<Array>( "bools" ).value();
        EXPECT_EQ( boolArray[0].get<bool>( "" ).value_or( false ), true );
        EXPECT_EQ( boolArray[1].get<bool>( "" ).value_or( false ), false );
        EXPECT_EQ( boolArray[2].get<bool>( "" ).value_or( false ), true );
        EXPECT_FALSE( ( boolArray.size() > 10 && boolArray[10].get<bool>( "" ).has_value() ) ); // Out of bounds

        // Test type safety - accessing wrong types should return nullopt
        EXPECT_FALSE( numberArray[0].get<std::string>( "" ).has_value() );                  // int accessed as string
        EXPECT_FALSE( stringArray[0].get<int64_t>( "" ).has_value() );                      // string accessed as int
        EXPECT_FALSE( boolArray[0].get<double>( "" ).has_value() );                         // bool accessed as double
        EXPECT_FALSE( doc.get<Array>( "strings" ).value()[0].get<bool>( "" ).has_value() ); // string accessed as bool

        // Test non-existent arrays
        EXPECT_FALSE( doc.get<Array>( "nonexistent" ).has_value() );
        EXPECT_FALSE( doc.get<Array>( "nonexistent" ).has_value() );
        EXPECT_FALSE( doc.get<Array>( "nonexistent" ).has_value() );
        EXPECT_FALSE( doc.get<Array>( "nonexistent" ).has_value() );

        // Test accessing non-array fields
        doc.set<std::string>( "notArray", "value" );
        EXPECT_FALSE( doc.get<Array>( "notArray" ).has_value() );
        EXPECT_FALSE( doc.get<Array>( "notArray" ).has_value() );
    }

    TEST( DocumentTest, ArrayElementAccessWithComplexJson )
    {
        std::string jsonStr = R"({
            "users": [
                "alice", "bob", "charlie"
            ],
            "scores": [95, 87, 92, 78],
            "prices": [19.99, 25.50, 12.75],
            "flags": [true, false, true, false]
        })";

        auto maybeDoc = Document::fromString( jsonStr );
        ASSERT_TRUE( maybeDoc.has_value() );
        Document doc = maybeDoc.value();

        // Test string array from parsed JSON
        EXPECT_EQ( doc.get<Array>( "users" ).value()[0].get<std::string>( "" ).value_or( "" ), "alice" );
        EXPECT_EQ( doc.get<Array>( "users" ).value()[1].get<std::string>( "" ).value_or( "" ), "bob" );
        EXPECT_EQ( doc.get<Array>( "users" ).value()[2].get<std::string>( "" ).value_or( "" ), "charlie" );

        // Test integer array from parsed JSON
        EXPECT_EQ( doc.get<Array>( "scores" ).value()[0].get<int64_t>( "" ).value_or( 0 ), 95 );
        EXPECT_EQ( doc.get<Array>( "scores" ).value()[1].get<int64_t>( "" ).value_or( 0 ), 87 );
        EXPECT_EQ( doc.get<Array>( "scores" ).value()[2].get<int64_t>( "" ).value_or( 0 ), 92 );
        EXPECT_EQ( doc.get<Array>( "scores" ).value()[3].get<int64_t>( "" ).value_or( 0 ), 78 );

        // Test double array from parsed JSON
        EXPECT_EQ( doc.get<Array>( "prices" ).value()[0].get<double>( "" ).value_or( 0.0 ), 19.99 );
        EXPECT_EQ( doc.get<Array>( "prices" ).value()[1].get<double>( "" ).value_or( 0.0 ), 25.50 );
        EXPECT_EQ( doc.get<Array>( "prices" ).value()[2].get<double>( "" ).value_or( 0.0 ), 12.75 );

        // Test boolean array from parsed JSON
        EXPECT_EQ( doc.get<Array>( "flags" ).value()[0].get<bool>( "" ).value_or( false ), true );
        EXPECT_EQ( doc.get<Array>( "flags" ).value()[1].get<bool>( "" ).value_or( false ), false );
        EXPECT_EQ( doc.get<Array>( "flags" ).value()[2].get<bool>( "" ).value_or( false ), true );
        EXPECT_EQ( doc.get<Array>( "flags" ).value()[3].get<bool>( "" ).value_or( false ), false );
    }

    //----------------------------------------------
    // Integer Types
    //----------------------------------------------

    TEST( DocumentTest, AllIntegerTypes )
    {
        Document doc;

        // Test values for each integer type
        int8_t val_i8 = -127;
        int16_t val_i16 = -32767;
        int32_t val_i32 = -2147483647;
        int64_t val_i64 = -9223372036854775807LL;
        uint8_t val_u8 = 255;
        uint16_t val_u16 = 65535;
        uint32_t val_u32 = 4294967295U;
        uint64_t val_u64 = 18446744073709551615ULL;

        // Test Document-level set/get for all integer types
        doc.set<int8_t>( "signed/i8", val_i8 );
        doc.set<int16_t>( "signed/i16", val_i16 );
        doc.set<int32_t>( "signed/i32", val_i32 );
        doc.set<int64_t>( "signed/i64", val_i64 );
        doc.set<uint8_t>( "unsigned/u8", val_u8 );
        doc.set<uint16_t>( "unsigned/u16", val_u16 );
        doc.set<uint32_t>( "unsigned/u32", val_u32 );
        doc.set<uint64_t>( "unsigned/u64", val_u64 );

        // Test Document-level get for all integer types
        EXPECT_EQ( doc.get<int8_t>( "signed/i8" ).value(), val_i8 );
        EXPECT_EQ( doc.get<int16_t>( "signed/i16" ).value(), val_i16 );
        EXPECT_EQ( doc.get<int32_t>( "signed/i32" ).value(), val_i32 );
        EXPECT_EQ( doc.get<int64_t>( "signed/i64" ).value(), val_i64 );
        EXPECT_EQ( doc.get<uint8_t>( "unsigned/u8" ).value(), val_u8 );
        EXPECT_EQ( doc.get<uint16_t>( "unsigned/u16" ).value(), val_u16 );
        EXPECT_EQ( doc.get<uint32_t>( "unsigned/u32" ).value(), val_u32 );
        EXPECT_EQ( doc.get<uint64_t>( "unsigned/u64" ).value(), val_u64 );

        // Test Object-level set/get for all integer types using Document API
        doc.set<int8_t>( "/testObject/signed/i8", val_i8 );
        doc.set<int16_t>( "/testObject/signed/i16", val_i16 );
        doc.set<int32_t>( "/testObject/signed/i32", val_i32 );
        doc.set<int64_t>( "/testObject/signed/i64", val_i64 );
        doc.set<uint8_t>( "/testObject/unsigned/u8", val_u8 );
        doc.set<uint16_t>( "/testObject/unsigned/u16", val_u16 );
        doc.set<uint32_t>( "/testObject/unsigned/u32", val_u32 );
        doc.set<uint64_t>( "/testObject/unsigned/u64", val_u64 );

        EXPECT_EQ( doc.get<int8_t>( "/testObject/signed/i8" ).value(), val_i8 );
        EXPECT_EQ( doc.get<int16_t>( "/testObject/signed/i16" ).value(), val_i16 );
        EXPECT_EQ( doc.get<int32_t>( "/testObject/signed/i32" ).value(), val_i32 );
        EXPECT_EQ( doc.get<int64_t>( "/testObject/signed/i64" ).value(), val_i64 );
        EXPECT_EQ( doc.get<uint8_t>( "/testObject/unsigned/u8" ).value(), val_u8 );
        EXPECT_EQ( doc.get<uint16_t>( "/testObject/unsigned/u16" ).value(), val_u16 );
        EXPECT_EQ( doc.get<uint32_t>( "/testObject/unsigned/u32" ).value(), val_u32 );
        EXPECT_EQ( doc.get<uint64_t>( "/testObject/unsigned/u64" ).value(), val_u64 );

        // Test Array-level add/get for all integer types
        doc.set<Array>( "/testArray" );
        doc.set<int8_t>( "/testArray/0", val_i8 );    // index 0
        doc.set<int16_t>( "/testArray/1", val_i16 );  // index 1
        doc.set<int32_t>( "/testArray/2", val_i32 );  // index 2
        doc.set<int64_t>( "/testArray/3", val_i64 );  // index 3
        doc.set<uint8_t>( "/testArray/4", val_u8 );   // index 4
        doc.set<uint16_t>( "/testArray/5", val_u16 ); // index 5
        doc.set<uint32_t>( "/testArray/6", val_u32 ); // index 6
        doc.set<uint64_t>( "/testArray/7", val_u64 ); // index 7

        auto arr = doc.get<Array>( "/testArray" );
        ASSERT_TRUE( arr.has_value() );
        EXPECT_EQ( arr->at( 0 ).get<int8_t>( "" ).value(), val_i8 );
        EXPECT_EQ( arr->at( 1 ).get<int16_t>( "" ).value(), val_i16 );
        EXPECT_EQ( arr->at( 2 ).get<int32_t>( "" ).value(), val_i32 );
        EXPECT_EQ( arr->at( 3 ).get<int64_t>( "" ).value(), val_i64 );
        EXPECT_EQ( arr->at( 4 ).get<uint8_t>( "" ).value(), val_u8 );
        EXPECT_EQ( arr->at( 5 ).get<uint16_t>( "" ).value(), val_u16 );
        EXPECT_EQ( arr->at( 6 ).get<uint32_t>( "" ).value(), val_u32 );
        EXPECT_EQ( arr->at( 7 ).get<uint64_t>( "" ).value(), val_u64 );

        // Test Array-level set for all integer types
        doc.set<int8_t>( "/testArray/0", static_cast<int8_t>( -100 ) );
        doc.set<int16_t>( "/testArray/1", static_cast<int16_t>( -30000 ) );
        doc.set<int32_t>( "/testArray/2", static_cast<int32_t>( -2000000000 ) );
        doc.set<int64_t>( "/testArray/3", static_cast<int64_t>( -8000000000000000000LL ) );
        doc.set<uint8_t>( "/testArray/4", static_cast<uint8_t>( 200 ) );
        doc.set<uint16_t>( "/testArray/5", static_cast<uint16_t>( 60000 ) );
        doc.set<uint32_t>( "/testArray/6", static_cast<uint32_t>( 3000000000U ) );
        doc.set<uint64_t>( "/testArray/7", static_cast<uint64_t>( 15000000000000000000ULL ) );

        arr = doc.get<Array>( "/testArray" );
        EXPECT_EQ( arr->at( 0 ).get<int8_t>( "" ).value(), -100 );
        EXPECT_EQ( arr->at( 1 ).get<int16_t>( "" ).value(), -30000 );
        EXPECT_EQ( arr->at( 2 ).get<int32_t>( "" ).value(), -2000000000 );
        EXPECT_EQ( arr->at( 3 ).get<int64_t>( "" ).value(), -8000000000000000000LL );
        EXPECT_EQ( arr->at( 4 ).get<uint8_t>( "" ).value(), 200 );
        EXPECT_EQ( arr->at( 5 ).get<uint16_t>( "" ).value(), 60000 );
        EXPECT_EQ( arr->at( 6 ).get<uint32_t>( "" ).value(), 3000000000U );
        EXPECT_EQ( arr->at( 7 ).get<uint64_t>( "" ).value(), 15000000000000000000ULL );

        // Test Array-level insert for all integer types
        doc.set<int8_t>( "/insertArray/0", val_i8 );
        doc.set<int16_t>( "/insertArray/1", val_i16 );
        doc.set<int32_t>( "/insertArray/2", val_i32 );
        doc.set<int64_t>( "/insertArray/3", val_i64 );
        doc.set<uint8_t>( "/insertArray/4", val_u8 );
        doc.set<uint16_t>( "/insertArray/5", val_u16 );
        doc.set<uint32_t>( "/insertArray/6", val_u32 );
        doc.set<uint64_t>( "/insertArray/7", val_u64 );

        auto insertArr = doc.get<Array>( "/insertArray" );
        ASSERT_TRUE( insertArr.has_value() );
        EXPECT_EQ( insertArr->at( 0 ).get<int8_t>( "" ).value(), val_i8 );
        EXPECT_EQ( insertArr->at( 1 ).get<int16_t>( "" ).value(), val_i16 );
        EXPECT_EQ( insertArr->at( 2 ).get<int32_t>( "" ).value(), val_i32 );
        EXPECT_EQ( insertArr->at( 3 ).get<int64_t>( "" ).value(), val_i64 );
        EXPECT_EQ( insertArr->at( 4 ).get<uint8_t>( "" ).value(), val_u8 );
        EXPECT_EQ( insertArr->at( 5 ).get<uint16_t>( "" ).value(), val_u16 );
        EXPECT_EQ( insertArr->at( 6 ).get<uint32_t>( "" ).value(), val_u32 );
        EXPECT_EQ( insertArr->at( 7 ).get<uint64_t>( "" ).value(), val_u64 );

        // Test range validation for smaller integer types
        // Test that values outside the range return nullopt
        doc.set( "overflow_test", static_cast<int64_t>( 300 ) ); // Out of int8_t range
        EXPECT_FALSE( doc.get<int8_t>( "overflow_test" ).has_value() );

        doc.set( "overflow_test", static_cast<int64_t>( 70000 ) ); // Out of int16_t range
        EXPECT_FALSE( doc.get<int16_t>( "overflow_test" ).has_value() );

        doc.set( "overflow_test", static_cast<uint64_t>( 300 ) ); // Out of uint8_t range
        EXPECT_FALSE( doc.get<uint8_t>( "overflow_test" ).has_value() );

        doc.set( "overflow_test", static_cast<uint64_t>( 70000 ) ); // Out of uint16_t range
        EXPECT_FALSE( doc.get<uint16_t>( "overflow_test" ).has_value() );

        // Test edge values
        doc.set( "edge/i8_min", std::numeric_limits<int8_t>::min() );
        doc.set( "edge/i8_max", std::numeric_limits<int8_t>::max() );
        doc.set( "edge/u8_max", std::numeric_limits<uint8_t>::max() );

        EXPECT_EQ( doc.get<int8_t>( "edge/i8_min" ).value(), std::numeric_limits<int8_t>::min() );
        EXPECT_EQ( doc.get<int8_t>( "edge/i8_max" ).value(), std::numeric_limits<int8_t>::max() );
        EXPECT_EQ( doc.get<uint8_t>( "edge/u8_max" ).value(), std::numeric_limits<uint8_t>::max() );
    }

    //----------------------------------------------
    // Floating-Point Types
    //----------------------------------------------

    TEST( DocumentTest, AllFloatingPointTypes )
    {
        Document doc;

        // Test values
        float val_float = 3.14159f;
        double val_double = 2.718281828459045;

        // Set floating-point values
        doc.set<float>( "floats/float_val", val_float );
        doc.set<double>( "floats/double_val", val_double );

        // Test retrieval
        EXPECT_EQ( doc.get<float>( "floats/float_val" ).value(), val_float );
        EXPECT_EQ( doc.get<double>( "floats/double_val" ).value(), val_double );

        // Test type checking
        EXPECT_TRUE( doc.is<float>( "floats/float_val" ) );
        EXPECT_TRUE( doc.is<double>( "floats/double_val" ) );

        // Test cross-type compatibility (float to double, double to float)
        EXPECT_TRUE( doc.is<double>( "floats/float_val" ) ); // float should also be valid as double
        EXPECT_TRUE( doc.is<float>( "floats/double_val" ) ); // double should also be valid as float

        // Test array operations with floating-point types
        doc.set<Array>( "float_array" );
        // set elements using Document JSON Pointer paths
        doc.set<float>( "/float_array/0", val_float );
        doc.set<double>( "/float_array/1", val_double );

        auto floatArray = doc.get<Array>( "float_array" );
        ASSERT_TRUE( floatArray.has_value() );
        EXPECT_EQ( floatArray->size(), 2 );
        EXPECT_EQ( floatArray->at( 0 ).get<float>( "" ).value(), val_float );
        EXPECT_EQ( floatArray->at( 1 ).get<double>( "" ).value(), val_double );

        // Test edge cases
        float float_zero = 0.0f;
        float float_negative = -123.456f;
        double double_large = 1.7976931348623157e+308; // Near max double
        double double_small = 2.2250738585072014e-308; // Near min positive double

        doc.set<float>( "edge/float_zero", float_zero );
        doc.set<float>( "edge/float_negative", float_negative );
        doc.set<double>( "edge/double_large", double_large );
        doc.set<double>( "edge/double_small", double_small );

        EXPECT_EQ( doc.get<float>( "edge/float_zero" ).value(), float_zero );
        EXPECT_EQ( doc.get<float>( "edge/float_negative" ).value(), float_negative );
        EXPECT_EQ( doc.get<double>( "edge/double_large" ).value(), double_large );
        EXPECT_EQ( doc.get<double>( "edge/double_small" ).value(), double_small );

        // Test JSON Pointer syntax
        doc.set<float>( "/json_pointer/float", val_float );
        doc.set<double>( "/json_pointer/double", val_double );

        EXPECT_EQ( doc.get<float>( "/json_pointer/float" ).value(), val_float );
        EXPECT_EQ( doc.get<double>( "/json_pointer/double" ).value(), val_double );

        // Test serialization round-trip
        std::string jsonStr = doc.toString();
        auto docFromJson = Document::fromString( jsonStr );
        ASSERT_TRUE( docFromJson.has_value() );

        EXPECT_EQ( docFromJson->get<float>( "floats/float_val" ).value(), val_float );
        EXPECT_EQ( docFromJson->get<double>( "floats/double_val" ).value(), val_double );
    }

    //----------------------------------------------
    // Cross-type integer conversion from parsed JSON
    //----------------------------------------------

    TEST( DocumentTest, ParsedIntegerToUnsignedConversion )
    {
        // Test 1: Simple case
        {
            auto doc = Document::fromString( R"({"x":42})" );
            ASSERT_TRUE( doc.has_value() );
            auto result = doc->get<uint8_t>( "x" );
            ASSERT_TRUE( result.has_value() ) << "Test 1 failed";
            EXPECT_EQ( result.value(), 42 );
        }

        // Test 2: Multiple values
        {
            auto doc = Document::fromString( R"({
                "age": 25,
                "count": 100
            })" );
            ASSERT_TRUE( doc.has_value() );

            auto age = doc->get<uint8_t>( "age" );
            ASSERT_TRUE( age.has_value() ) << "Test 2a failed: age";
            EXPECT_EQ( age.value(), 25 );

            auto count = doc->get<uint16_t>( "count" );
            ASSERT_TRUE( count.has_value() ) << "Test 2b failed: count";
            EXPECT_EQ( count.value(), 100 );
        }

        // Test 3: Edge values
        {
            auto doc = Document::fromString( R"({
                "max_byte": 255,
                "max_short": 65535
            })" );
            ASSERT_TRUE( doc.has_value() );

            auto maxByte = doc->get<uint8_t>( "max_byte" );
            ASSERT_TRUE( maxByte.has_value() ) << "Test 3a failed: max_byte=255";
            EXPECT_EQ( maxByte.value(), 255 );

            auto maxShort = doc->get<uint16_t>( "max_short" );
            ASSERT_TRUE( maxShort.has_value() ) << "Test 3b failed: max_short=65535";
            EXPECT_EQ( maxShort.value(), 65535 );
        }

        // Test 4: Large value requiring uint32_t
        {
            auto doc = Document::fromString( R"({"large_value": 4000000000})" );
            ASSERT_TRUE( doc.has_value() );

            auto large = doc->get<uint32_t>( "large_value" );
            ASSERT_TRUE( large.has_value() ) << "Test 4 failed: large_value=4000000000";
            EXPECT_EQ( large.value(), 4000000000U );
        }

        // Test 5: Negative values should fail for unsigned types
        {
            auto doc = Document::fromString( R"({"negative": -5})" );
            ASSERT_TRUE( doc.has_value() );

            auto result_u8 = doc->get<uint8_t>( "negative" );
            EXPECT_FALSE( result_u8.has_value() ) << "Test 5a: Negative value should not convert to uint8_t";

            auto result_u16 = doc->get<uint16_t>( "negative" );
            EXPECT_FALSE( result_u16.has_value() ) << "Test 5b: Negative value should not convert to uint16_t";

            auto result_u32 = doc->get<uint32_t>( "negative" );
            EXPECT_FALSE( result_u32.has_value() ) << "Test 5c: Negative value should not convert to uint32_t";
        }

        // Test 6: Overflow values should fail
        {
            auto doc = Document::fromString( R"({
                "overflow_u8": 256,
                "overflow_u16": 65536
            })" );
            ASSERT_TRUE( doc.has_value() );

            auto overflow_u8 = doc->get<uint8_t>( "overflow_u8" );
            EXPECT_FALSE( overflow_u8.has_value() ) << "Test 6a: 256 should overflow uint8_t";

            auto overflow_u16 = doc->get<uint16_t>( "overflow_u16" );
            EXPECT_FALSE( overflow_u16.has_value() ) << "Test 6b: 65536 should overflow uint16_t";
        }

        // Test 7: Zero value
        {
            auto doc = Document::fromString( R"({"zero": 0})" );
            ASSERT_TRUE( doc.has_value() );

            auto zero_u8 = doc->get<uint8_t>( "zero" );
            ASSERT_TRUE( zero_u8.has_value() ) << "Test 7a failed: zero as uint8_t";
            EXPECT_EQ( zero_u8.value(), 0 );

            auto zero_u16 = doc->get<uint16_t>( "zero" );
            ASSERT_TRUE( zero_u16.has_value() ) << "Test 7b failed: zero as uint16_t";
            EXPECT_EQ( zero_u16.value(), 0 );

            auto zero_u32 = doc->get<uint32_t>( "zero" );
            ASSERT_TRUE( zero_u32.has_value() ) << "Test 7c failed: zero as uint32_t";
            EXPECT_EQ( zero_u32.value(), 0 );
        }

        // Test 8: Array element access
        {
            auto doc = Document::fromString( R"({"values": [10, 20, 30]})" );
            ASSERT_TRUE( doc.has_value() );

            auto val0 = doc->get<uint8_t>( "values[0]" );
            ASSERT_TRUE( val0.has_value() ) << "Test 8a failed: array[0]";
            EXPECT_EQ( val0.value(), 10 );

            auto val1 = doc->get<uint8_t>( "values[1]" );
            ASSERT_TRUE( val1.has_value() ) << "Test 8b failed: array[1]";
            EXPECT_EQ( val1.value(), 20 );

            auto val2 = doc->get<uint8_t>( "values[2]" );
            ASSERT_TRUE( val2.has_value() ) << "Test 8c failed: array[2]";
            EXPECT_EQ( val2.value(), 30 );
        }
    }

    //----------------------------------------------
    // Advanced Document operations
    //----------------------------------------------

    TEST( DocumentTest, DocumentArrayOperations )
    {
        Document arrayDoc;
        arrayDoc.set<Array>( "" );

        Document item1;
        item1.set<std::string>( "name", "Alice" );
        item1.set<int64_t>( "age", 30 );

        Document item2;
        item2.set<std::string>( "name", "Bob" );
        item2.set<int64_t>( "age", 25 );

        // Get array wrapper and add documents
        auto array = arrayDoc.get<Array>( "" );
        ASSERT_TRUE( array.has_value() );
        array->push_back( item1 );
        array->push_back( item2 );

        EXPECT_EQ( array->size(), 2 );
    }

    TEST( DocumentTest, SetArrayDocument )
    {
        Document mainDoc;

        Document arrayDoc;
        arrayDoc.set<Array>( "" );

        // Insert values into the array (Array is a vector<Document>)
        arrayDoc.set<std::string>( "/0", "value1" );
        arrayDoc.set<std::string>( "/1", "value2" );

        mainDoc.set<Document>( "myArray", arrayDoc );

        EXPECT_TRUE( mainDoc.is<Array>( "myArray" ) );
        EXPECT_EQ( mainDoc.get<Array>( "myArray" ).value().size(), 2 );
    }

    //----------------------------------------------
    // Type checking
    //----------------------------------------------

    TEST( DocumentTest, TypeCheckingMethods )
    {
        Document doc;

        // Set up different types
        doc.set<std::string>( "stringField", "hello" );
        doc.set<int64_t>( "intField", 42 );
        doc.set<double>( "doubleField", 3.14 );
        doc.set<bool>( "boolField", true );
        doc.setNull( "nullField" );

        // Create nested object
        doc.set<std::string>( "nested.object.field", "nested_value" );

        // Create array
        auto arrayOpt = doc.get<Array>( "arrayField" );
        if( !arrayOpt.has_value() )
        {
            doc.set<Array>( "arrayField" );
            arrayOpt = doc.get<Array>( "arrayField" );
        }
        doc.set<std::string>( "/arrayField/0", "item1" );

        // Test string type checking
        EXPECT_TRUE( doc.is<std::string>( "stringField" ) );
        EXPECT_FALSE( doc.is<std::string>( "intField" ) );
        EXPECT_FALSE( doc.is<std::string>( "nonexistent" ) );

        // Test integer type checking
        EXPECT_TRUE( doc.is<int>( "intField" ) );
        EXPECT_FALSE( doc.is<int>( "stringField" ) );
        EXPECT_FALSE( doc.is<int>( "doubleField" ) );
        EXPECT_FALSE( doc.is<int>( "nonexistent" ) );

        // Test double type checking
        EXPECT_TRUE( doc.is<double>( "doubleField" ) );
        EXPECT_FALSE( doc.is<double>( "intField" ) ); // Note: integers are not floats
        EXPECT_FALSE( doc.is<double>( "stringField" ) );
        EXPECT_FALSE( doc.is<double>( "nonexistent" ) );

        // Test boolean type checking
        EXPECT_TRUE( doc.is<bool>( "boolField" ) );
        EXPECT_FALSE( doc.is<bool>( "stringField" ) );
        EXPECT_FALSE( doc.is<bool>( "intField" ) );
        EXPECT_FALSE( doc.is<bool>( "nonexistent" ) );

        // Test null type checking
        EXPECT_TRUE( doc.isNull( "nullField" ) );
        EXPECT_FALSE( doc.isNull( "stringField" ) );
        EXPECT_FALSE( doc.isNull( "nonexistent" ) );

        // Test object type checking (nested objects)
        EXPECT_TRUE( doc.is<Object>( "nested" ) );
        EXPECT_TRUE( doc.is<Object>( "nested.object" ) );
        EXPECT_FALSE( doc.is<Object>( "nested.object.field" ) ); // This is a string, not object
        EXPECT_FALSE( doc.is<Object>( "stringField" ) );
        EXPECT_FALSE( doc.is<Object>( "nonexistent" ) );

        // Test array type checking
        EXPECT_TRUE( doc.is<Array>( "arrayField" ) );
        EXPECT_FALSE( doc.is<Array>( "stringField" ) );
        EXPECT_FALSE( doc.is<Array>( "nonexistent" ) );
    }

    TEST( DocumentTest, TypeCheckingWithComplexJson )
    {
        std::string complexJson = R"({
            "user": {
            "name": "Alice",
                "age": 30,
                "height": 1.65,
                "active": true,
                "spouse": null,
                "preferences": {
                    "theme": "dark"
                },
            "hobbies": ["reading", "gaming"]
            }
        })";

        auto maybeDoc = Document::fromString( complexJson );
        ASSERT_TRUE( maybeDoc.has_value() );
        Document doc = maybeDoc.value();

        // Root level
        EXPECT_TRUE( doc.is<Object>( "user" ) );

        // String fields
        EXPECT_TRUE( doc.is<std::string>( "user.name" ) );
        EXPECT_TRUE( doc.is<std::string>( "user.preferences.theme" ) );

        // Numeric fields
        EXPECT_TRUE( doc.is<int>( "user.age" ) );
        EXPECT_TRUE( doc.is<double>( "user.height" ) );

        // Boolean field
        EXPECT_TRUE( doc.is<bool>( "user.active" ) );

        // Null field
        EXPECT_TRUE( doc.isNull( "user.spouse" ) );

        // Nested object
        EXPECT_TRUE( doc.is<Object>( "user.preferences" ) );

        // Array field
        EXPECT_TRUE( doc.is<Array>( "user.hobbies" ) );

        // Cross-validation (ensuring fields are not other types)
        EXPECT_FALSE( doc.is<int>( "user.name" ) );
        EXPECT_FALSE( doc.is<std::string>( "user.age" ) );
        EXPECT_FALSE( doc.is<Array>( "user.active" ) );
        EXPECT_FALSE( doc.is<Object>( "user.hobbies" ) ); // Array, not object
    }

    TEST( DocumentTest, IsRootTypeChecking )
    {
        // Test with Object at root
        Document objectDoc;
        objectDoc.set<std::string>( "name", "test" );
        objectDoc.set<int64_t>( "value", 42 );

        EXPECT_TRUE( objectDoc.isRoot<Object>() );
        EXPECT_FALSE( objectDoc.isRoot<Array>() );
        EXPECT_FALSE( objectDoc.isRoot<std::string>() );
        EXPECT_FALSE( objectDoc.isRoot<int64_t>() );
        EXPECT_FALSE( objectDoc.isRoot<bool>() );

        // Test with Array at root
        auto arrayDocOpt = Document::fromString( R"([1, 2, 3, "test", true])" );
        ASSERT_TRUE( arrayDocOpt.has_value() );
        Document& arrayDoc = arrayDocOpt.value();

        EXPECT_TRUE( arrayDoc.isRoot<Array>() );
        EXPECT_FALSE( arrayDoc.isRoot<Object>() );
        EXPECT_FALSE( arrayDoc.isRoot<std::string>() );
        EXPECT_FALSE( arrayDoc.isRoot<int64_t>() );

        // Test with primitive at root
        Document stringDoc;
        stringDoc.set<std::string>( "", "hello" );

        EXPECT_TRUE( stringDoc.isRoot<std::string>() );
        EXPECT_FALSE( stringDoc.isRoot<Object>() );
        EXPECT_FALSE( stringDoc.isRoot<Array>() );
        EXPECT_FALSE( stringDoc.isRoot<int64_t>() );

        Document intDoc;
        intDoc.set<int64_t>( "", 123 );

        EXPECT_TRUE( intDoc.isRoot<int64_t>() );
        EXPECT_FALSE( intDoc.isRoot<std::string>() );
        EXPECT_FALSE( intDoc.isRoot<Object>() );
        EXPECT_FALSE( intDoc.isRoot<Array>() );

        Document boolDoc;
        boolDoc.set<bool>( "", true );

        EXPECT_TRUE( boolDoc.isRoot<bool>() );
        EXPECT_FALSE( boolDoc.isRoot<int64_t>() );
        EXPECT_FALSE( boolDoc.isRoot<std::string>() );

        Document doubleDoc;
        doubleDoc.set<double>( "", 3.14 );

        EXPECT_TRUE( doubleDoc.isRoot<double>() );
        EXPECT_FALSE( doubleDoc.isRoot<int64_t>() );
        EXPECT_FALSE( doubleDoc.isRoot<std::string>() );

        // Test consistency with is<T>("") - both should give same result
        EXPECT_EQ( objectDoc.isRoot<Object>(), objectDoc.is<Object>( "" ) );
        EXPECT_EQ( arrayDoc.isRoot<Array>(), arrayDoc.is<Array>( "" ) );
        EXPECT_EQ( stringDoc.isRoot<std::string>(), stringDoc.is<std::string>( "" ) );
        EXPECT_EQ( intDoc.isRoot<int64_t>(), intDoc.is<int64_t>( "" ) );
        EXPECT_EQ( boolDoc.isRoot<bool>(), boolDoc.is<bool>( "" ) );
        EXPECT_EQ( doubleDoc.isRoot<double>(), doubleDoc.is<double>( "" ) );

        // Test consistency with root<T>() availability
        // If isRoot<T>() returns true, root<T>() should return a value
        if( objectDoc.isRoot<Object>() )
        {
            EXPECT_TRUE( objectDoc.root<Object>().has_value() );
        }
        if( arrayDoc.isRoot<Array>() )
        {
            EXPECT_TRUE( arrayDoc.root<Array>().has_value() );
        }
        if( stringDoc.isRoot<std::string>() )
        {
            EXPECT_TRUE( stringDoc.root<std::string>().has_value() );
            EXPECT_EQ( stringDoc.root<std::string>().value(), "hello" );
        }
        if( intDoc.isRoot<int64_t>() )
        {
            EXPECT_TRUE( intDoc.root<int64_t>().has_value() );
            EXPECT_EQ( intDoc.root<int64_t>().value(), 123 );
        }

        // Test that isRoot is more readable than is<T>("")
        // This is a documentation/clarity test
        auto complexArrayDoc = Document::fromString( R"([
            {"name": "Alice", "age": 30},
            {"name": "Bob", "age": 25}
        ])" );
        ASSERT_TRUE( complexArrayDoc.has_value() );

        // Clear intent: checking if root is an array
        EXPECT_TRUE( complexArrayDoc->isRoot<Array>() );

        // Less clear: empty string doesn't obviously mean "root"
        EXPECT_TRUE( complexArrayDoc->is<Array>( "" ) );

        // But they're equivalent
        EXPECT_EQ( complexArrayDoc->isRoot<Array>(), complexArrayDoc->is<Array>( "" ) );
    }

    //----------------------------------------------
    // Field operations
    //----------------------------------------------

    TEST( DocumentTest, RemoveField )
    {
        Document doc;
        doc.set<std::string>( "name", "Alice" );
        doc.set<int64_t>( "age", 30 );
        doc.set<std::string>( "user.profile.firstName", "Bob" );
        doc.set<std::string>( "user.profile.lastName", "Smith" );
        doc.set<int64_t>( "user.settings.theme", 1 );

        // Test removing root-level field using Document API
        EXPECT_TRUE( doc.contains( "name" ) );
        EXPECT_EQ( doc.erase( "name" ), 1 );
        EXPECT_FALSE( doc.contains( "name" ) );

        // Test removing nested field using Document API
        EXPECT_TRUE( doc.contains( "user.profile.firstName" ) );
        EXPECT_EQ( doc.erase( "user.profile.firstName" ), 1 );
        EXPECT_FALSE( doc.contains( "user.profile.firstName" ) );
        EXPECT_TRUE( doc.contains( "user.profile.lastName" ) ); // Other fields should remain

        // Test removing non-existent field
        EXPECT_EQ( doc.erase( "nonexistent" ), 0 );
        EXPECT_EQ( doc.erase( "user.nonexistent" ), 0 );

        // Test removing empty path (should return 0)
        EXPECT_EQ( doc.erase( "" ), 0 );

        // Verify remaining structure is intact
        EXPECT_EQ( doc.get<int64_t>( "age" ), 30 );
        EXPECT_EQ( doc.get<std::string>( "user.profile.lastName" ), "Smith" );
        EXPECT_EQ( doc.get<int64_t>( "user.settings.theme" ), 1 );
    }

    //----------------------------------------------
    // Merge / update operations
    //----------------------------------------------

    TEST( DocumentTest, MergeDocuments )
    {
        // Create base document
        Document base;
        base.set<std::string>( "name", "Alice" );
        base.set<int64_t>( "age", 30 );
        base.set<std::string>( "user.profile.city", "New York" );
        auto hobbiesOpt = base.get<Array>( "hobbies" );
        if( !hobbiesOpt.has_value() )
        {
            base.set<Array>( "hobbies" );
            hobbiesOpt = base.get<Array>( "hobbies" );
        }
        // Insert hobbies into base document
        base.set<std::string>( "/hobbies/0", "reading" );
        base.set<std::string>( "/hobbies/1", "gaming" );

        // Create merge document
        Document merge;
        merge.set<std::string>( "name", "Bob" );                 // Should overwrite
        merge.set<std::string>( "email", "bob@example.com" );    // Should add new field
        merge.set<std::string>( "user.profile.country", "USA" ); // Should add to nested object
        merge.set<std::string>( "user.profile.city", "Boston" ); // Should overwrite nested field
        auto mergeHobbiesOpt = merge.get<Array>( "hobbies" );
        if( !mergeHobbiesOpt.has_value() )
        {
            merge.set<Array>( "hobbies" );
            mergeHobbiesOpt = merge.get<Array>( "hobbies" );
        }
        merge.set<std::string>( "/hobbies/0", "hiking" ); // Should merge with existing array

        auto skillsOpt = merge.get<Array>( "skills" );
        if( !skillsOpt.has_value() )
        {
            merge.set<Array>( "skills" );
            skillsOpt = merge.get<Array>( "skills" );
        }
        merge.set<std::string>( "/skills/0", "C++" ); // Should create new array

        // Test merge with array merging (default behavior)
        Document result1 = base;
        result1.merge( merge, false ); // Don't overwrite arrays

        EXPECT_EQ( result1.get<std::string>( "name" ), "Bob" );                 // Overwritten
        EXPECT_EQ( result1.get<std::string>( "email" ), "bob@example.com" );    // Added
        EXPECT_EQ( result1.get<int64_t>( "age" ), 30 );                         // Preserved
        EXPECT_EQ( result1.get<std::string>( "user.profile.city" ), "Boston" ); // Overwritten
        EXPECT_EQ( result1.get<std::string>( "user.profile.country" ), "USA" ); // Added
        EXPECT_EQ( result1.get<Array>( "hobbies" ).value().size(), 3 );         // Merged array
        EXPECT_EQ( result1.get<Array>( "skills" ).value().size(), 1 );          // New array

        // Test merge with array overwriting
        Document result2 = base;
        result2.merge( merge, true ); // Overwrite arrays

        EXPECT_EQ( result2.get<std::string>( "name" ), "Bob" );
        EXPECT_EQ( result2.get<Array>( "hobbies" ).value().size(), 1 ); // Overwritten array
    }

    TEST( DocumentTest, UpdateField )
    {
        Document doc;
        doc.set<std::string>( "name", "Alice" );
        doc.set<int64_t>( "age", 30 );

        // Create update value
        Document updateValue;
        updateValue.set<std::string>( "firstName", "Bob" );
        updateValue.set<std::string>( "lastName", "Smith" );
        updateValue.set<int64_t>( "id", 123 );

        // Update existing field
        doc.update( "name", updateValue );
        EXPECT_FALSE( doc.is<std::string>( "name" ) ); // Should no longer be a string
        EXPECT_TRUE( doc.is<Object>( "name" ) );       // Should now be an object
        EXPECT_EQ( doc.get<std::string>( "name.firstName" ), "Bob" );
        EXPECT_EQ( doc.get<std::string>( "name.lastName" ), "Smith" );
        EXPECT_EQ( doc.get<int64_t>( "name.id" ), 123 );

        // Update nested path (should create intermediate objects)
        Document nestedUpdate;
        nestedUpdate.set<std::string>( "theme", "dark" );
        nestedUpdate.set<bool>( "notifications", true );

        doc.update( "user.settings", nestedUpdate );
        EXPECT_EQ( doc.get<std::string>( "user.settings.theme" ), "dark" );
        EXPECT_EQ( doc.get<bool>( "user.settings.notifications" ), true );

        // Verify other fields remain unchanged
        EXPECT_EQ( doc.get<int64_t>( "age" ), 30 );
    }

    //----------------------------------------------
    // Validation and error handling
    //----------------------------------------------

    TEST( DocumentTest, PublicValidationMethods )
    {
        Document doc;
        doc.set<std::string>( "validString", "test" );
        doc.set<Array>( "validArray" );
        doc.set<std::string>( "/validArray/0", "item" );

        // Test public validation methods
        EXPECT_TRUE( doc.isValid() );
        EXPECT_TRUE( doc.contains( "validString" ) );
        EXPECT_FALSE( doc.contains( "missingField" ) );

        // Test array validation
        EXPECT_TRUE( doc.is<Array>( "validArray" ) );
        EXPECT_FALSE( doc.is<Array>( "validString" ) );
        EXPECT_FALSE( doc.is<Array>( "nonexistent" ) );
    }

    //----------------------------------------------
    // JSON output
    //----------------------------------------------

    TEST( DocumentTest, JsonOutputFormatting )
    {
        Document doc;
        doc.set<std::string>( "name", "Test" );
        doc.set<int64_t>( "value", 123 );

        // Compact output
        std::string compact = doc.toString( 0 );
        EXPECT_TRUE( compact.find( "\"name\"" ) != std::string::npos );
        EXPECT_TRUE( compact.find( "\"Test\"" ) != std::string::npos );
        EXPECT_TRUE( compact.find( "\"value\"" ) != std::string::npos );
        EXPECT_TRUE( compact.find( "123" ) != std::string::npos );

        // Pretty-printed output
        std::string pretty = doc.toString( 2 );
        EXPECT_GT( pretty.length(), compact.length() );
    }

    TEST( DocumentTest, JsonBytesOutput )
    {
        Document doc;
        doc.set<std::string>( "test", "value" );

        std::vector<uint8_t> bytes = doc.toBytes();
        std::string jsonStr = doc.toString( 0 );

        EXPECT_EQ( bytes.size(), jsonStr.length() );
    }

    TEST( DocumentTest, FromJsonBytesValidDocument )
    {
        std::string jsonStr = R"({"name": "Alice", "age": 30, "active": true})";
        std::vector<uint8_t> jsonBytes( jsonStr.begin(), jsonStr.end() );

        auto maybeDoc = Document::fromBytes( jsonBytes );
        ASSERT_TRUE( maybeDoc.has_value() );
        Document doc = maybeDoc.value();

        EXPECT_EQ( doc.get<std::string>( "name" ).value_or( "" ), "Alice" );
        EXPECT_EQ( doc.get<int64_t>( "age" ).value_or( 0 ), 30 );
        EXPECT_EQ( doc.get<bool>( "active" ).value_or( false ), true );
    }

    TEST( DocumentTest, FromJsonBytesInvalidDocument )
    {
        std::string invalidJson = R"({"name": "Alice", "age":})";
        std::vector<uint8_t> jsonBytes( invalidJson.begin(), invalidJson.end() );

        auto maybeDoc = Document::fromBytes( jsonBytes );
        EXPECT_FALSE( maybeDoc.has_value() );
    }

    TEST( DocumentTest, FromJsonBytesEmptyInput )
    {
        std::vector<uint8_t> emptyBytes;
        auto maybeDoc = Document::fromBytes( emptyBytes );
        EXPECT_FALSE( maybeDoc.has_value() );
    }

    TEST( DocumentTest, DocumentRoundtripSerializationBytes )
    {
        // Create a complex document
        Document originalDoc;
        originalDoc.set<std::string>( "name", "Bob" );
        originalDoc.set<int64_t>( "age", 25 );
        originalDoc.set<double>( "height", 1.80 );
        originalDoc.set<bool>( "active", true );
        originalDoc.set<std::string>( "address.city", "Seattle" );
        originalDoc.set<std::string>( "address.country", "USA" );

        // Serialize to bytes
        std::vector<uint8_t> jsonBytes = originalDoc.toBytes();
        EXPECT_FALSE( jsonBytes.empty() );

        // Deserialize back
        auto deserializedDoc = Document::fromBytes( jsonBytes );
        ASSERT_TRUE( deserializedDoc.has_value() );

        // Verify all fields are preserved
        EXPECT_EQ( deserializedDoc.value().get<std::string>( "name" ).value_or( "" ), "Bob" );
        EXPECT_EQ( deserializedDoc.value().get<int64_t>( "age" ).value_or( 0 ), 25 );
        EXPECT_EQ( deserializedDoc.value().get<double>( "height" ).value_or( 0.0 ), 1.80 );
        EXPECT_EQ( deserializedDoc.value().get<bool>( "active" ).value_or( false ), true );
        EXPECT_EQ( deserializedDoc.value().get<std::string>( "address.city" ).value_or( "" ), "Seattle" );
        EXPECT_EQ( deserializedDoc.value().get<std::string>( "address.country" ).value_or( "" ), "USA" );
    }

    TEST( DocumentTest, DocumentRoundtripSerializationString )
    {
        // Test string roundtrip with arrays
        Document originalDoc;
        originalDoc.set<std::string>( "title", "Test Document" );

        // Add array
        originalDoc.set<int64_t>( "/numbers/0", 1 );
        originalDoc.set<int64_t>( "/numbers/1", 2 );
        originalDoc.set<int64_t>( "/numbers/2", 3 );

        // Serialize to string
        std::string jsonStr = originalDoc.toString();
        EXPECT_FALSE( jsonStr.empty() );

        // Deserialize back
        auto deserializedDoc = Document::fromString( jsonStr );
        ASSERT_TRUE( deserializedDoc.has_value() );

        // Verify fields are preserved
        EXPECT_EQ( deserializedDoc.value().get<std::string>( "title" ).value_or( "" ), "Test Document" );

        auto deserializedNumbers = deserializedDoc.value().get<Array>( "numbers" );
        ASSERT_TRUE( deserializedNumbers.has_value() );
        EXPECT_EQ( deserializedNumbers.value().size(), 3 );
        EXPECT_EQ( deserializedNumbers->at( 0 ).get<int64_t>( "" ).value_or( 0 ), 1 );
        EXPECT_EQ( deserializedNumbers->at( 1 ).get<int64_t>( "" ).value_or( 0 ), 2 );
        EXPECT_EQ( deserializedNumbers->at( 2 ).get<int64_t>( "" ).value_or( 0 ), 3 );
    }

    //----------------------------------------------
    // JSON Pointer (RFC 6901)
    //----------------------------------------------

    TEST( DocumentTest, JsonPointerBasicAccess )
    {
        // Create a document structure
        Document doc;
        doc.set<std::string>( "/name", "Alice Johnson" );
        doc.set<int64_t>( "/age", 30 );
        doc.set<double>( "/height", 1.75 );
        doc.set<bool>( "/active", true );
        doc.setNull( "/spouse" );

        // Test basic JSON Pointer access
        EXPECT_EQ( doc.get<std::string>( "/name" ), "Alice Johnson" );
        EXPECT_EQ( doc.get<int64_t>( "/age" ), 30 );
        EXPECT_EQ( doc.get<double>( "/height" ), 1.75 );
        EXPECT_EQ( doc.get<bool>( "/active" ), true );

        // Test field existence
        EXPECT_TRUE( doc.contains( "/name" ) );
        EXPECT_TRUE( doc.contains( "/age" ) );
        EXPECT_TRUE( doc.contains( "/spouse" ) );
        EXPECT_FALSE( doc.contains( "/nonexistent" ) );

        // Test root document access
        EXPECT_TRUE( doc.contains( "" ) );
    }

    TEST( DocumentTest, JsonPointerNestedObjects )
    {
        Document doc;

        // Create nested structure using JSON Pointers
        doc.set<std::string>( "/user/profile/firstName", "John" );
        doc.set<std::string>( "/user/profile/lastName", "Doe" );
        doc.set<int64_t>( "/user/profile/age", 25 );
        doc.set<std::string>( "/user/settings/theme", "dark" );
        doc.set<bool>( "/user/settings/notifications", false );

        // Verify nested access
        EXPECT_EQ( doc.get<std::string>( "/user/profile/firstName" ), "John" );
        EXPECT_EQ( doc.get<std::string>( "/user/profile/lastName" ), "Doe" );
        EXPECT_EQ( doc.get<int64_t>( "/user/profile/age" ), 25 );
        EXPECT_EQ( doc.get<std::string>( "/user/settings/theme" ), "dark" );
        EXPECT_EQ( doc.get<bool>( "/user/settings/notifications" ), false );

        // Test intermediate path existence
        EXPECT_TRUE( doc.contains( "/user" ) );
        EXPECT_TRUE( doc.contains( "/user/profile" ) );
        EXPECT_TRUE( doc.contains( "/user/settings" ) );
    }

    TEST( DocumentTest, JsonPointerArrayAccess )
    {
        Document doc;

        // Create arrays using JSON Pointers
        doc.set<std::string>( "/users/0/name", "Alice" );
        doc.set<int64_t>( "/users/0/age", 28 );
        doc.set<std::string>( "/users/1/name", "Bob" );
        doc.set<int64_t>( "/users/1/age", 32 );

        doc.set<double>( "/scores/0", 95.5 );
        doc.set<double>( "/scores/1", 87.2 );
        doc.set<double>( "/scores/2", 91.8 );

        // Verify array element access
        EXPECT_EQ( doc.get<std::string>( "/users/0/name" ), "Alice" );
        EXPECT_EQ( doc.get<int64_t>( "/users/0/age" ), 28 );
        EXPECT_EQ( doc.get<std::string>( "/users/1/name" ), "Bob" );
        EXPECT_EQ( doc.get<int64_t>( "/users/1/age" ), 32 );

        EXPECT_EQ( doc.get<double>( "/scores/0" ), 95.5 );
        EXPECT_EQ( doc.get<double>( "/scores/1" ), 87.2 );
        EXPECT_EQ( doc.get<double>( "/scores/2" ), 91.8 );

        // Test field existence using Document API
        EXPECT_TRUE( doc.contains( "users" ) );  // Field in root object
        EXPECT_TRUE( doc.contains( "scores" ) ); // Field in root object

        // Test field existence in nested objects
        EXPECT_TRUE( doc.contains( "/users/0/name" ) ); // Field in nested object
        EXPECT_TRUE( doc.contains( "/users/0/age" ) );  // Field in nested object

        // Test value existence (any JSON value)
        EXPECT_TRUE( doc.contains( "/users" ) );        // Array value
        EXPECT_TRUE( doc.contains( "/users/0" ) );      // Array element (object)
        EXPECT_TRUE( doc.contains( "/users/1" ) );      // Array element (object)
        EXPECT_TRUE( doc.contains( "/scores" ) );       // Array value
        EXPECT_TRUE( doc.contains( "/scores/0" ) );     // Array element (primitive)
        EXPECT_TRUE( doc.contains( "/users/0/name" ) ); // Object field value
        EXPECT_FALSE( doc.contains( "/users/5" ) );     // Non-existent array element
        EXPECT_FALSE( doc.contains( "/nonexistent" ) ); // Non-existent field
    }

    TEST( DocumentTest, JsonPointerEscapedCharacters )
    {
        Document doc;

        // Test escaped characters according to RFC 6901
        // ~0 represents ~ and ~1 represents /
        doc.set<std::string>( "/field~1with~0tilde", "value1" );
        doc.set<std::string>( "/normal~1field", "value2" );
        doc.set<int64_t>( "/path~1to~1data", 42 );

        // Verify escaped character handling
        EXPECT_EQ( doc.get<std::string>( "/field~1with~0tilde" ), "value1" );
        EXPECT_EQ( doc.get<std::string>( "/normal~1field" ), "value2" );
        EXPECT_EQ( doc.get<int64_t>( "/path~1to~1data" ), 42 );

        // Verify field existence with escaped names
        EXPECT_TRUE( doc.contains( "/field~1with~0tilde" ) );
        EXPECT_TRUE( doc.contains( "/normal~1field" ) );
        EXPECT_TRUE( doc.contains( "/path~1to~1data" ) );
    }

    TEST( DocumentTest, JsonPointerErrorHandling )
    {
        Document doc;
        doc.set<std::string>( "/existing/field", "value" );

        // Test invalid pointers
        EXPECT_FALSE( doc.contains( "invalid" ) ); // Must start with /
        EXPECT_FALSE( doc.contains( "/nonexistent/field" ) );

        // Test type mismatches
        EXPECT_FALSE( doc.get<int64_t>( "/existing/field" ).has_value() ); // string accessed as int
        EXPECT_FALSE( doc.get<bool>( "/existing/field" ).has_value() );    // string accessed as bool

        // Test array index errors
        doc.set<int64_t>( "/numbers/0", 10 );
        doc.set<int64_t>( "/numbers/1", 20 );

        EXPECT_FALSE( doc.contains( "/numbers/5" ) ); // Out of bounds
        EXPECT_FALSE( doc.get<int64_t>( "/numbers/5" ).has_value() );

        // Test invalid array indices
        EXPECT_FALSE( doc.contains( "/numbers/01" ) );  // Leading zero not allowed
        EXPECT_FALSE( doc.contains( "/numbers/abc" ) ); // Non-numeric
    }

    TEST( DocumentTest, HasFieldVsHasValueNewAPI )
    {
        // Create a document with mixed object and array structure
        auto docOpt = Document::fromString( R"({
            "users": [
                {"name": "Alice", "age": 30},
                {"name": "Bob", "age": 25}
            ],
            "count": 2,
            "settings": {
                "theme": "dark",
                "notifications": true
            }
        })" );
        ASSERT_TRUE( docOpt.has_value() );
        Document& doc = docOpt.value();

        // Object fields - use Document API
        EXPECT_TRUE( doc.contains( "/users" ) );    // Field in root object
        EXPECT_TRUE( doc.contains( "/count" ) );    // Field in root object
        EXPECT_TRUE( doc.contains( "/settings" ) ); // Field in root object

        // Nested object fields - use Document API
        EXPECT_TRUE( doc.contains( "/settings/theme" ) );         // Field in nested object
        EXPECT_TRUE( doc.contains( "/settings/notifications" ) ); // Field in nested object

        EXPECT_TRUE( doc.contains( "/users/0/name" ) ); // Field in array element object
        EXPECT_TRUE( doc.contains( "/users/0/age" ) );  // Field in array element object

        // Array elements - use Document API with index bounds check
        auto usersArray = doc.get<Array>( "/users" );
        ASSERT_TRUE( usersArray.has_value() );
        EXPECT_TRUE( usersArray->size() > 0 );  // Array element exists
        EXPECT_TRUE( usersArray->size() > 1 );  // Array element exists
        EXPECT_FALSE( usersArray->size() > 5 ); // Out of bounds

        // Universal value existence - use Document API
        EXPECT_TRUE( doc.contains( "/users" ) );          // Array value exists
        EXPECT_TRUE( doc.contains( "/users/0" ) );        // Array element exists
        EXPECT_TRUE( doc.contains( "/users/1" ) );        // Array element exists
        EXPECT_TRUE( doc.contains( "/settings/theme" ) ); // Nested field value
        EXPECT_TRUE( doc.contains( "/users/0/name" ) );   // Field in array element

        // Non-existent paths should return false
        EXPECT_FALSE( doc.contains( "/nonexistent" ) );
        EXPECT_FALSE( usersArray->size() > 5 );             // Out of bounds
        EXPECT_FALSE( doc.contains( "/users/5" ) );         // Out of bounds
        EXPECT_FALSE( doc.contains( "/users/0/invalid" ) ); // Non-existent field

        // Test with array at root
        auto arrayDocOpt = Document::fromString( R"([1, 2, {"key": "value"}])" );
        ASSERT_TRUE( arrayDocOpt.has_value() );
        Document& arrayDoc = arrayDocOpt.value();

        auto rootArray = arrayDoc.get<Array>( "" );
        ASSERT_TRUE( rootArray.has_value() );
        EXPECT_TRUE( rootArray->size() > 0 );         // Array element exists
        EXPECT_TRUE( rootArray->size() > 1 );         // Array element exists
        EXPECT_TRUE( rootArray->size() > 2 );         // Array element exists
        EXPECT_TRUE( arrayDoc.contains( "/0" ) );     // Array element exists
        EXPECT_TRUE( arrayDoc.contains( "/2/key" ) ); // Field within object in array

        EXPECT_TRUE( arrayDoc.contains( "/2/key" ) ); // Field within object in array
    }

    TEST( DocumentTest, TypeSpecificMethods )
    {
        // Create a test document with all JSON types
        auto docOpt = Document::fromString( R"({
            "stringField": "hello world",
            "intField": 42,
            "doubleField": 3.14159,
            "boolField": true,
            "nullField": null,
            "objectField": {
                "nested": "value"
            },
            "arrayField": [1, 2, 3],
            "mixedArray": [
                "string",
                123,
                4.56,
                false,
                null,
                {"key": "value"},
                [7, 8, 9]
            ]
        })" );
        ASSERT_TRUE( docOpt.has_value() );
        Document& doc = docOpt.value();

        // Test is<std::string>
        EXPECT_TRUE( doc.is<std::string>( "/stringField" ) );
        EXPECT_TRUE( doc.is<std::string>( "/objectField/nested" ) );
        EXPECT_TRUE( doc.is<std::string>( "/mixedArray/0" ) );
        EXPECT_FALSE( doc.is<std::string>( "/intField" ) );    // Not a string
        EXPECT_FALSE( doc.is<std::string>( "/arrayField" ) );  // Not a string
        EXPECT_FALSE( doc.is<std::string>( "/nonexistent" ) ); // Doesn't exist

        // Test is<int>
        EXPECT_TRUE( doc.is<int>( "/intField" ) );
        EXPECT_TRUE( doc.is<int>( "/arrayField/0" ) );
        EXPECT_TRUE( doc.is<int>( "/mixedArray/1" ) );
        EXPECT_FALSE( doc.is<int>( "/doubleField" ) ); // Not an int
        EXPECT_FALSE( doc.is<int>( "/stringField" ) ); // Not an int
        EXPECT_FALSE( doc.is<int>( "/nonexistent" ) ); // Doesn't exist

        // Test is<double>
        EXPECT_TRUE( doc.is<double>( "/doubleField" ) );
        EXPECT_TRUE( doc.is<double>( "/mixedArray/2" ) );
        EXPECT_FALSE( doc.is<double>( "/intField" ) );    // Not a double
        EXPECT_FALSE( doc.is<double>( "/stringField" ) ); // Not a double
        EXPECT_FALSE( doc.is<double>( "/nonexistent" ) ); // Doesn't exist

        // Test is<bool>
        EXPECT_TRUE( doc.is<bool>( "/boolField" ) );
        EXPECT_TRUE( doc.is<bool>( "/mixedArray/3" ) );
        EXPECT_FALSE( doc.is<bool>( "/stringField" ) ); // Not a bool
        EXPECT_FALSE( doc.is<bool>( "/intField" ) );    // Not a bool
        EXPECT_FALSE( doc.is<bool>( "/nonexistent" ) ); // Doesn't exist

        // Test isNull
        EXPECT_TRUE( doc.isNull( "/nullField" ) );
        EXPECT_TRUE( doc.isNull( "/mixedArray/4" ) );
        EXPECT_FALSE( doc.isNull( "/stringField" ) ); // Not null
        EXPECT_FALSE( doc.isNull( "/intField" ) );    // Not null
        EXPECT_FALSE( doc.isNull( "/nonexistent" ) ); // Doesn't exist (different from null)

        // Test is<Object>
        EXPECT_TRUE( doc.is<Object>( "/objectField" ) );
        EXPECT_TRUE( doc.is<Object>( "/mixedArray/5" ) );
        EXPECT_TRUE( doc.is<Object>( "" ) );              // Root is object
        EXPECT_FALSE( doc.is<Object>( "/arrayField" ) );  // Not an object
        EXPECT_FALSE( doc.is<Object>( "/stringField" ) ); // Not an object
        EXPECT_FALSE( doc.is<Object>( "/nonexistent" ) ); // Doesn't exist

        // Test is<Array>
        EXPECT_TRUE( doc.is<Array>( "/arrayField" ) );
        EXPECT_TRUE( doc.is<Array>( "/mixedArray" ) );
        EXPECT_TRUE( doc.is<Array>( "/mixedArray/6" ) );
        EXPECT_FALSE( doc.is<Array>( "/objectField" ) ); // Not an array
        EXPECT_FALSE( doc.is<Array>( "/stringField" ) ); // Not an array
        EXPECT_FALSE( doc.is<Array>( "" ) );             // Root is object, not array
        EXPECT_FALSE( doc.is<Array>( "/nonexistent" ) ); // Doesn't exist

        // Test consistency with corresponding get methods
        // If type check returns true, get method should return a value
        if( doc.is<std::string>( "/stringField" ) )
        {
            EXPECT_TRUE( doc.get<std::string>( "/stringField" ).has_value() );
        }
        if( doc.is<int>( "/intField" ) )
        {
            EXPECT_TRUE( doc.get<int64_t>( "/intField" ).has_value() );
        }
        if( doc.is<double>( "/doubleField" ) )
        {
            EXPECT_TRUE( doc.get<double>( "/doubleField" ).has_value() );
        }
        if( doc.is<bool>( "/boolField" ) )
        {
            EXPECT_TRUE( doc.get<bool>( "/boolField" ).has_value() );
        }
        if( doc.is<Array>( "/arrayField" ) )
        {
            EXPECT_TRUE( doc.get<Document>( "/arrayField" ).has_value() );
        }
        if( doc.is<Object>( "/objectField" ) )
        {
            EXPECT_TRUE( doc.get<Document>( "/objectField" ).has_value() );
        }

        // Test array document at root
        auto arrayDoc = Document::fromString( R"([
            "string",
            42,
            3.14,
            true,
            null,
            {"key": "value"},
            [1, 2, 3]
        ])" );
        ASSERT_TRUE( arrayDoc.has_value() );

        EXPECT_TRUE( arrayDoc->isRoot<Array>() );   // Root is array
        EXPECT_FALSE( arrayDoc->isRoot<Object>() ); // Root is not object
        EXPECT_TRUE( arrayDoc->is<std::string>( "/0" ) );
        EXPECT_TRUE( arrayDoc->is<int>( "/1" ) );
        EXPECT_TRUE( arrayDoc->is<double>( "/2" ) );
        EXPECT_TRUE( arrayDoc->is<bool>( "/3" ) );
        EXPECT_TRUE( arrayDoc->isNull( "/4" ) );
        EXPECT_TRUE( arrayDoc->is<Object>( "/5" ) );
        EXPECT_TRUE( arrayDoc->is<Array>( "/6" ) );
    }

    TEST( DocumentTest, JsonPointerCompatibilityWithDotNotation )
    {
        Document doc;

        // Set values using dot notation
        doc.set<std::string>( "user.name", "Alice" );
        doc.set<int64_t>( "user.age", 25 );
        doc.set<std::string>( "settings.theme", "dark" );

        // Access same values using JSON Pointer
        EXPECT_EQ( doc.get<std::string>( "/user/name" ), "Alice" );
        EXPECT_EQ( doc.get<int64_t>( "/user/age" ), 25 );
        EXPECT_EQ( doc.get<std::string>( "/settings/theme" ), "dark" );

        // Set values using JSON Pointer
        doc.set<std::string>( "/profile/email", "alice@example.com" );
        doc.set<bool>( "/profile/verified", true );

        // Access same values using dot notation
        EXPECT_EQ( doc.get<std::string>( "profile.email" ), "alice@example.com" );
        EXPECT_EQ( doc.get<bool>( "profile.verified" ), true );

        // Both notations should see the same structure
        EXPECT_TRUE( doc.contains( "user.name" ) );
        EXPECT_TRUE( doc.contains( "/user/name" ) );
        EXPECT_TRUE( doc.contains( "profile.email" ) );
        EXPECT_TRUE( doc.contains( "/profile/email" ) );
    }

    TEST( DocumentTest, JsonPointerComplexDocument )
    {
        // Test with a complex, realistic JSON structure
        Document doc;

        // API response structure
        doc.set<std::string>( "/status", "success" );
        doc.set<int64_t>( "/code", 200 );
        doc.set<std::string>( "/data/user/id", "12345" );
        doc.set<std::string>( "/data/user/profile/name", "Jane Smith" );
        doc.set<std::string>( "/data/user/profile/email", "jane@example.com" );
        doc.set<bool>( "/data/user/profile/verified", true );

        // Array of permissions
        doc.set<std::string>( "/data/permissions/0/resource", "users" );
        doc.set<std::string>( "/data/permissions/0/action", "read" );
        doc.set<std::string>( "/data/permissions/1/resource", "posts" );
        doc.set<std::string>( "/data/permissions/1/action", "write" );
        doc.set<std::string>( "/data/permissions/2/resource", "admin" );
        doc.set<std::string>( "/data/permissions/2/action", "manage" );

        // Metadata
        doc.set<std::string>( "/metadata/timestamp", "2025-10-03T14:30:00Z" );
        doc.set<double>( "/metadata/version", 2.1 );

        // Verify the entire structure
        EXPECT_EQ( doc.get<std::string>( "/status" ), "success" );
        EXPECT_EQ( doc.get<int64_t>( "/code" ), 200 );
        EXPECT_EQ( doc.get<std::string>( "/data/user/profile/name" ), "Jane Smith" );
        EXPECT_EQ( doc.get<bool>( "/data/user/profile/verified" ), true );

        EXPECT_EQ( doc.get<std::string>( "/data/permissions/0/resource" ), "users" );
        EXPECT_EQ( doc.get<std::string>( "/data/permissions/1/action" ), "write" );
        EXPECT_EQ( doc.get<std::string>( "/data/permissions/2/resource" ), "admin" );

        EXPECT_EQ( doc.get<double>( "/metadata/version" ), 2.1 );

        // Test the generated JSON structure
        std::string jsonOutput = doc.toString( 2 );
        EXPECT_TRUE( jsonOutput.find( "\"status\"" ) != std::string::npos );
        EXPECT_TRUE( jsonOutput.find( "\"Jane Smith\"" ) != std::string::npos );
        EXPECT_TRUE( jsonOutput.find( "\"permissions\"" ) != std::string::npos );
    }

    //----------------------------------------------
    // JSON Pointer array methods tests
    //----------------------------------------------

    TEST( DocumentTest, JsonPointerArrayMethods_BasicSetAndGet )
    {
        Document doc;

        // Create an array with mixed types using JSON Pointer
        doc.set<std::string>( "/hobbies/0", "first" );
        doc.set<std::string>( "/hobbies/1", "second" );
        doc.set<std::string>( "/hobbies/2", "third" );

        // Verify the array was set correctly
        EXPECT_TRUE( doc.contains( "/hobbies" ) );
        EXPECT_TRUE( doc.is<Array>( "hobbies" ) );

        auto hobbiesArray = doc.get<Array>( "hobbies" );
        ASSERT_TRUE( hobbiesArray.has_value() );
        EXPECT_EQ( hobbiesArray->size(), 3 );

        // Verify individual elements through array access
        EXPECT_EQ( hobbiesArray->at( 0 ).root<std::string>(), "first" );
        EXPECT_EQ( hobbiesArray->at( 1 ).root<std::string>(), "second" );
        EXPECT_EQ( hobbiesArray->at( 2 ).root<std::string>(), "third" );
    }

    TEST( DocumentTest, JsonPointerArrayMethods_NestedArrays )
    {
        Document doc;

        // Create a nested structure with arrays
        Document numbersArray;
        numbersArray.set<int64_t>( "/0", 10 );
        numbersArray.set<int64_t>( "/1", 20 );
        numbersArray.set<int64_t>( "/2", 30 );

        Document stringsArray;
        stringsArray.set<std::string>( "/0", "alpha" );
        stringsArray.set<std::string>( "/1", "beta" );

        // Set arrays at nested paths
        doc.set<Document>( "/data/numbers", numbersArray );
        doc.set<Document>( "/data/strings", stringsArray );

        // Verify both arrays exist
        EXPECT_TRUE( doc.contains( "/data/numbers" ) );
        EXPECT_TRUE( doc.contains( "/data/strings" ) );

        // Get and verify the arrays
        auto retrievedNumbers = doc.get<Document>( "/data/numbers" );
        auto retrievedStrings = doc.get<Document>( "/data/strings" );

        ASSERT_TRUE( retrievedNumbers.has_value() );
        ASSERT_TRUE( retrievedStrings.has_value() );

        // Get Array wrappers to access size
        auto numbersArrayWrapper = retrievedNumbers->get<Array>( "" );
        ASSERT_TRUE( numbersArrayWrapper.has_value() );
        EXPECT_EQ( numbersArrayWrapper->size(), 3 );
        auto stringsArrayWrapper = retrievedStrings->get<Array>( "" );
        ASSERT_TRUE( stringsArrayWrapper.has_value() );
        EXPECT_EQ( stringsArrayWrapper->size(), 2 );

        // Verify content through direct JSON Pointer access
        EXPECT_EQ( doc.get<int64_t>( "/data/numbers/0" ), 10 );
        EXPECT_EQ( doc.get<std::string>( "/data/strings/1" ), "beta" );
    }

    TEST( DocumentTest, JsonPointerArrayMethods_EmptyArray )
    {
        Document doc;

        // Create and set an empty array
        Document emptyArray;
        emptyArray.set<Array>( "" );

        doc.set<Document>( "/empty", emptyArray );

        // Verify empty array handling
        EXPECT_TRUE( doc.contains( "/empty" ) );
        EXPECT_TRUE( doc.is<Array>( "empty" ) );
        EXPECT_EQ( doc.get<Array>( "empty" ).value().size(), 0 );

        auto retrievedEmpty = doc.get<Document>( "/empty" );
        ASSERT_TRUE( retrievedEmpty.has_value() );
        // Get Array wrapper to access size
        auto emptyArrayWrapper = retrievedEmpty->get<Array>( "" );
        ASSERT_TRUE( emptyArrayWrapper.has_value() );
        EXPECT_EQ( emptyArrayWrapper->size(), 0 );
    }

    TEST( DocumentTest, JsonPointerArrayMethods_ReplaceExistingArray )
    {
        Document doc;

        // Create initial array
        Document originalArray;
        originalArray.set<Array>( "" ); // Initialize as array first
        originalArray.set<std::string>( "/0", "old1" );
        originalArray.set<std::string>( "/1", "old2" );
        doc.set<Document>( "/items", originalArray );

        // Verify original array
        auto items = doc.get<Array>( "items" );
        ASSERT_TRUE( items.has_value() );
        EXPECT_EQ( items->size(), 2 );
        EXPECT_EQ( items->at( 0 ).root<std::string>(), "old1" );

        // Replace with new array
        Document newArray;
        newArray.set<Array>( "" ); // Initialize as array first
        newArray.set<std::string>( "/0", "new1" );
        newArray.set<std::string>( "/1", "new2" );
        newArray.set<std::string>( "/2", "new3" );
        doc.set<Document>( "/items", newArray );

        // Verify replacement
        items = doc.get<Array>( "items" );
        EXPECT_EQ( items->size(), 3 );
        EXPECT_EQ( items->at( 0 ).root<std::string>(), "new1" );
        EXPECT_EQ( items->at( 2 ).root<std::string>(), "new3" );

        auto retrievedNew = doc.get<Document>( "/items" );
        ASSERT_TRUE( retrievedNew.has_value() );
        // Get Array wrapper to access size
        auto newArrayWrapper = retrievedNew->get<Array>( "" );
        ASSERT_TRUE( newArrayWrapper.has_value() );
        EXPECT_EQ( newArrayWrapper->size(), 3 );
    }

    TEST( DocumentTest, JsonPointerArrayMethods_ErrorHandling )
    {
        Document doc;
        doc.set<std::string>( "notArray", "this is a string" );

        // Try to get array from non-array field
        auto result = doc.get<Array>( "/notArray" );
        EXPECT_FALSE( result.has_value() );

        // Try to get array from non-existent path
        auto nonExistent = doc.get<Document>( "/nonExistent" );
        EXPECT_FALSE( nonExistent.has_value() );

        // Try to get array with invalid pointer
        auto invalid = doc.get<Document>( "/invalid/deep/path" );
        EXPECT_FALSE( invalid.has_value() );
    }

    TEST( DocumentTest, JsonPointerArrayMethods_ComplexArrayWithObjects )
    {
        Document doc;

        Document objectArray;
        objectArray.set<Array>( "" );

        // Add first object to array
        Document obj1;

        obj1.set<std::string>( "name", "Alice" );
        obj1.set<int64_t>( "age", 30 );

        // Add second object to array
        Document obj2;

        obj2.set<std::string>( "name", "Bob" );
        obj2.set<int64_t>( "age", 25 );

        // Add documents to array directly
        objectArray.push_back( obj1 );
        objectArray.push_back( obj2 );

        // Set the complex array
        doc.set<Document>( "/users", objectArray );

        // Verify the complex array was set
        EXPECT_TRUE( doc.contains( "/users" ) );
        EXPECT_TRUE( doc.is<Array>( "users" ) );
        EXPECT_EQ( doc.get<Array>( "users" ).value().size(), 2 );

        // Get and verify the complex array
        auto retrievedUsers = doc.get<Document>( "/users" );
        ASSERT_TRUE( retrievedUsers.has_value() );
        // Get Array wrapper to access size
        auto usersArrayWrapper = retrievedUsers->get<Array>( "" );
        ASSERT_TRUE( usersArrayWrapper.has_value() );
        EXPECT_EQ( usersArrayWrapper->size(), 2 );

        // Verify we can access nested object data through JSON Pointers
        EXPECT_EQ( doc.get<std::string>( "/users/0/name" ), "Alice" );
        EXPECT_EQ( doc.get<int64_t>( "/users/0/age" ), 30 );
        EXPECT_EQ( doc.get<std::string>( "/users/1/name" ), "Bob" );
        EXPECT_EQ( doc.get<int64_t>( "/users/1/age" ), 25 );
    }

    TEST( DocumentTest, JsonPointerArrayMethods_RoundtripSerialization )
    {
        Document original;

        // Create a mixed array
        Document mixedArray;
        mixedArray.set<Array>( "" ); // Initialize as array first
        mixedArray.set<std::string>( "/0", "string_value" );
        mixedArray.set<int64_t>( "/1", 42 );
        mixedArray.set<double>( "/2", 3.14 );
        mixedArray.set<bool>( "/3", true );

        original.set<Document>( "/mixed", mixedArray );

        // Serialize and deserialize
        std::string jsonString = original.toString();
        auto deserialized = Document::fromString( jsonString );

        ASSERT_TRUE( deserialized.has_value() );

        // Verify the array survived serialization
        auto mixedArr = deserialized->get<Array>( "mixed" );
        ASSERT_TRUE( mixedArr.has_value() );
        EXPECT_EQ( mixedArr->size(), 4 );

        // Verify individual elements
        EXPECT_EQ( mixedArr->at( 0 ).root<std::string>(), "string_value" );
        EXPECT_EQ( mixedArr->at( 1 ).root<int64_t>(), 42 );
        EXPECT_EQ( mixedArr->at( 2 ).root<double>(), 3.14 );
        EXPECT_EQ( mixedArr->at( 3 ).root<bool>(), true );
    }

    //----------------------------------------------
    // Generic Document Pointer methods tests
    //----------------------------------------------

    TEST( DocumentTest, JsonPointerGenericMethods_BasicSetAndGet )
    {
        Document doc;

        // Test setting primitives with generic method
        Document stringDoc;

        stringDoc.set<std::string>( "", "test string" );
        doc.set<Document>( "/text", stringDoc );

        Document numberDoc;

        numberDoc.set<int64_t>( "", 42 );
        doc.set<Document>( "/number", numberDoc );

        Document boolDoc;
        boolDoc.set<bool>( "", true );
        doc.set<Document>( "/flag", boolDoc );

        // Test getting with generic method
        auto retrievedText = doc.get<Document>( "/text" );
        auto retrievedNumber = doc.get<Document>( "/number" );
        auto retrievedFlag = doc.get<Document>( "/flag" );

        ASSERT_TRUE( retrievedText.has_value() );
        ASSERT_TRUE( retrievedNumber.has_value() );
        ASSERT_TRUE( retrievedFlag.has_value() );

        EXPECT_EQ( retrievedText->get<std::string>( "" ), "test string" );
        EXPECT_EQ( retrievedNumber->get<int64_t>( "" ), 42 );
        EXPECT_EQ( retrievedFlag->get<bool>( "" ), true );
    }

    TEST( DocumentTest, JsonPointerGenericMethods_ArraysAndObjects )
    {
        Document doc;

        // Create an array
        Document arrayDoc;
        arrayDoc.set<Array>( "" ); // Initialize as array first
        arrayDoc.set<std::string>( "/0", "item1" );
        arrayDoc.set<std::string>( "/1", "item2" );
        arrayDoc.set<std::string>( "/2", "item3" );

        // Create an object
        Document objectDoc;
        objectDoc.set<std::string>( "/name", "Test Object" );
        objectDoc.set<int64_t>( "/value", 100 );

        // Set using generic method
        doc.set<Document>( "/data/items", arrayDoc );
        doc.set<Document>( "/data/config", objectDoc );

        // Get using generic method
        auto retrievedArray = doc.get<Document>( "/data/items" );
        auto retrievedObject = doc.get<Document>( "/data/config" );

        ASSERT_TRUE( retrievedArray.has_value() );
        ASSERT_TRUE( retrievedObject.has_value() );

        // Verify array content
        auto arrayWrapper = retrievedArray->get<Array>( "" );
        ASSERT_TRUE( arrayWrapper.has_value() );
        EXPECT_EQ( arrayWrapper->size(), 3 );
        EXPECT_TRUE( retrievedArray->is<Array>( "" ) );

        // Verify object content
        EXPECT_EQ( retrievedObject->get<std::string>( "/name" ), "Test Object" );
        EXPECT_EQ( retrievedObject->get<int64_t>( "/value" ), 100 );
        EXPECT_TRUE( retrievedObject->is<Object>( "" ) );

        // Also verify type-specific getters still work
        auto typedArray = doc.get<Document>( "/data/items" );
        auto typedObject = doc.get<Document>( "/data/config" );

        ASSERT_TRUE( typedArray.has_value() );
        ASSERT_TRUE( typedObject.has_value() );
    }

    TEST( DocumentTest, JsonPointerGenericMethods_TypeSafetyComparison )
    {
        Document doc;

        // Create array and object
        Document arrayDoc;
        arrayDoc.set<Array>( "" ); // Initialize as array first
        arrayDoc.set<std::string>( "/0", "test" );

        Document objectDoc;
        objectDoc.set<std::string>( "/key", "value" );

        doc.set<Document>( "/myarray", arrayDoc );
        doc.set<Document>( "/myobject", objectDoc );

        // Generic getter returns both
        EXPECT_TRUE( doc.get<Document>( "/myarray" ).has_value() );
        EXPECT_TRUE( doc.get<Document>( "/myobject" ).has_value() );

        // Type-specific getters are selective
        EXPECT_TRUE( doc.get<Array>( "/myarray" ).has_value() );
        EXPECT_FALSE( doc.get<Array>( "/myobject" ).has_value() ); // Object, not array

        EXPECT_TRUE( doc.get<Object>( "/myobject" ).has_value() );
        EXPECT_FALSE( doc.get<Object>( "/myarray" ).has_value() ); // Array, not object
    }

    TEST( DocumentTest, JsonPointerGenericMethods_ErrorHandling )
    {
        Document doc;
        doc.set<std::string>( "/test", "value" );

        // Non-existent paths return nullopt
        EXPECT_FALSE( doc.get<Document>( "/nonexistent" ).has_value() );
        EXPECT_FALSE( doc.get<Document>( "/test/nested" ).has_value() );

        // Empty and root paths
        auto rootDoc = doc.get<Document>( "" );
        EXPECT_TRUE( rootDoc.has_value() );
        EXPECT_TRUE( rootDoc->is<Object>( "" ) );
    }

    //----------------------------------------------
    // Object Pointer array methods tests
    //----------------------------------------------

    TEST( DocumentTest, JsonPointerObjectMethods_BasicSetAndGet )
    {
        Document doc;

        // Create a nested object
        Document profileObj;

        profileObj.set<std::string>( "/name", "John Doe" );
        profileObj.set<int64_t>( "/age", 30 );
        profileObj.set<bool>( "/active", true );

        // Set the object using JSON Pointer
        doc.set<Document>( "/profile", profileObj );

        // Verify the object was set
        EXPECT_TRUE( doc.contains( "/profile" ) );
        EXPECT_TRUE( doc.contains( "/profile/name" ) );
        EXPECT_TRUE( doc.contains( "/profile/age" ) );
        EXPECT_TRUE( doc.contains( "/profile/active" ) );

        // Get the object back
        auto retrievedProfile = doc.get<Document>( "/profile" );
        ASSERT_TRUE( retrievedProfile.has_value() );

        // Verify the content
        EXPECT_EQ( retrievedProfile->get<std::string>( "/name" ), "John Doe" );
        EXPECT_EQ( retrievedProfile->get<int64_t>( "/age" ), 30 );
        EXPECT_EQ( retrievedProfile->get<bool>( "/active" ), true );
    }

    TEST( DocumentTest, JsonPointerObjectMethods_NestedObjects )
    {
        Document doc;

        // Create deeply nested structure
        Document addressObj;

        addressObj.set<std::string>( "/street", "123 Main St" );
        addressObj.set<std::string>( "/city", "Anytown" );
        addressObj.set<int64_t>( "/zipcode", 12345 );

        Document userObj;

        userObj.set<std::string>( "/name", "Jane Smith" );
        userObj.set<int64_t>( "/id", 456 );
        userObj.set<Document>( "/address", addressObj );

        // Set the nested user object
        doc.set<Document>( "/user", userObj );

        // Verify nested access
        EXPECT_TRUE( doc.contains( "/user/name" ) );
        EXPECT_TRUE( doc.contains( "/user/address/street" ) );
        EXPECT_TRUE( doc.contains( "/user/address/city" ) );

        // Retrieve and verify nested object
        auto retrievedUser = doc.get<Document>( "/user" );
        ASSERT_TRUE( retrievedUser.has_value() );

        auto retrievedAddress = retrievedUser->get<Document>( "/address" );
        ASSERT_TRUE( retrievedAddress.has_value() );

        EXPECT_EQ( retrievedAddress->get<std::string>( "/street" ), "123 Main St" );
        EXPECT_EQ( retrievedAddress->get<std::string>( "/city" ), "Anytown" );
        EXPECT_EQ( retrievedAddress->get<int64_t>( "/zipcode" ), 12345 );
    }

    TEST( DocumentTest, JsonPointerObjectMethods_ErrorHandling )
    {
        Document doc;
        doc.set<std::string>( "/name", "Test" );
        doc.set<int64_t>( "/value", 42 );

        // Try to get object from non-object fields
        EXPECT_FALSE( doc.get<Object>( "/name" ).has_value() );
        EXPECT_FALSE( doc.get<Object>( "/value" ).has_value() );

        // Try to get non-existent object
        EXPECT_FALSE( doc.get<Object>( "/nonexistent" ).has_value() );

        // Try to get object from array
        Document arrayDoc;
        arrayDoc.set<std::string>( "/0", "item1" );
        doc.set<Document>( "/items", arrayDoc );

        EXPECT_FALSE( doc.get<Object>( "/items" ).has_value() );
    }

    TEST( DocumentTest, JsonPointerObjectMethods_ReplaceExistingObject )
    {
        Document doc;

        // Set initial object
        Document obj1;

        obj1.set<std::string>( "/type", "original" );
        obj1.set<int64_t>( "/version", 1 );
        doc.set<Document>( "/config", obj1 );

        // Verify initial state
        auto retrieved1 = doc.get<Document>( "/config" );
        ASSERT_TRUE( retrieved1.has_value() );
        EXPECT_EQ( retrieved1->get<std::string>( "/type" ), "original" );
        EXPECT_EQ( retrieved1->get<int64_t>( "/version" ), 1 );

        // Replace with new object
        Document obj2;

        obj2.set<std::string>( "/type", "updated" );
        obj2.set<int64_t>( "/version", 2 );
        obj2.set<bool>( "/active", true );
        doc.set<Document>( "/config", obj2 );

        // Verify replacement
        auto retrieved2 = doc.get<Document>( "/config" );
        ASSERT_TRUE( retrieved2.has_value() );
        EXPECT_EQ( retrieved2->get<std::string>( "/type" ), "updated" );
        EXPECT_EQ( retrieved2->get<int64_t>( "/version" ), 2 );
        EXPECT_EQ( retrieved2->get<bool>( "/active" ), true );
    }

    TEST( DocumentTest, JsonPointerObjectMethods_ComplexObjectWithArrays )
    {
        Document doc;

        // Create object with mixed content including arrays
        Document complexObj;

        complexObj.set<std::string>( "/title", "Complex Object" );

        // Add an array to the object
        Document tagsArray;
        tagsArray.set<std::string>( "/0", "tag1" );
        tagsArray.set<std::string>( "/1", "tag2" );
        tagsArray.set<std::string>( "/2", "tag3" );
        complexObj.set<Document>( "/tags", tagsArray );

        // Add a nested object
        Document metaObj;
        metaObj.set<std::string>( "/author", "Test Author" );
        metaObj.set<int64_t>( "/created", 1234567890 );
        complexObj.set<Document>( "/metadata", metaObj );

        // Set the complex object
        doc.set<Document>( "/data", complexObj );

        // Retrieve and verify
        auto retrieved = doc.get<Document>( "/data" );
        ASSERT_TRUE( retrieved.has_value() );

        EXPECT_EQ( retrieved->get<std::string>( "/title" ), "Complex Object" );

        auto retrievedTags = retrieved->get<Document>( "/tags" );
        ASSERT_TRUE( retrievedTags.has_value() );
        // Get Array wrapper to access size
        auto tagsArrayWrapper = retrievedTags->get<Array>( "" );
        ASSERT_TRUE( tagsArrayWrapper.has_value() );
        EXPECT_EQ( tagsArrayWrapper->size(), 3 );

        auto retrievedMeta = retrieved->get<Document>( "/metadata" );
        ASSERT_TRUE( retrievedMeta.has_value() );
        EXPECT_EQ( retrievedMeta->get<std::string>( "/author" ), "Test Author" );
        EXPECT_EQ( retrievedMeta->get<int64_t>( "/created" ), 1234567890 );
    }

    TEST( DocumentTest, JsonPointerObjectMethods_RoundtripSerialization )
    {
        Document original;

        // Create a complex nested structure
        Document userObj;

        userObj.set<std::string>( "/username", "testuser" );
        userObj.set<int64_t>( "/userId", 12345 );

        Document prefsObj;

        prefsObj.set<bool>( "/emailNotifications", true );
        prefsObj.set<std::string>( "/theme", "dark" );
        userObj.set<Document>( "/preferences", prefsObj );

        original.set<Document>( "/user", userObj );

        // Serialize to JSON string
        std::string jsonStr = original.toString();

        // Deserialize back
        auto deserialized = Document::fromString( jsonStr );
        ASSERT_TRUE( deserialized.has_value() );

        // Verify the object survived serialization
        auto deserializedUser = deserialized->get<Document>( "/user" );
        ASSERT_TRUE( deserializedUser.has_value() );

        EXPECT_EQ( deserializedUser->get<std::string>( "/username" ), "testuser" );
        EXPECT_EQ( deserializedUser->get<int64_t>( "/userId" ), 12345 );

        auto deserializedPrefs = deserializedUser->get<Document>( "/preferences" );
        ASSERT_TRUE( deserializedPrefs.has_value() );
        EXPECT_EQ( deserializedPrefs->get<bool>( "/emailNotifications" ), true );
        EXPECT_EQ( deserializedPrefs->get<std::string>( "/theme" ), "dark" );
    }

    //----------------------------------------------
    // Generic Document operations tests
    //----------------------------------------------

    TEST( DocumentTest, GenericGetDocument )
    {
        Document doc;

        // Test getting primitive values as documents
        doc.set<std::string>( "name", "Alice" );
        doc.set<int64_t>( "age", 30 );
        doc.set<bool>( "active", true );

        auto nameDoc = doc.get<Document>( "name" );
        ASSERT_TRUE( nameDoc.has_value() );
        EXPECT_EQ( nameDoc->get<std::string>( "" ), "Alice" );

        auto ageDoc = doc.get<Document>( "age" );
        ASSERT_TRUE( ageDoc.has_value() );
        EXPECT_EQ( ageDoc->get<int64_t>( "" ), 30 );

        auto activeDoc = doc.get<Document>( "active" );
        ASSERT_TRUE( activeDoc.has_value() );
        EXPECT_EQ( activeDoc->get<bool>( "" ), true );

        // Test getting nested objects
        doc.set<std::string>( "user.profile.firstName", "Bob" );
        doc.set<std::string>( "user.profile.lastName", "Smith" );

        auto userDoc = doc.get<Document>( "user" );
        ASSERT_TRUE( userDoc.has_value() );
        EXPECT_EQ( userDoc->get<std::string>( "profile.firstName" ), "Bob" );
        EXPECT_EQ( userDoc->get<std::string>( "profile.lastName" ), "Smith" );

        auto profileDoc = doc.get<Document>( "user.profile" );
        ASSERT_TRUE( profileDoc.has_value() );
        EXPECT_EQ( profileDoc->get<std::string>( "firstName" ), "Bob" );
        EXPECT_EQ( profileDoc->get<std::string>( "lastName" ), "Smith" );

        // Test getting arrays
        doc.set<std::string>( "/hobbies/0", "reading" );
        doc.set<std::string>( "/hobbies/1", "coding" );

        auto hobbiesArr = doc.get<Array>( "hobbies" );
        ASSERT_TRUE( hobbiesArr.has_value() );
        EXPECT_EQ( hobbiesArr->size(), 2 );
        EXPECT_EQ( hobbiesArr->at( 0 ).root<std::string>(), "reading" );
        EXPECT_EQ( hobbiesArr->at( 1 ).root<std::string>(), "coding" );

        // Test non-existent path
        auto nonExistent = doc.get<Document>( "doesnotexist" );
        EXPECT_FALSE( nonExistent.has_value() );
    }

    TEST( DocumentTest, GenericSetDocument )
    {
        Document doc;

        // Test setting primitive documents
        Document nameDoc;
        nameDoc.set<std::string>( "", "Alice" );

        doc.set<Document>( "name", nameDoc );
        EXPECT_EQ( doc.get<std::string>( "name" ), "Alice" );

        Document ageDoc;
        ageDoc.set<int64_t>( "", 25 );
        doc.set<Document>( "age", ageDoc );
        EXPECT_EQ( doc.get<int64_t>( "age" ), 25 );

        // Test setting complex objects
        Document profileDoc;
        profileDoc.set<std::string>( "firstName", "Bob" );
        profileDoc.set<std::string>( "lastName", "Smith" );
        profileDoc.set<int64_t>( "experience", 5 );

        doc.set<Document>( "user.profile", profileDoc );
        EXPECT_EQ( doc.get<std::string>( "user.profile.firstName" ), "Bob" );
        EXPECT_EQ( doc.get<std::string>( "user.profile.lastName" ), "Smith" );
        EXPECT_EQ( doc.get<int64_t>( "user.profile.experience" ), 5 );

        // Test setting arrays
        Document hobbiesDoc;
        hobbiesDoc.set<std::string>( "/0", "reading" );
        hobbiesDoc.set<std::string>( "/1", "gaming" );
        hobbiesDoc.set<std::string>( "/2", "traveling" );

        doc.set<Document>( "hobbies", hobbiesDoc );
        auto hobbies = doc.get<Array>( "hobbies" );
        ASSERT_TRUE( hobbies.has_value() );
        EXPECT_EQ( hobbies->size(), 3 );
        EXPECT_EQ( hobbies->at( 0 ).root<std::string>(), "reading" );
        EXPECT_EQ( hobbies->at( 1 ).root<std::string>(), "gaming" );
        EXPECT_EQ( hobbies->at( 2 ).root<std::string>(), "traveling" );

        // Test overwriting existing values
        Document newNameDoc;
        newNameDoc.set<std::string>( "", "Charlie" );
        doc.set<Document>( "name", newNameDoc );
        EXPECT_EQ( doc.get<std::string>( "name" ), "Charlie" );

        // Verify JSON structure contains all expected elements (accounting for pretty-printing)
        std::string jsonStr = doc.toString( 0 );
        EXPECT_TRUE( jsonStr.find( "\"age\"" ) != std::string::npos );
        EXPECT_TRUE( jsonStr.find( "25" ) != std::string::npos );
        EXPECT_TRUE( jsonStr.find( "\"name\"" ) != std::string::npos );
        EXPECT_TRUE( jsonStr.find( "\"Charlie\"" ) != std::string::npos );
        EXPECT_TRUE( jsonStr.find( "\"hobbies\"" ) != std::string::npos );
        EXPECT_TRUE( jsonStr.find( "\"reading\"" ) != std::string::npos );
        EXPECT_TRUE( jsonStr.find( "\"gaming\"" ) != std::string::npos );
        EXPECT_TRUE( jsonStr.find( "\"traveling\"" ) != std::string::npos );
        EXPECT_TRUE( jsonStr.find( "\"firstName\"" ) != std::string::npos );
        EXPECT_TRUE( jsonStr.find( "\"Bob\"" ) != std::string::npos );
        EXPECT_TRUE( jsonStr.find( "\"lastName\"" ) != std::string::npos );
        EXPECT_TRUE( jsonStr.find( "\"Smith\"" ) != std::string::npos );
        EXPECT_TRUE( jsonStr.find( "\"experience\"" ) != std::string::npos );
        EXPECT_TRUE( jsonStr.find( "5" ) != std::string::npos );
    }

    TEST( DocumentTest, GenericAddToArrayWithDocument )
    {
        Document doc;

        // Test adding primitive documents to array
        Document str1;

        str1.set<std::string>( "", "first" );
        auto stringsOpt = doc.get<Array>( "strings" );
        if( !stringsOpt.has_value() )
        {
            doc.set<Array>( "strings" );
        }

        // Use rootRef for direct array modification
        auto stringsRefOpt = doc.at( "strings" ).rootRef<Array>();
        ASSERT_TRUE( stringsRefOpt.has_value() );
        Array& stringsRef = stringsRefOpt->get();
        stringsRef.push_back( str1 );

        Document str2;

        str2.set<std::string>( "", "second" );
        stringsRef.push_back( str2 );

        EXPECT_EQ( doc.get<Array>( "strings" ).value().size(), 2 );
        auto stringsArray = doc.get<Array>( "strings" ).value();
        EXPECT_EQ( stringsArray.at( 0 ).root<std::string>(), "first" );
        EXPECT_EQ( stringsArray.at( 1 ).root<std::string>(), "second" );

        // Test adding complex objects to array
        Document user1;
        user1.set<std::string>( "name", "Alice" );
        user1.set<int64_t>( "age", 30 );
        user1.set<bool>( "active", true );

        Document user2;
        user2.set<std::string>( "name", "Bob" );
        user2.set<int64_t>( "age", 25 );
        user2.set<bool>( "active", false );

        auto usersOpt = doc.get<Array>( "users" );
        if( !usersOpt.has_value() )
        {
            doc.set<Array>( "users" );
        }

        auto usersRefOpt = doc.at( "users" ).rootRef<Array>();
        ASSERT_TRUE( usersRefOpt.has_value() );
        Array& usersRef = usersRefOpt->get();
        usersRef.push_back( user1 );
        usersRef.push_back( user2 );

        EXPECT_EQ( doc.get<Array>( "users" ).value().size(), 2 );

        // Verify first user using direct access
        auto usersArray = doc.get<Array>( "users" ).value();
        auto& firstUser = usersArray.at( 0 );
        EXPECT_EQ( firstUser.get<std::string>( "name" ), "Alice" );
        EXPECT_EQ( firstUser.get<int64_t>( "age" ), 30 );
        EXPECT_EQ( firstUser.get<bool>( "active" ), true );

        // Verify second user
        auto& secondUser = usersArray.at( 1 );
        EXPECT_EQ( secondUser.get<std::string>( "name" ), "Bob" );
        EXPECT_EQ( secondUser.get<int64_t>( "age" ), 25 );
        EXPECT_EQ( secondUser.get<bool>( "active" ), false );

        // Test adding nested arrays
        Document nestedArray;
        nestedArray.set<Array>( "" );

        nestedArray.set<std::string>( "/0", "item1" );
        nestedArray.set<std::string>( "/1", "item2" );

        auto nestedArraysOpt = doc.get<Array>( "nested" );
        if( !nestedArraysOpt.has_value() )
        {
            doc.set<Array>( "nested" );
        }

        auto nestedArraysRefOpt = doc.at( "nested" ).rootRef<Array>();
        ASSERT_TRUE( nestedArraysRefOpt.has_value() );
        Array& nestedArraysRef = nestedArraysRefOpt->get();
        nestedArraysRef.push_back( nestedArray );
        EXPECT_EQ( doc.get<Array>( "nested" ).value().size(), 1 );

        auto nestedArr = doc.get<Array>( "nested" ).value();
        auto retrievedNestedArray = nestedArr.at( 0 );
        // Get Array wrapper to access size
        auto nestedArrayWrapper = retrievedNestedArray.get<Array>( "" );
        ASSERT_TRUE( nestedArrayWrapper.has_value() );
        EXPECT_EQ( nestedArrayWrapper->size(), 2 );
        auto innerArr = retrievedNestedArray.get<Array>( "" ).value();
        EXPECT_EQ( innerArr.at( 0 ).root<std::string>(), "item1" );
        EXPECT_EQ( innerArr.at( 1 ).root<std::string>(), "item2" );

        // Test adding to non-existent array (should create it)
        Document newItem;
        newItem.set<std::string>( "id", "test123" );
        auto itemsOpt = doc.get<Array>( "items" );
        if( !itemsOpt.has_value() )
        {
            doc.set<Array>( "items" );
        }

        auto itemsRefOpt = doc.at( "items" ).rootRef<Array>();
        ASSERT_TRUE( itemsRefOpt.has_value() );
        Array& itemsRef = itemsRefOpt->get();
        itemsRef.push_back( newItem );

        EXPECT_EQ( doc.get<Array>( "items" ).value().size(), 1 );
        auto itemsArr = doc.get<Array>( "items" ).value();
        auto item = itemsArr.at( 0 );
        EXPECT_EQ( item.get<std::string>( "" ).value_or( "missing" ), "missing" ); // Not a string, it's an object

        auto retrievedItem = itemsArr.at( 0 );
        EXPECT_EQ( retrievedItem.get<std::string>( "id" ), "test123" );
    }

    TEST( DocumentTest, GenericMethodsConsistency )
    {
        Document doc;

        // Test consistency between generic and specialized methods
        Document complexDoc;
        complexDoc.set<std::string>( "type", "user" );

        complexDoc.set<int64_t>( "id", 12345 );
        complexDoc.set<bool>( "verified", true );

        // Set using generic method
        doc.set<Document>( "profile", complexDoc );

        // Retrieve using specialized methods
        EXPECT_EQ( doc.get<std::string>( "profile.type" ), "user" );
        EXPECT_EQ( doc.get<int64_t>( "profile.id" ), 12345 );
        EXPECT_EQ( doc.get<bool>( "profile.verified" ), true );

        // Retrieve using generic method and verify
        auto retrievedProfile = doc.get<Document>( "profile" );
        ASSERT_TRUE( retrievedProfile.has_value() );
        EXPECT_EQ( retrievedProfile->get<std::string>( "type" ), "user" );
        EXPECT_EQ( retrievedProfile->get<int64_t>( "id" ), 12345 );
        EXPECT_EQ( retrievedProfile->get<bool>( "verified" ), true );

        // Test array consistency
        Document arrayItem;
        arrayItem.set<std::string>( "value", "test" );

        auto itemsConsistencyOpt = doc.get<Array>( "items" );
        if( !itemsConsistencyOpt.has_value() )
        {
            doc.set<Array>( "items" );
        }

        auto itemsConsistencyRefOpt = doc.at( "items" ).rootRef<Array>();
        ASSERT_TRUE( itemsConsistencyRefOpt.has_value() );
        Array& itemsConsistencyRef = itemsConsistencyRefOpt->get();
        itemsConsistencyRef.push_back( arrayItem );
        // Verify via specialized array methods
        EXPECT_EQ( doc.get<Array>( "items" ).value().size(), 1 );
        auto itemsArr = doc.get<Array>( "items" ).value();
        auto item = itemsArr.at( 0 );
        EXPECT_EQ( item.get<std::string>( "value" ), "test" );

        // Verify via generic methods
        auto itemsArray = doc.get<Document>( "items" );
        ASSERT_TRUE( itemsArray.has_value() );
        // Get Array wrapper to access size
        auto itemsArrayWrapper = itemsArray->get<Array>( "" );
        ASSERT_TRUE( itemsArrayWrapper.has_value() );
        EXPECT_EQ( itemsArrayWrapper->size(), 1 );
        auto innerArr = itemsArray->get<Array>( "" ).value();
        auto& itemFromGeneric = innerArr.at( 0 );
        EXPECT_EQ( itemFromGeneric.get<std::string>( "value" ), "test" );
    }

    //----------------------------------------------
    // Output parameter get() tests
    //----------------------------------------------

    TEST( DocumentTest, GetOutputParameter_Document )
    {
        std::string jsonStr = R"({
            "name": "Alice",
            "age": 25,
            "score": 98.5,
            "active": true,
            "tags": ["a", "b", "c"],
            "profile": {"level": 5}
        })";
        auto maybeDoc = Document::fromString( jsonStr );
        ASSERT_TRUE( maybeDoc.has_value() );
        Document doc = maybeDoc.value();

        // Test string output parameter
        std::string name;
        EXPECT_TRUE( doc.get( "/name", name ) );
        EXPECT_EQ( name, "Alice" );

        // Test with default value preservation on failure
        std::string missing = "default";
        EXPECT_FALSE( doc.get( "/nonexistent", missing ) );
        EXPECT_EQ( missing, "default" ); // Unchanged

        // Test integer output parameter
        int64_t age = 0;
        EXPECT_TRUE( doc.get( "/age", age ) );
        EXPECT_EQ( age, 25 );

        // Test double output parameter
        double score = 0.0;
        EXPECT_TRUE( doc.get( "/score", score ) );
        EXPECT_DOUBLE_EQ( score, 98.5 );

        // Test bool output parameter
        bool active = false;
        EXPECT_TRUE( doc.get( "/active", active ) );
        EXPECT_TRUE( active );

        // Test nested path
        int64_t level = 0;
        EXPECT_TRUE( doc.get( "/profile/level", level ) );
        EXPECT_EQ( level, 5 );

        // Test type mismatch - should fail
        int64_t wrongType = 42;
        EXPECT_FALSE( doc.get( "/name", wrongType ) ); // name is string, not int
        EXPECT_EQ( wrongType, 42 );                    // Unchanged
    }

    TEST( DocumentTest, GetOutputParameter_Object )
    {
        std::string jsonStr = R"({
            "user": {
                "name": "Bob",
                "id": 123,
                "nested": {"value": 999}
            }
        })";
        auto maybeDoc = Document::fromString( jsonStr );
        ASSERT_TRUE( maybeDoc.has_value() );
        Document doc = maybeDoc.value();

        // Test string output parameter using Document path
        std::string name;
        EXPECT_TRUE( doc.get( "/user/name", name ) );
        EXPECT_EQ( name, "Bob" );

        // Test integer output parameter
        int64_t id = 0;
        EXPECT_TRUE( doc.get( "/user/id", id ) );
        EXPECT_EQ( id, 123 );

        // Test nested path
        int64_t nestedValue = 0;
        EXPECT_TRUE( doc.get( "/user/nested/value", nestedValue ) );
        EXPECT_EQ( nestedValue, 999 );

        // Test missing field
        std::string missing = "unchanged";
        EXPECT_FALSE( doc.get( "/user/nonexistent", missing ) );
        EXPECT_EQ( missing, "unchanged" );
    }

    TEST( DocumentTest, GetOutputParameter_ArrayByIndex )
    {
        std::string jsonStr = R"({
            "numbers": [10, 20, 30, 40, 50],
            "strings": ["alpha", "beta", "gamma"],
            "mixed": [1, "two", 3.0, true]
        })";
        auto maybeDoc = Document::fromString( jsonStr );
        ASSERT_TRUE( maybeDoc.has_value() );
        Document doc = maybeDoc.value();

        // Test integer by index using Document path
        int64_t num = 0;
        EXPECT_TRUE( doc.get( "/numbers/0", num ) );
        EXPECT_EQ( num, 10 );

        EXPECT_TRUE( doc.get( "/numbers/2", num ) );
        EXPECT_EQ( num, 30 );

        EXPECT_TRUE( doc.get( "/numbers/4", num ) );
        EXPECT_EQ( num, 50 );

        // Test out of bounds
        int64_t outOfBounds = 999;
        EXPECT_FALSE( doc.get( "/numbers/100", outOfBounds ) );
        EXPECT_EQ( outOfBounds, 999 ); // Unchanged

        // Test strings
        std::string str;
        EXPECT_TRUE( doc.get( "/strings/0", str ) );
        EXPECT_EQ( str, "alpha" );

        EXPECT_TRUE( doc.get( "/strings/1", str ) );
        EXPECT_EQ( str, "beta" );

        // Test type mismatch in mixed array
        int64_t intVal = -1;
        EXPECT_TRUE( doc.get( "/mixed/0", intVal ) ); // First element is int
        EXPECT_EQ( intVal, 1 );

        std::string strVal;
        EXPECT_TRUE( doc.get( "/mixed/1", strVal ) ); // Second element is string
        EXPECT_EQ( strVal, "two" );

        double dblVal = 0.0;
        EXPECT_TRUE( doc.get( "/mixed/2", dblVal ) ); // Third element is double
        EXPECT_DOUBLE_EQ( dblVal, 3.0 );

        bool boolVal = false;
        EXPECT_TRUE( doc.get( "/mixed/3", boolVal ) ); // Fourth element is bool
        EXPECT_TRUE( boolVal );
    }

    TEST( DocumentTest, GetOutputParameter_ArrayByPath )
    {
        std::string jsonStr = R"({
        "data": [
                {"name": "first", "value": 100},
                {"name": "second", "value": 200},
                {"name": "third", "value": 300}
            ]
        })";
        auto maybeDoc = Document::fromString( jsonStr );
        ASSERT_TRUE( maybeDoc.has_value() );
        Document doc = maybeDoc.value();

        // Access nested elements by path using Document API
        std::string name;
        EXPECT_TRUE( doc.get( "/data/0/name", name ) );
        EXPECT_EQ( name, "first" );

        int64_t value = 0;
        EXPECT_TRUE( doc.get( "/data/1/value", value ) );
        EXPECT_EQ( value, 200 );

        EXPECT_TRUE( doc.get( "/data/2/name", name ) );
        EXPECT_EQ( name, "third" );

        // Test missing path
        std::string missing = "default";
        EXPECT_FALSE( doc.get( "/data/99/name", missing ) );
        EXPECT_EQ( missing, "default" );
    }

    TEST( DocumentTest, GetOutputParameter_AllNumericTypes )
    {
        Document doc;
        doc.set<int8_t>( "i8", static_cast<int8_t>( -8 ) );
        doc.set<int16_t>( "i16", static_cast<int16_t>( -16 ) );
        doc.set<int32_t>( "i32", static_cast<int32_t>( -32 ) );
        doc.set<int64_t>( "i64", static_cast<int64_t>( -64 ) );
        doc.set<uint8_t>( "u8", static_cast<uint8_t>( 8 ) );
        doc.set<uint16_t>( "u16", static_cast<uint16_t>( 16 ) );
        doc.set<uint32_t>( "u32", static_cast<uint32_t>( 32 ) );
        doc.set<uint64_t>( "u64", static_cast<uint64_t>( 64 ) );
        doc.set<float>( "f32", 3.14f );
        doc.set<double>( "f64", 6.28 );

        int8_t i8 = 0;
        EXPECT_TRUE( doc.get( "i8", i8 ) );
        EXPECT_EQ( i8, -8 );

        int16_t i16 = 0;
        EXPECT_TRUE( doc.get( "i16", i16 ) );
        EXPECT_EQ( i16, -16 );

        int32_t i32 = 0;
        EXPECT_TRUE( doc.get( "i32", i32 ) );
        EXPECT_EQ( i32, -32 );

        int64_t i64 = 0;
        EXPECT_TRUE( doc.get( "i64", i64 ) );
        EXPECT_EQ( i64, -64 );

        uint8_t u8 = 0;
        EXPECT_TRUE( doc.get( "u8", u8 ) );
        EXPECT_EQ( u8, 8 );

        uint16_t u16 = 0;
        EXPECT_TRUE( doc.get( "u16", u16 ) );
        EXPECT_EQ( u16, 16 );

        uint32_t u32 = 0;
        EXPECT_TRUE( doc.get( "u32", u32 ) );
        EXPECT_EQ( u32, 32 );

        uint64_t u64 = 0;
        EXPECT_TRUE( doc.get( "u64", u64 ) );
        EXPECT_EQ( u64, 64 );

        float f32 = 0.0f;
        EXPECT_TRUE( doc.get( "f32", f32 ) );
        EXPECT_FLOAT_EQ( f32, 3.14f );

        double f64 = 0.0;
        EXPECT_TRUE( doc.get( "f64", f64 ) );
        EXPECT_DOUBLE_EQ( f64, 6.28 );
    }

    TEST( DocumentTest, GetOutputParameter_DocumentAndContainers )
    {
        std::string jsonStr = R"({
            "obj": {"key": "value"},
            "arr": [1, 2, 3],
            "nested": {"inner": {"deep": "data"}}
        })";
        auto maybeDoc = Document::fromString( jsonStr );
        ASSERT_TRUE( maybeDoc.has_value() );
        Document doc = maybeDoc.value();

        // Test getting Document
        Document subDoc;
        EXPECT_TRUE( doc.get( "/obj", subDoc ) );
        EXPECT_EQ( subDoc.get<std::string>( "key" ).value_or( "" ), "value" );

        // Test getting Object
        Object obj;
        EXPECT_TRUE( doc.get( "/obj", obj ) );
        std::string key;
        EXPECT_TRUE( doc.get( "/obj/key", key ) );
        EXPECT_EQ( key, "value" );

        // Test getting Array
        Array arr;
        EXPECT_TRUE( doc.get( "/arr", arr ) );
        EXPECT_EQ( arr.size(), 3 );
        int64_t first = 0;
        EXPECT_TRUE( doc.get( "/arr/0", first ) );
        EXPECT_EQ( first, 1 );

        // Test deeply nested
        Object nested;
        EXPECT_TRUE( doc.get( "/nested", nested ) );
        std::string deep;
        EXPECT_TRUE( doc.get( "/nested/inner/deep", deep ) );
        EXPECT_EQ( deep, "data" );
    }

    //=====================================================================
    // Silent Type Conversion Tests
    //=====================================================================

    TEST( DocumentTest, SilentConversion_StringToObjectWithDotNotation )
    {
        Document doc;
        doc.set<std::string>( "user", "Alice" );
        EXPECT_EQ( doc.get<std::string>( "user" ), "Alice" );

        doc.set<int>( "user.age", 30 ); // Try to add subfield to string

        EXPECT_TRUE( doc.is<std::string>( "user" ) ) << "BUG: user was silently converted from string to object!";
        EXPECT_EQ( doc.get<std::string>( "user" ), "Alice" ) << "BUG: Original string value was lost!";
        EXPECT_FALSE( doc.contains( "user.age" ) ) << "BUG: Should not create subfield on a string!";
    }

    TEST( DocumentTest, SilentConversion_IntToObjectWithJsonPointer )
    {
        Document doc;
        doc.set<int>( "/config", 42 );
        EXPECT_EQ( doc.get<int>( "/config" ), 42 );

        doc.set<std::string>( "/config/mode", "debug" ); // Try to add subfield to int

        EXPECT_TRUE( doc.is<int>( "/config" ) ) << "BUG: config was silently converted from int to object!";
        EXPECT_EQ( doc.get<int>( "/config" ), 42 ) << "BUG: Original int value was lost!";
        EXPECT_FALSE( doc.contains( "/config/mode" ) ) << "BUG: Should not create subfield on an int!";
    }

    TEST( DocumentTest, SilentConversion_ArrayToObjectWithDotNotation )
    {
        Document doc;
        doc.set<std::string>( "items[0]", "first" );
        doc.set<std::string>( "items[1]", "second" );
        EXPECT_TRUE( doc.is<Array>( "items" ) );

        doc.set<std::string>( "items.name", "MyArray" ); // Try to add named field to array

        EXPECT_TRUE( doc.is<Array>( "items" ) ) << "BUG: items was silently converted from array to object!";
        EXPECT_FALSE( doc.contains( "items.name" ) ) << "BUG: Should not create named field on an array!";
    }

    TEST( DocumentTest, SilentConversion_EmptyObjectToArray )
    {
        Document doc;
        doc.set<std::string>( "/data/config", "value" );
        EXPECT_TRUE( doc.is<Object>( "/data" ) );

        doc.set<std::string>( "/data/0", "first" ); // Try to use array notation on object

        EXPECT_TRUE( doc.is<Object>( "/data" ) ) << "BUG: data was silently converted from object to array!";
        EXPECT_TRUE( doc.contains( "/data/config" ) ) << "BUG: Original object field was lost!";
    }

    TEST( DocumentTest, SilentConversion_BoolToObjectWithJsonPointer )
    {
        Document doc;
        doc.set<bool>( "/active", true );
        EXPECT_EQ( doc.get<bool>( "/active" ), true );

        doc.set<std::string>( "/active/status", "enabled" ); // Try to add subfield to bool

        EXPECT_TRUE( doc.is<bool>( "/active" ) ) << "BUG: active was silently converted from bool to object!";
        EXPECT_EQ( doc.get<bool>( "/active" ), true ) << "BUG: Original bool value was lost!";
        EXPECT_FALSE( doc.contains( "/active/status" ) ) << "BUG: Should not create subfield on a bool!";
    }

    TEST( DocumentTest, ValidConversion_NullToObject )
    {
        Document doc{ nullptr }; // Create null document
        EXPECT_TRUE( doc.isNull( "" ) );

        doc.set<std::string>( "name", "Alice" );

        EXPECT_TRUE( doc.is<Object>( "" ) );
        EXPECT_EQ( doc.get<std::string>( "name" ), "Alice" );
    }

    TEST( DocumentTest, ValidConversion_NullToArray )
    {
        Document doc{ nullptr }; // Create null document
        EXPECT_TRUE( doc.isNull( "" ) );

        doc.set<std::string>( "/0", "first" );

        EXPECT_TRUE( doc.is<Array>( "" ) );
        EXPECT_EQ( doc.get<std::string>( "/0" ), "first" );
    }

    //=====================================================================
    // empty() method tests
    //=====================================================================

    TEST( DocumentTest, EmptyArray )
    {
        Document arr = Document::array();
        EXPECT_TRUE( arr.empty() );

        arr.push_back( int64_t{ 1 } );
        EXPECT_FALSE( arr.empty() );
    }

    TEST( DocumentTest, EmptyObject )
    {
        Document obj = Document::object();
        EXPECT_TRUE( obj.empty() );

        obj.set( "key", int64_t{ 1 } );
        EXPECT_FALSE( obj.empty() );
    }

    TEST( DocumentTest, EmptyString )
    {
        Document str{ std::string( "" ) };
        EXPECT_TRUE( str.empty() );

        Document str2{ std::string( "hello" ) };
        EXPECT_FALSE( str2.empty() );
    }

    TEST( DocumentTest, EmptyPrimitives )
    {
        EXPECT_FALSE( Document{ nullptr }.empty() );
        EXPECT_FALSE( Document{ true }.empty() );
        EXPECT_FALSE( Document{ false }.empty() );
        EXPECT_FALSE( Document{ int64_t{ 0 } }.empty() );
        EXPECT_FALSE( Document{ 0.0 }.empty() );
    }

    //=====================================================================
    // capacity() method tests
    //=====================================================================

    TEST( DocumentTest, CapacityArray )
    {
        Document arr = Document::array();
        EXPECT_EQ( arr.capacity(), 0u );

        arr.reserve( 100 );
        EXPECT_GE( arr.capacity(), 100u );

        arr.push_back( int64_t{ 1 } );
        EXPECT_GE( arr.capacity(), 100u );
    }

    TEST( DocumentTest, CapacityObject )
    {
        Document obj = Document::object();
        EXPECT_EQ( obj.capacity(), 0u );

        obj.reserve( 50 );
        EXPECT_GE( obj.capacity(), 50u );

        obj.set( "key", int64_t{ 1 } );
        EXPECT_GE( obj.capacity(), 50u );
    }

    TEST( DocumentTest, CapacityPrimitives )
    {
        EXPECT_EQ( Document{ nullptr }.capacity(), 0u );
        EXPECT_EQ( Document{ true }.capacity(), 0u );
        EXPECT_EQ( Document{ int64_t{ 42 } }.capacity(), 0u );
        EXPECT_EQ( Document{ std::string( "test" ) }.capacity(), 0u );
    }

    //=====================================================================
    // front() method tests
    //=====================================================================

    TEST( DocumentTest, FrontArray )
    {
        Document arr = Document::array();
        arr.push_back( int64_t{ 10 } );
        arr.push_back( int64_t{ 20 } );
        arr.push_back( int64_t{ 30 } );

        EXPECT_EQ( arr.front().get<int64_t>( "" ), 10 );
        EXPECT_EQ( arr.back().get<int64_t>( "" ), 30 );
    }

    TEST( DocumentTest, FrontArrayConst )
    {
        Document arr = Document::array();
        arr.push_back( int64_t{ 42 } );
        const Document& constArr = arr;

        EXPECT_EQ( constArr.front().get<int64_t>( "" ), 42 );
    }

    TEST( DocumentTest, FrontEmptyArray )
    {
        Document arr = Document::array();
        EXPECT_THROW( { (void)arr.front(); }, std::out_of_range );
    }

    TEST( DocumentTest, FrontModify )
    {
        Document arr = Document::array();
        arr.push_back( int64_t{ 100 } );

        arr.front() = int64_t{ 200 };
        EXPECT_EQ( arr[0].get<int64_t>( "" ), 200 );
    }

    //=====================================================================
    // Path syntax: Auto-creation and bracket notation tests
    //=====================================================================

    TEST( DocumentTest, BracketNotation_AutoCreateArrayElements )
    {
        Document doc{};

        // Bracket notation should automatically create the array and elements
        doc.set<std::string>( "users[0].name", "Alice" );
        doc.set<int64_t>( "users[0].age", 30 );

        // Verify the array was created
        ASSERT_TRUE( doc.contains( "users" ) );
        ASSERT_TRUE( doc.is<Array>( "users" ) );

        auto usersOpt = doc.get<Array>( "users" );
        ASSERT_TRUE( usersOpt.has_value() );
        EXPECT_EQ( usersOpt->size(), 1 );

        // Verify the values were set
        EXPECT_EQ( doc.get<std::string>( "users[0].name" ).value_or( "" ), "Alice" );
        EXPECT_EQ( doc.get<int64_t>( "users[0].age" ).value_or( 0 ), 30 );
    }

    TEST( DocumentTest, BracketNotation_AutoCreateMultipleElements )
    {
        Document doc{};

        // Should create array with 3 elements automatically
        doc.set<std::string>( "items[0]", "first" );
        doc.set<std::string>( "items[1]", "second" );
        doc.set<std::string>( "items[2]", "third" );

        auto itemsOpt = doc.get<Array>( "items" );
        ASSERT_TRUE( itemsOpt.has_value() );
        EXPECT_EQ( itemsOpt->size(), 3 );

        EXPECT_EQ( doc.get<std::string>( "items[0]" ).value_or( "" ), "first" );
        EXPECT_EQ( doc.get<std::string>( "items[1]" ).value_or( "" ), "second" );
        EXPECT_EQ( doc.get<std::string>( "items[2]" ).value_or( "" ), "third" );
    }

    TEST( DocumentTest, BracketNotation_FillGapsWithNull )
    {
        Document doc{};

        // Setting index 2 should create indices 0, 1 (as null) and 2
        doc.set<std::string>( "sparse[2]", "value" );

        auto sparseOpt = doc.get<Array>( "sparse" );
        ASSERT_TRUE( sparseOpt.has_value() );
        EXPECT_EQ( sparseOpt->size(), 3 ); // indices 0, 1, 2

        EXPECT_EQ( ( *sparseOpt )[0].type(), Type::Null );
        EXPECT_EQ( ( *sparseOpt )[1].type(), Type::Null );
        EXPECT_EQ( doc.get<std::string>( "sparse[2]" ).value_or( "" ), "value" );
    }

    TEST( DocumentTest, BracketNotation_NestedArrays )
    {
        Document doc{};

        // Should create 2D array (matrix)
        doc.set<int64_t>( "matrix[0][0]", 1 );
        doc.set<int64_t>( "matrix[0][1]", 2 );
        doc.set<int64_t>( "matrix[1][0]", 3 );
        doc.set<int64_t>( "matrix[1][1]", 4 );

        // Verify structure
        ASSERT_TRUE( doc.is<Array>( "matrix" ) );
        auto matrixOpt = doc.get<Array>( "matrix" );
        ASSERT_TRUE( matrixOpt.has_value() );
        EXPECT_EQ( matrixOpt->size(), 2 );

        // Verify first row is an array
        EXPECT_EQ( ( *matrixOpt )[0].type(), Type::Array );
        auto row0Opt = ( *matrixOpt )[0].get<Array>( "" );
        ASSERT_TRUE( row0Opt.has_value() );
        EXPECT_EQ( row0Opt->size(), 2 );

        // Verify values using bracket notation
        EXPECT_EQ( doc.get<int64_t>( "matrix[0][0]" ).value_or( 0 ), 1 );
        EXPECT_EQ( doc.get<int64_t>( "matrix[0][1]" ).value_or( 0 ), 2 );
        EXPECT_EQ( doc.get<int64_t>( "matrix[1][0]" ).value_or( 0 ), 3 );
        EXPECT_EQ( doc.get<int64_t>( "matrix[1][1]" ).value_or( 0 ), 4 );
    }

    TEST( DocumentTest, BracketNotation_3DArrays )
    {
        Document doc{};

        // Create 3D array
        doc.set<std::string>( "cube[0][0][0]", "corner" );
        doc.set<std::string>( "cube[1][2][3]", "deep" );

        EXPECT_EQ( doc.get<std::string>( "cube[0][0][0]" ).value_or( "" ), "corner" );
        EXPECT_EQ( doc.get<std::string>( "cube[1][2][3]" ).value_or( "" ), "deep" );
    }

    TEST( DocumentTest, BracketNotation_NestedArraysWithObjects )
    {
        Document doc{};

        // Array of arrays of objects
        doc.set<std::string>( "grid[0][0].id", "A1" );
        doc.set<int64_t>( "grid[0][0].value", 100 );
        doc.set<std::string>( "grid[0][1].id", "A2" );
        doc.set<int64_t>( "grid[0][1].value", 200 );
        doc.set<std::string>( "grid[1][0].id", "B1" );
        doc.set<int64_t>( "grid[1][0].value", 300 );

        EXPECT_EQ( doc.get<std::string>( "grid[0][0].id" ).value_or( "" ), "A1" );
        EXPECT_EQ( doc.get<int64_t>( "grid[0][0].value" ).value_or( 0 ), 100 );
        EXPECT_EQ( doc.get<std::string>( "grid[1][0].id" ).value_or( "" ), "B1" );
        EXPECT_EQ( doc.get<int64_t>( "grid[1][0].value" ).value_or( 0 ), 300 );
    }

    TEST( DocumentTest, DotNotation_NumericIndicesReadAccess )
    {
        Document doc{};

        // Create array using JSON Pointer
        doc.set<Array>( "items" );
        doc.set<std::string>( "/items/0", "first" );
        doc.set<std::string>( "/items/1", "second" );
        doc.set<std::string>( "/items/2", "third" );

        // All three syntaxes should work for the same value
        EXPECT_EQ( doc.get<std::string>( "/items/0" ).value_or( "" ), "first" ); // JSON Pointer
        EXPECT_EQ( doc.get<std::string>( "items[0]" ).value_or( "" ), "first" ); // Bracket
        EXPECT_EQ( doc.get<std::string>( "items.0" ).value_or( "" ), "first" );  // Dot notation
    }

    TEST( DocumentTest, DotNotation_NumericIndicesInNestedPaths )
    {
        Document doc{};

        // Create nested structure
        doc.set<Array>( "users" );
        doc.set<Object>( "/users/0" );
        doc.set<std::string>( "/users/0/name", "Alice" );
        doc.set<int64_t>( "/users/0/age", 30 );

        // All syntaxes should be equivalent
        EXPECT_EQ( doc.get<std::string>( "/users/0/name" ).value_or( "" ), "Alice" );
        EXPECT_EQ( doc.get<std::string>( "users[0].name" ).value_or( "" ), "Alice" );
        EXPECT_EQ( doc.get<std::string>( "users.0.name" ).value_or( "" ), "Alice" );
    }

    TEST( DocumentTest, DotNotation_AutoCreateWithNumericIndices )
    {
        Document doc{};

        // Should create array with numeric index in dot notation
        doc.set<std::string>( "items.0", "first" );
        doc.set<std::string>( "items.1", "second" );

        ASSERT_TRUE( doc.is<Array>( "items" ) );
        auto itemsOpt = doc.get<Array>( "items" );
        ASSERT_TRUE( itemsOpt.has_value() );
        EXPECT_EQ( itemsOpt->size(), 2 );

        EXPECT_EQ( doc.get<std::string>( "items.0" ).value_or( "" ), "first" );
        EXPECT_EQ( doc.get<std::string>( "items.1" ).value_or( "" ), "second" );
    }

    TEST( DocumentTest, DotNotation_MixedNumericAndStringKeys )
    {
        Document doc{};

        // Should distinguish between array index and object key
        doc.set<Array>( "data" );
        doc.set<std::string>( "data.0", "index0" );    // Array element at index 0
        doc.set<Object>( "data.1" );                   // Array element at index 1 (object)
        doc.set<std::string>( "data.1.key", "value" ); // Object field

        EXPECT_EQ( doc.get<std::string>( "data.0" ).value_or( "" ), "index0" );
        EXPECT_EQ( doc.get<std::string>( "data.1.key" ).value_or( "" ), "value" );
    }

    TEST( DocumentTest, PathSyntax_AllEquivalent )
    {
        Document doc{};

        // Set using bracket notation
        doc.set<std::string>( "users[0].profile.name", "Alice" );

        // All syntaxes should read the same value
        EXPECT_EQ( doc.get<std::string>( "users[0].profile.name" ).value_or( "" ), "Alice" );
        EXPECT_EQ( doc.get<std::string>( "users.0.profile.name" ).value_or( "" ), "Alice" );
        EXPECT_EQ( doc.get<std::string>( "/users/0/profile/name" ).value_or( "" ), "Alice" );
    }

    TEST( DocumentTest, BracketNotation_EmptyBracketsRejected )
    {
        Document doc{};

        // Empty brackets should not work
        doc.set<std::string>( "items[]", "invalid" );

        // Should not create anything
        EXPECT_FALSE( doc.contains( "items" ) );
    }

    TEST( DocumentTest, BracketNotation_NonNumericIndexRejected )
    {
        Document doc{};

        // Non-numeric index should fail
        doc.set<std::string>( "items[abc]", "invalid" );

        // Should not create array
        EXPECT_FALSE( doc.contains( "items" ) );
    }
} // namespace nfx::json::test
