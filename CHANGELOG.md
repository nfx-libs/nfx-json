# Changelog

## [Unreleased]

### Added

- NIL

### Changed

- Bump nfx-stringutils from 0.4.0 to 0.5.0
- Remove unused variables from `std::from_chars` and `std::to_chars` calls

### Deprecated

- NIL

### Removed

- NIL

### Fixed

- NIL

### Security

- NIL

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
