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
 * @file Sample_JsonFileIO.cpp
 * @brief Demonstrates JSON file loading and saving patterns
 * @details Real-world examples showing configuration file management, data persistence,
 *          error handling strategies, and UTF-8 BOM handling
 */

#include <nfx/Json.h>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

using namespace nfx::json;
namespace fs = std::filesystem;

// Helper function to load JSON from file
std::optional<Document> loadJsonFile( const std::string& filepath )
{
    try
    {
        // Check if file exists
        if( !fs::exists( filepath ) )
        {
            std::cerr << "File not found: " << filepath << "\n";
            return std::nullopt;
        }

        // Open file in binary mode to handle UTF-8 BOM correctly
        std::ifstream file( filepath, std::ios::binary );
        if( !file.is_open() )
        {
            std::cerr << "Failed to open file: " << filepath << "\n";
            return std::nullopt;
        }

        // Read entire file into string
        std::string content( ( std::istreambuf_iterator<char>( file ) ), std::istreambuf_iterator<char>() );
        file.close();

        if( content.empty() )
        {
            std::cerr << "File is empty: " << filepath << "\n";
            return std::nullopt;
        }

        // Parse JSON (automatically handles UTF-8 BOM)
        auto doc = Document::fromString( content );
        if( !doc.has_value() )
        {
            std::cerr << "Failed to parse JSON from file: " << filepath << "\n";
            return std::nullopt;
        }

        return doc;
    }
    catch( const std::exception& e )
    {
        std::cerr << "Exception while loading file: " << e.what() << "\n";
        return std::nullopt;
    }
}

// Helper function to save JSON to file
bool saveJsonFile( const std::string& filepath, const Document& doc, int indent = 2 )
{
    try
    {
        // Create parent directories if they don't exist
        fs::path path( filepath );
        if( path.has_parent_path() )
        {
            fs::create_directories( path.parent_path() );
        }

        // Open file for writing
        std::ofstream file( filepath );
        if( !file.is_open() )
        {
            std::cerr << "Failed to create file: " << filepath << "\n";
            return false;
        }

        // Serialize and write JSON
        std::string jsonStr = doc.toString( indent );
        file << jsonStr;

        if( !file.good() )
        {
            std::cerr << "Error writing to file: " << filepath << "\n";
            return false;
        }

        file.close();
        return true;
    }
    catch( const std::exception& e )
    {
        std::cerr << "Exception while saving file: " << e.what() << "\n";
        return false;
    }
}

