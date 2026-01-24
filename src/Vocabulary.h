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
 * @file Vocabulary.h
 * @brief JSON Schema vocabulary constants for validation keywords
 * @details Defines string constants for JSON Schema Draft 2020-12 keywords including
 *          core, applicator, validation, meta-data, format, and content vocabularies
 * @see https://www.learnjsonschema.com/2020-12/
 */

#pragma once

#include <string_view>

namespace nfx::json::vocabulary
{
    //=====================================================================
    // JSON Schema Draft URIs
    //=====================================================================

    /// @brief JSON Schema Draft 2020-12 URI
    inline constexpr std::string_view SCHEMA_DRAFT_2020_12{ "https://json-schema.org/draft/2020-12/schema" };

    /// @brief JSON Schema Draft 2019-09 URI
    inline constexpr std::string_view SCHEMA_DRAFT_2019_09{ "https://json-schema.org/draft/2019-09/schema" };

    /// @brief JSON Schema Draft-07 URI
    inline constexpr std::string_view SCHEMA_DRAFT_07{ "http://json-schema.org/draft-07/schema#" };

    /// @brief JSON Schema Draft-06 URI
    inline constexpr std::string_view SCHEMA_DRAFT_06{ "http://json-schema.org/draft-06/schema#" };

    /// @brief JSON Schema Draft-04 URI
    inline constexpr std::string_view SCHEMA_DRAFT_04{ "http://json-schema.org/draft-04/schema#" };

    //=====================================================================
    // JSON vocabulary
    //=====================================================================

    //----------------------------------------------
    // Core
    //----------------------------------------------

    /// @brief This keyword declares an identifier for the schema resource.
    inline constexpr std::string_view CORE_ID{ "$id" };

    /// @brief This keyword is both used as a JSON Schema dialect identifier and as a reference to a JSON Schema which
    /// describes the set of valid schemas written for this particular dialect.
    inline constexpr std::string_view CORE_SCHEMA{ "$schema" };

    /// @brief This keyword is used to reference a statically identified schema.
    inline constexpr std::string_view CORE_REF{ "$ref" };

    /// @brief This keyword reserves a location for comments from schema authors to readers or maintainers of the
    /// schema.
    inline constexpr std::string_view CORE_COMMENT{ "$comment" };

    /// @brief This keyword reserves a location for schema authors to inline reusable JSON Schemas into a more general
    /// schema.
    inline constexpr std::string_view CORE_DEFS{ "$defs" };

    /// @brief This keyword is used to create plain name fragments that are not tied to any particular structural
    /// location for referencing purposes, which are taken into consideration for static referencing.
    inline constexpr std::string_view CORE_ANCHOR{ "$anchor" };

    /// @brief This keyword is used to create plain name fragments that are not tied to any particular structural
    /// location for referencing purposes, which are taken into consideration for dynamic referencing.
    inline constexpr std::string_view CORE_DYNAMIC_ANCHOR{ "$dynamicAnchor" };

    /// @brief This keyword is used to reference an identified schema, deferring the full resolution until runtime, at
    /// which point it is resolved each time it is encountered while evaluating an instance.
    inline constexpr std::string_view CORE_DYNAMIC_REF{ "$dynamicRef" };

    /// @brief This keyword is used in dialect meta-schemas to identify the required and optional vocabularies available
    /// for use in schemas described by that dialect.
    inline constexpr std::string_view CORE_VOCABULARY{ "$vocabulary" };

    //----------------------------------------------
    // Applicator
    //----------------------------------------------

    /// @brief An instance validates successfully against this keyword if it validates successfully against all schemas
    /// defined by this keyword’s value.
    inline constexpr std::string_view APPLICATOR_ALL_OF{ "allOf" };

    /// @brief An instance validates successfully against this keyword if it validates successfully against at least one
    /// schema defined by this keyword’s value.
    inline constexpr std::string_view APPLICATOR_ANY_OF{ "anyOf" };

    /// @brief An instance validates successfully against this keyword if it validates successfully against exactly one
    /// schema defined by this keyword’s value.
    inline constexpr std::string_view APPLICATOR_ONE_OF{ "oneOf" };

    /// @brief This keyword declares a condition based on the validation result of the given schema.
    inline constexpr std::string_view APPLICATOR_IF{ "if" };

    /// @brief When if is present, and the instance successfully validates against its subschema, then validation
    /// succeeds if the instance also successfully validates against this keyword's subschema.
    inline constexpr std::string_view APPLICATOR_THEN{ "then" };

