#==============================================================================
# nfx-json - Dependencies configuration
#==============================================================================

#----------------------------------------------
# Output configuration
#----------------------------------------------

set(_SAVED_CMAKE_REQUIRED_QUIET    ${CMAKE_REQUIRED_QUIET})
set(_SAVED_CMAKE_MESSAGE_LOG_LEVEL ${CMAKE_MESSAGE_LOG_LEVEL})
set(_SAVED_CMAKE_FIND_QUIETLY      ${CMAKE_FIND_QUIETLY})

set(CMAKE_REQUIRED_QUIET    ON     )
set(CMAKE_MESSAGE_LOG_LEVEL VERBOSE) # [ERROR, WARNING, NOTICE, STATUS, VERBOSE, DEBUG]
set(CMAKE_FIND_QUIETLY      ON     )

#----------------------------------------------
# Dependency versions
#----------------------------------------------

set(NFX_JSON_DEPS_NFX_STRINGUTILS_VERSION    "0.6.0")
set(NFX_JSON_DEPS_NFX_STRINGBUILDER_VERSION  "0.5.0")
set(NFX_JSON_DEPS_NFX_CONTAINERS_VERSION     "0.3.2")
set(NFX_JSON_DEPS_NFX_HASHING_VERSION        "0.1.2")
set(NFX_JSON_DEPS_NFX_RESOURCE_VERSION       "1.1.0")

#----------------------------------------------
# FetchContent dependencies
#----------------------------------------------

include(FetchContent)

if(DEFINED ENV{CI})
    set(FETCHCONTENT_UPDATES_DISCONNECTED OFF)
else()
    set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
endif()
set(FETCHCONTENT_QUIET OFF)

