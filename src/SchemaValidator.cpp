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
 * @file SchemaValidator.cpp
 * @brief Implementation of SchemaValidator class for JSON Schema validation
 * @details Provides concrete implementation for JSON Schema validation with detailed error reporting.
 */

#include "nfx/json/SchemaValidator.h"

#include "nfx/json/Document.h"
#include "Hash.h"
#include "Vocabulary.h"

#include <nfx/containers/FastHashSet.h>
#include <nfx/StringUtils.h>

#include <algorithm>
#include <cmath>
#include <regex>
#include <set>
#include <stdexcept>
#include <sstream>

namespace nfx::json
{
    //=====================================================================
    // ValidationError class
    //=====================================================================

    //----------------------------------------------
    // Construction
    //----------------------------------------------

    ValidationError::ValidationError( const ErrorEntry& entry )
        : m_error{ entry }
    {
    }

    ValidationError::ValidationError(
        std::string path,
        std::string message,
        std::string constraint,
        std::string expectedValue,
        std::string actualValue )
        : m_error{ std::move( path ),
                   std::move( message ),
                   std::move( constraint ),
                   std::move( expectedValue ),
                   std::move( actualValue ) }
    {
    }

    //----------------------------------------------
    // Accessors
    //----------------------------------------------

    std::string ValidationError::toString() const
    {
        std::ostringstream oss;
        oss << "Validation error at path '" << m_error.path << "': " << m_error.message;

        if( !m_error.constraint.empty() )
        {
            oss << " (constraint: " << m_error.constraint;
            if( !m_error.expectedValue.empty() )
            {
                oss << ", expected: " << m_error.expectedValue;
            }
            if( !m_error.actualValue.empty() )
            {
                oss << ", actual: " << m_error.actualValue;
            }
            oss << ")";
        }

        return oss.str();
    }

    //=====================================================================
    // ValidationResult class
    //=====================================================================

    //----------------------------------------------
    // Construction
    //----------------------------------------------

    ValidationResult::ValidationResult( std::vector<ValidationError> errors )
        : m_errors{ std::move( errors ) }
    {
    }

    //----------------------------------------------
    // Error access
    //----------------------------------------------

    const ValidationError& ValidationResult::error( size_t index ) const
    {
        if( index >= m_errors.size() )
        {
            throw std::out_of_range{ "ValidationResult error index out of range" };
        }
        return m_errors[index];
    }

    std::string ValidationResult::errorSummary() const
    {
        if( m_errors.empty() )
        {
            return "No validation errors";
        }

        std::ostringstream oss;
        oss << "Validation failed with " << m_errors.size() << " error(s):\n";

        for( size_t i = 0; i < m_errors.size(); ++i )
        {
            oss << "  " << ( i + 1 ) << ". " << m_errors[i].toString() << "\n";
        }

        return oss.str();
    }

    //----------------------------------------------
    // Error manipulation
    //----------------------------------------------

    void ValidationResult::addError( const ValidationError::ErrorEntry& entry )
    {
        m_errors.emplace_back( entry );
    }

    void ValidationResult::addError( const ValidationError& error )
    {
        m_errors.push_back( error );
    }

    void ValidationResult::addError(
        std::string_view path,
        std::string_view message,
        std::string_view constraint,
        std::string_view expectedValue,
        std::string_view actualValue )
    {
        m_errors.emplace_back(
            std::string{ path },
            std::string{ message },
            std::string{ constraint },
            std::string{ expectedValue },
            std::string{ actualValue } );
    }

    //=====================================================================
    // SchemaValidator class
    //=====================================================================

    //----------------------------------------------
    // Construction
    //----------------------------------------------

    SchemaValidator::SchemaValidator()
        : m_schema{},
          m_schemaLoaded{ false },
          m_strictMode{ false },
          m_maxDepth{ DEFAULT_MAX_DEPTH },
          m_schemaDraft{ SchemaDraft::Unknown }
    {
    }

    SchemaValidator::SchemaValidator( const Document& schema )
        : SchemaValidator( schema, Options{} )
    {
    }

    SchemaValidator::SchemaValidator( const Document& schema, const Options& options )
        : m_schema{ schema },
          m_schemaLoaded{ true },
          m_strictMode{ options.strictMode },
          m_maxDepth{ options.maxDepth },
          m_schemaDraft{ SchemaDraft::Unknown }
    {
        detectDraft();
        buildAnchorIndex();
    }

    SchemaValidator::SchemaValidator( const SchemaValidator& other )
        : m_schema{ other.m_schema },
          m_schemaLoaded{ other.m_schemaLoaded },
          m_strictMode{ other.m_strictMode },
          m_maxDepth{ other.m_maxDepth },
          m_schemaDraft{ other.m_schemaDraft },
          m_anchorIndex{ other.m_anchorIndex }
    {
    }

    SchemaValidator::SchemaValidator( SchemaValidator&& other ) noexcept
        : m_schema{ std::move( other.m_schema ) },
          m_schemaLoaded{ other.m_schemaLoaded },
          m_strictMode{ other.m_strictMode },
          m_maxDepth{ other.m_maxDepth },
          m_schemaDraft{ other.m_schemaDraft },
          m_refCache{ std::move( other.m_refCache ) },
          m_anchorIndex{ std::move( other.m_anchorIndex ) }
    {
        other.m_schemaLoaded = false;
        other.m_strictMode = false;
        other.m_maxDepth = 0;
        other.m_schemaDraft = SchemaDraft::Unknown;
    }

    //----------------------------------------------
    // Destruction
    //----------------------------------------------

    SchemaValidator::~SchemaValidator() = default;

    //----------------------------------------------
    // Assignment
    //----------------------------------------------

    SchemaValidator& SchemaValidator::operator=( const SchemaValidator& other )
    {
        if( this != &other )
        {
            m_schema = other.m_schema;
            m_schemaLoaded = other.m_schemaLoaded;
            m_strictMode = other.m_strictMode;
            m_maxDepth = other.m_maxDepth;
            m_schemaDraft = other.m_schemaDraft;
            m_anchorIndex = other.m_anchorIndex;
        }
        return *this;
    }

    SchemaValidator& SchemaValidator::operator=( SchemaValidator&& other ) noexcept
    {
        if( this != &other )
        {
            m_schema = std::move( other.m_schema );
            m_schemaLoaded = other.m_schemaLoaded;
            m_strictMode = other.m_strictMode;
            m_maxDepth = other.m_maxDepth;
            m_schemaDraft = other.m_schemaDraft;
            m_refCache = std::move( other.m_refCache );
            m_anchorIndex = std::move( other.m_anchorIndex );

            other.m_schemaLoaded = false;
            other.m_strictMode = false;
            other.m_maxDepth = 0;
            other.m_schemaDraft = SchemaDraft::Unknown;
        }
        return *this;
    }

    //----------------------------------------------
    // Schema management
    //----------------------------------------------

    bool SchemaValidator::load( const Document& schema )
    {
        if( !schema.isValid() )
        {
            return false;
        }

        if( !schema.is<Object>( "" ) )
        {
            return false;
        }

        m_schema = schema;
        m_schemaLoaded = true;
        detectDraft();
        buildAnchorIndex();
        return true;
    }

    bool SchemaValidator::load( std::string_view schemaJson )
    {
        auto maybeSchema = Document::fromString( schemaJson );
        if( !maybeSchema.has_value() )
        {
            return false;
        }

        return load( maybeSchema.value() );
    }

    bool SchemaValidator::hasSchema() const
    {
        return m_schemaLoaded;
    }

    void SchemaValidator::clear()
    {
        m_schema = Document{};
        m_schemaLoaded = false;
        m_refCache.clear();
        m_anchorIndex.clear();
    }

    Document SchemaValidator::schema() const
    {
        return m_schema;
    }

    //----------------------------------------------
    // Validation operations
    //----------------------------------------------

    ValidationResult SchemaValidator::validate( const Document& document ) const
    {
        if( !m_schemaLoaded )
        {
            throw std::runtime_error{ "No schema loaded for validation" };
        }

        ValidationResult result;
        validateNode( document, m_schema, "", result, 0 );
        return result;
    }

