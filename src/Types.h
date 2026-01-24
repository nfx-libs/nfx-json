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
 * @file Types.h
 * @brief JSON Schema type constants
 * @details Defines string constants for JSON Schema type names.
 *
 * @see https://json-schema.org/draft/2020-12/json-schema-core#section-4.2.1
 */

#pragma once

#include <string_view>

namespace nfx::json::types
{
    //=====================================================================
    // JSON Schema type constants
    //=====================================================================

    /// @brief JSON string type
    inline constexpr std::string_view STRING{ "string" };

    /// @brief JSON number type (includes integers)
    inline constexpr std::string_view NUMBER{ "number" };

    /// @brief JSON integer type (subset of number)
    inline constexpr std::string_view INTEGER{ "integer" };

    /// @brief JSON object type
    inline constexpr std::string_view OBJECT{ "object" };

    /// @brief JSON array type
    inline constexpr std::string_view ARRAY{ "array" };

    /// @brief JSON boolean type
    inline constexpr std::string_view BOOLEAN{ "boolean" };

    /// @brief JSON null type
    inline constexpr std::string_view NULL_TYPE{ "null" };
} // namespace nfx::json::types
