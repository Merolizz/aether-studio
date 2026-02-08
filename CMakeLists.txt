cmake_minimum_required(VERSION 3.20)
project(AetherStudio VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CMake 3.28+ removed compatibility with <3.5 in third-party projects that
# still declare very old `cmake_minimum_required()` values (e.g. some releases
# of nlohmann/json). Setting this avoids hard errors during FetchContent
# configure steps.
if(NOT DEFINED CMAKE_POLICY_VERSION_MINIMUM)
    set(CMAKE_POLICY_VERSION_MINIMUM 3.10 CACHE STRING "Minimum policy version for dependencies")
endif()

# Generate compile_commands.json for IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# CMAKE_INSTALL_PREFIX is set by CMakeSettings.json or command line
# Don't redefine it here to avoid conflicts

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Qt6 frontend (optional second executable: AetherStudioQt)
set(AETHER_QT6_FRONTEND OFF CACHE BOOL "Build Qt6 frontend executable (AetherStudioQt)")
# Optional: set to your Qt6 install dir (e.g. C:/Qt/6.7.0/msvc2019_64 or C:/Qt/6.x.x/mingw_64) to find Qt6
set(Qt6_ROOT "" CACHE PATH "Qt6 install directory (used when AETHER_QT6_FRONTEND=ON)")

# DirectX 11/12 option (Windows only)
set(AETHER_USE_DIRECTX OFF CACHE BOOL "Use DirectX 11/12 instead of Vulkan")
set(AETHER_DIRECTX_VERSION 12 CACHE STRING "DirectX version when AETHER_USE_DIRECTX=ON (11 or 12)")
if(WIN32 AND AETHER_USE_DIRECTX)
    if(AETHER_DIRECTX_VERSION STREQUAL "11")
        set(AETHER_DIRECTX_11 ON)
        set(AETHER_DIRECTX_12 OFF)
    else()
        set(AETHER_DIRECTX_11 OFF)
        set(AETHER_DIRECTX_12 ON)
    endif()
    message(STATUS "DirectX ${AETHER_DIRECTX_VERSION} enabled (Vulkan disabled)")
endif()

# Find Vulkan
# Allow manual specification of Vulkan SDK path
if(NOT DEFINED VULKAN_SDK)
    # Common default install path (helps when env var isn't set)
    if(WIN32 AND EXISTS "C:/VulkanSDK/1.4.335.0")
        set(VULKAN_SDK "C:/VulkanSDK/1.4.335.0")
        message(STATUS "Using Vulkan SDK at: ${VULKAN_SDK}")
    endif()

    # First check environment variable
    if(DEFINED ENV{VULKAN_SDK} AND EXISTS "$ENV{VULKAN_SDK}")
        set(VULKAN_SDK "$ENV{VULKAN_SDK}")
        message(STATUS "Found VULKAN_SDK from environment: ${VULKAN_SDK}")
    else()
        # Try common Windows installation locations and find latest version
        set(POSSIBLE_PATHS
            "C:/VulkanSDK"
            "$ENV{ProgramFiles}/VulkanSDK"
            "C:/Program Files (x86)/VulkanSDK"
        )
        
        set(FOUND_SDK_PATH "")
        foreach(BASE_PATH ${POSSIBLE_PATHS})
            if(EXISTS "${BASE_PATH}")
                # Look for versioned subdirectories (e.g., 1.4.328.1)
                file(GLOB VERSION_DIRS "${BASE_PATH}/*")
                foreach(VERSION_DIR ${VERSION_DIRS})
                    if(IS_DIRECTORY "${VERSION_DIR}" AND EXISTS "${VERSION_DIR}/Include/vulkan/vulkan.h")
                        set(FOUND_SDK_PATH "${VERSION_DIR}")
                        break()
                    endif()
                endforeach()
                
                # Also check if the base path itself contains the SDK
                if(NOT FOUND_SDK_PATH AND EXISTS "${BASE_PATH}/Include/vulkan/vulkan.h")
                    set(FOUND_SDK_PATH "${BASE_PATH}")
                endif()
                
                if(FOUND_SDK_PATH)
                    break()
                endif()
            endif()
        endforeach()
        
        if(FOUND_SDK_PATH)
            set(VULKAN_SDK "${FOUND_SDK_PATH}")
            message(STATUS "Found Vulkan SDK at: ${VULKAN_SDK}")
        endif()
    endif()
endif()

# If VULKAN_SDK is set, help CMake find it
if(DEFINED VULKAN_SDK)
    # Set the Vulkan_DIR to help find_package locate it
    if(EXISTS "${VULKAN_SDK}/CMake")
        set(Vulkan_DIR "${VULKAN_SDK}/CMake")
    endif()
    # Add to prefix path
    set(CMAKE_PREFIX_PATH "${VULKAN_SDK};${CMAKE_PREFIX_PATH}")
    # Set include and library paths directly if needed
    if(EXISTS "${VULKAN_SDK}/Include")
        set(Vulkan_INCLUDE_DIR "${VULKAN_SDK}/Include" CACHE PATH "Vulkan include directory")
    endif()
    if(EXISTS "${VULKAN_SDK}/Lib")
        set(Vulkan_LIBRARY_DIR "${VULKAN_SDK}/Lib" CACHE PATH "Vulkan library directory")
    endif()
    message(STATUS "Configuring CMake to use Vulkan SDK from: ${VULKAN_SDK}")
endif()

# Try to find Vulkan.
#
# NOTE:
# Visual Studio's bundled CMake (some environments) can error inside the built-in
# `FindVulkan.cmake` / `FindPackageHandleStandardArgs.cmake` with:
#   "Flow control statements are not properly nested."
# To keep configuration reliable, we avoid `find_package(Vulkan)` and instead
# locate Vulkan directly (prefer VULKAN_SDK when available).
set(Vulkan_FOUND FALSE)
set(Vulkan_INCLUDE_DIRS "")
set(Vulkan_LIBRARIES "")

set(_AETHER_VULKAN_HINT_INCLUDE "")
set(_AETHER_VULKAN_HINT_LIB "")
if(DEFINED VULKAN_SDK)
    set(_AETHER_VULKAN_HINT_INCLUDE "${VULKAN_SDK}/Include")
    set(_AETHER_VULKAN_HINT_LIB "${VULKAN_SDK}/Lib")
endif()

# Vulkan SDK on Windows is typically installed into a versioned subfolder:
#   C:/VulkanSDK/1.3.xxx.x/{Include,Lib}
# The previous logic only searched C:/VulkanSDK directly, which fails unless
# VULKAN_SDK is set. Collect candidate versioned SDK roots and use them as hints.
set(_AETHER_VULKAN_SDK_ROOT_HINTS "")
if(DEFINED VULKAN_SDK)
    list(APPEND _AETHER_VULKAN_SDK_ROOT_HINTS "${VULKAN_SDK}")
endif()
if(DEFINED ENV{VULKAN_SDK})
    list(APPEND _AETHER_VULKAN_SDK_ROOT_HINTS "$ENV{VULKAN_SDK}")
endif()

set(_AETHER_VULKAN_SDK_GLOB_ROOTS
    "C:/VulkanSDK"
    "$ENV{ProgramFiles}/VulkanSDK"
    "C:/Program Files (x86)/VulkanSDK"
)
foreach(_root IN LISTS _AETHER_VULKAN_SDK_GLOB_ROOTS)
    if(EXISTS "${_root}")
        file(GLOB _aether_vk_versions LIST_DIRECTORIES true "${_root}/*")
        foreach(_cand IN LISTS _aether_vk_versions)
            if(IS_DIRECTORY "${_cand}")
                list(APPEND _AETHER_VULKAN_SDK_ROOT_HINTS "${_cand}")
            endif()
        endforeach()
    endif()
endforeach()

list(REMOVE_DUPLICATES _AETHER_VULKAN_SDK_ROOT_HINTS)

set(_AETHER_VULKAN_INCLUDE_HINTS "${_AETHER_VULKAN_HINT_INCLUDE}" "$ENV{VULKAN_SDK}/Include")
set(_AETHER_VULKAN_LIB_HINTS "${_AETHER_VULKAN_HINT_LIB}" "$ENV{VULKAN_SDK}/Lib")
foreach(_sdk_root IN LISTS _AETHER_VULKAN_SDK_ROOT_HINTS)
    list(APPEND _AETHER_VULKAN_INCLUDE_HINTS "${_sdk_root}/Include")
    list(APPEND _AETHER_VULKAN_LIB_HINTS "${_sdk_root}/Lib")
endforeach()

# Fast-path: if VULKAN_SDK is known, set include/lib directly.
# This avoids edge-cases where find_path/find_library miss versioned installs.
if(NOT Vulkan_INCLUDE_DIRS AND DEFINED VULKAN_SDK AND EXISTS "${VULKAN_SDK}/Include/vulkan/vulkan.h")
    set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK}/Include")
