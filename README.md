# nfx-json

<!-- Project Info -->

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=flat-square)](https://github.com/nfx-libs/nfx-json/blob/main/LICENSE.txt) [![GitHub release (latest by date)](https://img.shields.io/github/v/release/nfx-libs/nfx-json?style=flat-square)](https://github.com/nfx-libs/nfx-json/releases) [![GitHub tag (latest by date)](https://img.shields.io/github/tag/nfx-libs/nfx-json?style=flat-square)](https://github.com/nfx-libs/nfx-json/tags)<br/>

![C++20](https://img.shields.io/badge/C%2B%2B-20-blue?style=flat-square) ![CMake](https://img.shields.io/badge/CMake-3.20%2B-green.svg?style=flat-square) ![Cross Platform](https://img.shields.io/badge/Platform-Linux_Windows-lightgrey?style=flat-square)

<!-- CI/CD Status -->

[![Linux GCC](https://img.shields.io/github/actions/workflow/status/nfx-libs/nfx-json/build-linux-gcc.yml?branch=main&label=Linux%20GCC&style=flat-square)](https://github.com/nfx-libs/nfx-json/actions/workflows/build-linux-gcc.yml) [![Linux Clang](https://img.shields.io/github/actions/workflow/status/nfx-libs/nfx-json/build-linux-clang.yml?branch=main&label=Linux%20Clang&style=flat-square)](https://github.com/nfx-libs/nfx-json/actions/workflows/build-linux-clang.yml) [![Windows MinGW](https://img.shields.io/github/actions/workflow/status/nfx-libs/nfx-json/build-windows-mingw.yml?branch=main&label=Windows%20MinGW&style=flat-square)](https://github.com/nfx-libs/nfx-json/actions/workflows/build-windows-mingw.yml) [![Windows MSVC](https://img.shields.io/github/actions/workflow/status/nfx-libs/nfx-json/build-windows-msvc.yml?branch=main&label=Windows%20MSVC&style=flat-square)](https://github.com/nfx-libs/nfx-json/actions/workflows/build-windows-msvc.yml) [![CodeQL](https://img.shields.io/github/actions/workflow/status/nfx-libs/nfx-json/codeql.yml?branch=main&label=CodeQL&style=flat-square)](https://github.com/nfx-libs/nfx-json/actions/workflows/codeql.yml)

> A modern C++20 JSON library with type-safe manipulation, zero-copy navigation, and intuitive path-based access

## Overview

nfx-json is a modern C++20 library for working with JSON documents. It provides type-safe value access, zero-copy document navigation with JSON Pointer and dot notation paths, STL-compatible iterators for arrays and objects, and efficient parsing and generation - all optimized for performance across multiple platforms and compilers.

## Key Features

### 📦 Core JSON Components

- **Builder**: High-performance streaming JSON construction with fluent method chaining and SIMD optimizations
- **Document**: Generic JSON document abstraction with type-safe value access and manipulation
- **PathView**: Zero-copy document traversal with JSON Pointer and dot notation path iteration

### 🔒 Type-Safe Value Access

- Template-based `get<T>()` and `set<T>()` methods with compile-time type checking
- Support for primitive types (integers, floats, booleans, strings)
- C++20 concepts for type constraints (`Primitive`, `Value`, `Container`)
- Returns `std::optional<T>` for safe value retrieval
- Direct access to `Array` and `Object` containers

### 🛤️ Flexible Path Navigation

- **JSON Pointer (RFC 6901)**: `/user/name`, `/items/0/id`
- **Dot Notation**: `user.name`, `items[0].id`
- Auto-detection based on path syntax (paths starting with `/` use JSON Pointer)
- Create intermediate objects/arrays automatically with `createPath` option
- Navigate deeply nested structures safely

### 📊 Real-World Applications

- Configuration management (app settings, environment configs)
- API request/response handling (REST, GraphQL)
- Data persistence and caching
- Inter-process communication (IPC)
- Log processing and analysis
- Database document storage (NoSQL, MongoDB-style)
- Game save states and player data
- Message queue payloads (Kafka, RabbitMQ)

### 🔄 STL-Compatible Iteration

- Range-for loops over arrays: `for (const auto& item : array) { ... }`
- Range-for loops over objects: `for (const auto& [key, value] : object) { ... }`
- `ObjectIterator` with `.key()` and `.value()` accessors
- `PathView` for iterating all document paths with depth tracking

### ⚡ Performance Optimized

- SIMD-accelerated JSON serialization (SSE2 string escaping)
- Zero-copy document navigation with JSON Pointers
- Fast parsing with `std::from_chars` (no locale overhead)
- Direct buffer writing for JSON generation (no intermediate DOM)
- Pre-allocated buffers with smart memory management
- Compile-time type detection and optimization

### 🌍 Cross-Platform Support

- Linux, Windows
- GCC 14+, Clang 18+, MSVC 2022+
- Thread-safe operations
- Consistent behavior across platforms

## Quick Start

### Requirements

- C++20 compatible compiler:
  - **GCC 14+** (14.2.0 tested)
  - **Clang 18+** (19.1.7 tested)
  - **MSVC 2022+** (19.44+ tested)
- CMake 3.20 or higher

### CMake Integration

```cmake
# --- Library build types ---
option(NFX_JSON_BUILD_STATIC        "Build static library"              ON )
option(NFX_JSON_BUILD_SHARED        "Build shared library"              OFF)

# --- Build components ---
option(NFX_JSON_BUILD_TESTS         "Build tests"                       OFF)
option(NFX_JSON_BUILD_SAMPLES       "Build samples"                     OFF)
option(NFX_JSON_BUILD_BENCHMARKS    "Build benchmarks"                  OFF)
option(NFX_JSON_BUILD_DOCUMENTATION "Build Doxygen documentation"       OFF)

# --- Installation ---
option(NFX_JSON_INSTALL_PROJECT     "Install project"                   OFF)

# --- Packaging ---
option(NFX_JSON_PACKAGE_SOURCE      "Enable source package generation"  OFF)
option(NFX_JSON_PACKAGE_ARCHIVE     "Enable TGZ/ZIP package generation" OFF)
option(NFX_JSON_PACKAGE_DEB         "Enable DEB package generation"     OFF)
option(NFX_JSON_PACKAGE_RPM         "Enable RPM package generation"     OFF)
```

### Using in Your Project

#### Option 1: Using FetchContent (Recommended)

```cmake
include(FetchContent)
FetchContent_Declare(
  nfx-json
  GIT_REPOSITORY https://github.com/nfx-libs/nfx-json.git
  GIT_TAG        main  # or use specific version tag like "1.0.0"
)
FetchContent_MakeAvailable(nfx-json)

# Link with static library
target_link_libraries(your_target PRIVATE nfx-json::static)
```

#### Option 2: As a Git Submodule

```bash
# Add as submodule
git submodule add https://github.com/nfx-libs/nfx-json.git third-party/nfx-json
```

```cmake
# In your CMakeLists.txt
add_subdirectory(third-party/nfx-json)
target_link_libraries(your_target PRIVATE nfx-json::static)
```

#### Option 3: Using find_package (After Installation)

```cmake
find_package(nfx-json REQUIRED)
target_link_libraries(your_target PRIVATE nfx-json::static)
```

### Building

**Build Commands:**

```bash
# Clone the repository
git clone https://github.com/nfx-libs/nfx-json.git
cd nfx-json

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the library
cmake --build . --config Release --parallel

# Run tests (optional)
ctest -C Release --output-on-failure

# Run benchmarks (optional)
./bin/Release/BM_JsonAccess
```

### Documentation

nfx-json includes API documentation generated with Doxygen.

#### 📚 Online Documentation

The complete API documentation is available online at:
**https://nfx-libs.github.io/nfx-json**

#### Building Documentation Locally

```bash
# Configure with documentation enabled
cmake .. -DCMAKE_BUILD_TYPE=Release -DNFX_JSON_BUILD_DOCUMENTATION=ON

# Build the documentation
cmake --build . --target nfx-json-documentation
```

#### Requirements

- **Doxygen** - Documentation generation tool
- **Graphviz Dot** (optional) - For generating class diagrams

#### Accessing Local Documentation

After building, open `./build/doc/html/index.html` in your web browser.

## Usage Examples

### Builder - High-Performance JSON Construction

```cpp
#include <nfx/json/Builder.h>

using namespace nfx::json;

// Create JSON with compact output (default)
Builder builder;
builder.writeStartObject()
    .write("name", "Alice Johnson")
    .write("age", 30)
    .write("email", "alice@example.com")
    .write("active", true)
    .writeKey("tags").writeStartArray()
        .write("developer")
        .write("admin")
    .writeEndArray()
    .writeEndObject();

std::string json = builder.toString();
// {"name":"Alice Johnson","age":30,"email":"alice@example.com","active":true,"tags":["developer","admin"]}

// Pretty-print with 2-space indentation
Builder prettyBuilder{{2}};  // Options: {indent, bufferSize}
prettyBuilder.writeStartObject()
    .write("id", 12345)
    .write("username", "alice_dev")
    .writeKey("profile").writeStartObject()
        .write("firstName", "Alice")
        .write("lastName", "Johnson")
    .writeEndObject()
    .writeEndObject();

std::string prettyJson = prettyBuilder.toString();
// {
//   "id": 12345,
//   "username": "alice_dev",
//   "profile": {
//     "firstName": "Alice",
//     "lastName": "Johnson"
//   }
// }
```

### Document - JSON Manipulation

```cpp
#include <nfx/json/Document.h>

using namespace nfx::json;

// Create and manipulate JSON document
Document doc;

// Set values using JSON Pointer notation
doc.set<std::string>("/name", "John Doe");
doc.set<int>("/age", 30);
doc.set<std::string>("/email", "john.doe@example.com");

// Get values with type safety
auto name = doc.get<std::string>("/name");  // optional<string>
auto age = doc.get<int>("/age");            // optional<int>

// Work with nested objects
doc.set<std::string>("/address/city", "New York");
doc.set<std::string>("/address/zip", "10001");

// Arrays
doc.set<std::string>("/hobbies/0", "reading");
doc.set<std::string>("/hobbies/1", "gaming");
doc.set<std::string>("/hobbies/2", "coding");

// Serialize to JSON string
std::string json = doc.toString(2); // Pretty-print with 2-space indent
```
### Array Iteration with Range-For

```cpp
#include <nfx/json/Document.h>

using namespace nfx::json;

auto docOpt = Document::fromString(R"({
    "items": [
        {"id": 1, "name": "Item A"},
        {"id": 2, "name": "Item B"},
        {"id": 3, "name": "Item C"}
    ]
})");

if (!docOpt)
{
    return 1;
}

// Get array and iterate with range-for
auto itemsOpt = docOpt->get<Array>("items");
if (itemsOpt)
{
    for (const auto& item : itemsOpt.value())
    {
        auto id = item.get<int64_t>("id");
        auto name = item.get<std::string>("name");
        if (id && name)
        {
            std::cout << "ID: " << *id << ", Name: " << *name << std::endl;
        }
    }
}
```

### Object Iteration with Range-For

```cpp
#include <nfx/json/Document.h>

using namespace nfx::json;

auto docOpt = Document::fromString(R"({
    "config": {
        "timeout": 30,
        "retries": 3,
        "debug": true
    }
})");

if (!docOpt)
{
    return 1;
}

// Get object and iterate with range-for (structured bindings)
auto configOpt = docOpt->get<Object>("config");
if (configOpt)
{
    for (const auto& [key, value] : configOpt.value())
    {
        std::cout << "Key: " << key << ", Value: " << value.toString() << std::endl;
    }
}
```

### PathView - Document Traversal

```cpp
#include <nfx/json/Document.h>

using namespace nfx::json;

auto docOpt = Document::fromString(R"({
    "user": {
        "name": "Alice",
        "age": 30,
        "address": {
            "city": "Seattle",
            "zip": "98101"
        }
    },
    "tags": ["developer", "manager"]
})");

if (!docOpt)
{
    return 1;
}

// Iterate all paths with JSON Pointer format (default)
for (const auto& entry : Document::PathView(*docOpt))
{
    std::cout << entry.path << " (depth: " << entry.depth << ")" << std::endl;
}
// Output:
// /user (depth: 1)
// /user/name (depth: 2)
// /user/age (depth: 2)
// /user/address (depth: 2)
// /user/address/city (depth: 3)
// /user/address/zip (depth: 3)
// /tags (depth: 1)
// /tags/0 (depth: 2)
// /tags/1 (depth: 2)

// Use dot notation format
for (const auto& entry : Document::PathView(*docOpt, Document::PathView::Format::DotNotation))
{
    std::cout << entry.path << std::endl;
}
// Output: user.name, user.age, user.address.city, tags[0], tags[1], etc.

// Iterate only leaf values (primitives)
auto paths = Document::PathView(*docOpt);
auto leaves = paths.leaves();
for (const auto& entry : leaves)
{
    std::cout << entry.path << " = " << entry.value().toString() << std::endl;
}
// Output:
// /user/name = "Alice"
// /user/age = 30
// /user/address/city = "Seattle"
// /user/address/zip = "98101"
// /tags/0 = "developer"
// /tags/1 = "manager"
```

### Complete Example

```cpp
#include <iostream>
#include <nfx/json/Document.h>

using namespace nfx::json;

int main()
{
    // Parse JSON from string
    auto docOpt = Document::fromString(R"({
        "server": {
            "host": "localhost",
            "port": 8080,
            "ssl": true
        },
        "users": [
            {"name": "Alice", "role": "admin"},
            {"name": "Bob", "role": "user"}
        ]
    })");

    if (!docOpt) {
        std::cerr << "Failed to parse JSON\n";
        return 1;
    }

    Document& config = *docOpt;

    // Access values with type safety
    auto host = config.get<std::string>("server.host");
    auto port = config.get<int64_t>("/server/port");  // JSON Pointer syntax
    
    std::cout << "Server: " << host.value_or("unknown") 
              << ":" << port.value_or(0) << "\n";

    // Iterate over array
    if (auto users = config.get<Array>("users")) {
        for (const auto& user : *users) {
            auto name = user.get<std::string>("name");
            auto role = user.get<std::string>("role");
            std::cout << "User: " << *name << " (" << *role << ")\n";
        }
    }

    // Modify document
    config.set<int64_t>("/server/port", 9090);
    config.set<std::string>("/server/environment", "production");

    // Serialize back to JSON
    std::cout << "\nUpdated config:\n" << config.toString(2) << "\n";

    return 0;
}
```

**Sample Output:**

```
Server: localhost:8080
User: Alice (admin)
User: Bob (user)

Updated config:
{
  "server": {
    "host": "localhost",
    "port": 9090,
    "ssl": true,
    "environment": "production"
  },
  "users": [
    {
      "name": "Alice",
      "role": "admin"
    },
    {
      "name": "Bob",
      "role": "user"
    }
  ]
}
```

## Installation & Packaging

nfx-json provides packaging options for distribution.

### Package Generation

```bash
# Configure with packaging options
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DNFX_JSON_BUILD_STATIC=ON \
         -DNFX_JSON_BUILD_SHARED=ON \
         -DNFX_JSON_PACKAGE_ARCHIVE=ON \
         -DNFX_JSON_PACKAGE_DEB=ON \
         -DNFX_JSON_PACKAGE_RPM=ON

# Generate binary packages
cmake --build . --target package
# or
cd build && cpack

# Generate source packages
cd build && cpack --config CPackSourceConfig.cmake
```

### Supported Package Formats

| Format      | Platform       | Description                        | Requirements |
| ----------- | -------------- | ---------------------------------- | ------------ |
| **TGZ/ZIP** | Cross-platform | Compressed archive packages        | None         |
| **DEB**     | Debian/Ubuntu  | Native Debian packages             | `dpkg-dev`   |
| **RPM**     | RedHat/SUSE    | Native RPM packages                | `rpm-build`  |
| **Source**  | Cross-platform | Source code distribution (TGZ+ZIP) | None         |

### Installation

```bash
# Linux (DEB-based systems)
sudo dpkg -i nfx-json_*_amd64.deb

# Linux (RPM-based systems)
sudo rpm -ivh nfx-json-*-Linux.rpm

# Windows (extract ZIP archive)
Expand-Archive nfx-json-*-Windows.zip -DestinationPath C:\nfx\

# Manual installation (extract archive)
tar -xzf nfx-json-*-Linux.tar.gz -C /usr/local/
```

## Project Structure

```
nfx-json/
├── benchmark/             # Performance benchmarks with Google Benchmark
├── cmake/                 # CMake modules and configuration
├── include/nfx/           # Public headers
├── samples/               # Example usage and demonstrations
├── src/                   # Implementation files
└── test/                  # Unit tests with GoogleTest
```

## Performance

For detailed performance metrics and benchmarks, see the [benchmark documentation](benchmark/README.md).

## Roadmap

See [TODO.md](TODO.md) for upcoming features and project roadmap.

## Changelog

See the [CHANGELOG.md](CHANGELOG.md) for a detailed history of changes, new features, and bug fixes.

## License

This project is licensed under the MIT License.

## Dependencies

### Core Dependencies

- **[nfx-stringutils](https://github.com/nfx-libs/nfx-stringutils)**: String manipulation utilities (MIT License)
- **[nfx-stringbuilder](https://github.com/nfx-libs/nfx-stringbuilder)**: High-performance string builder with SBO (MIT License)
- **[nfx-containers](https://github.com/nfx-libs/nfx-containers)**: Container utilities (MIT License)
- **[nfx-hashing](https://github.com/nfx-libs/nfx-hashing)**: CRC32-C hashing for uniqueItems validation (MIT License)
- **[nfx-resource](https://github.com/nfx-libs/nfx-resource)**: Resource embedding framework (MIT License) - Development only

### Development Dependencies

- **[GoogleTest](https://github.com/google/googletest)**: Testing framework (BSD 3-Clause License) - Development only
- **[Google Benchmark](https://github.com/google/benchmark)**: Performance benchmarking framework (Apache 2.0 License) - Development only

All dependencies are automatically fetched via CMake FetchContent when building the library, tests, or benchmarks.

---

_Updated on February 04, 2026_
