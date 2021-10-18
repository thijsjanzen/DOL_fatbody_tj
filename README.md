# DOL_fatbody_tj  

This repository contains code to perform a model of division of labour coupled with nutritional status and dominance status

![example workflow](https://github.com/thijsjanzen/DOL_fatbody_tj/actions/workflows/c-cpp.yml/badge.svg)

Branch | Code Coverage
---|---
Main | [![codecov](https://codecov.io/gh/thijsjanzen/DOL_fatbody_tj/branch/main/graph/badge.svg?token=eeq1caqXLQ)](https://codecov.io/gh/thijsjanzen/DOL_fatbody_tj)
Develop | [![codecov](https://codecov.io/gh/thijsjanzen/DOL_fatbody_tj/branch/develop/graph/badge.svg?token=eeq1caqXLQ)](https://codecov.io/gh/thijsjanzen/DOL_fatbody_tj)


## Build

```
mkdir build && cd build
cmake ..
cmake --build . --target install
```

To play with different toolchains, set `CXX` before invoking cmake e.g.:

```
CXX=clang++-12 cmake ..
...
```
Anyhow, the binary is placed in the `./bin` folder below the top-level directory.

## Minor things

Nice access violation if no argument is given:

```cpp
// main.cpp
if (argc < 1) { // needs run name
    std::cerr << "Usage: " << argv[0] << "parameter name" << std::endl;
    return 1;
}
std::string file_name = argv[1];
```

Great idea to name something `ERROR` in global namespace! All-capital names are usually 'reserved' for macros.
As it happens, `ERROR` *is* defined as a macro in `<wingdi.h>`. Requires some preprocessor defines in `CMakeLists.txt`:

```cpp
// config_parser.h
struct ERROR { ... };
```

## BOOM

```cpp
void update_colony() {
    auto next_ind = time_queue.begin();
    time_queue.erase(next_ind);
    ...
    auto focal_individual = next_ind->ind;
```
Ok, *that* is UB!

