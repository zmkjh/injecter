/**
 * @file injecter.cpp
 * @brief Platform-specific implementations for the injecter library
 * @author zmkjh
 * @version 1.0.0
 * @date 2026
 * @copyright Public Domain - No rights reserved. Use freely for any purpose.
 *
 * Implements the module class using native Windows and Linux APIs
 * for dynamic library loading.
 */

#if defined(_WIN32) || defined(_WIN64)
#include <libloaderapi.h>
#include <exception>

#include "injecter.h"

namespace injecter {

/**
 * @brief Constructs a module by loading a Windows DLL
 * @param[in] name Path to the DLL file
 * @throws std::runtime_error if LoadLibraryA fails
 * @note Uses LoadLibraryA for ASCII path support
 */
module::module(std::string_view name) {
    m_handle = LoadLibraryA(name.data());
    if (!m_handle) {
        m_handle = nullptr;
        throw std::runtime_error("injecter: Failed to load module");
    }
}

/**
 * @brief Destroys the module and frees the library
 * @note Calls FreeLibrary to decrement the reference count
 */
module::~module() {
    FreeLibrary(m_handle);
}

/**
 * @brief Loads a function from the Windows DLL
 * @param[in] name Name of the exported function
 * @return Pointer to the function
 * @throws std::runtime_error if GetProcAddress fails
 */
module::func_t module::load(std::string_view name) {
    func_t func = reinterpret_cast<func_t>(GetProcAddress(m_handle, name.data()));
    if (!func) {
        throw std::runtime_error("injecter: Failed to load function");
    }
    return func;
}

/**
 * @brief Unloads the module and invalidates the handle
 * @note Safe to call multiple times (check via operator bool)
 */
void module::reset() {
    if (m_handle) {
        FreeLibrary(m_handle);
        m_handle = nullptr;
    }
}
} // namespace injecter

#elif defined(__linux__)
#include <dlfcn.h>
#include <exception>

#include "injecter.h"

namespace injecter {

/**
 * @brief Constructs a module by loading a Linux shared object
 * @param[in] name Path to the .so file
 * @throws std::runtime_error if dlopen fails
 * @note Uses RTLD_LAZY for lazy symbol resolution
 */
module::module(std::string_view name) {
    m_handle = dlopen(name.data(), RTLD_LAZY);
    if (!m_handle) {
        m_handle = nullptr;
        throw std::runtime_error("injecter: Failed to load module");
    }
}

/**
 * @brief Destroys the module and closes the shared object
 * @note Calls dlclose to decrement the reference count
 */
module::~module() {
    dlclose(m_handle);
}

/**
 * @brief Loads a function from the shared object
 * @param[in] name Name of the exported function
 * @return Pointer to the function
 * @throws std::runtime_error if dlsym fails
 */
module::func_t module::load(std::string_view name) {
    func_t func = reinterpret_cast<func_t>(dlsym(m_handle, name.data()));
    if (!func) {
        throw std::runtime_error("injecter: Failed to load function");
    }
    return func;
}

/**
 * @brief Unloads the module and invalidates the handle
 * @note Safe to call multiple times (check via operator bool)
 */
void module::reset() {
    if (m_handle) {
        dlclose(m_handle);
        m_handle = nullptr;
    }
}
} // namespace injecter

#else
#error "injecter: unsupported platform"
#endif
