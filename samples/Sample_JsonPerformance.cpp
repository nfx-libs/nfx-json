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
 * @file Sample_JsonPerformance.cpp
 * @brief Demonstrates zero-copy access and performance optimization patterns
 * @details Real-world examples showing getRef() for large containers, efficient iteration,
 *          memory-conscious document manipulation, and performance best practices
 */

#include <nfx/Json.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

using namespace nfx::json;

// Simple timing utility
class Timer
{
public:
    Timer( const std::string& name )
        : m_name{ name },
          m_start{ std::chrono::high_resolution_clock::now() }
    {
    }

    ~Timer()
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( end - m_start );
        std::cout << "  " << m_name << ": " << duration.count() << " μs\n";
    }

private:
    std::string m_name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
};

int main()
{
    std::cout << "=== nfx-json Performance Optimization Patterns ===\n\n";

    try
    {
        //=====================================================================
        // 1. Zero-Copy Access with getRef()
        //=====================================================================
        {
            std::cout << "1. Zero-Copy Access with getRef()\n";
            std::cout << "-----------------------------------\n";

            // Create document with large array
            Document doc{};
            const size_t arraySize = 1000;

            std::cout << "Creating array with " << arraySize << " elements...\n";
            for( size_t i = 0; i < arraySize; ++i )
            {
                doc.set<int64_t>( "/data/" + std::to_string( i ), static_cast<int64_t>( i ) );
            }

            std::cout << "\nComparison: get() vs getRef()\n";

            // Method 1: get() - Creates a copy
            {
                Timer timer( "get() with copy" );
                auto arrayCopy = doc.get<Array>( "/data" );
                if( arrayCopy.has_value() )
                {
                    size_t sum = 0;
                    for( const auto& elem : arrayCopy.value() )
                    {
                        sum += elem.get<int64_t>( "" ).value_or( 0 );
                    }
                    // Use sum to prevent optimization
                    if( sum > 0 )
                    {
                        // Prevent dead code elimination
                    }
                }
            }

            // Method 2: getRef() - Zero-copy reference
            {
                Timer timer( "getRef() zero-copy" );
                auto arrayRef = doc.getRef<Array>( "/data" );
                if( arrayRef.has_value() )
                {
                    size_t sum = 0;
                    for( const auto& elem : arrayRef->get() )
                    {
                        sum += elem.get<int64_t>( "" ).value_or( 0 );
                    }
                    // Use sum to prevent optimization
                    if( sum > 0 )
                    {
                        // Prevent dead code elimination
                    }
                }
            }

            std::cout << "\n[OK] getRef() avoids copying " << arraySize << " elements\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 2. Efficient Large Object Iteration
        //=====================================================================
        {
            std::cout << "2. Efficient Large Object Iteration\n";
            std::cout << "------------------------------------\n";

            // Create document with large object
            Document doc{};
            const size_t fieldCount = 500;

            std::cout << "Creating object with " << fieldCount << " fields...\n";
            for( size_t i = 0; i < fieldCount; ++i )
            {
                std::string key = "field_" + std::to_string( i );
                doc.set<std::string>( key, "value_" + std::to_string( i ) );
            }

            std::cout << "\nComparison: Copying vs Reference iteration\n";

            // Method 1: Copy the object
            {
                Timer timer( "Copy and iterate" );
                auto objCopy = doc.get<Object>( "" );
                if( objCopy.has_value() )
                {
                    size_t count = 0;
                    for( const auto& [key, value] : objCopy.value() )
                    {
                        ++count;
                    }
                }
            }

            // Method 2: Reference to object
            {
                Timer timer( "Reference iteration" );
                auto objRef = doc.getRef<Object>( "" );
                if( objRef.has_value() )
                {
                    size_t count = 0;
                    for( const auto& [key, value] : objRef->get() )
                    {
                        ++count;
                    }
                }
            }

            std::cout << "\n[OK] Reference iteration is more efficient for large objects\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 3. In-Place Modification with Mutable References
        //=====================================================================
        {
            std::cout << "3. In-Place Modification with Mutable References\n";
            std::cout << "-------------------------------------------------\n";

            Document doc{};

            // Create initial array
            for( int i = 0; i < 100; ++i )
            {
                doc.set<int64_t>( "/scores/" + std::to_string( i ), i );
            }

            std::cout << "Initial array size: " << doc.get<Array>( "/scores" ).value_or( Array{} ).size() << "\n";

            // Method 1: Modify using get/set (inefficient)
            std::cout << "\nMethod 1: Using get/set (creates copies)\n";
            {
                Timer timer( "get/set approach" );

                // Get copy, modify, set back
                auto scoresCopy = doc.get<Array>( "/scores" );
                if( scoresCopy.has_value() )
                {
                    Array& arr = scoresCopy.value();
                    // Modify array elements
                    for( size_t i = 0; i < arr.size(); ++i )
                    {
                        auto val = arr[i].get<int64_t>( "" ).value_or( 0 );
                        arr[i] = Document{ val * 2 };
                    }
                    doc.set<Array>( "/scores", std::move( arr ) );
                }
            }

            // Reset for fair comparison
            for( int i = 0; i < 100; ++i )
            {
                doc.set<int64_t>( "/scores/" + std::to_string( i ), i );
            }

            // Method 2: Direct modification using mutable reference
            std::cout << "Method 2: Using mutable reference (zero-copy)\n";
            {
                Timer timer( "Mutable reference" );

                auto scoresRef = doc.getRef<Array>( "/scores" );
                if( scoresRef.has_value() )
                {
                    Array& arr = scoresRef->get();
                    // Directly modify in-place
                    for( size_t i = 0; i < arr.size(); ++i )
                    {
                        auto val = arr[i].get<int64_t>( "" ).value_or( 0 );
                        arr[i] = Document{ val * 2 };
                    }
                }
            }

            std::cout << "\n[OK] Mutable references enable efficient in-place modification\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 4. Nested Object Access Patterns
        //=====================================================================
        {
            std::cout << "4. Nested Object Access Patterns\n";
            std::cout << "---------------------------------\n";

            // Create deeply nested structure
            Document doc{};
            for( int i = 0; i < 50; ++i )
            {
                std::string path = "users/" + std::to_string( i ) + "/profile";
                doc.set<std::string>( path + "/name", "User " + std::to_string( i ) );
                doc.set<int64_t>( path + "/age", 20 + i );
                doc.set<std::string>( path + "/city", "City " + std::to_string( i % 10 ) );
            }

            std::cout << "Created nested structure with 50 users\n\n";

            // Inefficient: Multiple path navigations
            std::cout << "Method 1: Multiple separate path accesses\n";
            {
                Timer timer( "Multiple get() calls" );

                for( int i = 0; i < 50; ++i )
                {
                    std::string basePath = "users/" + std::to_string( i ) + "/profile";
                    auto name = doc.get<std::string>( basePath + "/name" );
                    auto age = doc.get<int64_t>( basePath + "/age" );
                    auto city = doc.get<std::string>( basePath + "/city" );
                    // Use values
                    (void)name;
                    (void)age;
                    (void)city;
                }
            }

            // Efficient: Get reference to parent object once
            std::cout << "Method 2: Single reference to parent object\n";
            {
                Timer timer( "Single getRef() per user" );

                for( int i = 0; i < 50; ++i )
                {
                    std::string basePath = "users/" + std::to_string( i ) + "/profile";
                    auto profileRef = doc.getRef<Object>( basePath );
                    if( profileRef.has_value() )
                    {
                        const Object& profile = profileRef->get();
                        // Direct field access from object reference
                        for( const auto& [key, value] : profile )
                        {
                            (void)key;
                            (void)value;
                        }
                    }
                }
            }

            std::cout << "\n[OK] Accessing parent object once is more efficient than multiple path lookups\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 5. Memory-Efficient Document Construction
        //=====================================================================
        {
            std::cout << "5. Memory-Efficient Document Construction\n";
            std::cout << "------------------------------------------\n";

            const size_t recordCount = 200;

            // Method 1: Building with set() (multiple allocations)
            std::cout << "Method 1: Using set() for each field\n";
            {
                Timer timer( "Individual set() calls" );

                Document doc{};
                for( size_t i = 0; i < recordCount; ++i )
                {
                    std::string base = "records/" + std::to_string( i );
                    doc.set<std::string>( base + "/id", "REC-" + std::to_string( i ) );
                    doc.set<int64_t>( base + "/value", static_cast<int64_t>( i * 100 ) );
                    doc.set<bool>( base + "/active", i % 2 == 0 );
                }
            }

            // Method 2: Pre-creating arrays (more efficient)
            std::cout << "Method 2: Pre-creating array structure\n";
            {
                Timer timer( "Pre-create array" );

                Document doc{};
                doc.set<Array>( "records" );

                for( size_t i = 0; i < recordCount; ++i )
                {
                    std::string base = "records/" + std::to_string( i );
                    doc.set<std::string>( base + "/id", "REC-" + std::to_string( i ) );
                    doc.set<int64_t>( base + "/value", static_cast<int64_t>( i * 100 ) );
                    doc.set<bool>( base + "/active", i % 2 == 0 );
                }
            }

            std::cout << "\n[OK] Pre-creating container structures can improve performance\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 6. String Serialization Performance
        //=====================================================================
        {
            std::cout << "6. String Serialization Performance\n";
            std::cout << "------------------------------------\n";

            // Create moderately complex document
            Document doc{};
            for( int i = 0; i < 50; ++i )
            {
                doc.set<std::string>( "items/" + std::to_string( i ) + "/name", "Item " + std::to_string( i ) );
                doc.set<double>( "items/" + std::to_string( i ) + "/price", 19.99 * ( i + 1 ) );
                doc.set<bool>( "items/" + std::to_string( i ) + "/available", i % 3 != 0 );
            }

            std::cout << "Serialization with different indentation levels:\n\n";

            std::string compact, indent2, indent4;

            // Compact (no indentation)
            {
                Timer timer( "Compact (indent=0)" );
                compact = doc.toString( 0 );
            }
            std::cout << "    Size: " << compact.length() << " bytes\n\n";

            // 2-space indentation
            {
                Timer timer( "2-space indent" );
                indent2 = doc.toString( 2 );
            }
            std::cout << "    Size: " << indent2.length() << " bytes\n\n";

            // 4-space indentation
            {
                Timer timer( "4-space indent" );
                indent4 = doc.toString( 4 );
            }
            std::cout << "    Size: " << indent4.length() << " bytes\n\n";

            std::cout << "[OK] Compact format is fastest and smallest\n";
            std::cout << "[OK] Use compact for network transmission, indented for human-readability\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 7. Best Practices Summary
        //=====================================================================
        {
            std::cout << "7. Best Practices Summary\n";
            std::cout << "-------------------------\n\n";

            std::cout << "Performance Tips:\n";
            std::cout << "  [OK] Use getRef() instead of get() for large arrays/objects\n";
            std::cout << "  [OK] Use mutable references for in-place modifications\n";
            std::cout << "  [OK] Access parent objects once instead of repeated path lookups\n";
            std::cout << "  [OK] Pre-create container structures when building large documents\n";
            std::cout << "  [OK] Use compact serialization (indent=0) for network/storage\n";
            std::cout << "  [OK] Use indented serialization for debugging and logging\n\n";

            std::cout << "Memory Tips:\n";
            std::cout << "  [OK] References don't copy data - use them for read-only access\n";
            std::cout << "  [OK] Move semantics with set() to avoid copies\n";
            std::cout << "  [OK] Clear large documents when done: doc = Document{}\n\n";

            std::cout << "When to use get() vs getRef():\n";
            std::cout << "  - get(): Small values, need ownership, modifying copy\n";
            std::cout << "  - getRef(): Large containers, read-only iteration, in-place modification\n";
        }

        std::cout << "\n";
    }
    catch( const std::exception& e )
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
