#==============================================================================
# nfx-json - CMake targets
#==============================================================================

#----------------------------------------------
# Check if we have sources to build
#----------------------------------------------

if(NOT private_sources)
    return()
endif()

#----------------------------------------------
# Targets definition
#----------------------------------------------

# --- Create shared library if requested ---
if(NFX_JSON_BUILD_SHARED)
    add_library(${PROJECT_NAME} SHARED)
    target_sources(${PROJECT_NAME}
        PRIVATE
            ${private_sources}
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    )

    add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
endif()

# --- Create static library if requested ---
if(NFX_JSON_BUILD_STATIC)
    add_library(${PROJECT_NAME}-static STATIC)
    target_sources(${PROJECT_NAME}-static
        PRIVATE
            ${private_sources}
    )

    set_target_properties(${PROJECT_NAME}-static PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}-static
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    )

    add_library(${PROJECT_NAME}::static ALIAS ${PROJECT_NAME}-static)
endif()

#----------------------------------------------
# Targets properties
#----------------------------------------------

function(configure_target target_name)
    # --- Include directories ---
    target_include_directories(${target_name}
        PUBLIC
            $<BUILD_INTERFACE:${NFX_JSON_INCLUDE_DIR}>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${NFX_JSON_SOURCE_DIR}
    )

    # --- Add nfx-stringutils include directory ---
    if(NFX_STRINGUTILS_INCLUDE_DIR)
        target_include_directories(${target_name}
            PRIVATE ${NFX_STRINGUTILS_INCLUDE_DIR}
        )
    endif()

    # --- Link dependencies ---
    if(TARGET nfx-hashing::nfx-hashing)
        target_link_libraries(${target_name}
            PUBLIC
                nfx-hashing::nfx-hashing
        )
    endif()

    if(TARGET nfx-containers::nfx-containers)
        target_link_libraries(${target_name}
            PUBLIC
                nfx-containers::nfx-containers
        )
    endif()

    if(TARGET nfx-stringbuilder::static)
        target_link_libraries(${target_name}
            PUBLIC
                nfx-stringbuilder::static
        )
    endif()

    # --- Properties ---
    set_target_properties(${target_name}
        PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
            POSITION_INDEPENDENT_CODE ON
            VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR}
            DEBUG_POSTFIX "-d"
    )

    # --- Enable specific CPU features ---
    target_compile_options(${target_name}
        PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>
            $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-march=native>
    )
endfunction()

# --- Apply configuration to both targets ---
if(NFX_JSON_BUILD_SHARED)
    configure_target(${PROJECT_NAME})
    if(WIN32)
        set_target_properties(${PROJECT_NAME}
            PROPERTIES
                WINDOWS_EXPORT_ALL_SYMBOLS TRUE
        )
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake/nfxJsonVersion.rc.in
            ${CMAKE_BINARY_DIR}/nfxJsonVersion.rc
            @ONLY
        )
        target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/nfxJsonVersion.rc)
    endif()
endif()

if(NFX_JSON_BUILD_STATIC)
    configure_target(${PROJECT_NAME}-static)
endif()
