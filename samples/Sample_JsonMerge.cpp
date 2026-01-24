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
 * @file Sample_JsonMerge.cpp
 * @brief Demonstrates Document merging and update operations
 * @details Real-world examples showing configuration overlays, partial updates,
 *          API response composition, and incremental document construction
 */

#include <nfx/Json.h>

#include <iostream>
#include <string>

using namespace nfx::json;

int main()
{
    std::cout << "=== nfx-json Document Merge and Update Operations ===\n\n";

    try
    {
        //=====================================================================
        // 1. Configuration File Overlay Pattern
        //=====================================================================
        {
            std::cout << "1. Configuration File Overlay Pattern\n";
            std::cout << "--------------------------------------\n";

            // Base configuration (defaults)
            Document baseConfig{};
            baseConfig.set<std::string>( "app.name", "MyApplication" );
            baseConfig.set<std::string>( "app.version", "1.0.0" );
            baseConfig.set<std::string>( "app.environment", "development" );

            baseConfig.set<std::string>( "database.host", "localhost" );
            baseConfig.set<int64_t>( "database.port", 5432 );
            baseConfig.set<std::string>( "database.name", "myapp_dev" );
            baseConfig.set<int64_t>( "database.poolSize", 10 );

            baseConfig.set<std::string>( "logging.level", "DEBUG" );
            baseConfig.set<bool>( "logging.colorOutput", true );

            baseConfig.set<bool>( "features.enableCache", false );
            baseConfig.set<bool>( "features.enableMetrics", false );

            std::cout << "Base Configuration:\n";
            std::cout << baseConfig.toString( 2 ) << "\n\n";

            // Production overrides
            Document prodOverrides{};
            prodOverrides.set<std::string>( "app.environment", "production" );
            prodOverrides.set<std::string>( "database.host", "prod-db.example.com" );
            prodOverrides.set<std::string>( "database.name", "myapp_prod" );
            prodOverrides.set<int64_t>( "database.poolSize", 50 );
            prodOverrides.set<std::string>( "logging.level", "INFO" );
            prodOverrides.set<bool>( "logging.colorOutput", false );
            prodOverrides.set<bool>( "features.enableCache", true );
            prodOverrides.set<bool>( "features.enableMetrics", true );

            std::cout << "Production Overrides:\n";
            std::cout << prodOverrides.toString( 2 ) << "\n\n";

            // Merge production overrides into base
            baseConfig.merge( prodOverrides );

            std::cout << "Merged Production Configuration:\n";
            std::cout << baseConfig.toString( 2 ) << "\n\n";

            std::cout << "Verification:\n";
            std::cout << "Environment: " << baseConfig.get<std::string>( "app.environment" ).value_or( "Unknown" )
                      << "\n";
            std::cout << "Database Host: " << baseConfig.get<std::string>( "database.host" ).value_or( "Unknown" )
                      << "\n";
            std::cout << "Pool Size: " << baseConfig.get<int64_t>( "database.poolSize" ).value_or( 0 ) << "\n";
            std::cout << "Cache Enabled: "
                      << ( baseConfig.get<bool>( "features.enableCache" ).value_or( false ) ? "Yes" : "No" ) << "\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 2. API Response Composition
        //=====================================================================
        {
            std::cout << "2. API Response Composition\n";
            std::cout << "---------------------------\n";

            // User basic info from one API endpoint
            Document userBasic{};
            userBasic.set<std::string>( "user.id", "usr_12345" );
            userBasic.set<std::string>( "user.email", "alice@example.com" );
            userBasic.set<std::string>( "user.displayName", "Alice Johnson" );
            userBasic.set<bool>( "user.verified", true );

            std::cout << "User Basic Info:\n";
            std::cout << userBasic.toString( 2 ) << "\n\n";

            // User profile details from another endpoint
            Document userProfile{};
            userProfile.set<std::string>( "user.profile.firstName", "Alice" );
            userProfile.set<std::string>( "user.profile.lastName", "Johnson" );
            userProfile.set<int64_t>( "user.profile.age", 28 );
            userProfile.set<std::string>( "user.profile.city", "Seattle" );
            userProfile.set<std::string>( "user.profile.country", "USA" );

            std::cout << "User Profile Details:\n";
            std::cout << userProfile.toString( 2 ) << "\n\n";

            // User preferences from yet another endpoint
            Document userPrefs{};
            userPrefs.set<std::string>( "user.preferences.theme", "dark" );
            userPrefs.set<std::string>( "user.preferences.language", "en-US" );
            userPrefs.set<bool>( "user.preferences.notifications", true );

            std::cout << "User Preferences:\n";
            std::cout << userPrefs.toString( 2 ) << "\n\n";

            // Compose complete user object
            Document completeUser{};
            completeUser.merge( userBasic );
            completeUser.merge( userProfile );
            completeUser.merge( userPrefs );

            std::cout << "Complete User Profile (Composed):\n";
            std::cout << completeUser.toString( 2 ) << "\n\n";

            std::cout << "Complete profile has all data:\n";
            std::cout << "Email: " << completeUser.get<std::string>( "user.email" ).value_or( "N/A" ) << "\n";
            std::cout << "Name: " << completeUser.get<std::string>( "user.profile.firstName" ).value_or( "N/A" ) << " "
                      << completeUser.get<std::string>( "user.profile.lastName" ).value_or( "N/A" ) << "\n";
            std::cout << "Theme: " << completeUser.get<std::string>( "user.preferences.theme" ).value_or( "N/A" )
                      << "\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 3. Partial Document Updates
        //=====================================================================
        {
            std::cout << "3. Partial Document Updates\n";
            std::cout << "----------------------------\n";

            // Original document
            Document inventory{};
            inventory.set<std::string>( "products.laptop.name", "Pro Laptop" );
            inventory.set<double>( "products.laptop.price", 1299.99 );
            inventory.set<int64_t>( "products.laptop.stock", 50 );
            inventory.set<std::string>( "products.laptop.category", "Electronics" );

            inventory.set<std::string>( "products.mouse.name", "Wireless Mouse" );
            inventory.set<double>( "products.mouse.price", 29.99 );
            inventory.set<int64_t>( "products.mouse.stock", 200 );
            inventory.set<std::string>( "products.mouse.category", "Accessories" );

            std::cout << "Original Inventory:\n";
            std::cout << inventory.toString( 2 ) << "\n\n";

            // Simulate sale - update stock levels
            std::cout << "Processing sales...\n";
            inventory.update( "products.laptop.stock", Document{ static_cast<int64_t>( 45 ) } );
            inventory.update( "products.mouse.stock", Document{ static_cast<int64_t>( 195 ) } );

            // Price adjustment
            inventory.update( "products.laptop.price", Document{ 1249.99 } );

            std::cout << "\nInventory After Sales and Price Update:\n";
            std::cout << inventory.toString( 2 ) << "\n\n";

            std::cout << "Updated values:\n";
            std::cout << "Laptop stock: " << inventory.get<int64_t>( "products.laptop.stock" ).value_or( 0 ) << "\n";
            std::cout << "Mouse stock: " << inventory.get<int64_t>( "products.mouse.stock" ).value_or( 0 ) << "\n";
            std::cout << "Laptop price: $" << inventory.get<double>( "products.laptop.price" ).value_or( 0.0 ) << "\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 4. Array Merging Strategies
        //=====================================================================
        {
            std::cout << "4. Array Merging Strategies\n";
            std::cout << "----------------------------\n";

            // Document with arrays
            Document baseDoc{};
            baseDoc.set<std::string>( "permissions.roles/0", "user" );
            baseDoc.set<std::string>( "permissions.roles/1", "viewer" );

            baseDoc.set<std::string>( "/permissions/resources/0", "posts" );
            baseDoc.set<std::string>( "/permissions/resources/1", "comments" );

            std::cout << "Base Document:\n";
            std::cout << baseDoc.toString( 2 ) << "\n\n";

            // Additional permissions to merge
            Document additionalPerms{};
            additionalPerms.set<std::string>( "permissions.roles/0", "admin" );
            additionalPerms.set<std::string>( "permissions.roles/1", "moderator" );

            additionalPerms.set<std::string>( "/permissions/resources/0", "users" );
            additionalPerms.set<std::string>( "/permissions/resources/1", "settings" );

            std::cout << "Additional Permissions:\n";
            std::cout << additionalPerms.toString( 2 ) << "\n\n";

            // Strategy 1: Overwrite arrays (default behavior)
            Document overwriteDoc = baseDoc;
            overwriteDoc.merge( additionalPerms, true );

            std::cout << "Strategy 1 - Overwrite Arrays (overwriteArrays = true):\n";
            std::cout << overwriteDoc.toString( 2 ) << "\n";
            std::cout << "Roles array size: "
                      << overwriteDoc.get<Array>( "permissions.roles" ).value_or( Array{} ).size() << "\n\n";

            // Strategy 2: Merge arrays (append)
            Document mergeDoc = baseDoc;
            mergeDoc.merge( additionalPerms, false );

            std::cout << "Strategy 2 - Merge Arrays (overwriteArrays = false):\n";
            std::cout << mergeDoc.toString( 2 ) << "\n";
            std::cout << "Roles array size: " << mergeDoc.get<Array>( "permissions.roles" ).value_or( Array{} ).size()
                      << "\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 5. Incremental Document Construction
        //=====================================================================
        {
            std::cout << "5. Incremental Document Construction\n";
            std::cout << "-------------------------------------\n";

            // Build document incrementally from multiple sources
            Document report{};

            // Step 1: Add header
            std::cout << "Step 1: Adding report header...\n";
            Document header{};
            header.set<std::string>( "report.id", "RPT-2025-Q1" );
            header.set<std::string>( "report.title", "Quarterly Sales Report" );
            header.set<std::string>( "report.generated", "2025-10-03T10:00:00Z" );
            report.merge( header );

            // Step 2: Add summary statistics
            std::cout << "Step 2: Adding summary statistics...\n";
            Document summary{};
            summary.set<double>( "report.summary.totalRevenue", 1250000.00 );
            summary.set<int64_t>( "report.summary.totalOrders", 5420 );
            summary.set<double>( "report.summary.averageOrderValue", 230.63 );
            report.merge( summary );

            // Step 3: Add regional data
            std::cout << "Step 3: Adding regional breakdown...\n";
            Document regions{};
            regions.set<double>( "report.regions.north.revenue", 450000.00 );
            regions.set<int64_t>( "report.regions.north.orders", 1950 );
            regions.set<double>( "report.regions.south.revenue", 380000.00 );
            regions.set<int64_t>( "report.regions.south.orders", 1640 );
            regions.set<double>( "report.regions.east.revenue", 250000.00 );
            regions.set<int64_t>( "report.regions.east.orders", 1085 );
            regions.set<double>( "report.regions.west.revenue", 170000.00 );
            regions.set<int64_t>( "report.regions.west.orders", 745 );
            report.merge( regions );

            // Step 4: Add metadata
            std::cout << "Step 4: Adding metadata...\n";
            Document metadata{};
            metadata.set<std::string>( "report.metadata.author", "Analytics System" );
            metadata.set<std::string>( "report.metadata.version", "2.1.0" );
            metadata.set<bool>( "report.metadata.verified", true );
            report.merge( metadata );

            std::cout << "\nComplete Report (Built Incrementally):\n";
            std::cout << report.toString( 2 ) << "\n\n";

            std::cout << "Report Statistics:\n";
            std::cout << "Total Revenue: $" << report.get<double>( "report.summary.totalRevenue" ).value_or( 0.0 )
                      << "\n";
            std::cout << "Total Orders: " << report.get<int64_t>( "report.summary.totalOrders" ).value_or( 0 ) << "\n";
            std::cout << "Top Region: North ($" << report.get<double>( "report.regions.north.revenue" ).value_or( 0.0 )
                      << ")\n";
        }

        std::cout << "\n";

        //=====================================================================
        // 6. Selective Field Override
        //=====================================================================
        {
            std::cout << "6. Selective Field Override\n";
            std::cout << "----------------------------\n";

            // User template (defaults for all new users)
            Document userTemplate{};
            userTemplate.set<bool>( "settings.notifications.email", true );
            userTemplate.set<bool>( "settings.notifications.sms", false );
            userTemplate.set<bool>( "settings.notifications.push", true );
            userTemplate.set<std::string>( "settings.privacy.profile", "public" );
            userTemplate.set<std::string>( "settings.privacy.email", "private" );
            userTemplate.set<std::string>( "settings.theme", "light" );
            userTemplate.set<std::string>( "settings.language", "en-US" );
            userTemplate.set<int64_t>( "settings.fontSize", 14 );

            std::cout << "User Template (Defaults):\n";
            std::cout << userTemplate.toString( 2 ) << "\n\n";

            // User customizations
            Document userCustom{};
            userCustom.set<bool>( "settings.notifications.email", false );
            userCustom.set<std::string>( "settings.theme", "dark" );
            userCustom.set<int64_t>( "settings.fontSize", 16 );

            std::cout << "User Customizations:\n";
            std::cout << userCustom.toString( 2 ) << "\n\n";

            // Apply customizations to template
            Document finalSettings = userTemplate;
            finalSettings.merge( userCustom );

            std::cout << "Final User Settings:\n";
            std::cout << finalSettings.toString( 2 ) << "\n\n";

            std::cout << "Results:\n";
            std::cout << "Email notifications: "
                      << ( finalSettings.get<bool>( "settings.notifications.email" ).value_or( false ) ? "Enabled"
                                                                                                       : "Disabled" )
                      << " (customized)\n";
            std::cout << "SMS notifications: "
                      << ( finalSettings.get<bool>( "settings.notifications.sms" ).value_or( false ) ? "Enabled"
                                                                                                     : "Disabled" )
                      << " (from template)\n";
            std::cout << "Theme: " << finalSettings.get<std::string>( "settings.theme" ).value_or( "N/A" )
                      << " (customized)\n";
            std::cout << "Font size: " << finalSettings.get<int64_t>( "settings.fontSize" ).value_or( 0 )
                      << " (customized)\n";
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
