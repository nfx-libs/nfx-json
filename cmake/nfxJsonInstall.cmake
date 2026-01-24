#==============================================================================
# nfx-json - Library installation
#==============================================================================

#----------------------------------------------
# Installation condition check
#----------------------------------------------

if(NOT NFX_JSON_INSTALL_PROJECT)
    return()
endif()

#----------------------------------------------
# Install headers
#----------------------------------------------

install(
    DIRECTORY "${NFX_JSON_INCLUDE_DIR}/"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Development
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.inl"
)

#----------------------------------------------
# Install library targets
#----------------------------------------------

set(install_targets "")

if(NFX_JSON_BUILD_SHARED AND TARGET ${PROJECT_NAME})
    list(APPEND install_targets ${PROJECT_NAME})
endif()

if(NFX_JSON_BUILD_STATIC AND TARGET ${PROJECT_NAME}-static)
    list(APPEND install_targets ${PROJECT_NAME}-static)
endif()

if(install_targets)
    set(all_install_targets ${install_targets})

    if(TARGET nfx-hashing AND NOT nfx-hashing_FOUND)
        list(APPEND all_install_targets nfx-hashing)
    endif()

    if(TARGET nfx-containers AND NOT nfx-containers_FOUND)
        list(APPEND all_install_targets nfx-containers)
    endif()

    install(
        TARGETS ${all_install_targets}
        EXPORT nfx-json-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT Development
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT Runtime
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT Runtime
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    # Install PDB files for debug builds on Windows
    if(MSVC AND NFX_JSON_BUILD_SHARED AND TARGET ${PROJECT_NAME})
        install(
            FILES $<TARGET_PDB_FILE:${PROJECT_NAME}>
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            CONFIGURATIONS Debug RelWithDebInfo
            COMPONENT Development
            OPTIONAL
        )
    endif()
endif()

#----------------------------------------------
# Install CMake config files
#----------------------------------------------

if(install_targets)
    install(
        EXPORT nfx-json-targets
        FILE nfx-json-targets.cmake
        NAMESPACE nfx-json::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nfx-json
        COMPONENT Development
    )

    # Install separate target files for each configuration (multi-config generators)
    if(CMAKE_CONFIGURATION_TYPES)
        foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
            install(
                EXPORT nfx-json-targets
                FILE nfx-json-targets-${CONFIG}.cmake
                NAMESPACE nfx-json::
                DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nfx-json
                CONFIGURATIONS ${CONFIG}
                COMPONENT Development
            )
        endforeach()
    endif()
endif()

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/nfx-json-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/nfx-json-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/nfx-json-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nfx-json
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/nfx-json-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/nfx-json-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nfx-json
    COMPONENT Development
)

#----------------------------------------------
# Install license files
#----------------------------------------------

install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    DESTINATION "${CMAKE_INSTALL_DOCDIR}/licenses"
)

file(GLOB license_files "${CMAKE_CURRENT_SOURCE_DIR}/licenses/LICENSE-*")
foreach(license_file ${license_files})
    get_filename_component(license_name ${license_file} NAME)
    install(
        FILES ${license_file}
        DESTINATION "${CMAKE_INSTALL_DOCDIR}/licenses"
    )
endforeach()

#----------------------------------------------
# Install documentation
#----------------------------------------------

if(NFX_JSON_BUILD_DOCUMENTATION)
    install(
        DIRECTORY "${CMAKE_BINARY_DIR}/doc/html"
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
        OPTIONAL
        COMPONENT Documentation
    )

    if(WIN32)
        # Install Windows .cmd batch file
        install(
            FILES "${CMAKE_BINARY_DIR}/doc/index.html.cmd"
            DESTINATION ${CMAKE_INSTALL_DOCDIR}
            OPTIONAL
            COMPONENT Documentation
        )
    else()
        # Install Unix symlink
        install(
            FILES "${CMAKE_BINARY_DIR}/doc/index.html"
            DESTINATION ${CMAKE_INSTALL_DOCDIR}
            OPTIONAL
            COMPONENT Documentation
        )
    endif()
endif()

message(STATUS "Installation configured for targets: ${install_targets}")
