
# ==============================================

cmake_minimum_required(VERSION 3.15)

# ==============================================

set(NAME "MiniSynthFM")
set(CODE "Msfm")
set(COMPANY "Kaixo")
set(COMPANY_CODE "Kaix")
set(WEBSITE "https://kaixo.me")
set(BUNDLE_IDENTIFIER "me.kaixo.minisynthfm")
set(VERSION "1.0.0")
set(VERSION_TYPE "DEMO")
set(INITIAL_SIZE "1205, 681")
set(IS_SYNTH true)

# ==============================================

set(THEME "theme/theme.json")
set(THEME_SCHEMA "theme/schema.json")
set(PARAMETERS "resources/Parameters.xml")

# ==============================================

set (CMAKE_CXX_STANDARD 23)

project(${NAME} VERSION ${VERSION})

add_subdirectory(core)

# ==============================================

file(GLOB_RECURSE PRESET_GENERATOR_SOURCE
    "${CMAKE_SOURCE_DIR}/presets/*.cpp"
    "${CMAKE_SOURCE_DIR}/presets/*.hpp"
)

source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${PRESET_GENERATOR_SOURCE})
    
add_executable(PresetGenerator
    ${PRESET_GENERATOR_SOURCE})

target_include_directories(PresetGenerator
    PRIVATE
        ${CORE_SOURCE_DIRECTORY}/include/
        ${CMAKE_SOURCE_DIR}/presets/)

# ==============================================

file(GLOB_RECURSE DEFAULT_PRESETS
    "${CMAKE_SOURCE_DIR}/presets/*.minifm"
)

set(GENERATED_PRESETS "DefaultPresets.hpp")
set(GENERATE_PRESETS_FROM ${CMAKE_SOURCE_DIR}/presets/)
set(GENERATE_PRESETS_TO ${GENERATED_SOURCES_DIR}/${GENERATED_PRESETS})

message("${GENERATE_PRESETS_TO}")

add_custom_command(OUTPUT ${GENERATE_PRESETS_TO}
    COMMAND PresetGenerator ${GENERATE_PRESETS_FROM} ${GENERATE_PRESETS_TO}
    DEPENDS ${DEFAULT_PRESETS}
)

add_custom_target(GeneratePresets
    DEPENDS ${GENERATE_PRESETS_TO}
)

add_dependencies(${NAME} GeneratePresets)

# ==============================================