# --- nfx-stringutils ---
find_package(nfx-stringutils ${NFX_JSON_DEPS_NFX_STRINGUTILS_VERSION} QUIET)
if(NOT nfx-stringutils_FOUND)
    set(NFX_STRINGUTILS_BUILD_TESTS         OFF CACHE BOOL "")
    set(NFX_STRINGUTILS_BUILD_SAMPLES       OFF CACHE BOOL "")
    set(NFX_STRINGUTILS_BUILD_BENCHMARKS    OFF CACHE BOOL "")
    set(NFX_STRINGUTILS_BUILD_DOCUMENTATION OFF CACHE BOOL "")
    set(NFX_STRINGUTILS_INSTALL_PROJECT     OFF CACHE BOOL "")
    set(NFX_STRINGUTILS_PACKAGE_SOURCE      OFF CACHE BOOL "")
    set(NFX_STRINGUTILS_PACKAGE_ARCHIVE     OFF CACHE BOOL "")
    set(NFX_STRINGUTILS_PACKAGE_DEB         OFF CACHE BOOL "")
    set(NFX_STRINGUTILS_PACKAGE_RPM         OFF CACHE BOOL "")

    FetchContent_Declare(
        nfx-stringutils
            GIT_REPOSITORY https://github.com/nfx-libs/nfx-stringutils.git
            GIT_TAG        ${NFX_JSON_DEPS_NFX_STRINGUTILS_VERSION}
            GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(nfx-stringutils)
    set(NFX_STRINGUTILS_INCLUDE_DIR "${nfx-stringutils_SOURCE_DIR}/include" CACHE INTERNAL "nfx-stringutils include directory")
endif()

# --- nfx-stringbuilder ---
find_package(nfx-stringbuilder ${NFX_JSON_DEPS_NFX_STRINGBUILDER_VERSION} QUIET)
if(NOT nfx-stringbuilder_FOUND)
    set(NFX_STRINGBUILDER_BUILD_TESTS         OFF CACHE BOOL "")
    set(NFX_STRINGBUILDER_BUILD_SAMPLES       OFF CACHE BOOL "")
    set(NFX_STRINGBUILDER_BUILD_BENCHMARKS    OFF CACHE BOOL "")
    set(NFX_STRINGBUILDER_BUILD_DOCUMENTATION OFF CACHE BOOL "")
    set(NFX_STRINGBUILDER_INSTALL_PROJECT     OFF CACHE BOOL "")
    set(NFX_STRINGBUILDER_PACKAGE_SOURCE      OFF CACHE BOOL "")
    set(NFX_STRINGBUILDER_PACKAGE_ARCHIVE     OFF CACHE BOOL "")
    set(NFX_STRINGBUILDER_PACKAGE_DEB         OFF CACHE BOOL "")
    set(NFX_STRINGBUILDER_PACKAGE_RPM         OFF CACHE BOOL "")

    FetchContent_Declare(
        nfx-stringbuilder
            GIT_REPOSITORY https://github.com/nfx-libs/nfx-stringbuilder.git
            GIT_TAG        ${NFX_JSON_DEPS_NFX_STRINGBUILDER_VERSION}
            GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(nfx-stringbuilder)
endif()

# --- nfx-containers ---
find_package(nfx-containers ${NFX_JSON_DEPS_NFX_CONTAINERS_VERSION} QUIET)
if(NOT nfx-containers_FOUND)
    set(NFX_CONTAINERS_BUILD_TESTS         OFF CACHE BOOL "")
    set(NFX_CONTAINERS_BUILD_SAMPLES       OFF CACHE BOOL "")
    set(NFX_CONTAINERS_BUILD_BENCHMARKS    OFF CACHE BOOL "")
    set(NFX_CONTAINERS_BUILD_DOCUMENTATION OFF CACHE BOOL "")
    set(NFX_CONTAINERS_INSTALL_PROJECT     OFF CACHE BOOL "")
    set(NFX_CONTAINERS_PACKAGE_SOURCE      OFF CACHE BOOL "")
    set(NFX_CONTAINERS_PACKAGE_ARCHIVE     OFF CACHE BOOL "")
    set(NFX_CONTAINERS_PACKAGE_DEB         OFF CACHE BOOL "")
    set(NFX_CONTAINERS_PACKAGE_RPM         OFF CACHE BOOL "")

    FetchContent_Declare(
        nfx-containers
            GIT_REPOSITORY https://github.com/nfx-libs/nfx-containers
            GIT_TAG        ${NFX_JSON_DEPS_NFX_CONTAINERS_VERSION}
            GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(nfx-containers)
endif()

# --- nfx-hashing ---
find_package(nfx-hashing ${NFX_JSON_DEPS_NFX_HASHING_VERSION} QUIET)
if(NOT nfx-hashing_FOUND)
    set(NFX_HASHING_BUILD_TESTS         OFF CACHE BOOL "")
    set(NFX_HASHING_BUILD_SAMPLES       OFF CACHE BOOL "")
    set(NFX_HASHING_BUILD_BENCHMARKS    OFF CACHE BOOL "")
    set(NFX_HASHING_BUILD_DOCUMENTATION OFF CACHE BOOL "")
    set(NFX_HASHING_INSTALL_PROJECT     OFF CACHE BOOL "")
    set(NFX_HASHING_PACKAGE_SOURCE      OFF CACHE BOOL "")
    set(NFX_HASHING_PACKAGE_ARCHIVE     OFF CACHE BOOL "")
    set(NFX_HASHING_PACKAGE_DEB         OFF CACHE BOOL "")
    set(NFX_HASHING_PACKAGE_RPM         OFF CACHE BOOL "")

    FetchContent_Declare(
        nfx-hashing
            GIT_REPOSITORY https://github.com/nfx-libs/nfx-hashing.git
            GIT_TAG        ${NFX_JSON_DEPS_NFX_HASHING_VERSION}
            GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(nfx-hashing)
endif()

# --- nfx-resource ---
find_package(nfx-resource ${NFX_JSON_DEPS_NFX_RESOURCE_VERSION} QUIET)
if(NOT nfx-resource_FOUND)
    set(NFX_RESOURCE_INSTALL_PROJECT OFF CACHE BOOL "")
    set(NFX_RESOURCE_PACKAGE_SOURCE  OFF CACHE BOOL "")

    FetchContent_Declare(
        nfx-resource
            GIT_REPOSITORY https://github.com/nfx-libs/nfx-resource.git
            GIT_TAG        ${NFX_JSON_DEPS_NFX_RESOURCE_VERSION}
            GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(nfx-resource)
endif()

#----------------------------------------------
# Cleanup
#----------------------------------------------

set(CMAKE_REQUIRED_QUIET    ${_SAVED_CMAKE_REQUIRED_QUIET})
set(CMAKE_MESSAGE_LOG_LEVEL ${_SAVED_CMAKE_MESSAGE_LOG_LEVEL})
set(CMAKE_FIND_QUIETLY      ${_SAVED_CMAKE_FIND_QUIETLY})
