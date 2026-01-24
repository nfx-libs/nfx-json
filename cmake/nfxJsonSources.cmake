#==============================================================================
# nfx-json - CMake Sources
#==============================================================================

#----------------------------------------------
# Source files
#----------------------------------------------

set(private_sources "")

list(APPEND private_sources
    ${NFX_JSON_SOURCE_DIR}/Document.cpp
    ${NFX_JSON_SOURCE_DIR}/SchemaGenerator.cpp
    ${NFX_JSON_SOURCE_DIR}/SchemaValidator.cpp
)