endif()
if(NOT Vulkan_LIBRARIES AND DEFINED VULKAN_SDK)
    if(EXISTS "${VULKAN_SDK}/Lib/vulkan-1.lib")
        set(Vulkan_LIBRARIES "${VULKAN_SDK}/Lib/vulkan-1.lib")
    elseif(EXISTS "${VULKAN_SDK}/Lib32/vulkan-1.lib")
        set(Vulkan_LIBRARIES "${VULKAN_SDK}/Lib32/vulkan-1.lib")
    endif()
endif()

find_path(Vulkan_INCLUDE_DIRS
    NAMES vulkan/vulkan.h
    HINTS
        ${_AETHER_VULKAN_INCLUDE_HINTS}
    PATHS
        "C:/VulkanSDK"
        "$ENV{ProgramFiles}/VulkanSDK"
        "C:/Program Files (x86)/VulkanSDK"
    PATH_SUFFIXES
        Include
)

find_library(Vulkan_LIBRARIES
    NAMES vulkan-1 vulkan-1.lib
    HINTS
        ${_AETHER_VULKAN_LIB_HINTS}
    PATHS
        "C:/VulkanSDK"
        "$ENV{ProgramFiles}/VulkanSDK"
        "C:/Program Files (x86)/VulkanSDK"
    PATH_SUFFIXES
        Lib
        Lib32
)

if(Vulkan_INCLUDE_DIRS AND Vulkan_LIBRARIES)
    set(Vulkan_FOUND TRUE)
endif()

if(NOT Vulkan_FOUND)
    if(WIN32 AND AETHER_USE_DIRECTX)
        message(STATUS "Vulkan not required (using DirectX)")
    else()
        message("")
        message("==========================================")
        message("Vulkan SDK not found!")
        message("==========================================")
        message("Please install the Vulkan SDK from: https://vulkan.lunarg.com/sdk/home")
        message("")
        message("After installation, you can either:")
        message("  1. Set the VULKAN_SDK environment variable to the SDK path")
        message("     (e.g., C:/VulkanSDK/1.3.xxx.x)")
        message("  2. Or specify it when configuring CMake:")
        message("     cmake -DVULKAN_SDK=C:/VulkanSDK/1.3.xxx.x ...")
        message("  Or on Windows, use DirectX: cmake -DAETHER_USE_DIRECTX=ON ...")
        message("==========================================")
        message("")
        message(FATAL_ERROR "Vulkan SDK is required but not found. Please install it and try again.")
    endif()
else()
    message(STATUS "Vulkan found")
    message(STATUS "Vulkan include dir: ${Vulkan_INCLUDE_DIRS}")
    message(STATUS "Vulkan libraries: ${Vulkan_LIBRARIES}")
    include_directories("${Vulkan_INCLUDE_DIRS}")
endif()

# Find FFmpeg (optional but recommended for video processing)
# Try multiple methods to find FFmpeg on Windows
# You can manually specify FFmpeg path: cmake -DFFMPEG_ROOT=C:/path/to/ffmpeg
set(FFMPEG_FOUND FALSE)

# Allow manual specification of FFmpeg path
if(DEFINED FFMPEG_ROOT)
    if(EXISTS "${FFMPEG_ROOT}/include/libavcodec/avcodec.h")
        set(FFMPEG_FOUND TRUE)
        set(FFMPEG_INCLUDE_DIRS "${FFMPEG_ROOT}/include")
        # Find libraries
        set(FFMPEG_LIBS "")
        foreach(LIB_NAME avcodec avformat avutil swscale swresample)
            # Check for .lib first (Windows static/shared)
            if(EXISTS "${FFMPEG_ROOT}/lib/${LIB_NAME}.lib")
                list(APPEND FFMPEG_LIBS "${FFMPEG_ROOT}/lib/${LIB_NAME}.lib")
            # Check for lib prefix .lib (some builds use libavcodec.lib)
            elseif(EXISTS "${FFMPEG_ROOT}/lib/lib${LIB_NAME}.lib")
                list(APPEND FFMPEG_LIBS "${FFMPEG_ROOT}/lib/lib${LIB_NAME}.lib")
            # Check for .a (MinGW static)
            elseif(EXISTS "${FFMPEG_ROOT}/lib/lib${LIB_NAME}.a")
                list(APPEND FFMPEG_LIBS "${FFMPEG_ROOT}/lib/lib${LIB_NAME}.a")
            # Check for .a without lib prefix
            elseif(EXISTS "${FFMPEG_ROOT}/lib/${LIB_NAME}.a")
                list(APPEND FFMPEG_LIBS "${FFMPEG_ROOT}/lib/${LIB_NAME}.a")
            endif()
        endforeach()
        if(FFMPEG_LIBS)
            set(FFMPEG_LIBRARIES ${FFMPEG_LIBS})
            message(STATUS "FFmpeg found at (manual): ${FFMPEG_ROOT}")
        else()
            message(STATUS "FFmpeg path specified (${FFMPEG_ROOT}) but libraries not found")
            message(STATUS "  Looking for: ${FFMPEG_ROOT}/lib/*.lib")
        endif()
    else()
        message(STATUS "FFmpeg path specified (${FFMPEG_ROOT}) but include files not found")
        message(STATUS "  Looking for: ${FFMPEG_ROOT}/include/libavcodec/avcodec.h")
        message(STATUS "  Current contents: ${FFMPEG_ROOT}")
        if(EXISTS "${FFMPEG_ROOT}")
            file(GLOB FFMPEG_CONTENTS "${FFMPEG_ROOT}/*")
            foreach(ITEM ${FFMPEG_CONTENTS})
                get_filename_component(ITEM_NAME ${ITEM} NAME)
                message(STATUS "    - ${ITEM_NAME}")
            endforeach()
        endif()
    endif()
