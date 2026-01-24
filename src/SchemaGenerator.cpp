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
 * @file SchemaGenerator.cpp
 * @brief Implementation of SchemaGenerator class for JSON Schema generation
 */

#include "nfx/json/SchemaGenerator.h"

#include "nfx/json/Document.h"
#include "Types.h"
#include "Vocabulary.h"

#include <nfx/StringUtils.h>

#include <map>
#include <set>

namespace nfx::json
{
    //=====================================================================
    // SchemaGenerator class
    //=====================================================================

    //----------------------------------------------
    // Construction
    //----------------------------------------------

    SchemaGenerator::SchemaGenerator( const Document& document )
        : SchemaGenerator( document, Options{} )
    {
    }

    SchemaGenerator::SchemaGenerator( const Document& document, const Options& options )
        : m_schema{},
          m_options{ options }
    {
        addMetadata();

        // Merge schemaFor result into m_schema
        Document valueSchema = generateSchema( document );
        if( valueSchema.is<Object>( "" ) )
        {
            auto objOpt = valueSchema.get<Object>( "" );
            if( objOpt.has_value() )
            {
                for( const auto& [key, val] : *objOpt )
                {
                    m_schema.set<Document>( key, val );
                }
            }
        }
    }

    SchemaGenerator::SchemaGenerator( const std::vector<Document>& documents )
        : SchemaGenerator( documents, Options{} )
    {
    }

    SchemaGenerator::SchemaGenerator( const std::vector<Document>& documents, const Options& options )
        : m_schema{},
          m_options{ options }

    {
        addMetadata();

        if( documents.empty() )
        {
            return;
        }

        if( documents.size() == 1 )
        {
            Document valueSchema = generateSchema( documents[0] );
            if( valueSchema.is<Object>( "" ) )
            {
                auto objOpt = valueSchema.get<Object>( "" );
                if( objOpt.has_value() )
                {
                    for( const auto& [key, val] : *objOpt )
                    {
                        m_schema.set<Document>( key, val );
                    }
                }
            }
            return;
        }

        // Generate schema for each document
        std::vector<Document> schemas;
        schemas.reserve( documents.size() );
        for( const auto& doc : documents )
        {
            schemas.push_back( generateSchema( doc ) );
        }

        // Check if all schemas have the same type
        std::set<std::string> typeSet;
        for( const auto& schema : schemas )
        {
            auto typeOpt = schema.get<std::string>( std::string( vocabulary::VALIDATION_TYPE ) );
            if( typeOpt.has_value() )
            {
                typeSet.insert( *typeOpt );
            }
        }

        // If all same type and it's object, merge object schemas
        if( typeSet.size() == 1 && *typeSet.begin() == std::string( types::OBJECT ) )
        {
            m_schema.set<std::string>( std::string( vocabulary::VALIDATION_TYPE ), std::string( types::OBJECT ) );

            std::map<std::string, size_t> propertyCount;
            std::map<std::string, Document> propertySchemas;

            for( const auto& schema : schemas )
            {
                auto propsOpt = schema.get<Object>( std::string( vocabulary::APPLICATOR_PROPERTIES ) );
                if( propsOpt.has_value() )
                {
                    for( const auto& [key, value] : *propsOpt )
                    {
                        propertyCount[key]++;
                        if( propertySchemas.find( key ) == propertySchemas.end() )
                        {
                            propertySchemas[key] = value;
                        }
                    }
                }
            }

            // Build merged properties
            if( !propertySchemas.empty() )
            {
                Document properties;
                for( const auto& [key, schema] : propertySchemas )
                {
                    properties.set<Document>( key, schema );
                }
                m_schema.set<Document>( std::string( vocabulary::APPLICATOR_PROPERTIES ), properties );
            }

            // Required = properties that appear in ALL samples
            std::vector<std::string> required;
            for( const auto& [key, count] : propertyCount )
            {
                if( count == schemas.size() )
                {
                    required.push_back( key );
                }
            }

            if( !required.empty() )
            {
                Array requiredArray;
                for( const auto& field : required )
                {
                    requiredArray.push_back( Document( field ) );
                }
                m_schema.set<Array>( std::string( vocabulary::VALIDATION_REQUIRED ), std::move( requiredArray ) );
            }
        }
        else
        {
            // If multiple types, use first schema
            if( schemas[0].is<Object>( "" ) )
            {
                auto objOpt = schemas[0].get<Object>( "" );
                if( objOpt.has_value() )
                {
                    for( const auto& [key, val] : *objOpt )
                    {
                        m_schema.set<Document>( key, val );
                    }
                }
            }
        }
    }

    //----------------------------------------------
    // Private methods
    //----------------------------------------------

    void SchemaGenerator::addMetadata()
    {
        m_schema.set<std::string>(
            std::string( vocabulary::CORE_SCHEMA ), std::string( vocabulary::SCHEMA_DRAFT_2020_12 ) );

        if( !m_options.id.empty() )
        {
            m_schema.set<std::string>( std::string( vocabulary::CORE_ID ), m_options.id );
        }
        if( !m_options.title.empty() )
        {
            m_schema.set<std::string>( std::string( vocabulary::METADATA_TITLE ), m_options.title );
        }
        if( !m_options.description.empty() )
        {
            m_schema.set<std::string>( std::string( vocabulary::METADATA_DESCRIPTION ), m_options.description );
        }
    }

