# Create sources list with main sources
file(GLOB_RECURSE SUBSOURCE
    ${PROJECT_SOURCE_DIR}/src/vengeance/*.inl
    ${PROJECT_SOURCE_DIR}/src/vengeance/*.h
    ${PROJECT_SOURCE_DIR}/src/vengeance/*.c
    ${PROJECT_SOURCE_DIR}/src/vengeance/*.cc
    ${PROJECT_SOURCE_DIR}/src/vengeance/*.hpp
    ${PROJECT_SOURCE_DIR}/src/vengeance/*.cpp
    ${PROJECT_SOURCE_DIR}/src/vengeance/*.hxx
    ${PROJECT_SOURCE_DIR}/src/vengeance/*.cxx)
list(APPEND SOURCE ${SUBSOURCE})

# Append shaders into the sources list
set(VI_SHADERS ON CACHE BOOL "Enable built-in shaders")
set(BUFFER_OUT "${PROJECT_SOURCE_DIR}/src/vengeance/graphics/shaders/bundle")
if (VI_SHADERS)
	set(BUFFER_DIR "${PROJECT_SOURCE_DIR}/src/vengeance/graphics/shaders")
    set(BUFFER_DATA "#ifndef HAS_SHADER_BUNDLE\n#define HAS_SHADER_BUNDLE\n\nnamespace shader_bundle\n{\n\tvoid foreach(void* context, void(*callback)(void*, const char*, const unsigned char*, unsigned))\n\t{\n\t\tif (!callback)\n\t\t\treturn;\n")
    file(GLOB_RECURSE SOURCE_SHADERS ${BUFFER_DIR}/*.hlsl)
    foreach(BINARY ${SOURCE_SHADERS})
        string(REPLACE "${BUFFER_DIR}" "" FILENAME ${BINARY})
        string(REPLACE "${BUFFER_DIR}/" "" FILENAME ${BINARY})
        string(REGEX REPLACE "\\.| |-" "_" VARNAME ${FILENAME})
        string(REPLACE "/" "_" VARNAME ${VARNAME})
        string(TOLOWER ${VARNAME} VARNAME)
        file(READ ${BINARY} FILEDATA LIMIT 32768)
        if (NOT FILEDATA STREQUAL "")
            string(REGEX REPLACE "\t" "" FILEDATA "${FILEDATA}")
            string(REGEX REPLACE "[\r\n][\r\n]" "\n" FILEDATA "${FILEDATA}")
            string(REGEX REPLACE "  *" " " FILEDATA "${FILEDATA}")
            string(REGEX REPLACE "[\r\n]" "\\\\n" FILEDATA "${FILEDATA}")
            string(REGEX REPLACE "\\\"" "\\\\\"" FILEDATA "${FILEDATA}")
            string(APPEND BUFFER_DATA "\n\t\tconst unsigned char ${VARNAME}[] = \"${FILEDATA}\";\n\t\tcallback(context, \"${FILENAME}\", ${VARNAME}, sizeof(${VARNAME}));\n")
        endif()
    endforeach()
    string(APPEND BUFFER_DATA "\t}\n}\n#endif")
    file(WRITE ${BUFFER_OUT}.hpp "${BUFFER_DATA}")
    list(APPEND SOURCE ${SOURCE_SHADERS})
    list(APPEND SOURCE "${BUFFER_OUT}.hpp")
    set_source_files_properties(${SOURCE_SHADERS} PROPERTIES VS_TOOL_OVERRIDE "None")
    message(STATUS "Shaders have been written to: ${BUFFER_OUT}.hpp")
else()
    file(WRITE "${BUFFER_OUT}.hpp" "")
    message(STATUS "Shaders have been erased from: ${BUFFER_OUT}.hpp")
endif()

# Append source files of dependencies
set(VI_BULLET3 ON CACHE BOOL "Enable bullet3 built-in library")
set(VI_RMLUI ON CACHE BOOL "Enable rmlui built-in library")
set(VI_FREETYPE ON CACHE BOOL "Enable freetype library")
set(VI_TINYFILEDIALOGS ON CACHE BOOL "Enable tinyfiledialogs built-in library")
set(VI_STB ON CACHE BOOL "Enable stb built-in library")
set(VI_VECTORCLASS OFF CACHE BOOL "Enable vectorclass built-in library (release mode perf. gain)")
if (VI_BULLET3)
	file(GLOB_RECURSE SOURCE_BULLET3
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/BulletCollision/*.h*
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/BulletCollision/*.cpp*
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/BulletDynamics/*.h*
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/BulletDynamics/*.cpp*
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/BulletSoftBody/*.h*
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/BulletSoftBody/*.cpp*
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/LinearMath/*.h*
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/LinearMath/*.cpp*
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/btBulletCollisionCommon.h
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/btBulletDynamicsCommon.h)
	list(APPEND SOURCE ${SOURCE_BULLET3})
	message(STATUS "Bullet3 library enabled")
endif()
if (VI_RMLUI)
    file(GLOB_RECURSE SOURCE_RMLUI_ALL
        ${CMAKE_CURRENT_LIST_DIR}/rmlui/Include/RmlUi/Core.h
        ${CMAKE_CURRENT_LIST_DIR}/rmlui/Include/RmlUi/Config/*.h*
        ${CMAKE_CURRENT_LIST_DIR}/rmlui/Include/RmlUi/Core/*.h*
        ${CMAKE_CURRENT_LIST_DIR}/rmlui/Include/RmlUi/Core/*.hpp*
        ${CMAKE_CURRENT_LIST_DIR}/rmlui/Include/RmlUi/Core/*.inl*
        ${CMAKE_CURRENT_LIST_DIR}/rmlui/Source/Core/Elements/*.h*
        ${CMAKE_CURRENT_LIST_DIR}/rmlui/Source/Core/Elements/*.cpp*
        ${CMAKE_CURRENT_LIST_DIR}/rmlui/Source/Core/Layout/*.h*
        ${CMAKE_CURRENT_LIST_DIR}/rmlui/Source/Core/Layout/*.cpp*)
    file(GLOB SOURCE_RMLUI
        ${CMAKE_CURRENT_LIST_DIR}/rmlui/Source/Core/*.h
        ${CMAKE_CURRENT_LIST_DIR}/rmlui/Source/Core/*.cpp)
	find_path(FREETYPE_LOCATION ft2build.h PATH_SUFFIXES "freetype2")
	if (FREETYPE_LOCATION)
		find_package(Freetype QUIET)
		if (NOT Freetype_FOUND)
			find_library(Freetype_FOUND "freetype")
		endif()
	endif()
	if (VI_FREETYPE AND (Freetype_FOUND OR FREETYPE_LIBRARIES))
        file(GLOB SOURCE_RMLUI_FONT_ENGINE
            ${CMAKE_CURRENT_LIST_DIR}/rmlui/Source/Core/FontEngineDefault/*.h*
            ${CMAKE_CURRENT_LIST_DIR}/rmlui/Source/Core/FontEngineDefault/*.cpp*)
        list(APPEND SOURCE_RMLUI ${SOURCE_RMLUI_FONT_ENGINE})
    else()
        message("RmlUi default font engine disabled")
	endif()
	list(APPEND SOURCE
        ${SOURCE_RMLUI_ALL}
        ${SOURCE_RMLUI})
	message(STATUS "RmlUi library enabled")
    unset(Freetype_FOUND CACHE)
    unset(FREETYPE_LIBRARIES CACHE)
    unset(FREETYPE_LOCATION CACHE)
	unset(FREETYPE_DIR CACHE)
	unset(FREETYPE_LOCATION CACHE)
endif()
if (VI_TINYFILEDIALOGS)
    list(APPEND SOURCE 
        ${CMAKE_CURRENT_LIST_DIR}/tinyfiledialogs/tinyfiledialogs.h
        ${CMAKE_CURRENT_LIST_DIR}/tinyfiledialogs/tinyfiledialogs.c)
endif()
if (VI_STB)
    file(GLOB SOURCE_STB
        ${CMAKE_CURRENT_LIST_DIR}/stb/*.h
        ${CMAKE_CURRENT_LIST_DIR}/stb/*.c)
    list(APPEND SOURCE ${SOURCE_STB})
endif()
if (VI_VECTORCLASS)
	file(GLOB_RECURSE SOURCE_SIMD ${CMAKE_CURRENT_LIST_DIR}/vectorclass/*.h*)
	list(APPEND SOURCE ${SOURCE_SIMD})
	message(STATUS "Simd instructions using vectorclass enabled")
endif()

# Group all sources for nice IDE preview
foreach(ITEM IN ITEMS ${SOURCE})
    get_filename_component(ITEM_PATH "${ITEM}" PATH)
    string(REPLACE "${PROJECT_SOURCE_DIR}" "" ITEM_GROUP "${ITEM_PATH}")
    string(REPLACE "/deps/vitex/src/" "src/" ITEM_GROUP "${ITEM_GROUP}")
    string(REPLACE "/" "\\" ITEM_GROUP "${ITEM_GROUP}")
    source_group("${ITEM_GROUP}" FILES "${ITEM}")
endforeach()