    /// @brief When if is present, and the instance fails to validate against its subschema, then validation succeeds if
    /// the instance successfully validates against this keyword's subschema.
    inline constexpr std::string_view APPLICATOR_ELSE{ "else" };

    /// @brief An instance is valid against this keyword if it fails to validate successfully against the schema defined
    /// by this keyword.
    inline constexpr std::string_view APPLICATOR_NOT{ "not" };

    /// @brief Validation succeeds if, for each name that appears in both the instance and as a name within this
    /// keyword's value, the child instance for that name successfully validates against the corresponding schema.
    inline constexpr std::string_view APPLICATOR_PROPERTIES{ "properties" };

    /// @brief Validation succeeds if the schema validates against each value not matched by other object applicators in
    /// this vocabulary.
    inline constexpr std::string_view APPLICATOR_ADDITIONAL_PROPERTIES{ "additionalProperties" };

    /// @brief Validation succeeds if, for each instance name that matches any regular expressions that appear as a
    /// property name in this keyword's value, the child instance for that name successfully validates against each
    /// schema that corresponds to a matching regular expression.
    inline constexpr std::string_view APPLICATOR_PATTERN_PROPERTIES{ "patternProperties" };

    /// @brief This keyword specifies subschemas that are evaluated if the instance is an object and contains a certain
    /// property.
    inline constexpr std::string_view APPLICATOR_DEPENDENT_SCHEMAS{ "dependentSchemas" };

    /// @brief Validation succeeds if the schema validates against every property name in the instance.
    inline constexpr std::string_view APPLICATOR_PROPERTY_NAMES{ "propertyNames" };

    /// @brief Validation succeeds if the instance contains an element that validates against this schema.
    inline constexpr std::string_view APPLICATOR_CONTAINS{ "contains" };

    /// @brief Validation succeeds if each element of the instance not covered by prefixItems validates against this
    /// schema.
    inline constexpr std::string_view APPLICATOR_ITEMS{ "items" };

    /// @brief Validation succeeds if each element of the instance validates against the schema at the same position, if
    /// any.
    inline constexpr std::string_view APPLICATOR_PREFIX_ITEMS{ "prefixItems" };

    //----------------------------------------------
    // Defined formats (JSON Schema Draft 2020-12 Section 7.3)
    //----------------------------------------------

    //-----------------------------
    // Dates, Times, and Duration (RFC 3339)
    //-----------------------------

    /// @brief A string instance is valid if it is a valid representation according to the "date-time" ABNF rule.
    inline constexpr std::string_view FORMAT_DATETIME{ "date-time" };

    /// @brief A string instance is valid if it is a valid representation according to the "full-date" ABNF rule.
    inline constexpr std::string_view FORMAT_DATE{ "date" };

    /// @brief A string instance is valid if it is a valid representation according to the "full-time" ABNF rule.
    inline constexpr std::string_view FORMAT_TIME{ "time" };

    /// @brief A string instance is valid if it is a valid representation according to the "duration" ABNF rule.
    inline constexpr std::string_view FORMAT_DURATION{ "duration" };

    //-----------------------------
    // Email Addresses (RFC 5321/6531)
    //-----------------------------

    /// @brief A string instance is valid if it is a valid Internet email address as defined by RFC 5321, section 4.1.2.
    inline constexpr std::string_view FORMAT_EMAIL{ "email" };

    /// @brief A string instance is valid if it is a valid internationalized email address as defined by RFC 6531,
    /// section 3.3.
    inline constexpr std::string_view FORMAT_IDN_EMAIL{ "idn-email" };

    //-----------------------------
    // Hostnames (RFC 1123/5890)
    //-----------------------------

    /// @brief A string instance is valid if it is a valid representation for an Internet hostname as defined by RFC
    /// 1123, section 2.1.
    inline constexpr std::string_view FORMAT_HOSTNAME{ "hostname" };

    /// @brief A string instance is valid if it is a valid internationalized hostname as defined by RFC 5890,
    /// section 2.3.2.3.
    inline constexpr std::string_view FORMAT_IDN_HOSTNAME{ "idn-hostname" };

    //-----------------------------
    // IP Addresses (RFC 2673/4291)
    //-----------------------------

    /// @brief A string instance is valid if it is a valid IPv4 address according to the "dotted-quad" ABNF syntax as
    /// defined in RFC 2673, section 3.2.
    inline constexpr std::string_view FORMAT_IPV4{ "ipv4" };

    /// @brief A string instance is valid if it is a valid IPv6 address as defined in RFC 4291, section 2.2.
    inline constexpr std::string_view FORMAT_IPV6{ "ipv6" };