    ValidationResult SchemaValidator::validateAtPath(
        const Document& document, std::string_view documentPath, std::string_view schemaPath ) const
    {
        if( !m_schemaLoaded )
        {
            throw std::runtime_error{ "No schema loaded for validation" };
        }

        ValidationResult result;

        // If both paths are empty, validate entire document against entire schema
        if( documentPath.empty() && schemaPath.empty() )
        {
            return validate( document );
        }

        // Extract the target document value at the given path
        Document targetDocument;
        if( documentPath.empty() )
        {
            targetDocument = document;
        }
        else
        {
            // Check if path exists in document
            if( !document.contains( documentPath ) )
            {
                result.addError(
                    documentPath,
                    "Document path not found: " + std::string{ documentPath },
                    "path",
                    std::string{ documentPath },
                    "not found" );
                return result;
            }
            targetDocument = extractSubDocument( document, documentPath );
        }

        // Extract the target schema at the given path
        Document targetSchema;
        if( schemaPath.empty() )
        {
            targetSchema = m_schema;
        }
        else
        {
            // Check if schemaPath is a reference (starts with #)
            if( !schemaPath.empty() && schemaPath[0] == '#' )
            {
                // Check if reference exists first
                if( !referenceExists( schemaPath ) )
                {
                    result.addError(
                        documentPath,
                        "Schema path not found: " + std::string{ schemaPath },
                        "schemaPath",
                        std::string{ schemaPath },
                        "not found" );
                    return result;
                }

                // Resolve the reference
                targetSchema = resolveReference( schemaPath );
                if( !targetSchema.isValid() )
                {
                    result.addError(
                        documentPath,
                        "Schema path not found: " + std::string{ schemaPath },
                        "schemaPath",
                        std::string{ schemaPath },
                        "not found" );
                    return result;
                }
            }
            else
            {
                // Direct path in schema
                if( !m_schema.contains( schemaPath ) )
                {
                    result.addError(
                        documentPath,
                        "Schema path not found: " + std::string{ schemaPath },
                        "schemaPath",
                        std::string{ schemaPath },
                        "not found" );
                    return result;
                }
                targetSchema = extractSubDocument( m_schema, schemaPath );
            }
        }

        // Validate the extracted document against the extracted schema
        // Use empty path "" because targetDocument is already the extracted sub-document
        validateNode( targetDocument, targetSchema, "", result, 0 );
        return result;
    }

    //----------------------------------------------
    // Schema information
    //----------------------------------------------

    std::string SchemaValidator::version() const
    {
        if( !m_schemaLoaded )
        {
            return {};
        }

        auto result = m_schema.get<std::string>( vocabulary::CORE_SCHEMA );
        return result.value_or( "" );
    }

    SchemaDraft SchemaValidator::draft() const
    {
        if( !m_schemaLoaded )
        {
            return SchemaDraft::Unknown;
        }

        return m_schemaDraft;
    }

    std::string SchemaValidator::draftString() const
    {
        if( !m_schemaLoaded )
        {
            return {};
        }

        return draftToString();
    }

    std::string SchemaValidator::title() const
    {
        if( !m_schemaLoaded )
        {
            return {};
        }

        auto result = m_schema.get<std::string>( vocabulary::METADATA_TITLE );
        return result.value_or( "" );
    }

    std::string SchemaValidator::description() const
    {
        if( !m_schemaLoaded )
        {
            return {};
        }

        auto result = m_schema.get<std::string>( vocabulary::METADATA_DESCRIPTION );
        return result.value_or( "" );
    }

    const SchemaValidator::Options& SchemaValidator::options() const noexcept
    {
        static thread_local Options opts;
        opts.strictMode = m_strictMode;
        opts.maxDepth = m_maxDepth;
        return opts;
    }
    // =====================================================================
    // Internal validation implementation methods
    // =====================================================================
    void SchemaValidator::validateNode(
        const Document& document,
        const Document& schema,
        std::string_view path,
        ValidationResult& result,
        size_t currentDepth ) const
    {
        // Check depth limit to prevent stack overflow on circular $ref
        if( m_maxDepth > 0 && currentDepth > m_maxDepth )
        {
            result.addError(
                path,
                "Maximum validation depth exceeded (possible circular $ref)",
                "maxDepth",
                std::to_string( m_maxDepth ),
                std::to_string( currentDepth ) );

            return;
        }

        if( schema.contains( vocabulary::CORE_REF ) )
        {
            auto refOpt = schema.get<std::string>( vocabulary::CORE_REF );
            if( refOpt.has_value() )
            {
                Document resolvedSchema = resolveReference( refOpt.value() );
                if( resolvedSchema.isValid() )
                {
                    validateNode( document, resolvedSchema, path, result, currentDepth + 1 );
                    return;
                }
                else
                {
                    result.addError(
                        path,
                        "Could not resolve reference: " + refOpt.value(),
                        vocabulary::CORE_REF,
                        refOpt.value(),
                        "unresolved" );

                    return;
                }
            }
        }

        validateType( document, schema, path, result );

        if( schema.contains( vocabulary::VALIDATION_ENUM ) )
        {
            validateEnum( document, schema, path, result );
        }

        if( schema.contains( vocabulary::VALIDATION_CONST ) )
        {
            validateConst( document, schema, path, result );
        }

        // Validate applicator keywords (allOf, anyOf, oneOf, not)
        if( schema.contains( vocabulary::APPLICATOR_ALL_OF ) )
        {
            validateAllOf( document, schema, path, result, currentDepth );
        }

        if( schema.contains( vocabulary::APPLICATOR_ANY_OF ) )
        {
            validateAnyOf( document, schema, path, result, currentDepth );
        }

        if( schema.contains( vocabulary::APPLICATOR_ONE_OF ) )
        {
            validateOneOf( document, schema, path, result, currentDepth );
        }

        if( schema.contains( vocabulary::APPLICATOR_NOT ) )
        {
            validateNot( document, schema, path, result, currentDepth );
        }

        // Validate conditional keywords (if/then/else)
        if( schema.contains( vocabulary::APPLICATOR_IF ) )
        {
            validateIfThenElse( document, schema, path, result, currentDepth );
        }

        auto expectedType = schema.get<std::string>( vocabulary::VALIDATION_TYPE );

        // Check for object-related keywords even without explicit "type": "object"
        bool hasObjectKeywords = schema.contains( vocabulary::APPLICATOR_PROPERTIES ) ||
                                 schema.contains( vocabulary::VALIDATION_REQUIRED ) ||
                                 schema.contains( vocabulary::APPLICATOR_ADDITIONAL_PROPERTIES ) ||
                                 schema.contains( vocabulary::APPLICATOR_PATTERN_PROPERTIES ) ||
                                 schema.contains( vocabulary::VALIDATION_MIN_PROPERTIES ) ||
                                 schema.contains( vocabulary::VALIDATION_MAX_PROPERTIES );

        // Check for array-related keywords even without explicit "type": "array"
        bool hasArrayKeywords = schema.contains( vocabulary::APPLICATOR_ITEMS ) ||
                                schema.contains( vocabulary::APPLICATOR_PREFIX_ITEMS ) ||
                                schema.contains( vocabulary::VALIDATION_MIN_ITEMS ) ||
                                schema.contains( vocabulary::VALIDATION_MAX_ITEMS ) ||
                                schema.contains( vocabulary::VALIDATION_UNIQUE_ITEMS ) ||
                                schema.contains( vocabulary::APPLICATOR_CONTAINS );

        // Check for string-related keywords even without explicit "type": "string"
        bool hasStringKeywords = schema.contains( vocabulary::VALIDATION_MIN_LENGTH ) ||
                                 schema.contains( vocabulary::VALIDATION_MAX_LENGTH ) ||
                                 schema.contains( vocabulary::VALIDATION_PATTERN ) ||
                                 schema.contains( vocabulary::FORMAT_ANNOTATION );

        // Check for numeric-related keywords even without explicit "type": "number"/"integer"
        bool hasNumericKeywords = schema.contains( vocabulary::VALIDATION_MINIMUM ) ||
                                  schema.contains( vocabulary::VALIDATION_MAXIMUM ) ||
                                  schema.contains( vocabulary::VALIDATION_EXCLUSIVE_MINIMUM ) ||
                                  schema.contains( vocabulary::VALIDATION_EXCLUSIVE_MAXIMUM ) ||
                                  schema.contains( vocabulary::VALIDATION_MULTIPLE_OF );

        if( expectedType == "object" ||
            ( !expectedType.has_value() && hasObjectKeywords && document.is<Object>( path ) ) )
        {
            validateObject( document, schema, path, result, currentDepth );
        }

        if( expectedType == "array" || ( !expectedType.has_value() && hasArrayKeywords && document.is<Array>( path ) ) )
        {
            validateArray( document, schema, path, result, currentDepth );
        }

        if( expectedType == "string" ||
            ( !expectedType.has_value() && hasStringKeywords && document.is<std::string>( path ) ) )
        {
            validateStringConstraints( document, schema, path, result );
        }

        if( expectedType == "number" || expectedType == "integer" ||
            ( !expectedType.has_value() && hasNumericKeywords &&
              ( document.is<double>( path ) || document.is<int>( path ) ) ) )
        {
            validateNumericConstraints( document, schema, path, result );
        }
    }

