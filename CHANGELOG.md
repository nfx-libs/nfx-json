# Changelog

## [Unreleased]

### Added

- **Builder**: Fluent API for incremental JSON construction without DOM overhead
- **Builder**: SIMD-optimized string escaping using SSE2 intrinsics
- **Builder**: Support for pretty-printing with configurable indentation
- **Builder**: Template helpers for writing STL containers as arrays
- **Document**: Constructors for `int`, `unsigned int`, and string literals for ergonomics
- **Document**: SFINAE constructors for `long`, `unsigned long`, `long long`, `unsigned long long` (cross-platform compatibility)
- **Tests**: Unit test suite for Builder (90+ test cases)
- **Samples**: `Sample_JsonBuilder` demonstrating Builder API usage
- **Parser**: SIMD-optimized whitespace skipping and string parsing using SSE2 intrinsics
- **Parser**: Platform-independent `countTrailingZeros` helper function
- **Benchmarks**: Builder serialization benchmarks (compact and pretty-print variants)

### Changed

- **Document**: Replace internal `JsonWriter` with public `Builder` API
- **Parser**: Renamed internal `JsonParser` to `Parser` (implementation detail)

### Deprecated

- NIL

### Removed

- **Internal**: `JsonWriter` class replaced by public `Builder` API

### Fixed

- **Document**: Replace `thread_local` with `static` in `operator[]` const for better portability

### Security

- NIL
- 
## [1.0.3] - 2026-01-27

### Changed

- Bump nfx-containers from 0.2.0 to 0.3.0
- Bump nfx-resource from 1.0.0 to 1.1.0

## [1.0.2] - 2026-01-25

### Added

- Tests for invalid Unicode escape sequences (malformed hex digits, incomplete escapes)

### Changed

- Bump nfx-stringutils from 0.4.0 to 0.5.0
- Extract `parseUnicodeEscape` helper method to improve JsonParser readability
- Optimize Unicode escape parsing to eliminate string allocations

### Removed

- Unused variables from `std::from_chars` and `std::to_chars` calls

### Fixed

- Double-increment bug in `unescapeJsonPointerToken` that caused character skipping
- Sign-conversion warnings in JsonWriter and SchemaGenerator

## [1.0.1] - 2026-01-24

### Added

- **JsonWriter**:
  - SIMD-optimized string escaping using SSE2 intrinsics
  - Hex lookup table for `\uXXXX` character encoding

## [1.0.0] - 2026-01-23

### Added

- Variant-based Document type for JSON representation
- JSON Schema Draft 2020-12 validation and generation
- STL-compatible iterators (JsonArrayIterator, JsonObjectFieldIterator)
- RFC 6901 JSON Pointer implementation
- JsonPathView for zero-copy path-based access
- Hardware-accelerated hashing (CRC32-C via SSE4.2)
- Samples demonstrating API usage
- Packaging support (TGZ/ZIP, DEB, RPM)