    //-----------------------------
    // Resource Identifiers (RFC 3986/3987/4122)
    //-----------------------------

    /// @brief A string instance is valid if it is a valid URI according to RFC 3986.
    inline constexpr std::string_view FORMAT_URI{ "uri" };

    /// @brief A string instance is valid if it is a valid URI Reference (either a URI or a relative-reference)
    /// according to RFC 3986.
    inline constexpr std::string_view FORMAT_URI_REFERENCE{ "uri-reference" };

    /// @brief A string instance is valid if it is a valid IRI according to RFC 3987.
    inline constexpr std::string_view FORMAT_IRI{ "iri" };

    /// @brief A string instance is valid if it is a valid IRI Reference (either an IRI or a relative-reference)
    /// according to RFC 3987.
    inline constexpr std::string_view FORMAT_IRI_REFERENCE{ "iri-reference" };

    /// @brief A string instance is valid if it is a valid string representation of a UUID according to RFC 4122.
    inline constexpr std::string_view FORMAT_UUID{ "uuid" };

    //-----------------------------
    // URI Template (RFC 6570)
    //-----------------------------

    /// @brief A string instance is valid if it is a valid URI Template (of any level) according to RFC 6570.
    inline constexpr std::string_view FORMAT_URI_TEMPLATE{ "uri-template" };

    //-----------------------------
    // JSON Pointers (RFC 6901)
    //-----------------------------

    /// @brief A string instance is valid if it is a valid JSON string representation of a JSON Pointer according to RFC
    /// 6901, section 5.
    inline constexpr std::string_view FORMAT_JSON_POINTER{ "json-pointer" };

    /// @brief A string instance is valid if it is a valid Relative JSON Pointer.
    inline constexpr std::string_view FORMAT_RELATIVE_JSON_POINTER{ "relative-json-pointer" };

    //-----------------------------
    // Regex (ECMA-262)
    //-----------------------------

    /// @brief A regular expression which should be valid according to the ECMA-262 regular expression dialect.
    inline constexpr std::string_view FORMAT_REGEX{ "regex" };

    //----------------------------------------------
    // Validation
    //----------------------------------------------

    //-----------------------------
    // General validation
    //-----------------------------

    /// @brief Validation succeeds if the type of the instance matches the type represented by the given type, or
    /// matches at least one of the given types.
    inline constexpr std::string_view VALIDATION_TYPE{ "type" };

    /// @brief Validation succeeds if the instance is equal to one of the elements in this keyword's array value.
    inline constexpr std::string_view VALIDATION_ENUM{ "enum" };

    /// @brief Validation succeeds if the instance is equal to this keyword's value.
    inline constexpr std::string_view VALIDATION_CONST{ "const" };

    //-----------------------------
    /// String validation
    //-----------------------------

    /// @brief A string instance is valid against this keyword if its length is less than, or equal to, the value of
    /// this keyword.
    inline constexpr std::string_view VALIDATION_MAX_LENGTH{ "maxLength" };

    /// @brief A string instance is valid against this keyword if its length is greater than, or equal to, the value of
    /// this keyword.
    inline constexpr std::string_view VALIDATION_MIN_LENGTH{ "minLength" };

    /// @brief A string instance is considered valid if the regular expression matches the instance successfully.
    inline constexpr std::string_view VALIDATION_PATTERN{ "pattern" };

    //-----------------------------
    /// Numeric validation
    //-----------------------------

    /// @brief Validation succeeds if the numeric instance is less than or equal to the given number.
    inline constexpr std::string_view VALIDATION_MAXIMUM{ "maximum" };

    /// @brief Validation succeeds if the numeric instance is greater than or equal to the given number.
    inline constexpr std::string_view VALIDATION_MINIMUM{ "minimum" };

    /// @brief Validation succeeds if the numeric instance is less than the given number.
    inline constexpr std::string_view VALIDATION_EXCLUSIVE_MAXIMUM{ "exclusiveMaximum" };

    /// @brief Validation succeeds if the numeric instance is greater than the given number.
    inline constexpr std::string_view VALIDATION_EXCLUSIVE_MINIMUM{ "exclusiveMinimum" };

    /// @brief A numeric instance is valid only if division by this keyword's value results in an integer.
    inline constexpr std::string_view VALIDATION_MULTIPLE_OF{ "multipleOf" };

    //-----------------------------
    /// Object validation
    //-----------------------------

    /// @brief An object instance is valid if its number of properties is less than, or equal to, the value of this
    /// keyword.
    inline constexpr std::string_view VALIDATION_MAX_PROPERTIES{ "maxProperties" };