    void SchemaValidator::validateType(
        const Document& document, const Document& schema, std::string_view path, ValidationResult& result ) const
    {
        if( !schema.contains( vocabulary::VALIDATION_TYPE ) )
        {
            return;
        }

        auto expectedType = schema.get<std::string>( vocabulary::VALIDATION_TYPE ).value_or( "" );
        std::string type = actualType( document, path );

        bool typeMatches = false;

        if( expectedType == "object" && document.is<Object>( path ) )
        {
            typeMatches = true;
        }
        else if( expectedType == "array" && document.is<Array>( path ) )
        {
            typeMatches = true;
        }
        else if( expectedType == "string" && document.is<std::string>( path ) )
        {
            typeMatches = true;
        }
        else if( expectedType == "number" && ( document.is<double>( path ) || document.is<int>( path ) ) )
        {
            typeMatches = true;
        }
        else if( expectedType == "integer" && document.is<int>( path ) )
        {
            typeMatches = true;
        }
        else if( expectedType == "boolean" && document.is<bool>( path ) )
        {
            typeMatches = true;
        }
        else if( expectedType == "null" && document.isNull( path ) )
        {
            typeMatches = true;
        }

        if( !typeMatches )
        {
            result.addError( path, "Type mismatch", vocabulary::VALIDATION_TYPE, expectedType, type );
        }
    }

    void SchemaValidator::validateRequired(
        const Document& document, const Document& schema, std::string_view path, ValidationResult& result ) const
    {
        if( !schema.contains( vocabulary::VALIDATION_REQUIRED ) )
        {
            return;
        }

        auto requiredArray = schema.get<Array>( vocabulary::VALIDATION_REQUIRED );
        if( !requiredArray.has_value() )
        {
            return;
        }

        size_t requiredCount = requiredArray.value().size();
        for( size_t i = 0; i < requiredCount; ++i )
        {
            const Document& arrayElement = requiredArray.value().at( i );
            auto requiredFieldOpt = arrayElement.get<std::string>( "" );
            if( requiredFieldOpt.has_value() )
            {
                std::string requiredField = requiredFieldOpt.value();
                std::string fieldPath = path.empty() ? requiredField : std::string{ path } + "." + requiredField;

                if( !document.contains( fieldPath ) )
                {
                    result.addError(
                        fieldPath,
                        "Required field missing",
                        vocabulary::VALIDATION_REQUIRED,
                        requiredField,
                        "undefined" );
                }
            }
        }
    }

    void SchemaValidator::validateProperties(
        const Document& document,
        const Document& schema,
        std::string_view path,
        ValidationResult& result,
        size_t currentDepth ) const
    {
        if( !schema.contains( vocabulary::APPLICATOR_PROPERTIES ) )
        {
            return;
        }

        // Get the properties object from schema
        auto propertiesObjOpt = schema.get<Object>( vocabulary::APPLICATOR_PROPERTIES );
        if( !propertiesObjOpt.has_value() )
        {
            return;
        }

        Object propertiesObj = propertiesObjOpt.value();

        // Validate each property defined in the schema using iterator
        for( const auto& [propertyName, propertySchema] : propertiesObj )
        {
            // Build the path for this property in the document
            std::string propertyPath = path.empty() ? propertyName : std::string{ path } + "." + propertyName;

            // Check if the property exists in the document
            if( document.contains( propertyPath ) )
            {
                // Validate the property against its schema, incrementing depth
                validateNode( document, propertySchema, propertyPath, result, currentDepth + 1 );
            }
            // Note: Missing properties are handled by validateRequired()
        }

        // Check additionalProperties constraint
        if( schema.contains( vocabulary::APPLICATOR_ADDITIONAL_PROPERTIES ) &&
            schema.is<bool>( vocabulary::APPLICATOR_ADDITIONAL_PROPERTIES ) )
        {
            auto additionalProps = schema.get<bool>( vocabulary::APPLICATOR_ADDITIONAL_PROPERTIES );
            if( additionalProps.has_value() && !additionalProps.value() )
            {
                // additionalProperties: false - strict validation
                // We need to check if document has properties not in schema
                validateAdditionalProperties( document, schema, path, result );
            }
        }
    }

    void SchemaValidator::validateNumericConstraints(
        const Document& document, const Document& schema, std::string_view path, ValidationResult& result ) const
    {
        std::optional<double> value;
        if( document.is<int>( path ) )
        {
            auto intVal = document.get<int64_t>( path );
            if( intVal.has_value() )
            {
                value = static_cast<double>( intVal.value() );
            }
        }
        else if( document.is<double>( path ) )
        {
            value = document.get<double>( path );
        }

        if( !value.has_value() )
        {
            return; // Not a number
        }

        double val = value.value();

        // Validate minimum (inclusive: value >= minimum)
        if( schema.contains( vocabulary::VALIDATION_MINIMUM ) &&
            ( schema.is<double>( vocabulary::VALIDATION_MINIMUM ) ||
              schema.is<int>( vocabulary::VALIDATION_MINIMUM ) ) )
        {
            double minimum;
            if( schema.is<double>( vocabulary::VALIDATION_MINIMUM ) )
            {
                auto minOpt = schema.get<double>( vocabulary::VALIDATION_MINIMUM );
                if( !minOpt.has_value() )
                    return;
                minimum = minOpt.value();
            }
            else
            {
                auto minOpt = schema.get<int64_t>( vocabulary::VALIDATION_MINIMUM );
                if( !minOpt.has_value() )
                    return;
                minimum = static_cast<double>( minOpt.value() );
            }
            if( val < minimum )
            {
                result.addError(
                    path,
                    "Value below minimum",
                    vocabulary::VALIDATION_MINIMUM,
                    std::to_string( minimum ),
                    std::to_string( val ) );
            }
        }

        // Validate exclusiveMinimum (value > exclusiveMinimum)
        if( schema.contains( vocabulary::VALIDATION_EXCLUSIVE_MINIMUM ) &&
            ( schema.is<double>( vocabulary::VALIDATION_EXCLUSIVE_MINIMUM ) ||
              schema.is<int>( vocabulary::VALIDATION_EXCLUSIVE_MINIMUM ) ) )
        {
            double exclMin;
            if( schema.is<double>( vocabulary::VALIDATION_EXCLUSIVE_MINIMUM ) )
            {
                auto minOpt = schema.get<double>( vocabulary::VALIDATION_EXCLUSIVE_MINIMUM );
                if( !minOpt.has_value() )
                    return;
                exclMin = minOpt.value();
            }
            else
            {
                auto minOpt = schema.get<int64_t>( vocabulary::VALIDATION_EXCLUSIVE_MINIMUM );
                if( !minOpt.has_value() )
                    return;
                exclMin = static_cast<double>( minOpt.value() );
            }
            if( val <= exclMin )
            {
                result.addError(
                    path,
                    "Value must be greater than exclusiveMinimum",
                    vocabulary::VALIDATION_EXCLUSIVE_MINIMUM,
                    std::to_string( exclMin ),
                    std::to_string( val ) );
            }
        }

        // Validate maximum (inclusive: value <= maximum)
        if( schema.contains( vocabulary::VALIDATION_MAXIMUM ) &&
            ( schema.is<double>( vocabulary::VALIDATION_MAXIMUM ) ||
              schema.is<int>( vocabulary::VALIDATION_MAXIMUM ) ) )
        {
            double maximum;
            if( schema.is<double>( vocabulary::VALIDATION_MAXIMUM ) )
            {
                auto maxOpt = schema.get<double>( vocabulary::VALIDATION_MAXIMUM );
                if( !maxOpt.has_value() )
                    return;
                maximum = maxOpt.value();
            }
            else
            {
                auto maxOpt = schema.get<int64_t>( vocabulary::VALIDATION_MAXIMUM );
                if( !maxOpt.has_value() )
                    return;
                maximum = static_cast<double>( maxOpt.value() );
            }
            if( val > maximum )
            {
                result.addError(
                    path,
                    "Value above maximum",
                    vocabulary::VALIDATION_MAXIMUM,
                    std::to_string( maximum ),
                    std::to_string( val ) );
            }
        }

        // Validate exclusiveMaximum (value < exclusiveMaximum)
        if( schema.contains( vocabulary::VALIDATION_EXCLUSIVE_MAXIMUM ) &&
            ( schema.is<double>( vocabulary::VALIDATION_EXCLUSIVE_MAXIMUM ) ||
              schema.is<int>( vocabulary::VALIDATION_EXCLUSIVE_MAXIMUM ) ) )
        {
            double exclMax;
            if( schema.is<double>( vocabulary::VALIDATION_EXCLUSIVE_MAXIMUM ) )
            {
                auto maxOpt = schema.get<double>( vocabulary::VALIDATION_EXCLUSIVE_MAXIMUM );
                if( !maxOpt.has_value() )
                    return;
                exclMax = maxOpt.value();
            }
            else
            {
                auto maxOpt = schema.get<int64_t>( vocabulary::VALIDATION_EXCLUSIVE_MAXIMUM );
                if( !maxOpt.has_value() )
                    return;
                exclMax = static_cast<double>( maxOpt.value() );
            }
            if( val >= exclMax )
            {
                result.addError(
                    path,
                    "Value must be less than exclusiveMaximum",
                    vocabulary::VALIDATION_EXCLUSIVE_MAXIMUM,
                    std::to_string( exclMax ),
                    std::to_string( val ) );
            }
        }

        // Validate multipleOf (value must be divisible by multipleOf)
        if( schema.contains( vocabulary::VALIDATION_MULTIPLE_OF ) &&
            ( schema.is<double>( vocabulary::VALIDATION_MULTIPLE_OF ) ||
              schema.is<int>( vocabulary::VALIDATION_MULTIPLE_OF ) ) )
        {
            double divisor;
            if( schema.is<double>( vocabulary::VALIDATION_MULTIPLE_OF ) )
            {
                auto divOpt = schema.get<double>( vocabulary::VALIDATION_MULTIPLE_OF );
                if( !divOpt.has_value() || divOpt.value() <= 0 )
                    return;
                divisor = divOpt.value();
            }
            else
            {
                auto divOpt = schema.get<int64_t>( vocabulary::VALIDATION_MULTIPLE_OF );
                if( !divOpt.has_value() || divOpt.value() <= 0 )
                    return;
                divisor = static_cast<double>( divOpt.value() );
            }

            // Check if value is a multiple of divisor (with floating point tolerance)
            double quotient = val / divisor;
            double remainder = quotient - std::floor( quotient );
            constexpr double epsilon = 1e-10;

            if( remainder > epsilon && remainder < ( 1.0 - epsilon ) )
            {
                result.addError(
                    path,
                    "Value is not a multiple of constraint",
                    vocabulary::VALIDATION_MULTIPLE_OF,
                    std::to_string( divisor ),
                    std::to_string( val ) );
            }
        }
    }

