cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

include(cmake/prelude.cmake)

project(
    Midas
    VERSION 0.3.0
    DESCRIPTION "Midas - a Mutli Dimensional Analysis framework."
    HOMEPAGE_URL "https://github.com/rlalik/Midas"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)
include(cmake/find_or_fetch_package.cmake)

# find ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT QUIET REQUIRED COMPONENTS Core Gpad Hist)

find_or_fetch_package(fmt https://github.com/fmtlib/fmt GIT_TAG 10.0.0 VERSION 10.0.0)

# json parser
set(JSONCPP_WITH_PKGCONFIG_SUPPORT OFF)
set(JSONCPP_WITH_CMAKE_PACKAGE OFF)
set(JSONCPP_WITH_TESTS OFF)

# cmake-format: off
find_or_fetch_package(jsoncpp
  https://github.com/open-source-parsers/jsoncpp
  VERSION 1.9.5 GIT_TAG master FETCH)
# cmake-format: on

include_directories(/usr/include/jsoncpp/)
# set(jsoncpp_dep jsoncpp) else() set(jsoncpp_dep jsoncpp_lib) endif()

find_package(RootTools REQUIRED)
find_package(Pandora REQUIRED)
find_package(HelloFitty REQUIRED)

# conditional modules
option(ENABLE_ADVANCE_TOOLS "Enables advanced tools" OFF)
if(ENABLE_ADVANCE_TOOLS)
  include(FetchContent)
  FetchContent_Declare(
    cmake-scripts
    GIT_REPOSITORY https://github.com/StableCoder/cmake-scripts.git
    GIT_TAG main)

  FetchContent_MakeAvailable(cmake-scripts)
  list(APPEND CMAKE_MODULE_PATH ${cmake-scripts_SOURCE_DIR})
  message(STATUS "${cmake-scripts_SOURCE_DIR}")
  include(sanitizers)
  include(code-coverage)
  include(tools)
  include_what_you_use(-Xiwyu;--error_always)
endif()

# find_package(HistAsymmErrors QUIET)
if(HistAsymmErrors_FOUND)
  add_definitions("-DHAVE_HISTASYMMERRORS=1")
endif()

include(cmake/shared_or_static.cmake)
shared_or_static(${PROJECT_NAME})

# ---- Declare library ----
add_library(
    Midas
    ${Midas_LIBRARY_TYPE}
    source/axis_config.cpp
    source/context.cpp
    source/v_context.cpp
    source/DistributionFactory.cpp
    source/DifferentialFactory.cpp
    source/observable.cpp
    source/MultiDimAnalysisContext.cpp
    source/detail.cpp
)
add_library(Midas::Midas ALIAS Midas)

target_link_libraries(Midas PUBLIC ROOT::Core ROOT::Gpad PRIVATE RT::Pandora HelloFitty::HelloFitty jsoncpp)

if (fmt_FETCHED)
  set(FMT_TARGET fmt::fmt-header-only)
else()
  set(FMT_TARGET fmt::fmt)
endif()
target_link_libraries(Midas PRIVATE ${FMT_TARGET})

# if (HelloFitty_FOUND)
#   target_compile_definitions(Midas PRIVATE HELLOFITTY_FOUND)
# endif()

# configure_file(config.h.in midas_config.h)

include(GenerateExportHeader)
generate_export_header(
    Midas
    BASE_NAME Midas
    EXPORT_FILE_NAME export/Midas/midas_export.hpp
    CUSTOM_CONTENT_FROM_VARIABLE pragma_suppress_c4251
)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(Midas PUBLIC MIDAS_STATIC_DEFINE)
endif()

set_target_properties(
    Midas PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME Midas
    OUTPUT_NAME Midas
)

target_include_directories(
  Midas ${warning_guard}
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/inc>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_include_directories(
    Midas SYSTEM
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
)

option(BUILD_DICTIONARY "Build ROOT dictionary." OFF)
if (BUILD_DICTIONARY)
  # cmake-format: off
  root_generate_dictionary(G__Midas_cc
    DistributionContext.hpp
    DistributionFactory.hpp
    DifferentialContext.hpp
    DifferentialFactory.hpp
    ExtraDimensionMapper.hpp
    MultiDimDistributionContext.hpp
    MODULE Midas
    LINKDEF inc/LinkDef.h)
  # cmake-format: on
endif()

# target_compile_features(Midas PUBLIC cxx_std_11)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Examples ----

if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_EXAMPLES "Build examples tree." "${Midas_DEVELOPER_MODE}")
  if(BUILD_EXAMPLES)
    add_subdirectory(example)
  endif()
endif()

# ---- Developer mode ----

if(NOT Midas_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of HelloFitty"
  )
endif()

include(cmake/dev-mode.cmake)

# Export the package for use from the build-tree (this registers the build-tree
# with a global CMake-registry)
export(PACKAGE Midas)

# messages
message(
  STATUS
    "<<< Configuration >>>
Project:        ${PROJECT_NAME}

Architecture:   ${CMAKE_CXX_LIBRARY_ARCHITECTURE}
Build type      ${CMAKE_BUILD_TYPE}
Install path    ${CMAKE_INSTALL_PREFIX}

Compiler:
C++             ${CMAKE_CXX_COMPILER}

Linker:
Ld              ${CMAKE_LINKER}

Compiler flags:
C++             ${CMAKE_CXX_FLAGS}

Linker flags:
Executable      ${CMAKE_EXE_LINKER_FLAGS}
Module          ${CMAKE_MODULE_LINKER_FLAGS}
Shared          ${CMAKE_SHARED_LINKER_FLAGS}\n")

foreach(p LIB BIN INCLUDE CMAKE)
  message(STATUS "CMAKE_INSTALL_${p}DIR: ${CMAKE_INSTALL_${p}DIR}")
endforeach()