endif()

# Method 1: Try pkg-config (Linux/macOS)
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(FFMPEG QUIET libavcodec libavformat libavutil libswscale libswresample)
endif()

# Method 2: Try common Windows installation paths
if(NOT FFMPEG_FOUND AND WIN32)
    # Check C:/ffmpeg first (most common location for full dev builds)
    # First check for standard dev build structure (include/lib)
    if(EXISTS "C:/ffmpeg/include/libavcodec/avcodec.h")
        set(FFMPEG_POSSIBLE_PATHS "C:/ffmpeg")
    # Check for source code structure (libavcodec/avcodec.h directly)
    elseif(EXISTS "C:/ffmpeg/libavcodec/avcodec.h")
        # This is source code structure - check if there's a build directory
        if(EXISTS "C:/ffmpeg/build/include/libavcodec/avcodec.h")
            set(FFMPEG_POSSIBLE_PATHS "C:/ffmpeg/build")
        elseif(EXISTS "C:/ffmpeg/out/include/libavcodec/avcodec.h")
            set(FFMPEG_POSSIBLE_PATHS "C:/ffmpeg/out")
        elseif(EXISTS "C:/ffmpeg/install/include/libavcodec/avcodec.h")
            set(FFMPEG_POSSIBLE_PATHS "C:/ffmpeg/install")
        else()
            # Source code structure - will need to use source headers directly
            # Note: This requires building FFmpeg first to get libraries
            message(STATUS "FFmpeg source code found at C:/ffmpeg")
            message(STATUS "  Note: You need to build FFmpeg first to get .lib files")
            message(STATUS "  Or use a pre-built development package with include/ and lib/ folders")
        endif()
    endif()
    
    # Standard paths list
    if(NOT FFMPEG_FOUND)
        set(FFMPEG_POSSIBLE_PATHS
            "C:/ffmpeg"
            "C:/Program Files/ffmpeg"
            "C:/Program Files (x86)/ffmpeg"
            "$ENV{ProgramFiles}/ffmpeg"
            "C:/Program Files (x86)/ffmpeg"
            "C:/tools/ffmpeg"
            "D:/ffmpeg"
            "E:/ffmpeg"
            "$ENV{USERPROFILE}/ffmpeg"
            "C:/vcpkg/installed/x64-windows"
            "$ENV{VCPKG_ROOT}/installed/x64-windows"
        )
    endif()
    
    # Also check if ffmpeg.exe is in PATH and try to find dev files nearby
    find_program(FFMPEG_EXE ffmpeg PATHS ENV PATH)
    if(FFMPEG_EXE)
        get_filename_component(FFMPEG_BIN_DIR "${FFMPEG_EXE}" DIRECTORY)
        get_filename_component(FFMPEG_ROOT_DIR "${FFMPEG_BIN_DIR}" DIRECTORY)
        # Check if dev files are in the same directory as bin (common for full dev builds)
        if(EXISTS "${FFMPEG_ROOT_DIR}/include/libavcodec/avcodec.h")
            list(INSERT FFMPEG_POSSIBLE_PATHS 0 "${FFMPEG_ROOT_DIR}")
        else()
            list(APPEND FFMPEG_POSSIBLE_PATHS "${FFMPEG_ROOT_DIR}")
        endif()
        # Also check parent directories (for WinGet installations)
        get_filename_component(FFMPEG_PARENT "${FFMPEG_ROOT_DIR}" DIRECTORY)
        if(EXISTS "${FFMPEG_PARENT}/include/libavcodec/avcodec.h")
            list(INSERT FFMPEG_POSSIBLE_PATHS 0 "${FFMPEG_PARENT}")
        else()
            list(APPEND FFMPEG_POSSIBLE_PATHS "${FFMPEG_PARENT}")
        endif()
        get_filename_component(FFMPEG_GRANDPARENT "${FFMPEG_PARENT}" DIRECTORY)
        if(EXISTS "${FFMPEG_GRANDPARENT}/include/libavcodec/avcodec.h")
            list(INSERT FFMPEG_POSSIBLE_PATHS 0 "${FFMPEG_GRANDPARENT}")
        else()
            list(APPEND FFMPEG_POSSIBLE_PATHS "${FFMPEG_GRANDPARENT}")
        endif()
    endif()
    
    # Check WinGet installation path specifically
    if(EXISTS "$ENV{LOCALAPPDATA}/Microsoft/WinGet/Packages")
        file(GLOB WINGET_FFMPEG_DIRS "$ENV{LOCALAPPDATA}/Microsoft/WinGet/Packages/*/ffmpeg-*-full_build")
        foreach(WINGET_DIR ${WINGET_FFMPEG_DIRS})
            list(APPEND FFMPEG_POSSIBLE_PATHS "${WINGET_DIR}")
        endforeach()
    endif()
    
    foreach(FFMPEG_PATH ${FFMPEG_POSSIBLE_PATHS})
        # Check for standard dev build structure (include/libavcodec/avcodec.h)
        if(EXISTS "${FFMPEG_PATH}/include/libavcodec/avcodec.h")
            set(FFMPEG_FOUND TRUE)
            set(FFMPEG_INCLUDE_DIRS "${FFMPEG_PATH}/include")
            # Try to find libraries (both .lib and .a)
            set(FFMPEG_LIBS "")
            foreach(LIB_NAME avcodec avformat avutil swscale swresample)
                # Check for .lib first (Windows static/shared)
                if(EXISTS "${FFMPEG_PATH}/lib/${LIB_NAME}.lib")
                    list(APPEND FFMPEG_LIBS "${FFMPEG_PATH}/lib/${LIB_NAME}.lib")
                # Check for lib prefix .lib (some builds use libavcodec.lib)
                elseif(EXISTS "${FFMPEG_PATH}/lib/lib${LIB_NAME}.lib")
                    list(APPEND FFMPEG_LIBS "${FFMPEG_PATH}/lib/lib${LIB_NAME}.lib")
                # Check for .a (MinGW static)
                elseif(EXISTS "${FFMPEG_PATH}/lib/lib${LIB_NAME}.a")
                    list(APPEND FFMPEG_LIBS "${FFMPEG_PATH}/lib/lib${LIB_NAME}.a")
                # Check for .a without lib prefix
                elseif(EXISTS "${FFMPEG_PATH}/lib/${LIB_NAME}.a")
                    list(APPEND FFMPEG_LIBS "${FFMPEG_PATH}/lib/${LIB_NAME}.a")
                endif()
            endforeach()
            if(FFMPEG_LIBS)
                set(FFMPEG_LIBRARIES ${FFMPEG_LIBS})
                message(STATUS "FFmpeg found at: ${FFMPEG_PATH}")
                message(STATUS "  Include dir: ${FFMPEG_INCLUDE_DIRS}")
                message(STATUS "  Libraries: ${FFMPEG_LIBRARIES}")
                break()
            else()
                message(STATUS "FFmpeg headers found at ${FFMPEG_PATH} but libraries not found")
                message(STATUS "  Checked: ${FFMPEG_PATH}/lib/")
                message(STATUS "  You may need to build FFmpeg or use a pre-built development package")
            endif()
        # Check for source code structure (libavcodec/avcodec.h directly)
        elseif(EXISTS "${FFMPEG_PATH}/libavcodec/avcodec.h")
            message(STATUS "FFmpeg source code detected at: ${FFMPEG_PATH}")
            message(STATUS "  This is source code, not a development build")
            message(STATUS "  You need to build FFmpeg first to get libraries")
            message(STATUS "  Or download a pre-built development package")
            # Don't set FFMPEG_FOUND for source code without libraries
        endif()
    endforeach()
