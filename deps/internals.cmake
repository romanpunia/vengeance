# Include headers and sources of internal dependencies
if (VI_BULLET3)
    target_compile_definitions(vitex PUBLIC -DVI_BULLET3)
    target_compile_definitions(vitex PRIVATE -DBT_NO_PROFILE)
    target_include_directories(vitex PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/BulletCollision/BroadphaseCollision
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/BulletCollision/CollisionDispatch
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/BulletCollision/CollisionShapes
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/BulletCollision/Gimpact
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/BulletCollision/NarrowPhaseCollision
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/BulletDynamics/Character
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/BulletDynamics/ConstraintSolver
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/BulletDynamics/Dynamics
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/BulletDynamics/Featherstone
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/BulletDynamics/MLCPSolvers
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/BulletDynamics/Vehicle
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/BulletSoftBody
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/LinearMath
        ${CMAKE_CURRENT_LIST_DIR}/bullet3/src/LinearMath/TaskScheduler)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_definitions(vitex PRIVATE -DBT_NO_SIMD_OPERATOR_OVERLOADS)
    endif()
    if (MSVC)
        target_compile_options(vitex PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:/wd4305>
            $<$<COMPILE_LANGUAGE:CXX>:/wd4244>
            $<$<COMPILE_LANGUAGE:CXX>:/wd4018>
            $<$<COMPILE_LANGUAGE:CXX>:/wd4267>
            $<$<COMPILE_LANGUAGE:CXX>:/wd4056>)
    endif()
endif()
if (VI_RMLUI)
    target_compile_definitions(vitex PUBLIC -DVI_RMLUI)
    target_compile_definitions(vitex PRIVATE
        -DRMLUI_STATIC_LIB
        -DRMLUI_MATRIX_ROW_MAJOR
        -DRMLUI_CUSTOM_CONFIGURATION_FILE="${PROJECT_SOURCE_DIR}/src/vengeance/layer/gui/config.hpp")
    target_include_directories(vitex PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/rmlui
        ${CMAKE_CURRENT_LIST_DIR}/rmlui/Include)
    if (VI_FREETYPE AND (Freetype_FOUND OR FREETYPE_LIBRARIES))
        target_compile_definitions(vitex PRIVATE -DRMLUI_FONT_ENGINE_FREETYPE)
        unset(Freetype_FOUND CACHE)
        unset(FREETYPE_LIBRARIES CACHE)
    endif()
endif()
if (VI_VECTORCLASS)
    target_compile_definitions(vitex PUBLIC -DVI_VECTORCLASS)
    target_include_directories(vitex PRIVATE ${CMAKE_CURRENT_LIST_DIR}/vectorclass)
endif()
if (VI_TINYFILEDIALOGS)
    target_compile_definitions(vitex PUBLIC -DVI_TINYFILEDIALOGS)
    target_include_directories(vitex PRIVATE ${CMAKE_CURRENT_LIST_DIR}/tinyfiledialogs)
endif()
if (VI_STB)
    target_compile_definitions(vitex PUBLIC -DVI_STB)
    target_compile_definitions(vitex PRIVATE -DSTB_IMAGE_IMPLEMENTATION)
    target_include_directories(vitex PRIVATE ${CMAKE_CURRENT_LIST_DIR}/stb)
endif()