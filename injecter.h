/**
 * @file injecter.h
 * @brief A cross-platform dynamic library loader and function wrapper for C++
 * @author zmkjh
 * @version 1.0.0
 * @date 2026
 * @copyright Public Domain - No rights reserved. Use freely for any purpose.
 *
 * This header provides a lightweight, RAII-based interface for loading
 * dynamic libraries and accessing their exported functions safely.
 */

#ifndef INJECTER_H
#define INJECTER_H

#include <string_view>
#include <utility>

namespace injecter {

/**
 * @class module
 * @brief RAII wrapper for dynamic library handles
 *
 * Provides automatic loading, unloading, and function retrieval from
 * shared libraries/DLLs on Windows and Linux platforms.
 *
 * @note This class is non-copyable but movable
 * @note All operations throw std::runtime_error on failure
 */
class module {
public:
    /** @brief Type alias for function pointers loaded from the module */
    using func_t = void*;

private:
    /** @brief Internal handle to the loaded module (HMODULE on Windows, void* on Linux) */
    using handle_t = void*;
    handle_t m_handle = nullptr;

    module(const module&) = delete;
    module& operator=(const module&) = delete;

public:
    /**
     * @brief Constructs a module by loading a dynamic library
     * @param[in] name Path to the library file (.dll, .so, etc.)
     * @throws std::runtime_error if the library cannot be loaded
     */
    explicit module(std::string_view name);

    /**
     * @brief Move constructor
     * @param[in,out] other Module to move from
     * @post other is left in a valid but unspecified state
     */
    module(module&& other) noexcept
        : m_handle(other.m_handle) {
        other.m_handle = nullptr;
    }

    /**
     * @brief Move assignment operator
     * @param[in,out] other Module to move from
     * @return Reference to this module
     * @post other is left in a valid but unspecified state
     */
    module& operator=(module&& other) noexcept {
        if (this != &other) {
            reset();
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    /**
     * @brief Destructor - automatically unloads the module
     */
    ~module();

    /**
     * @brief Loads a function from the module
     * @param[in] name Name of the exported function
     * @return Pointer to the function
     * @throws std::runtime_error if the function cannot be found
     */
    func_t load(std::string_view name);

    /**
     * @brief Unloads the module and resets the handle
     * @note Safe to call multiple times
     */
    void reset();

    /**
     * @brief Checks if the module is currently loaded
     * @return true if module handle is valid, false otherwise
     */
    operator bool() {
        return m_handle != nullptr;
    }
};

/**
 * @class func
 * @brief Type-safe wrapper for function pointers from dynamic libraries
 *
 * @tparam Signature Function signature (e.g., int(int, int))
 *
 * Example usage:
 * @code
 *   injecter::module mod("mylib.dll");
 *   auto myfunc = injecter::make_func<int(int, int)>(mod, "my_function");
 *   int result = myfunc(1, 2);
 * @endcode
 */
template<typename>
class func;

/**
 * @class func<R(Args...)>
 * @brief Specialized func template for callable function types
 * @tparam R Return type of the function
 * @tparam Args Argument types of the function
 */
template <typename R, typename... Args>
class func<R(Args...)> {
    /** @brief Internal function pointer type */
    typedef R(*handle_t)(Args...);
    handle_t m_handle = nullptr;

public:
    /**
     * @brief Constructs from a raw function pointer
     * @param[in] handle Raw function pointer
     */
    func(handle_t handle) : m_handle(handle) {}

    /**
     * @brief Constructs by loading a function from a module
     * @param[in] mod Module containing the function
     * @param[in] name Name of the function to load
     * @throws May throw if the function cannot be loaded from the module
     */
    func(module mod, std::string_view name) {
        m_handle = reinterpret_cast<handle_t>(mod.load(name));
    }

    /**
     * @brief Invokes the wrapped function
     * @param[in] args Arguments to pass to the function
     * @return Result of the function call
     */
    R operator()(Args... args) {
        return m_handle(std::forward<Args>(args)...);
    }

    /**
     * @brief Checks if the function pointer is valid
     * @return true if the function is loaded, false otherwise
     */
    operator bool() {
        return m_handle != nullptr;
    }
};

/**
 * @brief Factory function to create a func with automatic type deduction
 * @tparam R Return type of the function
 * @tparam Args Argument types of the function
 * @param[in] mod Module containing the function
 * @param[in] name Name of the function to load
 * @return A func object wrapping the loaded function
 *
 * @code
 *   auto fn = injecter::make_func<void(const char*)>(mod, "print");
 * @endcode
 */
template <typename R, typename... Args>
func<R(Args...)> make_func(module mod, std::string_view name) {
    return func<R(Args...)>(mod, name);
}

} // namespace injecter

#endif // INJECTER_H
