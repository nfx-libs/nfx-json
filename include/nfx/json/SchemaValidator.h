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
 * @file SchemaValidator.h
 * @brief JSON Schema validation for Document instances
 * @details Provides JSON Schema Draft 2020-12 validation capabilities for verifying Document
 *          structure, types, constraints, and business rules against schema definitions.
 *          Supports validation with detailed error reporting and path tracking.
 */

#pragma once

#include "Document.h"

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace nfx::json
{
    //=====================================================================
    // ValidationError class
    //=====================================================================

    /**
     * @brief Represents a single JSON schema validation error
     * @details Contains detailed information about validation failures including
     *          the failed constraint, document path, expected vs actual values,
     *          and human-readable error messages.
     */
    class ValidationError final
    {
    public:
        /**
         * @brief Simple error entry structure for convenient ValidationError construction
         * @details Provides aggregate initialization syntax for creating validation errors
         *          with all error details in a single struct. Useful for functional-style
         *          error creation and collection patterns.
         */
        struct ErrorEntry
        {
            std::string path;               ///< JSON path where error occurred
            std::string message;            ///< Human-readable error message
            std::string constraint;         ///< Schema constraint that failed
            std::string expectedValue = {}; ///< Expected value or constraint
            std::string actualValue = {};   ///< Actual value found in document
        };

    public:
        //----------------------------------------------
        // Construction
        //----------------------------------------------
        /**
         * @brief Construct validation error from ErrorEntry struct
         * @param entry Error entry with all validation details
         */
        explicit ValidationError( const ErrorEntry& entry );

        /**
         * @brief Construct validation error
         * @param path JSON path where validation failed
         * @param message Human-readable error description
         * @param constraint The schema constraint that failed (e.g., "type", "minLength")
         * @param expectedValue Expected value or constraint (optional)
         * @param actualValue Actual value found in document (optional)
         */
        ValidationError(
            std::string path,
            std::string message,
            std::string constraint,
            std::string expectedValue = {},
            std::string actualValue = {} );

        //----------------------------------------------
        // Destruction
        //----------------------------------------------

        /**
         * @brief Destructor
         */
        ~ValidationError() = default;

        //----------------------------------------------
        // Accessors
        //----------------------------------------------

        /**
         * @brief Get the JSON path where validation failed
         * @return Path string (e.g., "/users/0/email")
         */
        inline const std::string& path() const noexcept;

        /**
         * @brief Get human-readable error message
         * @return Error description
         */
        inline const std::string& message() const noexcept;

        /**
         * @brief Get the constraint type that failed
         * @return Constraint name (e.g., "type", "minLength", "required")
         */
        inline const std::string& constraint() const noexcept;

        /**
         * @brief Get expected value or constraint
         * @return Expected value string, or empty if not applicable
         */
        inline const std::string& expectedValue() const noexcept;

        /**
         * @brief Get actual value found in document
         * @return Actual value string, or empty if not applicable
         */
        inline const std::string& actualValue() const noexcept;

        /**
         * @brief Get formatted error string
         * @return Complete formatted error message with path and details
         */
        std::string toString() const;

    private:
        ErrorEntry m_error;
    };

    //=====================================================================
    // ValidationResult class
    //=====================================================================

    /**
     * @brief Result of JSON schema validation operation
     * @details Contains validation success status and error reporting
     *          with all validation failures found during document validation.
     */
    class ValidationResult final
    {
    public:
        //----------------------------------------------
        // Construction
        //----------------------------------------------

        /**
         * @brief Construct successful validation result
         */
        ValidationResult() = default;

        /**
         * @brief Construct validation result with errors
         * @param errors List of validation errors found
         */
        explicit ValidationResult( std::vector<ValidationError> errors );

        //----------------------------------------------
        // Destruction
        //----------------------------------------------

        /**
         * @brief Destructor
         */
        ~ValidationResult() = default;

        //----------------------------------------------
        // Status checking
        //----------------------------------------------

        /**
         * @brief Check if validation was successful
         * @return True if no validation errors, false otherwise
         */
        inline bool isValid() const noexcept;

        /**
         * @brief Check if validation failed
         * @return True if validation errors exist, false otherwise
         */
        inline bool hasErrors() const noexcept;

        /**
         * @brief Get number of validation errors
         * @return Count of validation errors found
         */
        inline size_t errorCount() const noexcept;

        //----------------------------------------------
        // Error access
        //----------------------------------------------

        /**
         * @brief Get all validation errors
         * @return Vector of validation errors
         */
        inline const std::vector<ValidationError>& errors() const noexcept;

        /**
         * @brief Get validation error by index
         * @param index Error index (0-based)
         * @return Validation error at specified index
         * @throws std::out_of_range if index is invalid
         */
        const ValidationError& error( size_t index ) const;

        /**
         * @brief Get formatted error summary
         * @return Multi-line string with all validation errors
         */
        std::string errorSummary() const;

        //----------------------------------------------
        // Error manipulation
        //----------------------------------------------

        /**
         * @brief Add validation error from ErrorEntry
         * @param entry The error entry to add
         */
        void addError( const ValidationError::ErrorEntry& entry );

        /**
         * @brief Add validation error to result
         * @param error The validation error to add
         */
        void addError( const ValidationError& error );

        /**
         * @brief Add validation error with details
         * @param path JSON path where validation failed
         * @param message Human-readable error description
         * @param constraint The schema constraint that failed
         * @param expectedValue Expected value or constraint (optional)
         * @param actualValue Actual value found in document (optional)
         */
        void addError(
            std::string_view path,
            std::string_view message,
            std::string_view constraint,
            std::string_view expectedValue = {},
            std::string_view actualValue = {} );

    private:
        std::vector<ValidationError> m_errors;
    };

    //=====================================================================
    // SchemaDraft enumeration
    //=====================================================================

    /**
     * @brief Enumeration of supported JSON Schema draft versions
     * @details Used to identify and track which JSON Schema draft version a schema conforms to.
     *          Detection is based on the `$schema` keyword in the schema document.
     */
    enum class SchemaDraft
    {
        Unknown,     ///< Schema draft version is not recognized or not specified
        Draft04,     ///< JSON Schema Draft 4
        Draft06,     ///< JSON Schema Draft 6
        Draft07,     ///< JSON Schema Draft 7
        Draft201909, ///< JSON Schema Draft 2019-09
        Draft202012  ///< JSON Schema Draft 2020-12 (current)
    };

    //=====================================================================
    // SchemaValidator class
    //=====================================================================

    /**
     * @brief JSON Schema validator for Document instances
     * @details Provides JSON Schema Draft 2020-12 validation capabilities
     *          including type checking, constraint validation, object/array validation,
     *          and custom validation rules. Supports schema loading from Documents
     *          and detailed error reporting with path tracking.
     * @warning NOT thread-safe. Do not call validate() concurrently on the same instance.
     */
    class SchemaValidator final
    {
    public:
        //----------------------------------------------
        // Options
        //----------------------------------------------

        /**
         * @brief Configuration options for validation
         */
        struct Options
        {
            bool strictMode = false; ///< If true, unknown properties cause validation failure
            size_t maxDepth = 64;    ///< Maximum recursion depth (0 = unlimited)
        };

        //----------------------------------------------
        // Construction
        //----------------------------------------------

        /**
         * @brief Default constructor - creates empty validator
         * @details Creates validator without schema. Must load schema before validation.
         */
        SchemaValidator();

        /**
         * @brief Construct validator with schema (default options)
         * @param schema JSON Schema document to use for validation
         */
        explicit SchemaValidator( const Document& schema );

        /**
         * @brief Construct validator with schema
         * @param schema JSON Schema document to use for validation
         * @param options Validation options
         */
        SchemaValidator( const Document& schema, const Options& options );

        /**
         * @brief Copy constructor
         * @param other The validator to copy from
         */
        SchemaValidator( const SchemaValidator& other );

        /**
         * @brief Move constructor
         * @param other The validator to move from
         */
        SchemaValidator( SchemaValidator&& other ) noexcept;

        //----------------------------------------------
        // Destruction
        //----------------------------------------------

        /**
         * @brief Destructor
         */
        ~SchemaValidator();

        //----------------------------------------------
        // Assignment
        //----------------------------------------------

        /**
         * @brief Copy assignment operator
         * @param other The validator to copy from
         * @return Reference to this validator
         */
        SchemaValidator& operator=( const SchemaValidator& other );

        /**
         * @brief Move assignment operator
         * @param other The validator to move from
         * @return Reference to this validator
         */
        SchemaValidator& operator=( SchemaValidator&& other ) noexcept;

        //----------------------------------------------
        // Schema management
        //----------------------------------------------

        /**
         * @brief Load JSON schema from Document
         * @param schema JSON Schema document
         * @return True if schema loaded successfully, false on parse error
         */
        bool load( const Document& schema );

        /**
         * @brief Load JSON schema from string
         * @param schemaJson JSON Schema as string
         * @return True if schema loaded successfully, false on parse error
         */
        bool load( std::string_view schemaJson );

        /**
         * @brief Check if validator has valid schema loaded
         * @return True if schema is loaded and valid, false otherwise
         */
        bool hasSchema() const;

        /**
         * @brief Clear loaded schema
         */
        void clear();

        /**
         * @brief Get current schema as Document
         * @return Current schema Document, or empty Document if no schema loaded
         */
        Document schema() const;

        //----------------------------------------------
        // Validation operations
        //----------------------------------------------

        /**
         * @brief Validate Document against loaded schema
         * @param document The JSON document to validate
         * @return ValidationResult with success status and error details
         * @throws std::runtime_error if no schema is loaded
         * @warning NOT thread-safe (modifies internal cache)
         */
        ValidationResult validate( const Document& document ) const;

        /**
         * @brief Validate Document at specific path against schema
         * @param document The JSON document to validate
         * @param documentPath Path within document to validate (empty for root)
         * @param schemaPath Path within schema to validate against (empty for root schema)
         * @return ValidationResult with success status and error details
         * @warning NOT thread-safe (modifies internal cache)
         */
        ValidationResult validateAtPath(
            const Document& document, std::string_view documentPath = "", std::string_view schemaPath = "" ) const;

        //----------------------------------------------
        // Schema information
        //----------------------------------------------

        /**
         * @brief Get schema version/draft URI
         * @return Schema $schema URI string, or empty if not specified
         */
        std::string version() const;

        /**
         * @brief Get detected JSON Schema draft version
         * @return SchemaDraft enumeration value indicating the draft version
         */
        SchemaDraft draft() const;

        /**
         * @brief Get schema draft version as human-readable string
         * @return Draft version string (e.g., "2020-12", "07"), or empty if unknown
         */
        std::string draftString() const;

        /**
         * @brief Get schema title
         * @return Schema title string, or empty if not specified
         */
        std::string title() const;

        /**
         * @brief Get schema description
         * @return Schema description string, or empty if not specified
         */
        std::string description() const;

        /**
         * @brief Get validation options
         * @return Current validation options
         */
        const Options& options() const noexcept;

    private:
        //----------------------------------------------
        // Internal validation methods
        //----------------------------------------------

        /**
         * @brief Validate a document node against schema node
         * @param document Document to validate
         * @param schema Schema to validate against
         * @param path Current JSON path for error reporting
         * @param result ValidationResult to accumulate errors
         * @param currentDepth Current recursion depth for circular reference protection
         */
        void validateNode(
            const Document& document,
            const Document& schema,
            std::string_view path,
            ValidationResult& result,
            size_t currentDepth = 0 ) const;

        void validateType(
            const Document& document, const Document& schema, std::string_view path, ValidationResult& result ) const;

        void validateRequired(
            const Document& document, const Document& schema, std::string_view path, ValidationResult& result ) const;

        void validateProperties(
            const Document& document,
            const Document& schema,
            std::string_view path,
            ValidationResult& result,
            size_t currentDepth = 0 ) const;

        void validateNumericConstraints(
            const Document& document, const Document& schema, std::string_view path, ValidationResult& result ) const;

        void validateStringConstraints(
            const Document& document, const Document& schema, std::string_view path, ValidationResult& result ) const;

        void validateEnum(
            const Document& document, const Document& schema, std::string_view path, ValidationResult& result ) const;

        void validateConst(
            const Document& document, const Document& schema, std::string_view path, ValidationResult& result ) const;

        void validateAdditionalProperties(
            const Document& document, const Document& schema, std::string_view path, ValidationResult& result ) const;

        void validateObject(
            const Document& document,
            const Document& schema,
            std::string_view path,
            ValidationResult& result,
            size_t currentDepth = 0 ) const;

        void validateArray(
            const Document& document,
            const Document& schema,
            std::string_view path,
            ValidationResult& result,
            size_t currentDepth = 0 ) const;

        void validateAllOf(
            const Document& document,
            const Document& schema,
            std::string_view path,
            ValidationResult& result,
            size_t currentDepth = 0 ) const;

        void validateAnyOf(
            const Document& document,
            const Document& schema,
            std::string_view path,
            ValidationResult& result,
            size_t currentDepth = 0 ) const;

        void validateOneOf(
            const Document& document,
            const Document& schema,
            std::string_view path,
            ValidationResult& result,
            size_t currentDepth = 0 ) const;

        void validateNot(
            const Document& document,
            const Document& schema,
            std::string_view path,
            ValidationResult& result,
            size_t currentDepth = 0 ) const;

        void validateIfThenElse(
            const Document& document,
            const Document& schema,
            std::string_view path,
            ValidationResult& result,
            size_t currentDepth = 0 ) const;

        //----------------------------------------------
        // Helper methods
        //----------------------------------------------

        bool referenceExists( std::string_view reference ) const noexcept;
        Document resolveReference( std::string_view reference ) const;
        std::string actualType( const Document& document, std::string_view path ) const noexcept;
        Document extractSubDocument( const Document& sourceDocument, std::string_view path ) const;
        void buildAnchorIndex();
        void scanForAnchors( const Document& schema, const std::string& currentPath );
        void detectDraft();
        std::string draftToString() const noexcept;

        //----------------------------------------------
        // Member variables
        //----------------------------------------------

        static constexpr size_t DEFAULT_MAX_DEPTH = 64;

        Document m_schema;                                            ///< Schema document
        bool m_schemaLoaded = false;                                  ///< Whether a schema has been loaded
        bool m_strictMode = false;                                    ///< Whether strict validation mode is enabled
        size_t m_maxDepth = DEFAULT_MAX_DEPTH;                        ///< Maximum validation depth (0 = unlimited)
        SchemaDraft m_schemaDraft = SchemaDraft::Unknown;             ///< Detected schema draft version
        mutable std::unordered_map<std::string, Document> m_refCache; ///< Cache for resolved $ref references
        std::unordered_map<std::string, std::string> m_anchorIndex;   ///< Map of anchor names to JSON Pointer paths
    };
} // namespace nfx::json

#include "detail/SchemaValidator.inl"