endif()

if(FFMPEG_FOUND)
    message(STATUS "FFmpeg found: ${FFMPEG_VERSION}")
    get_filename_component(FFMPEG_ROOT "${FFMPEG_INCLUDE_DIRS}" DIRECTORY)
    include_directories(${FFMPEG_INCLUDE_DIRS})
    if(FFMPEG_CFLAGS_OTHER)
        add_definitions(${FFMPEG_CFLAGS_OTHER})
    endif()
else()
    # FFmpeg is optional - only show brief message, detailed info in BUILD_INSTRUCTIONS.md
    message(STATUS "FFmpeg not found (optional)")
    set(FFMPEG_LIBRARIES "")
    set(FFMPEG_INCLUDE_DIRS "")
endif()

# vcpkg toolchain support (if available)
if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# Find network library (asio) via vcpkg or system
find_package(asio CONFIG QUIET)
if(asio_FOUND)
    message(STATUS "asio found via vcpkg")
    set(AETHER_ASIO_ENABLED TRUE)
else()
    # Try to find asio header-only library
    find_path(ASIO_INCLUDE_DIR asio.hpp
        PATHS
            "${CMAKE_SOURCE_DIR}/third_party/asio/include"
            "${CMAKE_SOURCE_DIR}/third_party/asio"
            "C:/vcpkg/installed/x64-windows/include"
            "$ENV{VCPKG_ROOT}/installed/x64-windows/include"
    )
    if(ASIO_INCLUDE_DIR)
        message(STATUS "asio found at: ${ASIO_INCLUDE_DIR}")
        set(AETHER_ASIO_ENABLED TRUE)
    else()
        message(STATUS "asio not found - network features will be limited")
        set(AETHER_ASIO_ENABLED FALSE)
    endif()
endif()

# CUDA and NVENC support detection (modern CMake; no deprecated FindCUDA)
include(CheckLanguage)
check_language(CUDA)

if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)

    # Prefer CUDAToolkit package for headers/libs (e.g., for NVENC/CUDA runtime)
    find_package(CUDAToolkit QUIET)

    if(DEFINED CUDAToolkit_VERSION AND NOT "${CUDAToolkit_VERSION}" STREQUAL "")
        set(_AETHER_CUDA_VERSION "${CUDAToolkit_VERSION}")
    elseif(DEFINED CMAKE_CUDA_COMPILER_VERSION AND NOT "${CMAKE_CUDA_COMPILER_VERSION}" STREQUAL "")
        set(_AETHER_CUDA_VERSION "${CMAKE_CUDA_COMPILER_VERSION}")
    else()
        set(_AETHER_CUDA_VERSION "unknown")
    endif()

    message(STATUS "CUDA compiler found (version: ${_AETHER_CUDA_VERSION})")
    set(AETHER_CUDA_ENABLED TRUE)

    # NVENC is a driver/GPU feature; here we only gate by having a CUDA toolchain available.
    # (If you later add NVENC SDK checks, prefer a dedicated FindNVENC/CMake config.)
    set(AETHER_NVENC_ENABLED TRUE)
    message(STATUS "NVENC support enabled (toolchain available)")
else()
    message(STATUS "CUDA not found - GPU acceleration features will be limited")
    set(AETHER_CUDA_ENABLED FALSE)
    set(AETHER_NVENC_ENABLED FALSE)
endif()

# OpenGL support (optional – no error if not found)
set(AETHER_OPENGL_ENABLED FALSE)
find_package(OpenGL QUIET)
if(OpenGL_FOUND)
    set(AETHER_OPENGL_ENABLED TRUE)
    message(STATUS "OpenGL found - AETHER_OPENGL_ENABLED")
else()
    message(STATUS "OpenGL not found (optional)")
endif()

# Intel Quick Sync Video / oneVPL (optional – no error if not found)
set(AETHER_QSV_ENABLED FALSE)
find_package(oneVPL QUIET)
if(oneVPL_FOUND)
    set(AETHER_QSV_ENABLED TRUE)
    message(STATUS "Intel oneVPL (Quick Sync) found - AETHER_QSV_ENABLED")
else()
    # Fallback: legacy Intel Media SDK (mfx)
    if(WIN32)
        set(_QSV_HINTS "C:/Program Files (x86)/Intel/Media SDK" "C:/Intel/Media SDK" "$ENV{INTEL_MEDIA_SDK}")
        find_path(QSV_INCLUDE_DIR NAMES mfxvideo.h mfxdispatcher.h HINTS ${_QSV_HINTS} PATH_SUFFIXES include)
        find_library(QSV_LIBRARY NAMES mfx mfx32 mfx64 HINTS ${_QSV_HINTS} PATH_SUFFIXES lib lib/x64)
        if(QSV_INCLUDE_DIR AND QSV_LIBRARY)
            set(AETHER_QSV_ENABLED TRUE)
            set(QSV_INCLUDE_DIRS ${QSV_INCLUDE_DIR})
            set(QSV_LIBRARIES ${QSV_LIBRARY})
            message(STATUS "Intel Media SDK (Quick Sync) found - AETHER_QSV_ENABLED")
        endif()
    endif()
    if(NOT AETHER_QSV_ENABLED)
        message(STATUS "Intel Quick Sync / oneVPL not found (optional)")
    endif()