int main()
{
    std::cout << "=== nfx-json File I/O Operations ===\n\n";

    // Create temporary directory for samples
    std::string tempDir = "json_samples_temp";

    try
    {
        //=====================================================================
        // 1. Basic Configuration File Save and Load
        //=====================================================================
        {
            std::cout << "1. Basic Configuration File Save and Load\n";
            std::cout << "------------------------------------------\n";

            // Create application configuration
            Document config{};
            config.set<std::string>( "app.name", "MyApplication" );
            config.set<std::string>( "app.version", "1.2.3" );
            config.set<std::string>( "app.environment", "production" );

            config.set<std::string>( "database.host", "db.example.com" );
            config.set<int64_t>( "database.port", 5432 );
            config.set<std::string>( "database.name", "myapp_db" );
            config.set<int64_t>( "database.maxConnections", 100 );

            config.set<bool>( "features.enableCache", true );
            config.set<bool>( "features.enableMetrics", true );
            config.set<std::string>( "features.logLevel", "INFO" );

            std::cout << "Created configuration:\n";
            std::cout << config.toString( 2 ) << "\n\n";

            // Save to file
            std::string configPath = tempDir + "/config.json";
            std::cout << "Saving to: " << configPath << "\n";

            if( saveJsonFile( configPath, config ) )
            {
                std::cout << "[OK] Configuration saved successfully\n\n";

                // Load it back
                std::cout << "Loading from: " << configPath << "\n";
                auto loadedConfig = loadJsonFile( configPath );

                if( loadedConfig.has_value() )
                {
                    std::cout << "[OK] Configuration loaded successfully\n";
                    std::cout << "App Name: " << loadedConfig->get<std::string>( "app.name" ).value_or( "Unknown" )
                              << "\n";
                    std::cout << "DB Host: " << loadedConfig->get<std::string>( "database.host" ).value_or( "Unknown" )
                              << "\n";
                    std::cout << "Cache Enabled: "
                              << ( loadedConfig->get<bool>( "features.enableCache" ).value_or( false ) ? "Yes" : "No" )
                              << "\n";
                }
                else
                {
                    std::cerr << "[FAIL] Failed to load configuration\n";
                }
            }
            else
            {
                std::cerr << "[FAIL] Failed to save configuration\n";
            }
        }

        std::cout << "\n";

        //=====================================================================
        // 2. User Data Persistence
        //=====================================================================
        {
            std::cout << "2. User Data Persistence\n";
            std::cout << "------------------------\n";

            // Create user profile
            Document userProfile{};
            userProfile.set<std::string>( "user.id", "usr_12345" );
            userProfile.set<std::string>( "user.email", "alice@example.com" );
            userProfile.set<std::string>( "user.displayName", "Alice Johnson" );
            userProfile.set<bool>( "user.verified", true );

            userProfile.set<std::string>( "profile.firstName", "Alice" );
            userProfile.set<std::string>( "profile.lastName", "Johnson" );
            userProfile.set<int64_t>( "profile.age", 28 );
            userProfile.set<std::string>( "profile.city", "Seattle" );

            userProfile.set<std::string>( "preferences.theme", "dark" );
            userProfile.set<std::string>( "preferences.language", "en-US" );
            userProfile.set<bool>( "preferences.notifications", true );

            // Save user profile
            std::string userPath = tempDir + "/users/user_12345.json";
            std::cout << "Saving user profile to: " << userPath << "\n";

            if( saveJsonFile( userPath, userProfile ) )
            {
                std::cout << "[OK] User profile saved\n";
                std::cout << "File size: " << fs::file_size( userPath ) << " bytes\n";
            }
        }

        std::cout << "\n";

        //=====================================================================
        // 3. Multiple File Management
        //=====================================================================
        {
            std::cout << "3. Multiple File Management\n";
            std::cout << "---------------------------\n";

            // Create multiple product documents
            std::vector<std::pair<std::string, Document>> products;

            for( int i = 1; i <= 3; ++i )
            {
                Document product{};
                product.set<std::string>( "id", "PROD-" + std::to_string( 1000 + i ) );
                product.set<std::string>( "name", "Product " + std::to_string( i ) );
                product.set<double>( "price", 99.99 * i );
                product.set<int64_t>( "stock", 100 - ( i * 10 ) );
                product.set<std::string>( "category", i % 2 == 0 ? "Electronics" : "Accessories" );

                std::string filename = tempDir + "/products/product_" + std::to_string( 1000 + i ) + ".json";
                products.push_back( { filename, product } );
            }

            std::cout << "Saving " << products.size() << " product files...\n";

            int savedCount = 0;
            for( const auto& [filepath, doc] : products )
            {
                if( saveJsonFile( filepath, doc ) )
                {
                    ++savedCount;
                    std::cout << "  [OK] " << fs::path( filepath ).filename() << "\n";
                }
            }

            std::cout << "Saved " << savedCount << " of " << products.size() << " products\n\n";

            // Load all products back
            std::cout << "Loading products from directory...\n";

            std::string productsDir = tempDir + "/products";
            if( fs::exists( productsDir ) && fs::is_directory( productsDir ) )
            {
                int loadedCount = 0;
                for( const auto& entry : fs::directory_iterator( productsDir ) )
                {
                    if( entry.path().extension() == ".json" )
                    {
                        auto product = loadJsonFile( entry.path().string() );
                        if( product.has_value() )
                        {
                            ++loadedCount;
                            std::cout << "  [OK] " << entry.path().filename() << " - "
                                      << product->get<std::string>( "name" ).value_or( "Unknown" ) << "\n";
                        }
                    }
                }
                std::cout << "Loaded " << loadedCount << " products\n";
            }
        }

        std::cout << "\n";

        //=====================================================================
        // 4. Error Handling Patterns
        //=====================================================================
        {
            std::cout << "4. Error Handling Patterns\n";
            std::cout << "--------------------------\n";

            // Test 1: Non-existent file
            std::cout << "Test 1: Loading non-existent file\n";
            auto result1 = loadJsonFile( tempDir + "/nonexistent.json" );
            std::cout << "Result: " << ( result1.has_value() ? "Success (unexpected)" : "Failed (expected)" ) << "\n\n";

            // Test 2: Invalid JSON content
            std::cout << "Test 2: Loading invalid JSON\n";
            std::string invalidPath = tempDir + "/invalid.json";
            {
                std::ofstream file( invalidPath );
                file << "{ invalid json content }";
            }
            auto result2 = loadJsonFile( invalidPath );
            std::cout << "Result: " << ( result2.has_value() ? "Success (unexpected)" : "Failed (expected)" ) << "\n\n";

            // Test 3: Empty file
            std::cout << "Test 3: Loading empty file\n";
            std::string emptyPath = tempDir + "/empty.json";
            {
                std::ofstream file( emptyPath );
                // Write nothing
            }
            auto result3 = loadJsonFile( emptyPath );
            std::cout << "Result: " << ( result3.has_value() ? "Success (unexpected)" : "Failed (expected)" ) << "\n\n";

            // Test 4: Valid JSON
            std::cout << "Test 4: Loading valid JSON\n";
            Document validDoc{};
            validDoc.set<std::string>( "status", "ok" );
            validDoc.set<int64_t>( "code", 200 );

            std::string validPath = tempDir + "/valid.json";
            saveJsonFile( validPath, validDoc );

            auto result4 = loadJsonFile( validPath );
            std::cout << "Result: " << ( result4.has_value() ? "Success (expected)" : "Failed (unexpected)" ) << "\n";
            if( result4.has_value() )
            {
                std::cout << "Loaded: " << result4->get<std::string>( "status" ).value_or( "N/A" ) << "\n";
            }
        }

        std::cout << "\n";

        //=====================================================================
        // 5. Configuration with Formatting Options
        //=====================================================================
        {
            std::cout << "5. Configuration with Formatting Options\n";
            std::cout << "-----------------------------------------\n";

            Document formatTest{};
            formatTest.set<std::string>( "nested.deeply.very.much.key", "value" );
            formatTest.set<int64_t>( "numbers/0", 1 );
            formatTest.set<int64_t>( "numbers/1", 2 );
            formatTest.set<int64_t>( "numbers/2", 3 );

            // Compact format (no indentation)
            std::string compactPath = tempDir + "/compact.json";
            {
                std::ofstream file( compactPath );
                file << formatTest.toString( 0 ); // 0 = no indentation
            }
            std::cout << "Compact format: " << fs::file_size( compactPath ) << " bytes\n";

            // 2-space indentation
            std::string indent2Path = tempDir + "/indent2.json";
            saveJsonFile( indent2Path, formatTest, 2 );
            std::cout << "2-space indent: " << fs::file_size( indent2Path ) << " bytes\n";

            // 4-space indentation
            std::string indent4Path = tempDir + "/indent4.json";
            saveJsonFile( indent4Path, formatTest, 4 );
            std::cout << "4-space indent: " << fs::file_size( indent4Path ) << " bytes\n";

            std::cout << "\nContent with 2-space indent:\n";
            auto loaded = loadJsonFile( indent2Path );
            if( loaded.has_value() )
            {
                std::cout << loaded->toString( 2 ) << "\n";
            }
        }

        std::cout << "\n";

        //=====================================================================
        // 6. Atomic File Updates (Safe Save Pattern)
        //=====================================================================
        {
            std::cout << "6. Atomic File Updates (Safe Save Pattern)\n";
            std::cout << "------------------------------------------\n";

            std::string targetPath = tempDir + "/atomic_config.json";
            std::string tempPath = targetPath + ".tmp";

            // Original configuration
            Document originalConfig{};
            originalConfig.set<std::string>( "version", "1.0.0" );
            originalConfig.set<int64_t>( "setting", 100 );

            saveJsonFile( targetPath, originalConfig );
            std::cout << "Original config saved\n";

            // Update configuration using atomic pattern
            std::cout << "Performing atomic update...\n";

            Document updatedConfig{};
            updatedConfig.set<std::string>( "version", "1.0.1" );
            updatedConfig.set<int64_t>( "setting", 200 );
            updatedConfig.set<std::string>( "newField", "added" );

            // Step 1: Write to temporary file
            if( saveJsonFile( tempPath, updatedConfig ) )
            {
                std::cout << "  [OK] Wrote to temporary file\n";

                // Step 2: Verify temporary file is valid
                auto verification = loadJsonFile( tempPath );
                if( verification.has_value() )
                {
                    std::cout << "  [OK] Verified temporary file\n";

                    // Step 3: Atomically replace original file
                    try
                    {
                        fs::rename( tempPath, targetPath );
                        std::cout << "  [OK] Atomically replaced original file\n";
                    }
                    catch( const std::exception& e )
                    {
                        std::cerr << "  [FAIL] Failed to rename: " << e.what() << "\n";
                    }
                }
                else
                {
                    std::cerr << "  [FAIL] Temporary file verification failed\n";
                    fs::remove( tempPath );
                }
            }

            // Verify final result
            auto finalConfig = loadJsonFile( targetPath );
            if( finalConfig.has_value() )
            {
                std::cout << "\nFinal configuration:\n";
                std::cout << "Version: " << finalConfig->get<std::string>( "version" ).value_or( "N/A" ) << "\n";
                std::cout << "Setting: " << finalConfig->get<int64_t>( "setting" ).value_or( 0 ) << "\n";
                std::cout << "New Field: " << finalConfig->get<std::string>( "newField" ).value_or( "N/A" ) << "\n";
            }
        }

        std::cout << "\n";

        //=====================================================================
        // Cleanup
        //=====================================================================
        std::cout << "Cleaning up temporary files...\n";
        try
        {
            fs::remove_all( tempDir );
            std::cout << "[OK] Temporary directory removed\n";
        }
        catch( const std::exception& e )
        {
            std::cerr << "Warning: Failed to cleanup: " << e.what() << "\n";
        }
    }
    catch( const std::exception& e )
    {
        std::cerr << "Error: " << e.what() << "\n";

        // Attempt cleanup on error
        try
        {
            if( fs::exists( tempDir ) )
            {
                fs::remove_all( tempDir );
            }
        }
        catch( ... )
        {
            // Ignore cleanup errors
        }

        return 1;
    }

    return 0;
}