    /// @brief An object instance is valid if its number of properties is greater than, or equal to, the value of this
    /// keyword.
    inline constexpr std::string_view VALIDATION_MIN_PROPERTIES{ "minProperties" };

    /// @brief An object instance is valid against this keyword if every item in the array is the name of a property in
    /// the instance.
    inline constexpr std::string_view VALIDATION_REQUIRED{ "required" };

    /// @brief Validation succeeds if, for each name that appears in both the instance and as a name within this
    /// keyword's value, every item in the corresponding array is also the name of a property in the instance.
    inline constexpr std::string_view VALIDATION_DEPENDENT_REQUIRED{ "dependentRequired" };

    //-----------------------------
    /// Array validation
    //-----------------------------

    /// @brief An array instance is valid if its size is less than, or equal to, the value of this keyword.
    inline constexpr std::string_view VALIDATION_MAX_ITEMS{ "maxItems" };

    /// @brief An array instance is valid if its size is greater than, or equal to, the value of this keyword.
    inline constexpr std::string_view VALIDATION_MIN_ITEMS{ "minItems" };

    /// @brief The number of times that the contains keyword (if set) successfully validates against the instance must
    /// be less than or equal to the given integer.
    inline constexpr std::string_view VALIDATION_MAX_CONTAINS{ "maxContains" };

    /// @brief The number of times that the contains keyword (if set) successfully validates against the instance must
    /// be greater than or equal to the given integer.
    inline constexpr std::string_view VALIDATION_MIN_CONTAINS{ "minContains" };

    /// @brief If this keyword is set to the boolean value true, the instance validates successfully if all of its
    /// elements are unique.
    inline constexpr std::string_view VALIDATION_UNIQUE_ITEMS{ "uniqueItems" };

    //----------------------------------------------
    // Meta data
    //----------------------------------------------

    /// @brief A preferably short description about the purpose of the instance described by the schema.
    inline constexpr std::string_view METADATA_TITLE{ "title" };

    /// @brief An explanation about the purpose of the instance described by the schema.
    inline constexpr std::string_view METADATA_DESCRIPTION{ "description" };

    /// @brief This keyword can be used to supply a default JSON value associated with a particular schema.
    inline constexpr std::string_view METADATA_DEFAULT{ "default" };

    /// @brief This keyword indicates that applications should refrain from using the declared property.
    inline constexpr std::string_view METADATA_DEPRECATED{ "deprecated" };

    /// @brief This keyword is used to provide sample JSON values associated with a particular schema, for the purpose
    /// of illustrating usage.
    inline constexpr std::string_view METADATA_EXAMPLES{ "examples" };

    /// @brief This keyword indicates that the value of the instance is managed exclusively by the owning authority, and
    /// attempts by an application to modify the value of this property are expected to be ignored or rejected by that
    /// owning authority.
    inline constexpr std::string_view METADATA_READ_ONLY{ "readOnly" };

    /// @brief This keyword indicates that the value is never present when the instance is retrieved from the owning
    /// authority.
    inline constexpr std::string_view METADATA_WRITE_ONLY{ "writeOnly" };

    //----------------------------------------------
    // Format annotation
    //----------------------------------------------

    /// @brief Define semantic information about a string instance.
    inline constexpr std::string_view FORMAT_ANNOTATION{ "format" };

    //----------------------------------------------
    // Content
    //----------------------------------------------

    /// @brief The string instance should be interpreted as encoded binary data and decoded using the encoding named by
    /// this property.
    inline constexpr std::string_view CONTENT_ENCODING{ "contentEncoding" };

    /// @brief This keyword declares the media type of the string instance.
    inline constexpr std::string_view CONTENT_MEDIA_TYPE{ "contentMediaType" };

    /// @brief This keyword declares a schema which describes the structure of the string.
    inline constexpr std::string_view CONTENT_SCHEMA{ "contentSchema" };

    //----------------------------------------------
    // Unevaluated
    //----------------------------------------------

    /// @brief Validates array elements that did not successfully validate against other standard array applicators.
    inline constexpr std::string_view UNEVALUATED_ITEMS{ "unevaluatedItems" };

    /// @brief Validates object properties that did not successfully validate against other standard object applicators.
    inline constexpr std::string_view UNEVALUATED_PROPERTIES{ "unevaluatedProperties" };

    //----------------------------------------------
    // Format assertion
    //----------------------------------------------

    /// @brief Define and assert semantic information about a string instance.
    inline constexpr std::string_view FORMAT_ASSERTION{ "format" };
} // namespace nfx::json::vocabulary