endif()

# Find OpenSSL (optional - for enhanced cryptographic functions)
# Try multiple methods to find OpenSSL on Windows
find_package(OpenSSL QUIET)

# If not found via find_package, try common Windows paths
if(NOT OpenSSL_FOUND AND WIN32)
    set(OPENSSL_POSSIBLE_PATHS
        "C:/OpenSSL-Win64"
        "C:/OpenSSL"
        "$ENV{ProgramFiles}/OpenSSL-Win64"
        "C:/Program Files (x86)/OpenSSL-Win32"
        "C:/vcpkg/installed/x64-windows"
        "$ENV{VCPKG_ROOT}/installed/x64-windows"
    )
    
    foreach(OPENSSL_PATH ${OPENSSL_POSSIBLE_PATHS})
        if(EXISTS "${OPENSSL_PATH}/include/openssl/ssl.h")
            set(OpenSSL_FOUND TRUE)
            set(OPENSSL_INCLUDE_DIR "${OPENSSL_PATH}/include")
            set(OPENSSL_LIBRARIES
                "${OPENSSL_PATH}/lib/libssl.lib"
                "${OPENSSL_PATH}/lib/libcrypto.lib"
            )
            message(STATUS "OpenSSL found at: ${OPENSSL_PATH}")
            break()
        endif()
    endforeach()
endif()

if(OpenSSL_FOUND)
    message(STATUS "OpenSSL found: ${OPENSSL_VERSION}")
else()
    message(STATUS "OpenSSL not found. Using built-in SHA-256 implementation.")
    set(OPENSSL_LIBRARIES "")
endif()

# Fetch GLFW
include(FetchContent)

# Configure FetchContent
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Disable FetchContent updates after first fetch")
set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/_deps" CACHE PATH "FetchContent base directory")

# Check if Git is available and help CMake find it
find_package(Git QUIET)
if(NOT Git_FOUND)
    # Try to find Git in common installation locations
    set(GIT_POSSIBLE_PATHS
        "C:/Program Files/Git/cmd/git.exe"
        "C:/Program Files (x86)/Git/cmd/git.exe"
        "$ENV{ProgramFiles}/Git/cmd/git.exe"
    )
    
    foreach(GIT_PATH ${GIT_POSSIBLE_PATHS})
        if(EXISTS "${GIT_PATH}")
            get_filename_path(GIT_DIR "${GIT_PATH}")
            set(ENV{PATH} "${GIT_DIR};$ENV{PATH}")
            message(STATUS "Found Git at: ${GIT_PATH}")
            break()
        endif()
    endforeach()
    
    # Try finding Git again
    find_program(GIT_EXECUTABLE git PATHS ${GIT_POSSIBLE_PATHS})
    if(GIT_EXECUTABLE)
        message(STATUS "Using Git: ${GIT_EXECUTABLE}")
    else()
        message(WARNING "Git not found. FetchContent may fail. Please install Git or ensure it's in PATH.")
        message(WARNING "Download Git from: https://git-scm.com/download/win")
    endif()
else()
    message(STATUS "Found Git: ${GIT_EXECUTABLE}")
endif()

# Fetch GLFW with error handling
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    GIT_SUBMODULES ""
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Use local ImGui installation (manually downloaded)
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/third_party/imgui")
if(NOT EXISTS "${IMGUI_DIR}/imgui.cpp")
    message(FATAL_ERROR "ImGui not found at ${IMGUI_DIR}. Please run download_imgui.ps1 to download it.")
endif()
message(STATUS "Using local ImGui from: ${IMGUI_DIR}")

# Fetch nlohmann/json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    GIT_SUBMODULES ""
)

# Fetch stb
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

# Make dependencies available one by one for better error handling
message(STATUS "Fetching GLFW...")
FetchContent_MakeAvailable(glfw)
if(NOT EXISTS "${glfw_SOURCE_DIR}")
    message(FATAL_ERROR "Failed to fetch GLFW. Please check your internet connection and Git installation.")
endif()

# ImGui is already available locally, no need to fetch
message(STATUS "ImGui found at: ${IMGUI_DIR}")

message(STATUS "Fetching nlohmann/json...")
FetchContent_MakeAvailable(json)
if(NOT EXISTS "${json_SOURCE_DIR}")
    message(FATAL_ERROR "Failed to fetch nlohmann/json. Please check your internet connection and Git installation.")
endif()

message(STATUS "Fetching stb...")
FetchContent_MakeAvailable(stb)
if(NOT EXISTS "${stb_SOURCE_DIR}")
    message(FATAL_ERROR "Failed to fetch stb. Please check your internet connection and Git installation.")
endif()

# ImGui sources (IMGUI_DIR already set above)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui.h
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_internal.h
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imconfig.h
    ${IMGUI_DIR}/imstb_rectpack.h
    ${IMGUI_DIR}/imstb_textedit.h
    ${IMGUI_DIR}/imstb_truetype.h
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.h
)
if(WIN32 AND AETHER_USE_DIRECTX)
    if(AETHER_DIRECTX_11)
        list(APPEND IMGUI_SOURCES
            ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
            ${IMGUI_DIR}/backends/imgui_impl_dx11.h
        )
    else()
        list(APPEND IMGUI_SOURCES
            ${IMGUI_DIR}/backends/imgui_impl_dx12.cpp
            ${IMGUI_DIR}/backends/imgui_impl_dx12.h
        )
    endif()
else()
    list(APPEND IMGUI_SOURCES
        ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
        ${IMGUI_DIR}/backends/imgui_impl_vulkan.h
    )
endif()

# -----------------------------------------------------------------------------
# Main executable (GLFW/ImGui) – skip when building only Qt frontend
# -----------------------------------------------------------------------------
if(NOT AETHER_QT6_FRONTEND)
# Source files
#
# NOTE:
# Old pre-refactor folders (`src/render`, `src/media`, `src/network`) still exist in this repo as
# historical copies. Compiling both the new `src/engine/**` files and those legacy copies will
# usually cause duplicate symbol / ODR linker errors. We intentionally only compile the new
# locations.
file(GLOB_RECURSE SOURCES
    "src/main.cpp"
    "src/core/*.cpp"
    "src/ui/*.cpp"
    "src/engine/render/*.cpp"
    "src/engine/media/*.cpp"
    "src/engine/network/*.cpp"
    "src/workspaces/*.cpp"
)

# When using DirectX, exclude VulkanRenderer and add DirectX renderer
if(WIN32 AND AETHER_USE_DIRECTX)
    list(FILTER SOURCES EXCLUDE REGEX ".*VulkanRenderer\\.cpp$")
    if(AETHER_DIRECTX_11)
        list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/src/engine/render/DirectX11Renderer.cpp")
    else()
        list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/src/engine/render/DirectX12Renderer.cpp")
    endif()