    void SchemaValidator::validateStringConstraints(
        const Document& document, const Document& schema, std::string_view path, ValidationResult& result ) const
    {
        if( !document.is<std::string>( path ) )
        {
            return; // Not a string
        }

        auto valueOpt = document.get<std::string>( path );
        if( !valueOpt.has_value() )
        {
            return; // No value
        }
        const std::string& value = valueOpt.value();

        // Validate minLength
        if( schema.contains( vocabulary::VALIDATION_MIN_LENGTH ) &&
            schema.is<int>( vocabulary::VALIDATION_MIN_LENGTH ) )
        {
            auto minLengthOpt = schema.get<int64_t>( vocabulary::VALIDATION_MIN_LENGTH );
            if( minLengthOpt.has_value() )
            {
                int64_t minLength = minLengthOpt.value();
                if( minLength > 0 && value.length() < static_cast<size_t>( minLength ) )
                {
                    result.addError(
                        path,
                        "String too short",
                        vocabulary::VALIDATION_MIN_LENGTH,
                        std::to_string( minLength ),
                        std::to_string( value.length() ) );
                }
            }
        }

        // Validate maxLength
        if( schema.contains( vocabulary::VALIDATION_MAX_LENGTH ) &&
            schema.is<int>( vocabulary::VALIDATION_MAX_LENGTH ) )
        {
            auto maxLengthOpt = schema.get<int64_t>( vocabulary::VALIDATION_MAX_LENGTH );
            if( maxLengthOpt.has_value() )
            {
                int64_t maxLength = maxLengthOpt.value();
                if( maxLength > 0 && value.length() > static_cast<size_t>( maxLength ) )
                {
                    result.addError(
                        path,
                        "String too long",
                        vocabulary::VALIDATION_MAX_LENGTH,
                        std::to_string( maxLength ),
                        std::to_string( value.length() ) );
                }
            }
        }

        // Validate pattern (regex)
        if( schema.contains( vocabulary::VALIDATION_PATTERN ) )
        {
            auto patternOpt = schema.get<std::string>( vocabulary::VALIDATION_PATTERN );
            if( patternOpt.has_value() )
            {
                try
                {
                    std::regex patternRegex( patternOpt.value() );
                    if( !std::regex_search( value, patternRegex ) )
                    {
                        result.addError(
                            path,
                            "String does not match pattern",
                            vocabulary::VALIDATION_PATTERN,
                            patternOpt.value(),
                            value );
                    }
                }
                catch( const std::regex_error& e )
                {
                    result.addError(
                        path,
                        "Invalid regex pattern in schema: " + std::string( e.what() ),
                        vocabulary::VALIDATION_PATTERN,
                        patternOpt.value(),
                        "" );
                }
            }
        }

        // Validate format
        if( schema.contains( vocabulary::FORMAT_ANNOTATION ) )
        {
            auto format = schema.get<std::string>( vocabulary::FORMAT_ANNOTATION );
            bool formatValid = true;
            std::string expectedFormat;

            if( format == vocabulary::FORMAT_DATETIME )
            {
                formatValid = nfx::string::isDateTime( value );
                expectedFormat = "RFC 3339 date-time (e.g., 2025-11-29T14:30:00Z)";
            }
            else if( format == vocabulary::FORMAT_DATE )
            {
                formatValid = nfx::string::isDate( value );
                expectedFormat = "RFC 3339 full-date (e.g., 2025-11-29)";
            }
            else if( format == vocabulary::FORMAT_TIME )
            {
                formatValid = nfx::string::isTime( value );
                expectedFormat = "RFC 3339 full-time (e.g., 14:30:00Z)";
            }
            else if( format == vocabulary::FORMAT_DURATION )
            {
                formatValid = nfx::string::isDuration( value );
                expectedFormat = "ISO 8601 duration (e.g., P3Y6M4DT12H30M5S)";
            }
            else if( format == vocabulary::FORMAT_EMAIL )
            {
                formatValid = nfx::string::isEmail( value );
                expectedFormat = "RFC 5321 email (e.g., user@example.com)";
            }
            else if( format == vocabulary::FORMAT_IDN_EMAIL )
            {
                formatValid = nfx::string::isIdnEmail( value );
                expectedFormat = "RFC 6531 internationalized email";
            }
            else if( format == vocabulary::FORMAT_HOSTNAME )
            {
                formatValid = nfx::string::isHostname( value );
                expectedFormat = "RFC 1123 hostname (e.g., example.com)";
            }
            else if( format == vocabulary::FORMAT_IDN_HOSTNAME )
            {
                formatValid = nfx::string::isIdnHostname( value );
                expectedFormat = "RFC 5890 internationalized hostname";
            }
            else if( format == vocabulary::FORMAT_IPV4 )
            {
                formatValid = nfx::string::isIpv4Address( value );
                expectedFormat = "RFC 2673 IPv4 (e.g., 192.168.1.1)";
            }
            else if( format == vocabulary::FORMAT_IPV6 )
            {
                formatValid = nfx::string::isIpv6Address( value );
                expectedFormat = "RFC 4291 IPv6 (e.g., 2001:db8::1)";
            }
            else if( format == vocabulary::FORMAT_URI )
            {
                formatValid = nfx::string::isUri( value );
                expectedFormat = "RFC 3986 URI (e.g., https://example.com/path)";
            }
            else if( format == vocabulary::FORMAT_URI_REFERENCE )
            {
                formatValid = nfx::string::isUriReference( value );
                expectedFormat = "RFC 3986 URI-reference";
            }
            else if( format == vocabulary::FORMAT_IRI )
            {
                formatValid = nfx::string::isIri( value );
                expectedFormat = "RFC 3987 IRI";
            }
            else if( format == vocabulary::FORMAT_IRI_REFERENCE )
            {
                formatValid = nfx::string::isIriReference( value );
                expectedFormat = "RFC 3987 IRI-reference";
            }
            else if( format == vocabulary::FORMAT_UUID )
            {
                formatValid = nfx::string::isUuid( value );
                expectedFormat = "RFC 4122 UUID (e.g., 550e8400-e29b-41d4-a716-446655440000)";
            }
            else if( format == vocabulary::FORMAT_URI_TEMPLATE )
            {
                formatValid = nfx::string::isUriTemplate( value );
                expectedFormat = "RFC 6570 URI Template";
            }
            else if( format == vocabulary::FORMAT_JSON_POINTER )
            {
                formatValid = nfx::string::isJsonPointer( value );
                expectedFormat = "RFC 6901 JSON Pointer (e.g., /foo/bar/0)";
            }
            else if( format == vocabulary::FORMAT_RELATIVE_JSON_POINTER )
            {
                formatValid = nfx::string::isRelativeJsonPointer( value );
                expectedFormat = "Relative JSON Pointer (e.g., 1/foo)";
            }
            else if( format == vocabulary::FORMAT_REGEX )
            {
                // ECMA-262 regex: try to compile it
                try
                {
                    std::regex testRegex( value );
                    formatValid = true;
                }
                catch( const std::regex_error& )
                {
                    formatValid = false;
                }
                expectedFormat = "ECMA-262 regular expression";
            }

            if( !formatValid && !expectedFormat.empty() )
            {
                result.addError( path, "Invalid format", vocabulary::FORMAT_ANNOTATION, expectedFormat, value );
            }
        }
    }

