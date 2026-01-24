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
 * @file SchemaGenerator.h
 * @brief JSON Schema generation from Document instances
 * @details Provides JSON Schema Draft 2020-12 generation capabilities for inferring
 *          schema structure, types, formats, and constraints from JSON data samples.
 *          Supports single and multi-sample analysis with configurable inference options.
 */

#pragma once

#include "Document.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace nfx::json
{
    //=====================================================================
    // SchemaGenerator class
    //=====================================================================

    /**
     * @brief JSON Schema generator from Document instances
     * @details Analyzes JSON data to generate JSON Schema Draft 2020-12 compliant schemas.
     *          Infers types, detects string formats (email, date, URI, UUID, etc.),
     *          and generates constraints based on observed values.
     */
    class SchemaGenerator final
    {
    public:
        //----------------------------------------------
        // Options
        //----------------------------------------------

        /**
         * @brief Configuration options for schema generation
         */
        struct Options
        {
            bool inferFormats = true;      ///< Detect string formats (email, date, URI, UUID, etc.)
            bool inferConstraints = false; ///< Generate constraints (minLength, minimum, etc.) - requires multi-sample
            bool detectEnums = false;      ///< Detect limited value sets as enums - requires multi-sample
            size_t enumThreshold = 10;     ///< Maximum unique values to consider as enum
            std::string title = {};        ///< Schema title (optional)
            std::string description = {};  ///< Schema description (optional)
            std::string id = {};           ///< Schema $id URI (optional)
        };

        //----------------------------------------------
        // Construction
        //----------------------------------------------

        /**
         * @brief Generate JSON Schema from a single document
         * @param document The JSON document to analyze
         */
        explicit SchemaGenerator( const Document& document );

        /**
         * @brief Generate JSON Schema from a single document
         * @param document The JSON document to analyze
         * @param options Generation options
         */
        SchemaGenerator( const Document& document, const Options& options );

        /**
         * @brief Generate JSON Schema from multiple sample documents
         * @details Analyzes multiple samples to infer required properties,
         *          detect enums, and generate more accurate constraints.
         * @param documents Vector of JSON documents to analyze
         */
        explicit SchemaGenerator( const std::vector<Document>& documents );

        /**
         * @brief Generate JSON Schema from multiple sample documents
         * @details Analyzes multiple samples to infer required properties,
         *          detect enums, and generate more accurate constraints.
         * @param documents Vector of JSON documents to analyze
         * @param options Generation options
         */
        SchemaGenerator( const std::vector<Document>& documents, const Options& options );

        /** @brief Copy constructor */
        SchemaGenerator( const SchemaGenerator& ) = default;

        /** @brief Move constructor */
        SchemaGenerator( SchemaGenerator&& ) noexcept = default;

        //----------------------------------------------
        // Destruction
        //----------------------------------------------

        /** @brief Destructor */
        ~SchemaGenerator() = default;

        //----------------------------------------------
        // Assignment
        //----------------------------------------------

        /**
         * @brief Copy assignment operator
         * @return Reference to this generator
         */
        SchemaGenerator& operator=( const SchemaGenerator& ) = default;

        /**
         * @brief Move assignment operator
         * @return Reference to this generator
         */
        SchemaGenerator& operator=( SchemaGenerator&& ) noexcept = default;

        //----------------------------------------------
        // Accessors
        //----------------------------------------------

        /**
         * @brief Get the generated schema
         * @return Const reference to the generated JSON Schema
         */
        inline const Document& schema() const noexcept;

    private:
        //----------------------------------------------
        // Internal methods
        //----------------------------------------------

        /**
         * @brief Add schema metadata ($schema, $id, title, description)
         */
        void addMetadata();

        /**
         * @brief Generate schema for a JSON value
         * @param value The JSON value to analyze
         * @return Schema document for the value
         */
        Document generateSchema( const Document& value ) const;

        //----------------------------------------------
        // Member data
        //----------------------------------------------

        Document m_schema; ///< The generated JSON Schema
        Options m_options; ///< Generation options
    };
} // namespace nfx::json

#include "detail/SchemaGenerator.inl"
