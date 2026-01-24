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
 * @file SchemaValidator.inl
 * @brief Inline Implementation of SchemaValidator class
 */

namespace nfx::json
{
    //=====================================================================
    // ValidationError class
    //=====================================================================

    //----------------------------------------------
    // Accessors
    //----------------------------------------------

    inline const std::string& ValidationError::path() const noexcept
    {
        return m_error.path;
    }

    inline const std::string& ValidationError::message() const noexcept
    {
        return m_error.message;
    }

    inline const std::string& ValidationError::constraint() const noexcept
    {
        return m_error.constraint;
    }

    inline const std::string& ValidationError::expectedValue() const noexcept
    {
        return m_error.expectedValue;
    }

    inline const std::string& ValidationError::actualValue() const noexcept
    {
        return m_error.actualValue;
    }

    //=====================================================================
    // ValidationResult class
    //=====================================================================

    //----------------------------------------------
    // Status checking
    //----------------------------------------------

    inline bool ValidationResult::isValid() const noexcept
    {
        return m_errors.empty();
    }

    inline bool ValidationResult::hasErrors() const noexcept
    {
        return !m_errors.empty();
    }

    inline size_t ValidationResult::errorCount() const noexcept
    {
        return m_errors.size();
    }

    //----------------------------------------------
    // Error access
    //----------------------------------------------

    inline const std::vector<ValidationError>& ValidationResult::errors() const noexcept
    {
        return m_errors;
    }
} // namespace nfx::json
