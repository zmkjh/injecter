# Injecter

A lightweight, cross-platform C++ library for dynamic library loading and function invocation.

## Features

- **Cross-platform**: Works on Windows and Linux
- **RAII-based**: Automatic resource management
- **Type-safe**: Template-based function wrappers
- **Header-friendly**: Simple, minimal API
- **Exception-safe**: Uses exceptions for error handling

## Quick Start

### Basic Usage

```cpp
#include "injecter.h"
#include <iostream>

int main() {
    try {
        // Load a dynamic library
        injecter::module mod("mylib.dll");  // Windows
        // injecter::module mod("./libmylib.so");  // Linux

        // Load a function with known signature
        auto add = injecter::make_func<int(int, int)>(mod, "add");

        // Call the function
        int result = add(2, 3);
        std::cout << "Result: " << result << std::endl;

    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

### Alternative Syntax

```cpp
// Direct function wrapper instantiation
injecter::func<int(int, int)> add(mod, "add");

// Or with raw function pointer
auto ptr = mod.load("add");
injecter::func<int(int, int)> add(reinterpret_cast<int(*)(int,int)>(ptr));
```

## API Reference

### `class module`

RAII wrapper for dynamic library handles.

| Method | Description |
|--------|-------------|
| `explicit module(std::string_view name)` | Loads a library by path |
| `~module()` | Unloads the library |
| `func_t load(std::string_view name)` | Retrieves a function pointer |
| `void reset()` | Explicitly unloads the library |
| `operator bool()` | Checks if module is loaded |

### `class func<R(Args...)>`

Type-safe function wrapper.

| Method | Description |
|--------|-------------|
| `func(handle_t handle)` | Constructs from raw pointer |
| `func(module mod, std::string_view name)` | Loads from module |
| `R operator()(Args... args)` | Invokes the function |
| `operator bool()` | Checks if function is valid |

### `make_func<R(Args...)>`

Factory function for creating func objects with automatic type deduction.

```cpp
auto func = injecter::make_func<void(const char*)>(mod, "print_message");
```

## Platform Notes

### Windows
- Uses `LoadLibraryA`, `GetProcAddress`, `FreeLibrary`
- Supports `.dll` files

### Linux
- Uses `dlopen`, `dlsym`, `dlclose`
- Supports `.so` files
- Links with `-ldl`

## Building

### Windows (Visual Studio)
```bash
cl /EHsc injecter.cpp main.cpp /Fe:myapp.exe
```

### Linux (GCC/Clang)
```bash
g++ -std=c++17 injecter.cpp main.cpp -ldl -o myapp
```

## Error Handling

All operations throw `std::runtime_error` on failure:

- Module loading fails (file not found, invalid format)
- Function loading fails (symbol not found)

## License

**Public Domain** - No rights reserved.

This is free and unencumbered software released into the public domain. Anyone is free to copy, modify, publish, use, compile, sell, or distribute this software, either in source code form or as a compiled binary, for any purpose, commercial or non-commercial, and by any means.

The author (zmkjh) dedicates this software to the public domain worldwide.

## Contributing

Contributions are welcome! Please follow the existing code style and add Doxygen documentation for new features.

## Author

**zmkjh** - Original author
