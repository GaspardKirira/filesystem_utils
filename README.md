# filesystem_utils

Lightweight header-only C++ filesystem toolkit built on top of
std::filesystem.

Designed for real-world applications that need safe file operations,
atomic writes, directory management, and convenient utilities with
minimal dependencies.

C++17 compatible.

## Installation

### Using Vix Registry

``` bash
vix add gaspardkirira/filesystem_utils
```

### Manual download

Download the latest release:

👉 https://github.com/GaspardKirira/filesystem_utils/releases

Or clone:

``` bash
git clone https://github.com/GaspardKirira/filesystem_utils.git
```

Add the `include/` directory to your project.

## Quick Example

``` cpp
#include <filesystem_utils/filesystem_utils.hpp>
#include <iostream>

int main()
{
  namespace fsu = filesystem_utils;

  auto file = fsu::fs::temp_directory_path() / "example.txt";

  fsu::write_text(file, "Hello World\n", true);

  std::cout << fsu::read_text(file);

  fsu::atomic_write_text(file, "Updated\n");

  std::cout << fsu::read_text(file);
}
```

## Features

-   Header-only
-   Zero dependencies
-   C++17 compatible
-   Safe directory creation (`ensure_dir`)
-   Text and binary read/write
-   Atomic file write (best-effort, cross-platform)
-   Recursive copy and remove
-   Directory listing (files, dirs, recursive)
-   Path helpers (join, normalize)
-   Human-readable file sizes (`human_size`)
-   No-throw wrappers for common checks

## Atomic Write

Atomic write ensures that partially written files never appear.

Strategy:

-   Write to temporary file in same directory
-   Rename temporary file to target

On most platforms, rename is atomic on the same filesystem.

``` cpp
fsu::atomic_write_text("config.json", json_string);
```

## Directory Utilities

``` cpp
fsu::ensure_dir("data/cache");

auto files = fsu::list_files("data");
auto dirs  = fsu::list_dirs("data");

auto all   = fsu::list_recursive("data");
```

## Read and Write

### Text

``` cpp
fsu::write_text("file.txt", "content", true);
auto content = fsu::read_text("file.txt");
```

### Binary

``` cpp
std::vector<uint8_t> bytes = {1,2,3,4};
fsu::write_bytes("data.bin", bytes, true);

auto read_back = fsu::read_bytes("data.bin");
```

## Copy and Remove

``` cpp
fsu::copy_recursive("src", "backup");
fsu::remove_recursive("backup");
```

## Human Size

``` cpp
std::cout << fsu::human_size(1536); // 1.5 KB
```

## Examples

See the `examples/` directory:

-   read_write_text.cpp
-   atomic_write.cpp
-   list_tree.cpp

Run:

``` bash
vix run examples/read_write_text.cpp
```

## Tests

Run tests with:

``` bash
vix tests
```

## Why use filesystem_utils?

-   Thin layer over `std::filesystem`
-   Removes repetitive boilerplate
-   Provides safe atomic write primitive
-   Cleaner code in tools and services

Ideal for:

-   CLI tools
-   Build systems
-   Config managers
-   Caching layers
-   Offline-first engines
-   Embedded utilities

## License

MIT License.

