# It's recommended to set a minimum CMake version.
# If you use CMake features from higher versions, update this to match.
cmake_minimum_required(VERSION 3.21)

# Set your project name. This will be the name of your SKSE .dll file.
project(
        PrecisionLocationalDamage
        VERSION 2.0.0
        LANGUAGES CXX
)

# If you're not using a mod manager, you probably want the SKSE plugin to go
# inside of your Skyrim "Data" folder.
#
# To do this automatically, set the `SKYRIM_FOLDER` environment variable
# to the path of your Skyrim Special Edition folder
if (DEFINED ENV{SKYRIM_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_FOLDER}/Data")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_FOLDER}/Data")
endif ()

# If you're using Mod Organizer 2 or Vortex, you might want this to go inside
# of your "mods" folder, inside of a subfolder named "<your mod>".
#
# To do this automatically, set the `SKYRIM_MODS_FOLDER` environment variable
# to the path of your "mods" folder
if (DEFINED ENV{SKYRIM_MODS_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_MODS_FOLDER}")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_MODS_FOLDER}/${PROJECT_NAME}")
endif ()

# Otherwise, you can set OUTPUT_FOLDER to any place you'd like :)
# set(OUTPUT_FOLDER "C:/path/to/any/folder")

# out-of-source builds only (include guards)
if (${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
    message(FATAL_ERROR "In-source builds are not allowed.")
endif ()

# Setup your SKSE plugin as an SKSE plugin!
add_definitions(-DUNICODE -D_UNICODE)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

# Release optimization flag, turned off for debugging
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O0")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O0")


file(GLOB SOURCE_FILES "src/*.cpp" "src/*.h")

include(GNUInstallDirs)
include(FetchContent)

set(SPDLOG_INSTALL ON)

# in-place configuration
source_group(
        TREE
        ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
        ${SOURCE_FILES}
)

#FetchContent_Declare(
#        fmt
#        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
#        GIT_TAG master
#        OVERRIDE_FIND_PACKAGE
#)
#FetchContent_MakeAvailable(fmt)
#find_package(fmt CONFIG REQUIRED)

#find_package(boost-algorithm CONFIG REQUIRED)
#find_package(rapidxml CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

FetchContent_Declare(
        CommonLibSSE
        GIT_REPOSITORY https://github.com/CharmedBaryon/CommonLibSSE-NG.git
        GIT_TAG main
        OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(CommonLibSSE)
include(${CommonLibSSE_SOURCE_DIR}/cmake/CommonLibSSE.cmake) # for the add_commonlibsse_plugin to work

add_commonlibsse_plugin(
        ${PROJECT_NAME}
        SOURCES
        ${headers}
        ${SOURCE_FILES}
)

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        fmt::fmt
        spdlog::spdlog
        yaml-cpp::yaml-cpp
        CommonLibSSE::CommonLibSSE
)

target_compile_options(
        ${PROJECT_NAME}
        PRIVATE
        /W0
        /utf-8
        /MP
        /await
        /permissive-
        /Zc:alignedNew
        /Zc:auto
        /Zc:__cplusplus
        /Zc:externC
        /Zc:externConstexpr
        /Zc:forScope
        /Zc:hiddenFriend
        /Zc:implicitNoexcept
        /Zc:lambda
        /Zc:noexceptTypes
        /Zc:preprocessor
        /Zc:referenceBinding
        /Zc:rvalueCast
        /Zc:sizedDealloc
        /Zc:strictStrings
        /Zc:ternary
        /Zc:threadSafeInit
        /Zc:trigraphs
        /Zc:wchar_t
        /wd4200 # nonstandard extension used : zero-sized array in struct/union
)

target_compile_features(
        ${PROJECT_NAME}
        PRIVATE
        cxx_std_23
) # <--- use C++23 standard

target_precompile_headers(
        ${PROJECT_NAME}
        PRIVATE
        src/PCH.h
) # <--- PCH.h is required!

# When your SKSE .dll is compiled, this will automatically copy the .dll into your mods folder.
# Only works if you configure DEPLOY_ROOT above (or set the SKYRIM_MODS_FOLDER environment variable)
if (DEFINED OUTPUT_FOLDER)
    # If you specify an <OUTPUT_FOLDER> (including via environment variables)
    # then we'll copy your mod files into Skyrim or a mod manager for you!

    # Copy the SKSE plugin .dll files into the SKSE/Plugins/ folder
    set(DLL_FOLDER "${OUTPUT_FOLDER}/SKSE/Plugins")

    message(STATUS "SKSE plugin output folder: ${DLL_FOLDER}")

    add_custom_command(
            TARGET "${PROJECT_NAME}"
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
            VERBATIM
    )

    # If you perform a "Debug" build, also copy .pdb file (for debug symbols)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_custom_command(
                TARGET "${PROJECT_NAME}"
                POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
                VERBATIM
        )
    endif ()
endif ()