    Document SchemaGenerator::generateSchema( const Document& value ) const
    {
        Document schema;

        // Check type and generate appropriate schema
        if( value.isNull( "" ) )
        {
            schema.set<std::string>( std::string( vocabulary::VALIDATION_TYPE ), std::string( types::NULL_TYPE ) );
        }
        else if( value.is<bool>( "" ) )
        {
            schema.set<std::string>( std::string( vocabulary::VALIDATION_TYPE ), std::string( types::BOOLEAN ) );
        }
        else if( value.is<int64_t>( "" ) )
        {
            schema.set<std::string>( std::string( vocabulary::VALIDATION_TYPE ), std::string( types::INTEGER ) );
        }
        else if( value.is<double>( "" ) )
        {
            schema.set<std::string>( std::string( vocabulary::VALIDATION_TYPE ), std::string( types::NUMBER ) );
        }
        else if( value.is<std::string>( "" ) )
        {
            schema.set<std::string>( std::string( vocabulary::VALIDATION_TYPE ), std::string( types::STRING ) );

            // Optionally infer format
            if( m_options.inferFormats )
            {
                auto strValue = value.get<std::string>( "" );
                if( strValue.has_value() && !strValue->empty() )
                {
                    const std::string& str = *strValue;
                    std::string format;

                    // Date-time formats (most specific first)
                    if( nfx::string::isDateTime( str ) )
                    {
                        format = vocabulary::FORMAT_DATETIME;
                    }
                    else if( nfx::string::isDate( str ) )
                    {
                        format = vocabulary::FORMAT_DATE;
                    }
                    else if( nfx::string::isTime( str ) )
                    {
                        format = vocabulary::FORMAT_TIME;
                    }
                    else if( nfx::string::isDuration( str ) )
                    {
                        format = vocabulary::FORMAT_DURATION;
                    }
                    // Identifiers
                    else if( nfx::string::isUuid( str ) )
                    {
                        format = vocabulary::FORMAT_UUID;
                    }
                    // Email: check IDN first (isIdnEmail accepts all valid emails including ASCII-only)
                    else if( nfx::string::isIdnEmail( str ) )
                    {
                        // Determine if it's IDN (contains non-ASCII or Punycode)
                        bool isIdn = str.find( "xn--" ) != std::string::npos;
                        if( !isIdn )
                        {
                            for( unsigned char c : str )
                            {
                                if( c > 127 )
                                {
                                    isIdn = true;
                                    break;
                                }
                            }
                        }
                        format = isIdn ? vocabulary::FORMAT_IDN_EMAIL : vocabulary::FORMAT_EMAIL;
                    }
                    // Network
                    else if( nfx::string::isIpv4Address( str ) )
                    {
                        format = vocabulary::FORMAT_IPV4;
                    }
                    else if( nfx::string::isIpv6Address( str ) )
                    {
                        format = vocabulary::FORMAT_IPV6;
                    }
                    // Hostname: check IDN (isIdnHostname accepts all valid hostnames including ASCII-only)
                    else if( nfx::string::isIdnHostname( str ) && str.find( '.' ) != std::string::npos )
                    {
                        // Determine if it's IDN (contains non-ASCII or Punycode)
                        bool isIdn = str.find( "xn--" ) != std::string::npos;
                        if( !isIdn )
                        {
                            for( unsigned char c : str )
                            {
                                if( c > 127 )
                                {
                                    isIdn = true;
                                    break;
                                }
                            }
                        }
                        format = isIdn ? vocabulary::FORMAT_IDN_HOSTNAME : vocabulary::FORMAT_HOSTNAME;
                    }
                    // URIs/IRIs - check IRI first (accepts Unicode)
                    else if( nfx::string::isIri( str ) )
                    {
                        // Determine if it's IRI (contains non-ASCII)
                        bool isInternationalized = false;
                        for( unsigned char c : str )
                        {
                            if( c > 127 )
                            {
                                isInternationalized = true;
                                break;
                            }
                        }
                        format = isInternationalized ? vocabulary::FORMAT_IRI : vocabulary::FORMAT_URI;
                    }
                    // JSON Pointer
                    else if( nfx::string::isJsonPointer( str ) && !str.empty() && str[0] == '/' )
                    {
                        format = vocabulary::FORMAT_JSON_POINTER;
                    }

                    if( !format.empty() )
                    {
                        schema.set<std::string>( std::string( vocabulary::FORMAT_ANNOTATION ), format );
                    }
                }
            }
        }
        else if( value.is<Array>( "" ) )
        {
            // Array schema
            schema.set<std::string>( std::string( vocabulary::VALIDATION_TYPE ), std::string( types::ARRAY ) );

            auto arrOpt = value.get<Array>( "" );
            if( arrOpt.has_value() && arrOpt->size() > 0 )
            {
                const Document& firstItem = arrOpt->at( 0 );
                Document itemSchema = generateSchema( firstItem );
                schema.set<Document>( std::string( vocabulary::APPLICATOR_ITEMS ), itemSchema );
            }
        }
        else if( value.is<Object>( "" ) )
        {
            // Object schema
            schema.set<std::string>( std::string( vocabulary::VALIDATION_TYPE ), std::string( types::OBJECT ) );

            auto objOpt = value.get<Object>( "" );
            if( objOpt.has_value() && objOpt->size() > 0 )
            {
                Document properties;
                std::vector<std::string> requiredFields;

                for( const auto& [key, val] : *objOpt )
                {
                    Document propSchema = generateSchema( val );
                    properties.set<Document>( key, propSchema );
                    requiredFields.push_back( key );
                }

                schema.set<Document>( std::string( vocabulary::APPLICATOR_PROPERTIES ), properties );

                if( !requiredFields.empty() )
                {
                    Array requiredArray;
                    for( const auto& field : requiredFields )
                    {
                        requiredArray.push_back( Document( field ) );
                    }
                    schema.set<Array>( std::string( vocabulary::VALIDATION_REQUIRED ), std::move( requiredArray ) );
                }
            }
        }

        return schema;
    }
} // namespace nfx::json