    void SchemaValidator::validateEnum(
        const Document& document, const Document& schema, std::string_view path, ValidationResult& result ) const
    {
        // Extract the value at the path from the document
        Document docValue = path.empty() ? document : extractSubDocument( document, path );
        if( !docValue.isValid() )
        {
            return; // Path not found, other validation will catch this
        }

        // Get the enum array from schema
        auto enumArrayOpt = schema.get<Array>( vocabulary::VALIDATION_ENUM );
        if( !enumArrayOpt.has_value() )
        {
            return; // Invalid enum definition
        }

        const Array& enumArray = enumArrayOpt.value();

        // Check if document value matches any enum value
        bool found = false;
        for( size_t i = 0; i < enumArray.size(); ++i )
        {
            const Document& enumValue = enumArray[i];
            if( docValue == enumValue )
            {
                found = true;
                break;
            }
        }

        if( !found )
        {
            // Build allowed values string for error message
            std::string allowedValues;
            for( size_t i = 0; i < enumArray.size(); ++i )
            {
                if( i > 0 )
                    allowedValues += ", ";
                allowedValues += enumArray[i].toString();
            }
            result.addError(
                path, "Value not in enum", vocabulary::VALIDATION_ENUM, allowedValues, docValue.toString() );
        }
    }

    void SchemaValidator::validateConst(
        const Document& document, const Document& schema, std::string_view path, ValidationResult& result ) const
    {
        // Extract the value at the path from the document
        Document docValue = path.empty() ? document : extractSubDocument( document, path );
        if( !docValue.isValid() )
        {
            return; // Path not found, other validation will catch this
        }

        // Get the const value from schema
        Document constSchema = extractSubDocument( schema, vocabulary::VALIDATION_CONST );
        if( !constSchema.isValid() )
        {
            return; // Invalid const definition
        }

        // Compare values using Document equality
        if( docValue != constSchema )
        {
            result.addError(
                path,
                "Value does not match const",
                vocabulary::VALIDATION_CONST,
                constSchema.toString(),
                docValue.toString() );
        }
    }

    bool SchemaValidator::referenceExists( std::string_view reference ) const noexcept
    {
        if( reference.empty() || reference[0] != '#' )
        {
            // External references (HTTP/file URLs) not supported
            return false;
        }

        if( !m_schemaLoaded )
        {
            return false;
        }

        // Extract JSON Pointer from reference (remove leading '#')
        // e.g., "#/definitions/Package" -> "/definitions/Package"
        if( reference.starts_with( "#/" ) )
        {
            std::string_view jsonPointer = reference.substr( 1 ); // Keep the leading '/'
            return m_schema.contains( jsonPointer );
        }

        return false; // Invalid reference format
    }

    Document SchemaValidator::resolveReference( std::string_view reference ) const
    {
        if( reference.empty() || reference[0] != '#' )
        {
            // External references (HTTP/file URLs) not supported
            return Document{};
        }

        // Check cache first
        std::string refKey{ reference };
        auto cacheIt = m_refCache.find( refKey );
        if( cacheIt != m_refCache.end() )
        {
            return cacheIt->second;
        }

        Document resolved;

        // Handle JSON Pointer reference (e.g., "#/definitions/Package")
        if( reference.starts_with( "#/" ) )
        {
            std::string_view jsonPointer = reference.substr( 1 ); // Keep the leading '/'

            if( m_schemaLoaded && m_schema.contains( jsonPointer ) )
            {
                resolved = extractSubDocument( m_schema, jsonPointer );
            }
        }
        // Handle anchor reference (e.g., "#myAnchor") - no '/' after '#'
        else if( reference.length() > 1 )
        {
            std::string anchorName{ reference.substr( 1 ) }; // Remove leading '#'

            // Look up anchor in our index
            auto anchorIt = m_anchorIndex.find( anchorName );
            if( anchorIt != m_anchorIndex.end() )
            {
                const std::string& anchorPath = anchorIt->second;

                if( anchorPath.empty() )
                {
                    // Root schema has the anchor
                    resolved = m_schema;
                }
                else if( m_schemaLoaded && m_schema.contains( anchorPath ) )
                {
                    resolved = extractSubDocument( m_schema, anchorPath );
                }
            }
        }

        // Cache the result (only valid resolutions)
        if( resolved.isValid() )
        {
            m_refCache[refKey] = resolved;
        }

        return resolved;
    }

    std::string SchemaValidator::actualType( const Document& document, std::string_view path ) const noexcept
    {
        if( document.is<Object>( path ) )
        {
            return "object";
        }
        if( document.is<Array>( path ) )
        {
            return "array";
        }
        if( document.is<std::string>( path ) )
        {
            return "string";
        }
        if( document.is<int>( path ) )
        {
            return "integer";
        }
        if( document.is<double>( path ) )
        {
            return "number";
        }
        if( document.is<bool>( path ) )
        {
            return "boolean";
        }
        if( document.isNull( path ) )
        {
            return "null";
        }
        return "unknown";
    }

    void SchemaValidator::validateAdditionalProperties(
        const Document& document, const Document& schema, std::string_view path, ValidationResult& result ) const
    {
        // Get all properties defined in the schema
        std::set<std::string> schemaProperties;

        auto propertiesObjOpt = schema.get<Object>( "properties" );
        if( propertiesObjOpt.has_value() )
        {
            for( const auto& [key, value] : propertiesObjOpt.value() )
            {
                schemaProperties.insert( key );
            }
        }

        // Check all properties in document against schema
        auto docObjOpt = document.get<Object>( path );
        if( docObjOpt.has_value() )
        {
            for( const auto& [propertyName, value] : docObjOpt.value() )
            {
                // If property is not in schema and additionalProperties is false, it's an error
                if( schemaProperties.find( propertyName ) == schemaProperties.end() )
                {
                    std::string propertyPath = path.empty() ? propertyName : std::string{ path } + "." + propertyName;
                    result.addError(
                        propertyPath,
                        "Additional property not allowed",
                        vocabulary::APPLICATOR_ADDITIONAL_PROPERTIES,
                        "false",
                        propertyName );
                }
            }
        }
    }

    Document SchemaValidator::extractSubDocument( const Document& sourceDocument, std::string_view path ) const
    {
        // If path is empty, return the whole document
        if( path.empty() )
        {
            return sourceDocument;
        }

        // Use Document's public API to navigate to the path
        // First, try to get it as a Document (for objects/arrays/nested structures)
        if( sourceDocument.contains( path ) )
        {
            // Try to get as Object - use getRef<T>() for zero-copy!
            if( sourceDocument.is<Object>( path ) )
            {
                auto objRef = sourceDocument.getRef<Object>( path );
                if( objRef.has_value() )
                {
                    return Document{ objRef->get() }; // Single copy from reference
                }
            }
            // Try to get as Array - use getRef<T>() for zero-copy!
            else if( sourceDocument.is<Array>( path ) )
            {
                auto arrRef = sourceDocument.getRef<Array>( path );
                if( arrRef.has_value() )
                {
                    return Document{ arrRef->get() }; // Single copy from reference
                }
            }
            // Try primitives - get<T>() is fine for small values
            else if( sourceDocument.is<std::string>( path ) )
            {
                auto strOpt = sourceDocument.get<std::string>( path );
                if( strOpt.has_value() )
                {
                    return Document{ strOpt.value() };
                }
            }
            else if( sourceDocument.is<bool>( path ) )
            {
                auto boolOpt = sourceDocument.get<bool>( path );
                if( boolOpt.has_value() )
                {
                    return Document{ boolOpt.value() };
                }
            }
            else if( sourceDocument.is<int>( path ) )
            {
                auto intOpt = sourceDocument.get<int64_t>( path );
                if( intOpt.has_value() )
                {
                    return Document{ intOpt.value() };
                }
            }
            else if( sourceDocument.is<double>( path ) )
            {
                auto dblOpt = sourceDocument.get<double>( path );
                if( dblOpt.has_value() )
                {
                    return Document{ dblOpt.value() };
                }
            }
            else if( sourceDocument.isNull( path ) )
            {
                return Document{ nullptr };
            }
        }

        // Path not found or unsupported type
        return Document{};
    }