endif()

# Executable
add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
)

# Link Vulkan libraries (when not using DirectX)
if(NOT AETHER_USE_DIRECTX)
    if(Vulkan_LIBRARIES)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})
    elseif(Vulkan_LIBRARY)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARY})
    elseif(DEFINED VULKAN_SDK AND EXISTS "${VULKAN_SDK}/Lib")
        if(MSVC)
            target_link_libraries(${PROJECT_NAME} PRIVATE "${VULKAN_SDK}/Lib/vulkan-1.lib")
        else()
            target_link_libraries(${PROJECT_NAME} PRIVATE "${VULKAN_SDK}/Lib/libvulkan-1.a")
        endif()
        message(STATUS "Linking Vulkan from: ${VULKAN_SDK}/Lib")
    else()
        message(WARNING "Vulkan libraries not found, linking may fail")
    endif()
endif()

# DirectX compile definitions
if(AETHER_USE_DIRECTX)
    target_compile_definitions(${PROJECT_NAME} PRIVATE AETHER_USE_DIRECTX)
    if(AETHER_DIRECTX_11)
        target_compile_definitions(${PROJECT_NAME} PRIVATE AETHER_DIRECTX_11)
    endif()
    if(AETHER_DIRECTX_12)
        target_compile_definitions(${PROJECT_NAME} PRIVATE AETHER_DIRECTX_12)
    endif()
endif()

# Link FFmpeg if found
if(FFMPEG_FOUND AND FFMPEG_LIBRARIES)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${FFMPEG_LIBRARIES}
    )
    if(FFMPEG_INCLUDE_DIRS)
        target_include_directories(${PROJECT_NAME} PRIVATE
            ${FFMPEG_INCLUDE_DIRS}
        )
    endif()
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        AETHER_FFMPEG_ENABLED
    )
    message(STATUS "FFmpeg libraries linked successfully")
elseif(FFMPEG_FOUND)
    message(WARNING "FFmpeg found but libraries not located. FFmpeg features will be disabled.")
endif()

# Link OpenSSL if found (optional)
if(OpenSSL_FOUND)
    # Use modern CMake targets if available, otherwise fall back to variables
    if(TARGET OpenSSL::SSL AND TARGET OpenSSL::Crypto)
        target_link_libraries(${PROJECT_NAME} PRIVATE
            OpenSSL::SSL
            OpenSSL::Crypto
        )
    elseif(OPENSSL_LIBRARIES)
        target_link_libraries(${PROJECT_NAME} PRIVATE
            ${OPENSSL_LIBRARIES}
        )
        target_include_directories(${PROJECT_NAME} PRIVATE
            ${OPENSSL_INCLUDE_DIR}
        )
    endif()
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        AETHER_OPENSSL_ENABLED
    )
endif()

# Link network library (asio) if found
if(AETHER_ASIO_ENABLED)
    if(TARGET asio::asio)
        target_link_libraries(${PROJECT_NAME} PRIVATE asio::asio)
    elseif(ASIO_INCLUDE_DIR)
        target_include_directories(${PROJECT_NAME} PRIVATE ${ASIO_INCLUDE_DIR})
    endif()
    target_compile_definitions(${PROJECT_NAME} PRIVATE AETHER_ASIO_ENABLED)
endif()

# CUDA and NVENC compile definitions
if(AETHER_CUDA_ENABLED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE AETHER_CUDA_ENABLED)
    if(CUDA_FOUND)
        target_include_directories(${PROJECT_NAME} PRIVATE ${CUDA_INCLUDE_DIRS})
    endif()
endif()

if(AETHER_NVENC_ENABLED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE AETHER_NVENC_ENABLED)
endif()

# OpenGL (optional)
if(AETHER_OPENGL_ENABLED AND TARGET OpenGL::OpenGL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE AETHER_OPENGL_ENABLED)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::OpenGL)
endif()

# Intel Quick Sync (optional)
if(AETHER_QSV_ENABLED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE AETHER_QSV_ENABLED)
    if(TARGET oneVPL::oneVPL)
        target_link_libraries(${PROJECT_NAME} PRIVATE oneVPL::oneVPL)
        target_include_directories(${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:oneVPL::oneVPL,INTERFACE_INCLUDE_DIRECTORIES>)
    elseif(QSV_LIBRARIES)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${QSV_LIBRARIES})
        if(QSV_INCLUDE_DIRS)
            target_include_directories(${PROJECT_NAME} PRIVATE ${QSV_INCLUDE_DIRS})
        endif()
    endif()
endif()

# Windows-specific libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ws2_32      # Winsock2 for AetherLink
        psapi       # Process API for MemoryManager
        wbemuuid    # WMI for HardwareID
        ole32       # COM for HardwareID
        oleaut32    # COM automation for HardwareID
        pdh         # Performance Data Helper for BackgroundCache GPU monitoring
    )
    
    # Link DirectX libraries when using DirectX backend
    if(AETHER_USE_DIRECTX)
        target_link_libraries(${PROJECT_NAME} PRIVATE
            d3d11
            d3d12
            dxgi
            d3dcompiler
        )
        message(STATUS "Linking DirectX libraries")
    endif()
    
    # Link CUDA libraries if found (CUDAToolkit or legacy CUDA_LIBRARIES)
    if(AETHER_CUDA_ENABLED)
        if(TARGET CUDA::cudart)
            target_link_libraries(${PROJECT_NAME} PRIVATE CUDA::cudart)
        elseif(CUDA_FOUND AND CUDA_LIBRARIES)
            target_link_libraries(${PROJECT_NAME} PRIVATE ${CUDA_LIBRARIES})
        endif()
    endif()
endif()

# Include directories for ImGui and JSON
target_include_directories(${PROJECT_NAME} PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${json_SOURCE_DIR}/include
    ${stb_SOURCE_DIR}
)

# Add FFmpeg include directories if found
if(FFMPEG_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${FFMPEG_INCLUDE_DIRS}
    )
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
        /permissive-
        /Zc:__cplusplus
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _CRT_SECURE_NO_WARNINGS
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# Copy DLLs on Windows
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:glfw>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
    
    # Copy assets folder to build directory
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
        COMMENT "Copying assets to build directory"
    )
    # Copy FFmpeg DLLs from FFMPEG_ROOT/bin to exe dir so PATH is not required
    if(FFMPEG_FOUND AND EXISTS "${FFMPEG_ROOT}/bin")
        file(GLOB _ffmpeg_dlls "${FFMPEG_ROOT}/bin/*.dll")
        foreach(_dll ${_ffmpeg_dlls})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_dll}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
                COMMENT "Copying FFmpeg DLL to build dir"
            )
        endforeach()
        if(_ffmpeg_dlls)
            message(STATUS "FFmpeg: will copy ${FFMPEG_ROOT}/bin/*.dll to exe directory")
        endif()
    endif()
