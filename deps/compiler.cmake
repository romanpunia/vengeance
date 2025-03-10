# Setup required compiler flags and include system libs
if (MSVC)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        if (VI_BULLET3)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX512")
        elseif (VI_BULLET3)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
        endif()
    endif()
endif()
if (WIN32)
    target_link_libraries(vitex PRIVATE
        d3d11.lib
        d3dcompiler.lib
		dxguid.lib)
endif()

# Include main headers and sources as well as installation targets
target_include_directories(vitex PUBLIC ${PROJECT_SOURCE_DIR}/src/)