    void SchemaValidator::validateObject(
        const Document& document,
        const Document& schema,
        std::string_view path,
        ValidationResult& result,
        size_t currentDepth ) const
    {
        validateRequired( document, schema, path, result );
        validateProperties( document, schema, path, result, currentDepth );

        // Get document object for property iteration
        auto docObjOpt = document.get<Object>( path );
        if( !docObjOpt.has_value() )
        {
            return; // Not an object or path not found
        }
        const Object& docObj = docObjOpt.value();
        size_t propertyCount = docObj.size();

        // Validate minProperties
        if( schema.contains( vocabulary::VALIDATION_MIN_PROPERTIES ) &&
            schema.is<int>( vocabulary::VALIDATION_MIN_PROPERTIES ) )
        {
            auto minPropsOpt = schema.get<int64_t>( vocabulary::VALIDATION_MIN_PROPERTIES );
            if( minPropsOpt.has_value() )
            {
                int64_t minProps = minPropsOpt.value();
                if( minProps > 0 && propertyCount < static_cast<size_t>( minProps ) )
                {
                    result.addError(
                        path,
                        "Object has too few properties",
                        vocabulary::VALIDATION_MIN_PROPERTIES,
                        std::to_string( minProps ),
                        std::to_string( propertyCount ) );
                }
            }
        }

        // Validate maxProperties
        if( schema.contains( vocabulary::VALIDATION_MAX_PROPERTIES ) &&
            schema.is<int>( vocabulary::VALIDATION_MAX_PROPERTIES ) )
        {
            auto maxPropsOpt = schema.get<int64_t>( vocabulary::VALIDATION_MAX_PROPERTIES );
            if( maxPropsOpt.has_value() )
            {
                int64_t maxProps = maxPropsOpt.value();
                if( maxProps >= 0 && propertyCount > static_cast<size_t>( maxProps ) )
                {
                    result.addError(
                        path,
                        "Object has too many properties",
                        vocabulary::VALIDATION_MAX_PROPERTIES,
                        std::to_string( maxProps ),
                        std::to_string( propertyCount ) );
                }
            }
        }

        // Validate patternProperties - properties matching regex patterns
        if( schema.contains( vocabulary::APPLICATOR_PATTERN_PROPERTIES ) )
        {
            auto patternPropsOpt = schema.get<Object>( vocabulary::APPLICATOR_PATTERN_PROPERTIES );
            if( patternPropsOpt.has_value() )
            {
                const Object& patternProps = patternPropsOpt.value();

                // For each property in the document
                for( const auto& [propertyName, propertyValue] : docObj )
                {
                    // Check against each pattern
                    for( const auto& [pattern, patternSchema] : patternProps )
                    {
                        try
                        {
                            std::regex regex( pattern );
                            if( std::regex_search( propertyName, regex ) )
                            {
                                // Property matches pattern - validate against pattern schema
                                std::string propertyPath =
                                    path.empty() ? propertyName : std::string{ path } + "." + propertyName;
                                validateNode( document, patternSchema, propertyPath, result, currentDepth + 1 );
                            }
                        }
                        catch( const std::regex_error& )
                        {
                            // Invalid regex pattern - skip (could add warning)
                        }
                    }
                }
            }
        }

        // Validate propertyNames - all property names must match schema
        if( schema.contains( vocabulary::APPLICATOR_PROPERTY_NAMES ) )
        {
            Document propertyNamesSchema = extractSubDocument( schema, vocabulary::APPLICATOR_PROPERTY_NAMES );
            if( propertyNamesSchema.isValid() )
            {
                for( const auto& [propertyName, propertyValue] : docObj )
                {
                    // Create a temporary document with just the property name as a string
                    auto nameDocOpt = Document::fromString( "\"" + propertyName + "\"" );
                    if( nameDocOpt.has_value() )
                    {
                        // Validate the property name against the schema
                        ValidationResult nameResult;
                        validateNode( nameDocOpt.value(), propertyNamesSchema, "", nameResult, currentDepth + 1 );

                        if( nameResult.hasErrors() )
                        {
                            std::string propertyPath =
                                path.empty() ? propertyName : std::string{ path } + "." + propertyName;
                            result.addError(
                                propertyPath,
                                "Property name does not match schema",
                                vocabulary::APPLICATOR_PROPERTY_NAMES,
                                "valid property name",
                                propertyName );
                        }
                    }
                }
            }
        }

        // Validate dependentRequired - if property exists, other properties must exist
        if( schema.contains( vocabulary::VALIDATION_DEPENDENT_REQUIRED ) )
        {
            auto dependentReqOpt = schema.get<Object>( vocabulary::VALIDATION_DEPENDENT_REQUIRED );
            if( dependentReqOpt.has_value() )
            {
                const Object& dependentReq = dependentReqOpt.value();

                for( const auto& [triggerProperty, requiredArray] : dependentReq )
                {
                    // Check if trigger property exists in document
                    std::string triggerPath =
                        path.empty() ? triggerProperty : std::string{ path } + "." + triggerProperty;
                    if( document.contains( triggerPath ) )
                    {
                        // Trigger exists - check that all dependent properties exist
                        if( requiredArray.is<Array>( "" ) )
                        {
                            auto reqArrayOpt = requiredArray.get<Array>( "" );
                            if( reqArrayOpt.has_value() )
                            {
                                const Array& reqArr = reqArrayOpt.value();
                                for( size_t i = 0; i < reqArr.size(); ++i )
                                {
                                    auto depPropOpt = reqArr[i].get<std::string>( "" );
                                    if( depPropOpt.has_value() )
                                    {
                                        std::string depPath = path.empty()
                                                                  ? depPropOpt.value()
                                                                  : std::string{ path } + "." + depPropOpt.value();
                                        if( !document.contains( depPath ) )
                                        {
                                            result.addError(
                                                depPath,
                                                "Dependent property required when '" + triggerProperty + "' is present",
                                                vocabulary::VALIDATION_DEPENDENT_REQUIRED,
                                                depPropOpt.value(),
                                                "undefined" );
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Validate dependentSchemas - if property exists, apply additional schema
        if( schema.contains( vocabulary::APPLICATOR_DEPENDENT_SCHEMAS ) )
        {
            auto dependentSchemasOpt = schema.get<Object>( vocabulary::APPLICATOR_DEPENDENT_SCHEMAS );
            if( dependentSchemasOpt.has_value() )
            {
                const Object& dependentSchemas = dependentSchemasOpt.value();

                for( const auto& [triggerProperty, additionalSchema] : dependentSchemas )
                {
                    // Check if trigger property exists in document
                    std::string triggerPath =
                        path.empty() ? triggerProperty : std::string{ path } + "." + triggerProperty;
                    if( document.contains( triggerPath ) )
                    {
                        // Trigger exists - validate document against additional schema
                        validateNode( document, additionalSchema, path, result, currentDepth + 1 );
                    }
                }
            }
        }
    }

    void SchemaValidator::validateArray(
        const Document& document,
        const Document& schema,
        std::string_view path,
        ValidationResult& result,
        size_t currentDepth ) const
    {
        if( !document.is<Array>( path ) )
        {
            return; // Not an array, type validation will catch this
        }

        auto documentArray = document.get<Array>( path );
        if( !documentArray.has_value() )
        {
            return;
        }
        size_t arraySize = documentArray.value().size();

        // Validate minItems
        if( schema.contains( vocabulary::VALIDATION_MIN_ITEMS ) && schema.is<int>( vocabulary::VALIDATION_MIN_ITEMS ) )
        {
            auto minItemsOpt = schema.get<int64_t>( vocabulary::VALIDATION_MIN_ITEMS );
            if( minItemsOpt.has_value() )
            {
                int64_t minItems = minItemsOpt.value();
                if( minItems > 0 && arraySize < static_cast<size_t>( minItems ) )
                {
                    result.addError(
                        path,
                        "Array has too few items",
                        vocabulary::VALIDATION_MIN_ITEMS,
                        std::to_string( minItems ),
                        std::to_string( arraySize ) );
                }
            }
        }

        // Validate maxItems
        if( schema.contains( vocabulary::VALIDATION_MAX_ITEMS ) && schema.is<int>( vocabulary::VALIDATION_MAX_ITEMS ) )
        {
            auto maxItemsOpt = schema.get<int64_t>( vocabulary::VALIDATION_MAX_ITEMS );
            if( maxItemsOpt.has_value() )
            {
                int64_t maxItems = maxItemsOpt.value();
                if( maxItems > 0 && arraySize > static_cast<size_t>( maxItems ) )
                {
                    result.addError(
                        path,
                        "Array has too many items",
                        vocabulary::VALIDATION_MAX_ITEMS,
                        std::to_string( maxItems ),
                        std::to_string( arraySize ) );
                }
            }
        }

        // Track where prefixItems validation ends (for items validation)
        size_t prefixItemsCount = 0;

        // Validate prefixItems (Draft 2020-12) - tuple validation for array prefix
        if( schema.contains( vocabulary::APPLICATOR_PREFIX_ITEMS ) )
        {
            auto prefixItemsOpt = schema.get<Array>( vocabulary::APPLICATOR_PREFIX_ITEMS );
            if( prefixItemsOpt.has_value() )
            {
                const Array& prefixItems = prefixItemsOpt.value();
                prefixItemsCount = prefixItems.size();

                for( size_t i = 0; i < prefixItemsCount && i < arraySize; ++i )
                {
                    const Document& itemSchema = prefixItems[i];
                    std::string itemPath = std::string{ path } + "[" + std::to_string( i ) + "]";
                    validateNode( document, itemSchema, itemPath, result, currentDepth + 1 );
                }
            }
        }

        // Validate items - applies to all items (or items after prefixItems in Draft 2020-12)
        if( schema.contains( vocabulary::APPLICATOR_ITEMS ) )
        {
            // Extract the items schema definition
            Document itemsSchema = extractSubDocument( schema, vocabulary::APPLICATOR_ITEMS );

            // If items schema has a $ref, resolve it
            if( itemsSchema.contains( vocabulary::CORE_REF ) )
            {
                auto itemsRef = itemsSchema.get<std::string>( vocabulary::CORE_REF );
                if( itemsRef.has_value() )
                {
                    itemsSchema = resolveReference( itemsRef.value() );
                }
            }

            // In Draft 2020-12, items applies after prefixItems
            // In older drafts, items applies to all elements
            size_t startIndex = schema.contains( vocabulary::APPLICATOR_PREFIX_ITEMS ) ? prefixItemsCount : 0;

            for( size_t i = startIndex; i < arraySize; ++i )
            {
                std::string itemPath = std::string{ path } + "[" + std::to_string( i ) + "]";
                validateNode( document, itemsSchema, itemPath, result, currentDepth + 1 );
            }
        }

        // Validate contains, minContains, maxContains
        if( schema.contains( vocabulary::APPLICATOR_CONTAINS ) )
        {
            Document containsSchema = extractSubDocument( schema, vocabulary::APPLICATOR_CONTAINS );
            if( containsSchema.isValid() )
            {
                // Count how many items match the contains schema
                size_t matchCount = 0;

                for( size_t i = 0; i < arraySize; ++i )
                {
                    std::string itemPath = std::string{ path } + "[" + std::to_string( i ) + "]";
                    ValidationResult itemResult;
                    validateNode( document, containsSchema, itemPath, itemResult, currentDepth + 1 );

                    if( !itemResult.hasErrors() )
                    {
                        ++matchCount;
                    }
                }

                // Get minContains (default 1) and maxContains (default unlimited)
                int64_t minContains = 1;
                int64_t maxContains = -1; // -1 means no limit

                if( schema.contains( vocabulary::VALIDATION_MIN_CONTAINS ) &&
                    schema.is<int>( vocabulary::VALIDATION_MIN_CONTAINS ) )
                {
                    auto minOpt = schema.get<int64_t>( vocabulary::VALIDATION_MIN_CONTAINS );
                    if( minOpt.has_value() )
                    {
                        minContains = minOpt.value();
                    }
                }

                if( schema.contains( vocabulary::VALIDATION_MAX_CONTAINS ) &&
                    schema.is<int>( vocabulary::VALIDATION_MAX_CONTAINS ) )
                {
                    auto maxOpt = schema.get<int64_t>( vocabulary::VALIDATION_MAX_CONTAINS );
                    if( maxOpt.has_value() )
                    {
                        maxContains = maxOpt.value();
                    }
                }

                // Validate minContains
                if( matchCount < static_cast<size_t>( minContains ) )
                {
                    result.addError(
                        path,
                        "Array does not contain enough matching items",
                        vocabulary::APPLICATOR_CONTAINS,
                        "at least " + std::to_string( minContains ) + " matching item(s)",
                        std::to_string( matchCount ) + " matching item(s)" );
                }

                // Validate maxContains
                if( maxContains >= 0 && matchCount > static_cast<size_t>( maxContains ) )
                {
                    result.addError(
                        path,
                        "Array contains too many matching items",
                        vocabulary::VALIDATION_MAX_CONTAINS,
                        "at most " + std::to_string( maxContains ) + " matching item(s)",
                        std::to_string( matchCount ) + " matching item(s)" );
                }
            }
        }

        // Validate uniqueItems
        if( schema.contains( vocabulary::VALIDATION_UNIQUE_ITEMS ) &&
            schema.is<bool>( vocabulary::VALIDATION_UNIQUE_ITEMS ) )
        {
            auto uniqueItemsOpt = schema.get<bool>( vocabulary::VALIDATION_UNIQUE_ITEMS );
            if( uniqueItemsOpt.value_or( false ) )
            {
                // Check for duplicate items using O(n) hash-based approach
                const Array& arr = documentArray.value();
                nfx::containers::FastHashSet<Document, uint32_t> seen;
                seen.reserve( arr.size() );

                for( size_t i = 0; i < arr.size(); ++i )
                {
                    bool inserted = seen.insert( arr[i] );
                    if( !inserted )
                    {
                        // Find the original index of the duplicate (for error reporting)
                        size_t originalIndex = 0;
                        for( size_t j = 0; j < i; ++j )
                        {
                            if( arr[j] == arr[i] )
                            {
                                originalIndex = j;
                                break;
                            }
                        }

                        result.addError(
                            path,
                            "Array contains duplicate items",
                            vocabulary::VALIDATION_UNIQUE_ITEMS,
                            "true",
                            "duplicate at indices " + std::to_string( originalIndex ) + " and " + std::to_string( i ) );

                        // Report first duplicate only
                        break;
                    }
                }
            }
        }
    }

    //----------------------------------------------
    // Applicator validation (allOf, anyOf, oneOf, not)
    //----------------------------------------------

    void SchemaValidator::validateAllOf(
        const Document& document,
        const Document& schema,
        std::string_view path,
        ValidationResult& result,
        size_t currentDepth ) const
    {
        // allOf: instance must validate against ALL schemas in the array
        auto allOfOpt = schema.get<Array>( vocabulary::APPLICATOR_ALL_OF );
        if( !allOfOpt.has_value() )
        {
            return;
        }

        const Array& allOfArray = allOfOpt.value();
        size_t schemaCount = allOfArray.size();

        for( size_t i = 0; i < schemaCount; ++i )
        {
            const Document& subSchema = allOfArray[i];
            validateNode( document, subSchema, path, result, currentDepth + 1 );
            // Continue validating all schemas even if one fails (collect all errors)
        }
    }

    void SchemaValidator::validateAnyOf(
        const Document& document,
        const Document& schema,
        std::string_view path,
        ValidationResult& result,
        size_t currentDepth ) const
    {
        // anyOf: instance must validate against AT LEAST ONE schema in the array
        auto anyOfOpt = schema.get<Array>( vocabulary::APPLICATOR_ANY_OF );
        if( !anyOfOpt.has_value() )
        {
            return;
        }

        const Array& anyOfArray = anyOfOpt.value();
        size_t schemaCount = anyOfArray.size();

        bool anyValid = false;
        std::vector<std::string> allErrors;

        for( size_t i = 0; i < schemaCount; ++i )
        {
            const Document& subSchema = anyOfArray[i];
            ValidationResult subResult;
            validateNode( document, subSchema, path, subResult, currentDepth + 1 );

            if( !subResult.hasErrors() )
            {
                anyValid = true;
                break; // Found a valid schema, no need to continue
            }
            else
            {
                // Collect errors for diagnostic purposes
                for( const auto& error : subResult.errors() )
                {
                    allErrors.push_back( "Schema " + std::to_string( i ) + ": " + error.message() );
                }
            }
        }

        if( !anyValid )
        {
            result.addError(
                path,
                "Value does not match any of the schemas in anyOf",
                vocabulary::APPLICATOR_ANY_OF,
                "match at least one schema",
                "matched none" );
        }
    }

    void SchemaValidator::validateOneOf(
        const Document& document,
        const Document& schema,
        std::string_view path,
        ValidationResult& result,
        size_t currentDepth ) const
    {
        // oneOf: instance must validate against EXACTLY ONE schema in the array
        auto oneOfOpt = schema.get<Array>( vocabulary::APPLICATOR_ONE_OF );
        if( !oneOfOpt.has_value() )
        {
            return;
        }

        const Array& oneOfArray = oneOfOpt.value();
        size_t schemaCount = oneOfArray.size();

        size_t validCount = 0;
        std::vector<size_t> validIndices;

        for( size_t i = 0; i < schemaCount; ++i )
        {
            const Document& subSchema = oneOfArray[i];
            ValidationResult subResult;
            validateNode( document, subSchema, path, subResult, currentDepth + 1 );

            if( !subResult.hasErrors() )
            {
                ++validCount;
                validIndices.push_back( i );
            }
        }
        if( validCount == 0 )
        {
            result.addError(
                path,
                "Value does not match any of the schemas in oneOf",
                vocabulary::APPLICATOR_ONE_OF,
                "match exactly one schema",
                "matched none" );
        }
        else if( validCount > 1 )
        {
            std::string matchedStr;
            for( size_t i = 0; i < validIndices.size(); ++i )
            {
                if( i > 0 )
                    matchedStr += ", ";
                matchedStr += std::to_string( validIndices[i] );
            }
            result.addError(
                path,
                "Value matches multiple schemas in oneOf",
                vocabulary::APPLICATOR_ONE_OF,
                "match exactly one schema",
                "matched schemas: " + matchedStr );
        }
        // validCount == 1 is success, no error added
    }

    void SchemaValidator::validateNot(
        const Document& document,
        const Document& schema,
        std::string_view path,
        ValidationResult& result,
        size_t currentDepth ) const
    {
        // not: instance must NOT validate against the schema
        Document notSchema = extractSubDocument( schema, vocabulary::APPLICATOR_NOT );
        if( !notSchema.isValid() )
        {
            return;
        }

        ValidationResult subResult;
        validateNode( document, notSchema, path, subResult, currentDepth + 1 );

        if( !subResult.hasErrors() )
        {
            // The instance validated against the "not" schema, which means it FAILED
            result.addError(
                path,
                "Value must not match the schema in 'not'",
                vocabulary::APPLICATOR_NOT,
                "not match schema",
                "matched schema" );
        }
        // If subResult has errors, the instance did NOT validate against the schema, which is success
    }

    void SchemaValidator::validateIfThenElse(
        const Document& document,
        const Document& schema,
        std::string_view path,
        ValidationResult& result,
        size_t currentDepth ) const
    {
        // if/then/else: conditional validation
        // - If document validates against "if" schema, validate against "then" schema
        // - If document does NOT validate against "if" schema, validate against "else" schema (if present)

        Document ifSchema = extractSubDocument( schema, vocabulary::APPLICATOR_IF );
        if( !ifSchema.isValid() )
        {
            return;
        }

        // Test document against "if" schema
        ValidationResult ifResult;
        validateNode( document, ifSchema, path, ifResult, currentDepth + 1 );

        bool ifPassed = !ifResult.hasErrors();

        if( ifPassed )
        {
            // "if" matched - must validate against "then" (if present)
            if( schema.contains( vocabulary::APPLICATOR_THEN ) )
            {
                Document thenSchema = extractSubDocument( schema, vocabulary::APPLICATOR_THEN );
                if( thenSchema.isValid() )
                {
                    validateNode( document, thenSchema, path, result, currentDepth + 1 );
                }
            }
        }
        else
        {
            // "if" did not match - must validate against "else" (if present)
            if( schema.contains( vocabulary::APPLICATOR_ELSE ) )
            {
                Document elseSchema = extractSubDocument( schema, vocabulary::APPLICATOR_ELSE );
                if( elseSchema.isValid() )
                {
                    validateNode( document, elseSchema, path, result, currentDepth + 1 );
                }
            }
        }
    }

    //----------------------------------------------
    // Internal helper methods
    //----------------------------------------------

    void SchemaValidator::buildAnchorIndex()
    {
        if( !m_schemaLoaded || !m_schema.isValid() )
        {
            return;
        }

        // Start recursive scan from root
        scanForAnchors( m_schema, "" );
    }

    void SchemaValidator::scanForAnchors( const Document& schema, const std::string& currentPath )
    {
        // Only scan objects - arrays and primitives can't have $anchor
        if( !schema.is<Object>( "" ) )
        {
            return;
        }

        // Check if this node has $anchor
        if( schema.contains( vocabulary::CORE_ANCHOR ) )
        {
            auto anchorOpt = schema.get<std::string>( vocabulary::CORE_ANCHOR );
            if( anchorOpt.has_value() && !anchorOpt.value().empty() )
            {
                // Map anchor name to its JSON Pointer path
                m_anchorIndex[anchorOpt.value()] = currentPath;
            }
        }

        // Get schema as object to iterate over properties
        auto schemaObjOpt = schema.get<Object>( "" );
        if( !schemaObjOpt.has_value() )
        {
            return;
        }

        // Recursively scan all object properties
        // Note: Object iterator yields std::pair<std::string, Document>
        for( const auto& [key, value] : schemaObjOpt.value() )
        {
            // Build child path using JSON Pointer format
            std::string childPath = currentPath + "/" + key;

            // Handle special keys that contain object of schemas
            if( key == vocabulary::APPLICATOR_PROPERTIES || key == vocabulary::APPLICATOR_PATTERN_PROPERTIES ||
                key == vocabulary::CORE_DEFS || key == "definitions" ||
                key == vocabulary::APPLICATOR_DEPENDENT_SCHEMAS )
            {
                // These contain object of schemas - scan each sub-schema
                if( value.is<Object>( "" ) )
                {
                    auto subObjOpt = value.get<Object>( "" );
                    if( subObjOpt.has_value() )
                    {
                        for( const auto& [subKey, subValue] : subObjOpt.value() )
                        {
                            std::string subPath = childPath + "/" + subKey;
                            scanForAnchors( subValue, subPath );
                        }
                    }
                }
            }
            else if(
                key == vocabulary::APPLICATOR_ALL_OF || key == vocabulary::APPLICATOR_ANY_OF ||
                key == vocabulary::APPLICATOR_ONE_OF || key == vocabulary::APPLICATOR_PREFIX_ITEMS )
            {
                // These contain array of schemas - scan each
                if( value.is<Array>( "" ) )
                {
                    auto arrOpt = value.get<Array>( "" );
                    if( arrOpt.has_value() )
                    {
                        size_t arrSize = arrOpt.value().size();
                        for( size_t i = 0; i < arrSize; ++i )
                        {
                            std::string itemPath = childPath + "/" + std::to_string( i );
                            // Access item via extractSubDocument
                            Document itemDoc = extractSubDocument( value, "/" + std::to_string( i ) );
                            if( itemDoc.isValid() )
                            {
                                scanForAnchors( itemDoc, itemPath );
                            }
                        }
                    }
                }
            }
            else if(
                key == vocabulary::APPLICATOR_ITEMS || key == vocabulary::APPLICATOR_ADDITIONAL_PROPERTIES ||
                key == "additionalItems" || key == vocabulary::APPLICATOR_CONTAINS ||
                key == vocabulary::APPLICATOR_IF || key == vocabulary::APPLICATOR_THEN ||
                key == vocabulary::APPLICATOR_ELSE || key == vocabulary::APPLICATOR_NOT ||
                key == vocabulary::APPLICATOR_PROPERTY_NAMES )
            {
                // These are single schema values - scan directly
                if( value.is<Object>( "" ) )
                {
                    scanForAnchors( value, childPath );
                }
            }
            else if( value.is<Object>( "" ) )
            {
                // For any other object, check if it might be a schema
                scanForAnchors( value, childPath );
            }
        }
    }

    //----------------------------------------------
    // Schema draft detection
    //----------------------------------------------

    void SchemaValidator::detectDraft()
    {
        m_schemaDraft = SchemaDraft::Unknown;

        if( !m_schemaLoaded || !m_schema.isValid() )
        {
            return;
        }

        // Check for $schema keyword
        if( !m_schema.contains( vocabulary::CORE_SCHEMA ) )
        {
            return;
        }

        auto schemaUriOpt = m_schema.get<std::string>( vocabulary::CORE_SCHEMA );
        if( !schemaUriOpt.has_value() )
        {
            return;
        }

        const std::string& schemaUri = schemaUriOpt.value();

        // Detect draft from URI - check exact match first, then substring for flexibility
        if( schemaUri == vocabulary::SCHEMA_DRAFT_2020_12 || schemaUri.find( "2020-12" ) != std::string::npos )
        {
            m_schemaDraft = SchemaDraft::Draft202012;
        }
        else if( schemaUri == vocabulary::SCHEMA_DRAFT_2019_09 || schemaUri.find( "2019-09" ) != std::string::npos )
        {
            m_schemaDraft = SchemaDraft::Draft201909;
        }
        else if( schemaUri == vocabulary::SCHEMA_DRAFT_07 || schemaUri.find( "draft-07" ) != std::string::npos )
        {
            m_schemaDraft = SchemaDraft::Draft07;
        }
        else if( schemaUri == vocabulary::SCHEMA_DRAFT_06 || schemaUri.find( "draft-06" ) != std::string::npos )
        {
            m_schemaDraft = SchemaDraft::Draft06;
        }
        else if( schemaUri == vocabulary::SCHEMA_DRAFT_04 || schemaUri.find( "draft-04" ) != std::string::npos )
        {
            m_schemaDraft = SchemaDraft::Draft04;
        }
    }

    std::string SchemaValidator::draftToString() const noexcept
    {
        switch( m_schemaDraft )
        {
            case SchemaDraft::Draft04:
                return "draft-04";
            case SchemaDraft::Draft06:
                return "draft-06";
            case SchemaDraft::Draft07:
                return "draft-07";
            case SchemaDraft::Draft201909:
                return "2019-09";
            case SchemaDraft::Draft202012:
                return "2020-12";
            case SchemaDraft::Unknown:
            default:
                return "";
        }
    }
} // namespace nfx::json
