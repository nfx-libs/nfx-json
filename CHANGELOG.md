# Changelog

## [Unreleased]

### Added

- NIL

### Changed

- NIL

### Deprecated

- NIL

### Removed

- NIL

### Fixed

- NIL

### Security

- NIL

## [1.3.1] - 2026-02-06

### Changed

- Bump nfx-stringutils from 0.6.0 to 0.6.1 (forward slash escaping now RFC 8259 compliant)

## [1.3.0] - 2026-02-06

### Added

- **Builder**: `escapeNonAscii` option to encode non-ASCII UTF-8 characters as `\uXXXX` sequences (BMP) or surrogate pairs (supplementary planes)

### Changed

- Bump nfx-stringutils from 0.5.1 to 0.6.0

## [1.2.1] - 2026-02-04

- Bump nfx-stringutils from 0.5.0 to 0.5.1

## [1.2.0] - 2026-02-04

### Added

- **Dependencies**: nfx-stringbuilder v0.5.0 as public dependency

### Changed

- **Builder**: Replaced internal buffer implementation with `nfx::string::StringBuilder` for significantly improved serialization performance
- **Builder**: Optimized `writeNewlineAndIndent()` using `resize()` + `memset()` instead of character-by-character loop

## [1.1.2] - 2026-02-03

### Changed

- Bump nfx-containers from 0.3.1 to 0.3.2

## [1.1.1] - 2026-02-02

### Changed

- **Builder**: Refactored `writeDocument()` to extract long switch cases into separate helper methods `writeDocumentArray()` and `writeDocumentObject()`
- **Parser**:
  - Refactored `parseUnicodeEscape()` by extracting surrogate pair handling into `parseSurrogatePair()` and UTF-8 encoding into `encodeUtf8()`
  - Refactored `parseString()` by extracting escape sequence handling into `parseEscapeSequence()`
  - Added `SIMD_CHUNK_SIZE` constant to replace magic number 16 in SIMD code paths

### Fixed

- **Builder**: SIMD string escaping now continues processing after finding characters to escape instead of falling back to scalar code for the entire remaining string

## [1.1.0] - 2026-01-31

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
- Bump nfx-containers from 0.3.0 to 0.3.1

### Removed

- **Internal**: `JsonWriter` class replaced by public `Builder` API

### Fixed

- **Document**: Replace `thread_local` with `static` in `operator[]` const for better portability

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
