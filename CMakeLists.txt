#==============================================================================
# nfx-json - Top-level CMake configuration
#==============================================================================

#----------------------------------------------
# Project definition
#----------------------------------------------

cmake_minimum_required(VERSION 3.20)

project(nfx-json
    VERSION 1.4.1
    DESCRIPTION "Modern C++20 JSON library with schema validation and generation"
    HOMEPAGE_URL "https://github.com/nfx-libs/nfx-json"
    LANGUAGES CXX
)

#----------------------------------------------
# Installation prefix configuration
#----------------------------------------------

include(GNUInstallDirs)

# Set default installation path to nfx
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(WIN32)
        set(CMAKE_INSTALL_PREFIX "C:/Program Files/nfx" CACHE PATH "Installation directory" FORCE)
    else()
        set(CMAKE_INSTALL_PREFIX "/usr/local/nfx" CACHE PATH "Installation directory" FORCE)
    endif()
    message(STATUS "Setting install prefix to '${CMAKE_INSTALL_PREFIX}'")
endif()

#----------------------------------------------
# Version configuration
#----------------------------------------------

set(NFX_JSON_VERSION_MAJOR ${PROJECT_VERSION_MAJOR} CACHE INTERNAL "nfx-json major version")
set(NFX_JSON_VERSION_MINOR ${PROJECT_VERSION_MINOR} CACHE INTERNAL "nfx-json minor version")
set(NFX_JSON_VERSION_PATCH ${PROJECT_VERSION_PATCH} CACHE INTERNAL "nfx-json patch version")
set(NFX_JSON_VERSION       ${PROJECT_VERSION}       CACHE INTERNAL "nfx-json version")

#----------------------------------------------
# Project metadata
#----------------------------------------------

set(NFX_JSON_LICENSE "MIT")
set(NFX_JSON_LICENSE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

#----------------------------------------------
# Testing setup
#----------------------------------------------

enable_testing()

#----------------------------------------------
# Build options
#----------------------------------------------

# --- Library build types ---
option(NFX_JSON_BUILD_STATIC        "Build static library"              ON )
option(NFX_JSON_BUILD_SHARED        "Build shared library"              OFF)

# --- Build components ---
option(NFX_JSON_BUILD_TESTS         "Build tests"                       OFF)
option(NFX_JSON_BUILD_SAMPLES       "Build samples"                     OFF)
option(NFX_JSON_BUILD_BENCHMARKS    "Build benchmarks"                  OFF)
option(NFX_JSON_BUILD_DOCUMENTATION "Build Doxygen documentation"       OFF)

# --- Installation ---
option(NFX_JSON_INSTALL_PROJECT     "Install project"                   OFF)

# --- Packaging ---
option(NFX_JSON_PACKAGE_SOURCE      "Enable source package generation"  OFF)
option(NFX_JSON_PACKAGE_ARCHIVE     "Enable TGZ/ZIP package generation" OFF)
option(NFX_JSON_PACKAGE_DEB         "Enable DEB package generation"     OFF)
option(NFX_JSON_PACKAGE_RPM         "Enable RPM package generation"     OFF)

#----------------------------------------------
# CMake modules configuration
#----------------------------------------------

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#----------------------------------------------
# Main configuration
#----------------------------------------------

include(nfxJsonBuildConfig)
include(nfxJsonDependencies)
include(nfxJsonSources)
include(nfxJsonTargets)

#----------------------------------------------
# Installation and packaging
#----------------------------------------------

include(nfxJsonInstall)
include(nfxJsonPackaging)

#----------------------------------------------
# Targets
#----------------------------------------------

add_subdirectory(test)
add_subdirectory(samples)
add_subdirectory(benchmark)
add_subdirectory(doc)
