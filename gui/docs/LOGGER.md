# Zappy GUI Logger Documentation

![C++](https://img.shields.io/badge/C++-20-blue.svg?style=flat-square&logo=c%2B%2B)

> This documentation covers the complete and standardized overhaul of the logging system. It is now unified, asynchronous, **thread-safe**, and configurable at both compile-time and runtime.

---

## 1. Compilation & Profiles (Makefile)

To avoid rewriting CLI flags during development, the `Makefile` now embeds compilation profiles with default configurations:

- `make debug`: **(Recommended for Dev)** Enables console logging at `DEBUG` level and creates a `network.log` file at `TRACE` level.
- `make info`: Enables console logging at `INFO` level without generating a log file.
- `make release` / `make all`: Completely disables default logging (silent mode).

---

## 2. Low-Level Configuration (CMake)

If you wish to compile with a custom configuration without modifying the `Makefile`, the following CMake cache variables are exposed:

```bash
cmake -B build \
    -DGUI_CONSOLE_LOG_LEVEL="zappy::LogLevel::INFO" \
    -DGUI_FILE_LOG_LEVEL="zappy::LogLevel::TRACE" \
    -DGUI_LOG_FILE_PATH="\"custom.log\""
```

> The escaped quotes `\"` are **required** for the file path so that it is properly injected into the C++ code as a string literal.

---

## 3. Runtime Overrides (CLI)

The default behaviors set during compilation can always be overridden at runtime via command-line arguments.

| Flag | Argument | Description |
| :--- | :--- | :--- |
| `-v, --verbose` | `<level>` | Forces the console log level (`trace`, `debug`, `info`, `warn`, `error`, `none`). |
| `--log-file` | `<filename>` | Enables or changes the log file. (The default log level will be `TRACE`). |
| `--log-file-level` | `<level>` | Specifically overrides the file's log level. |

**Comprehensive Usage Example:**

```bash
./zappy_gui -p 35 -v info --log-file debug.log --log-file-level trace
```

---

## 4. Integrating `ContextLogger` in Your Classes

The `ContextLogger` is designed to automatically identify the origin of a log. Integrating it into any class is extremely straightforward:

### `MyObject.hpp`
```cpp
#pragma once
#include "logger/ContextLogger.hpp"

namespace zappy {
    class MyObject {
        public:
            MyObject();
            void doSomething();

        private:
            // 1. Declare the logger with the class/component name as a parameter
            ContextLogger _log{"MyObject"}; 
    };
}
```

### `MyObject.cpp`
```cpp
#include "MyObject.hpp"

namespace zappy {
    MyObject::MyObject() {
        // 2. Use _log directly (no complex formatted string needed)
        _log.info("Object initialization completed!");
    }

    void MyObject::doSomething() {
        int x = 42;
        
        // 3. You can pass as many arguments as you want!
        _log.debug("Value of x: ", x, " | this pointer: ", this);
        _log.error("An error occurred!");
    }
}
```

### Expected Output Format

The generated log output will automatically be formatted like this:

```log
[2026-05-29 10:25:30.145] [DEBUG] [MyObject] Value of x: 42 | this pointer: 0x7ffd5a...
```
