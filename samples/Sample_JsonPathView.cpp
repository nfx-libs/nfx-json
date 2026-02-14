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
 * @file Sample_JsonPathView.cpp
 * @brief Demonstrates PathView for iterating over all paths in a JSON document
 * @details Shows path iteration in both JSON Pointer and dot notation formats,
 *          depth tracking, leaf filtering, and practical use cases like schema discovery
 */

#include <nfx/Json.h>

#include <iostream>
#include <string>
#include <vector>

int main()
{
    using namespace nfx::json;
    using PathView = Document::PathView;
    using Format = PathView::Format;

    std::cout << "=== PathView Sample ===\n\n";

    // Create test JSON document with nested structure
    std::string jsonStr = R"({
        "user": {
            "name": "Alice Johnson",
            "age": 28,
            "email": "alice@example.com",
            "address": {
                "street": "123 Main St",
                "city": "Seattle",
                "zip": "98101"
            }
        },
        "orders": [
            {
                "id": "ORD-001",
                "total": 125.50,
                "items": ["Widget", "Gadget"]
            },
            {
                "id": "ORD-002",
                "total": 89.99,
                "items": ["Sprocket"]
            }
        ],
        "preferences": {
            "theme": "dark",
            "notifications": true
        }
    })";

    auto doc = Document::fromString( jsonStr );
    if( !doc.has_value() )
    {
        std::cerr << "Failed to parse JSON\n";
        return 1;
    }

    //=====================================================================
    // 1. Iterate all paths with JSON Pointer format
    //=====================================================================
    {
        std::cout << "1. All paths (JSON Pointer format)\n";
        std::cout << "-----------------------------------\n";

        size_t count = 0;
        for( const auto& entry : PathView{ *doc, Format::JsonPointer } )
        {
            std::string indent( entry.depth * 2, ' ' );
            std::string type = entry.isLeaf ? "[leaf]" : "[container]";
            std::cout << indent << entry.path << " " << type << "\n";
            ++count;
        }
        std::cout << "\nTotal paths: " << count << "\n\n";
    }

    //=====================================================================
    // 2. Iterate all paths with dot notation format
    //=====================================================================
    {
        std::cout << "2. All paths (Dot notation format)\n";
        std::cout << "-----------------------------------\n";

        for( const auto& entry : PathView{ *doc, Format::DotNotation } )
        {
            std::string indent( entry.depth * 2, ' ' );
            std::cout << indent << entry.path << "\n";
        }
        std::cout << "\n";
    }

    //=====================================================================
    // 3. Iterate only leaf paths (primitives)
    //=====================================================================
    {
        std::cout << "3. Leaf paths only\n";
        std::cout << "------------------\n";

        size_t leafCount = 0;
        for( const auto& entry : PathView{ *doc, Format::JsonPointer } )
        {
            if( !entry.isLeaf )
                continue;

            ++leafCount;

            // Get string representation of value
            std::string valueStr = entry.value().toString();
            if( valueStr.length() > 30 )
            {
                valueStr = valueStr.substr( 0, 27 ) + "...";
            }
            std::cout << entry.path << " = " << valueStr << "\n";
        }
        std::cout << "\nTotal leaves: " << leafCount << "\n\n";
    }

    //=====================================================================
    // 4. Use iterator interface with range-for (filtering by depth)
    //=====================================================================
    {
        std::cout << "4. iterator interface (depth >= 2)\n";
        std::cout << "----------------------------------\n";

        for( const auto& entry : PathView{ *doc, Format::DotNotation } )
        {
            if( entry.depth >= 2 )
            {
                std::cout << entry.path << " (depth=" << entry.depth << ")\n";
            }
        }
        std::cout << "\n";
    }

    //=====================================================================
    // 5. Schema discovery - find all unique types
    //=====================================================================
    {
        std::cout << "5. Schema discovery (unique type paths)\n";
        std::cout << "---------------------------------------\n";

        for( const auto& entry : PathView{ *doc, Format::JsonPointer } )
        {
            if( !entry.isLeaf )
                continue;

            std::string type;

            if( auto val = entry.value().get<std::string>( "" ) )
            {
                type = "string";
            }
            else if( auto valInt = entry.value().get<int>( "" ) )
            {
                type = "integer";
            }
            else if( auto valDbl = entry.value().get<double>( "" ) )
            {
                type = "number";
            }
            else if( auto valBool = entry.value().get<bool>( "" ) )
            {
                type = "boolean";
            }
            else if( entry.value().isNull( "" ) )
            {
                type = "null";
            }
            else
            {
                type = "unknown";
            }

            std::cout << entry.path << ": " << type << "\n";
        }
        std::cout << "\n";
    }

    //=====================================================================
    // 6. Find specific patterns in paths
    //=====================================================================
    {
        std::cout << "6. Find all 'id' fields\n";
        std::cout << "-----------------------\n";

        for( const auto& entry : PathView{ *doc, Format::JsonPointer } )
        {
            // Check if path ends with "/id"
            if( entry.path.length() >= 3 && entry.path.substr( entry.path.length() - 3 ) == "/id" )
            {
                auto val = entry.value().get<std::string>( "" );
                if( val )
                {
                    std::cout << entry.path << " = " << *val << "\n";
                }
            }
        }
        std::cout << "\n";
    }

    //=====================================================================
    // 7. Count elements at each depth
    //=====================================================================
    {
        std::cout << "7. Path count by depth\n";
        std::cout << "----------------------\n";

        // First pass: find max depth
        size_t maxDepth = 0;
        std::vector<PathView::Entry> allPaths;
        for( const auto& entry : PathView{ *doc } )
        {
            allPaths.push_back( entry );
            if( entry.depth > maxDepth )
                maxDepth = entry.depth;
        }

        for( size_t d = 1; d <= maxDepth; ++d )
        {
            size_t count = 0;
            for( const auto& entry : allPaths )
            {
                if( entry.depth == d )
                    ++count;
            }
            std::cout << "Depth " << d << ": " << count << " paths\n";
        }
    }

    std::cout << "\n";
    return 0;
}
