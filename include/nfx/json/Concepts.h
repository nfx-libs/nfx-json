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
 * @file Concepts.h
 * @brief C++20 concepts for JSON type constraints
 * @details Defines Primitive concept and type traits infrastructure for
 *          type-safe template constraints in the Document API.
 *
 * @note Container and Checkable concepts are defined at the end of
 *       Document.h after Object and Array are fully defined, since
 *       nested class forward declarations don't work with concepts.
 */

#pragma once

#include <concepts>
#include <string>
#include <type_traits>

namespace nfx::json
{
    class Document;

    //----------------------------------------------
    //  Type traits for JSON nested classes
    //----------------------------------------------

    /**
     * @brief Type trait to identify JSON container types (Object, Array)
     * @details Primary template returns false. Specializations defined in Document.h for
     *          Object and Array types.
     */
    template <typename T>
    struct is_json_container : std::false_type
    {
    };

    /**
     * @brief Specialization for Document
     */
    template <>
    struct is_json_container<Document> : std::true_type
    {
    };

    /**
     * @brief Helper variable template
     */
    template <typename T>
    inline constexpr bool is_json_container_v = is_json_container<std::decay_t<T>>::value;

    //----------------------------------------------
    //  C++20 Concepts for JSON value types
    //----------------------------------------------

    /**
     * @brief Concept for JSON-compatible primitive types
     * @details Matches strings, characters, booleans, integers, and floating-point types
     */
    template <typename T>
    concept Primitive = std::is_same_v<std::decay_t<T>, std::string> || std::is_same_v<std::decay_t<T>, char> ||
                        std::is_same_v<std::decay_t<T>, bool> ||
                        (std::is_integral_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool> &&
                         !std::is_same_v<std::decay_t<T>, char>) ||
                        std::is_floating_point_v<std::decay_t<T>>;

    /**
     * @brief Concept for all JSON-compatible value types
     * @details Matches primitives plus Document, Object, and Array.
     */
    template <typename T>
    concept Value = Primitive<T> || is_json_container_v<T>;

    /**
     * @brief Concept for JSON container types only
     * @details Matches Document, Object, and Array
     */
    template <typename T>
    concept Container = is_json_container_v<T>;

    /**
     * @brief Concept for types that can be checked with is<T>()
     * @details Same as Value but excludes Document itself
     */
    template <typename T>
    concept Checkable = Primitive<T> || ( is_json_container_v<T> && !std::is_same_v<std::decay_t<T>, Document> );
} // namespace nfx::json
