## Build: Including all submodules
Clone this repository recursively
```bash
git clone https://github.com/romanpunia/vengeance --recursive
```
Generate and build project files while being inside of repository
```bash
cmake . -DCMAKE_BUILD_TYPE=Release # -DVI_CXX=17
```
Build project files while being inside of repository
```bash
cmake --build . --config Release
```

## Build: Including specific submodules
Clone this repository at top level
```bash
git clone https://github.com/romanpunia/vengeance
```
Initialize needed submodules while being inside of repository
```bash
# Initialize required submodules:
git submodule update --init ./deps/concurrentqueue

# Initialize optional submodules, for example:
git submodule update --init ./deps/stb
```
Generate and build project files while being inside of repository (don't forget to disable missing submodules)
```bash
cmake . -DCMAKE_BUILD_TYPE=Release -DVI_ANGELSCRIPT=OFF -DVI_...=OFF # -DVI_CXX=17
```
Build project files while being inside of repository
```bash
cmake --build . --config Release
```

## Build: As a CMake dependency
Add Vitex toolchain. Add needed dependencies in vcpkg.json near your CMakeLists.txt if you use vcpkg:
```cmake
include(path/to/vengeance/deps/vitex/deps/toolchain.cmake)
# ...
project(app_name)
```
Add Vengeance as subproject.
```cmake
add_subdirectory(/path/to/vengeance vitex)
link_directories(/path/to/vengeance)
target_include_directories(app_name PRIVATE /path/to/vengeance)
target_link_libraries(app_name PRIVATE vitex)
```
Example [CMakeLists.txt](https://github.com/romanpunia/lynx/blob/master/CMakeLists.txt) with Vengeance as subproject