endif()
endif() # NOT AETHER_QT6_FRONTEND (main executable)

# -----------------------------------------------------------------------------
# Qt6 frontend: AetherStudioQt (optional second executable)
# -----------------------------------------------------------------------------
if(AETHER_QT6_FRONTEND)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    if(Qt6_ROOT AND EXISTS "${Qt6_ROOT}/lib/cmake/Qt6")
        list(INSERT CMAKE_PREFIX_PATH 0 "${Qt6_ROOT}")
        set(Qt6_DIR "${Qt6_ROOT}/lib/cmake/Qt6" CACHE PATH "Qt6 config location")
        message(STATUS "AetherStudioQt: Using Qt6 at ${Qt6_ROOT}")
    elseif(WIN32 AND EXISTS "C:/Qt/6.7.0/msvc2019_64/lib/cmake/Qt6")
        list(INSERT CMAKE_PREFIX_PATH 0 "C:/Qt/6.7.0/msvc2019_64")
        set(Qt6_DIR "C:/Qt/6.7.0/msvc2019_64/lib/cmake/Qt6" CACHE PATH "Qt6 config location")
    endif()
    set(Qt6_FOUND FALSE)
    if(Qt6_ROOT AND EXISTS "${Qt6_ROOT}/lib/cmake/Qt6/Qt6Config.cmake")
        set(_qt6_cmake_dir "${Qt6_ROOT}/lib/cmake/Qt6")
        set(CMAKE_PREFIX_PATH "${Qt6_ROOT};${CMAKE_PREFIX_PATH}")
        # Qt6Config.cmake only loads components when Qt6_FIND_COMPONENTS is set
        set(Qt6_FIND_COMPONENTS Core Gui Widgets)
        set(Qt6_FIND_REQUIRED_Core TRUE)
        set(Qt6_FIND_REQUIRED_Gui TRUE)
        set(Qt6_FIND_REQUIRED_Widgets TRUE)
        include("${_qt6_cmake_dir}/Qt6Config.cmake")
        if(TARGET Qt6::Core AND TARGET Qt6::Gui AND TARGET Qt6::Widgets)
            set(Qt6_FOUND TRUE)
            message(STATUS "AetherStudioQt: Qt6 found at ${Qt6_ROOT} (via include)")
        endif()
    endif()
    if(NOT Qt6_FOUND)
        find_package(Qt6 CONFIG REQUIRED COMPONENTS Core Gui Widgets)
    endif()
    find_package(Qt6 COMPONENTS Multimedia MultimediaWidgets QUIET)
    set(AETHER_QT_MULTIMEDIA OFF)
    if(Qt6_Multimedia_FOUND AND Qt6_MultimediaWidgets_FOUND)
        set(AETHER_QT_MULTIMEDIA ON)
        message(STATUS "AetherStudioQt: Qt6 Multimedia enabled (video playback)")
    endif()
    # Ensure Qt6_FOUND is set for add_executable(AetherStudioQt) when core targets exist
    if(TARGET Qt6::Core AND TARGET Qt6::Gui AND TARGET Qt6::Widgets)
        set(Qt6_FOUND TRUE)
    endif()
    if(NOT TARGET Qt6::Core OR NOT TARGET Qt6::Gui OR NOT TARGET Qt6::Widgets)
        message(WARNING "Qt6 not found. Set Qt6_ROOT to your Qt6 install (e.g. C:/Qt/6.7.0/msvc2019_64) or set CMAKE_PREFIX_PATH, then reconfigure. Or set AETHER_QT6_FRONTEND=OFF to skip the Qt frontend.")
    endif()
    # Vulkan already found above; required for QVulkanWindow
    if(NOT Vulkan_FOUND AND NOT Vulkan_LIBRARIES AND NOT (DEFINED VULKAN_SDK AND EXISTS "${VULKAN_SDK}/Lib"))
        message(WARNING "Vulkan not found; AetherStudioQt will require Vulkan at run time.")
    endif()
    # FFmpeg for Qt: prefer C:/ffmpeg
    if(NOT FFMPEG_ROOT)
        set(FFMPEG_ROOT "C:/ffmpeg" CACHE PATH "FFmpeg root for Qt frontend")
    endif()
    if(EXISTS "${FFMPEG_ROOT}/include" AND EXISTS "${FFMPEG_ROOT}/lib")
        set(QT_FFMPEG_INCLUDE "${FFMPEG_ROOT}/include")
        set(QT_FFMPEG_LIBS
            "${FFMPEG_ROOT}/lib/avcodec.lib"
            "${FFMPEG_ROOT}/lib/avformat.lib"
            "${FFMPEG_ROOT}/lib/avutil.lib"
            "${FFMPEG_ROOT}/lib/swscale.lib"
            "${FFMPEG_ROOT}/lib/swresample.lib"
        )
        if(EXISTS "${FFMPEG_ROOT}/lib/avcodec.lib")
            set(QT_FFMPEG_FOUND TRUE)
        else()
            set(QT_FFMPEG_FOUND FALSE)
        endif()
    else()
        set(QT_FFMPEG_FOUND FALSE)
    endif()
    if(NOT QT_FFMPEG_FOUND)
        message(WARNING "Qt frontend: FFmpeg at ${FFMPEG_ROOT} not found; linking may fail.")
    endif()

    if(Qt6_FOUND)
    set(QT_FRONTEND_SOURCES
        ${CMAKE_SOURCE_DIR}/src/main_qt.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/MainWindow.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/AetherRenderView.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/SettingsDialog.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/ProjectModel.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/FrameRingBuffer.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/PlaybackEngine.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/ProjectPanel.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/MonitorWidget.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/TimelineWidget.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/PageBarWidget.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/HomeWidget.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/NewProjectDialog.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/ProjectFile.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/MediaPageWidget.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/PlaceholderPageWidget.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/NodeGraphModel.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/KeyframeModel.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/NodeGraphView.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/KeyframeTimelineWidget.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/AnimationPageWidget.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/ColorWheelsWidget.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/VideoScopesWidget.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/ColorPageWidget.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/AudioMixerWidget.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/AudioPageWidget.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/RenderQueueWidget.cpp
        ${CMAKE_SOURCE_DIR}/src/qt/DeliverPageWidget.cpp
        ${CMAKE_SOURCE_DIR}/src/engine/vfx/VulkanVFXEngine.cpp
        ${CMAKE_SOURCE_DIR}/src/engine/encode/FFmpegEncoder.cpp
        ${CMAKE_SOURCE_DIR}/src/core/LicenseManager.cpp
        ${CMAKE_SOURCE_DIR}/src/core/HardwareID.cpp
        ${CMAKE_SOURCE_DIR}/src/core/SHA256.cpp
        ${CMAKE_SOURCE_DIR}/include/aether/PlaybackEngine.h
    )
    add_executable(AetherStudioQt WIN32 ${QT_FRONTEND_SOURCES})
    target_include_directories(AetherStudioQt PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${QT_FFMPEG_INCLUDE}
    )
    if(Vulkan_INCLUDE_DIRS)
        target_include_directories(AetherStudioQt PRIVATE ${Vulkan_INCLUDE_DIRS})
    elseif(DEFINED VULKAN_SDK AND EXISTS "${VULKAN_SDK}/Include")
        target_include_directories(AetherStudioQt PRIVATE "${VULKAN_SDK}/Include")
    endif()
    target_link_libraries(AetherStudioQt PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
    )
    if(AETHER_QT_MULTIMEDIA)
        target_link_libraries(AetherStudioQt PRIVATE Qt6::Multimedia Qt6::MultimediaWidgets)
        target_compile_definitions(AetherStudioQt PRIVATE AETHER_QT_MULTIMEDIA)
    endif()
    if(Vulkan_LIBRARIES)
        target_link_libraries(AetherStudioQt PRIVATE ${Vulkan_LIBRARIES})
    elseif(Vulkan_LIBRARY)
        target_link_libraries(AetherStudioQt PRIVATE ${Vulkan_LIBRARY})
    elseif(DEFINED VULKAN_SDK AND EXISTS "${VULKAN_SDK}/Lib")
        if(MSVC)
            target_link_libraries(AetherStudioQt PRIVATE "${VULKAN_SDK}/Lib/vulkan-1.lib")
        else()
            target_link_libraries(AetherStudioQt PRIVATE "${VULKAN_SDK}/Lib/libvulkan-1.a")
        endif()
    endif()
    if(QT_FFMPEG_FOUND)
        target_link_libraries(AetherStudioQt PRIVATE ${QT_FFMPEG_LIBS})
        target_compile_definitions(AetherStudioQt PRIVATE AETHER_FFMPEG_ENABLED)
    endif()
    if(OpenSSL_FOUND)
        if(TARGET OpenSSL::SSL AND TARGET OpenSSL::Crypto)
            target_link_libraries(AetherStudioQt PRIVATE OpenSSL::SSL OpenSSL::Crypto)
        elseif(OPENSSL_LIBRARIES)
            target_link_libraries(AetherStudioQt PRIVATE ${OPENSSL_LIBRARIES})
            target_include_directories(AetherStudioQt PRIVATE ${OPENSSL_INCLUDE_DIR})
        endif()
        target_compile_definitions(AetherStudioQt PRIVATE AETHER_OPENSSL_ENABLED)
    endif()
    # Optional: OpenGL (no error if not found)
    if(AETHER_OPENGL_ENABLED AND TARGET OpenGL::OpenGL)
        target_compile_definitions(AetherStudioQt PRIVATE AETHER_OPENGL_ENABLED)
        target_link_libraries(AetherStudioQt PRIVATE OpenGL::OpenGL)
    endif()
    # Optional: Intel Quick Sync (no error if not found)
    if(AETHER_QSV_ENABLED)
        target_compile_definitions(AetherStudioQt PRIVATE AETHER_QSV_ENABLED)
        if(TARGET oneVPL::oneVPL)
            target_link_libraries(AetherStudioQt PRIVATE oneVPL::oneVPL)
        elseif(QSV_LIBRARIES)
            target_link_libraries(AetherStudioQt PRIVATE ${QSV_LIBRARIES})
            if(QSV_INCLUDE_DIRS)
                target_include_directories(AetherStudioQt PRIVATE ${QSV_INCLUDE_DIRS})
            endif()
        endif()
    endif()
    # Optional: CUDA / NVENC (no error if not found)
    if(AETHER_CUDA_ENABLED)
        target_compile_definitions(AetherStudioQt PRIVATE AETHER_CUDA_ENABLED)
        if(AETHER_NVENC_ENABLED)
            target_compile_definitions(AetherStudioQt PRIVATE AETHER_NVENC_ENABLED)
        endif()
        if(TARGET CUDA::cudart_static OR TARGET CUDA::cudart)
            target_link_libraries(AetherStudioQt PRIVATE CUDA::cudart)
        elseif(CUDA_LIBRARIES)
            target_link_libraries(AetherStudioQt PRIVATE ${CUDA_LIBRARIES})
            if(CUDA_INCLUDE_DIRS)
                target_include_directories(AetherStudioQt PRIVATE ${CUDA_INCLUDE_DIRS})
            endif()
        endif()
    endif()
    if(MSVC)
        target_compile_options(AetherStudioQt PRIVATE /W4)
    else()
        target_compile_options(AetherStudioQt PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    if(WIN32)
        # Qt6 runtime DLLs (Core, Gui, Widgets + dependencies) – windeployqt
        get_filename_component(_qt_bin_dir "${Qt6_DIR}/../../../bin" ABSOLUTE)
        set(_windeployqt "${_qt_bin_dir}/windeployqt.exe")
        if(EXISTS "${_windeployqt}")
            # Debug build needs Qt6Cored.dll, Qt6Guid.dll, Qt6Widgetsd.dll; Release needs non-d suffix
            add_custom_command(TARGET AetherStudioQt POST_BUILD
                COMMAND "${_windeployqt}" --no-compiler-runtime
                    "$<IF:$<CONFIG:Debug>,--debug,--release>"
                    "$<TARGET_FILE:AetherStudioQt>"
                WORKING_DIRECTORY "$<TARGET_FILE_DIR:AetherStudioQt>"
                COMMENT "Deploying Qt6 DLLs (windeployqt)"
            )
            message(STATUS "AetherStudioQt: windeployqt will copy Qt6 runtime DLLs")
        else()
            message(WARNING "windeployqt not found at ${_windeployqt}; Qt6 DLLs must be in PATH or next to exe")
        endif()
        add_custom_command(TARGET AetherStudioQt POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:AetherStudioQt>/assets
            COMMENT "Copying assets for AetherStudioQt"
        )
        # Copy FFmpeg DLLs from FFMPEG_ROOT/bin so PATH is not required
        if(QT_FFMPEG_FOUND AND DEFINED FFMPEG_ROOT AND EXISTS "${FFMPEG_ROOT}/bin")
            file(GLOB _qt_ffmpeg_dlls "${FFMPEG_ROOT}/bin/*.dll")
            foreach(_dll ${_qt_ffmpeg_dlls})
                add_custom_command(TARGET AetherStudioQt POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_dll}" "$<TARGET_FILE_DIR:AetherStudioQt>/"
                    COMMENT "Copying FFmpeg DLL for AetherStudioQt"
                )
            endforeach()
            if(_qt_ffmpeg_dlls)
                message(STATUS "AetherStudioQt: will copy ${FFMPEG_ROOT}/bin/*.dll to exe directory")
            endif()
        endif()
    endif()
    message(STATUS "AetherStudioQt (Qt6 frontend) enabled")
    endif() # Qt6_FOUND
endif() # AETHER_QT6_FRONTEND
