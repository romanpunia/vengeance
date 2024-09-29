<br/>
<div align="center">
    <br />
    <img src="https://github.com/romanpunia/vengeance/blob/main/var/assets/logo.png?raw=true" alt="Vengeance Logo" width="300" />
    <h3>C++ Cross-Platform Framework</h3>
</div>
<div align="center">

  [![Build Status](https://github.com/romanpunia/vengeance/workflows/CMake/badge.svg)](https://github.com/eclipse-theia/theia/actions?query=branch%3Amaster+event%3Apush+event%3Aschedule)

</div>

## About ⚔️
Vengeance is a cross-platform game engine extension over Vitex framework. It's a 3D optimized engine, there is a posibility to render efficient 2D graphics with batching but that is not a priority. Main shading language is HLSL, it is transpiled or directly compiled and saved to cache when needed.

Rendering is based on stacking, that way you specify renderers for a camera (order matters), when rendering is initiated we process each renderer step by step, renderer can initiate sub-pass to render another part of scene for shadows, reflections, transparency and others. Before rendering begins, render lists are prepared for geometry rendering based on culling results, for sub-passes we use frustum and indexed culling, for main passes we also use occlusion culling.

Culling, physics, audio and rendering are done multhreaded if possible. In best case, we don't have any synchronization between threads but when we need to write to shared data, for example, destroy a rigid body owned by physics engine, we use scene transactions that are just callbacks that are guaranteed to be executed thread safe (in scope of scene) which in turn makes scene eventual consistent as transactions are fired later when all parallel tasks are finished.

Another important aspect of Vengeance is schemas, they are used to serialize and deserialize data, their main purpose is to provide containers for serialized game states such as meshes, animations, materials, scenes, configurations and other.

## Documentation
You may take a look at Wiki pages. There are some practical usage examples that can be drag-and-drop executed.

## Cross platform (tested platforms)
+ Windows 7/8/8.1/10+ x64/x86 (MSVC, MSVC ClangCL, MinGW)
+ Raspberian 3+ ARM (GCC)
+ Solaris 9+ x64/x86 (GCC)
+ FreeBSD 11+ x64/x86 (GCC)
+ Debian 11+ x64/x86 (GCC, LLVM)
+ Ubuntu 16.04+ x64/x86 (GCC)
+ MacOS Catalina 10.15+ x64 (Xcode)

## Building
There are several ways to build this project that are explained here:
* [Manually performed builds](var/MANUAL.md)
* [Precomposed docker builds](var/DOCKER.md)

### Configuration
+ **VI_CXX** is the C++ standard (17, 20, 23) which will be used, C++20 and higher will replace stateful coroutines with std coroutines, defaults to 20
+ **VI_LOGGING** is a logging level (errors, warnings, default, debug, verbose), defaults to "default"
+ **VI_PESSIMISTIC** will enable assertion statements in release mode, defaults to OFF
+ **VI_BINDINGS** will enable full script bindings otherwise only essentials will be used to reduce lib size, defaults to ON
+ **VI_ALLOCATOR** will enable custom allocator for all used standard containers, making them incompatible with std::allocator based ones but adding opportunity to use pool allocator, defaults to ON
+ **VI_FCONTEXT** will enable internal fcontext implementation for coroutines, defaults to ON
+ **VI_SHADERS** to embed shaders to **/src/vengeance/graphics/shaders/bundle.hpp**, defaults to ON

## Dependencies
* [concurrentqueue (submodule)](https://github.com/cameron314/concurrentqueue)

## Serialization features
* [pugixml (submodule)](https://github.com/zeux/pugixml): VI_PUGIXML=ON
* [rapidjson (submodule)](https://github.com/tencent/rapidjson): VI_RAPIDJSON=ON
* [stb (submodule)](https://github.com/nothings/stb): VI_STB=ON
* [assimp (so)](https://github.com/assimp/assimp): VI_ASSIMP=ON
* [zlib (so)](https://github.com/madler/zlib): VI_ZLIB=ON
* [freetype (so)](https://github.com/freetype/freetype): VI_FREETYPE=ON
* [spirv (so)](https://github.com/KhronosGroup/SPIRV-Cross): VI_SPIRV=ON
* [glslang (so)](https://github.com/KhronosGroup/glslang): VI_SPIRV=ON

## Networking features
* [wepoll (submodule)](https://github.com/piscisaureus/wepoll): VI_WEPOLL=ON
* [openssl (so)](https://github.com/openssl/openssl): VI_OPENSSL=ON
* [mongo-c-driver (so)](https://github.com/mongodb/mongo-c-driver): VI_MONGOC=ON
* [libpq (so)](https://github.com/postgres/postgres/tree/master/src/interfaces/libpq): VI_POSTGRESQL=ON
* [sqlite (so)](https://github.com/sqlite/sqlite): VI_SQLITE=ON

## Graphics features
* [d3d11 (so)](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
* [opengl (so)](https://github.com/KhronosGroup/OpenGL-Registry): VI_OPENGL=ON
* [glew (so)](https://github.com/nigels-com/glew): VI_GLEW=ON
* [sdl2 (so)](https://www.libsdl.org/download-2.0.php): VI_SDL2=ON

## GUI features
* [rmlui (submodule)](https://github.com/mikke89/RmlUi): VI_RMLUI=ON
* [tinyfiledialogs (submodule)](https://github.com/native-toolkit/tinyfiledialogs): VI_TINYFILEDIALOGS=ON

## Other features
* Physics [bullet3 (submodule)](https://github.com/bulletphysics/bullet3): VI_BULLET3=ON
* Scripting [angelscript (submodule)](https://github.com/codecat/angelscript-mirror): VI_ANGELSCRIPT=ON
* Performance [vectorclass (submodule)](https://github.com/vectorclass/version1): VI_VECTORCLASS=ON
* Debugging [backward-cpp (submodule)](https://github.com/bombela/backward-cpp): VI_BACKWARDCPP=ON
* Audio [openal-soft (so)](https://github.com/kcat/openal-soft): VI_OPENAL=ON

## License
Vengeance is licensed under the MIT license

## Known Issues
Documentation in it's usual form is non-existant at the moment. In the nearest future that could be changed.