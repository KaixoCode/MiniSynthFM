
# ==============================================

cmake_minimum_required(VERSION 3.15)

# ==============================================

set(NAME "MiniSynthFM")
set(CODE "Msfm")
set(COMPANY "Kaixo")
set(COMPANY_CODE "Kaix")
set(WEBSITE "https://kaixo.me")
set(BUNDLE_IDENTIFIER "me.kaixo.minisynthfm")
set(VERSION "0.0.1")
set(VERSION_TYPE "SNAPSHOT")
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
