cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(
  MultiDimensionalAnalyzer
  VERSION 0.2.0
  LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME})

# Depedencies
include(FetchContent)

# cmake_tools - tools to fetch external packages and build static/shared libs
FetchContent_Declare(
  cmake_tools
  GIT_REPOSITORY https://github.com/rlalik/cmake_tools
  GIT_TAG master)
FetchContent_MakeAvailable(cmake_tools)
list(APPEND CMAKE_MODULE_PATH ${cmake_tools_SOURCE_DIR})

include(find_or_fetch_package)

# json parser
set(JSONCPP_WITH_PKGCONFIG_SUPPORT OFF)
set(JSONCPP_WITH_CMAKE_PACKAGE OFF)

# cmake-format: off
find_or_fetch_package(jsoncpp
  https://github.com/open-source-parsers/jsoncpp
  VERSION 1.9.5 GIT_TAG master FETCH)
# cmake-format: on

include_directories(/usr/include/jsoncpp/)
# set(jsoncpp_dep jsoncpp) else() set(jsoncpp_dep jsoncpp_lib) endif()

find_package(RootTools REQUIRED)
find_package(Pandora REQUIRED)
find_package(FitterFactory REQUIRED)

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

# find ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(
  ROOT QUIET REQUIRED
  COMPONENTS Core
  OPTIONAL_COMPONENTS Cling Cint)

# include_directories( ${HISTASYMMERRORS_INCLUDE_DIR} )

# link_directories( ${HISTASYMMERRORS_LIBRARY_DIR} )

include(shared_or_static)
shared_or_static(${PROJECT_NAME})

add_library(
  ${PROJECT_NAME}
  ${${PROJECT_NAME}_LIBRARY_TYPE}
  src/DistributionContext.cxx
  src/DistributionFactory.cxx
  src/DifferentialContext.cxx
  src/DifferentialFactory.cxx
  src/ExtraDimensionMapper.cxx
  src/MultiDimAnalysisContext.cxx)
add_library(RT::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC ROOT::Core
  PRIVATE RT::Pandora RT::FitterFactory jsoncpp)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER
    "inc/DistributionContext.h;inc/DistributionFactory.h;inc/DifferentialContext.h;inc/DifferentialFactory.h;inc/ExtraDimensionMapper.h;inc/MultiDimDistributionContext.h"
)

# cmake-format: off
root_generate_dictionary(G__${PROJECT_NAME}_cc
  inc/DistributionContext.h
  inc/DistributionFactory.h
  inc/DifferentialContext.h
  inc/DifferentialFactory.h
  inc/ExtraDimensionMapper.h
  inc/MultiDimDistributionContext.h
  MODULE ${PROJECT_NAME}
  LINKDEF LinkDef.h)
# cmake-format: on

# Export the package for use from the build-tree (this registers the build-tree
# with a global CMake-registry)
export(PACKAGE ${PROJECT_NAME})

# Install the export set for use with the install-tree
install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  EXPORT ${PROJECT_NAME}Targets
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE RT::
  DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
  COMPONENT dev)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  Config.cmake.in ${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
  PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR CMAKE_INSTALL_BINDIR)

write_basic_package_version_file(
  ${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_CMAKEDIR})

add_subdirectory(examples)

enable_testing()
add_subdirectory(tests)

# messages
message(
  STATUS
    "<<< Configuration >>>
Project:        ${CMAKE_PROJECT_NAME}

Architecture:   ${CMAKE_LIBRARY_ARCHITECTURE}
Build type      ${CMAKE_BUILD_TYPE}
Install path    ${CMAKE_INSTALL_PREFIX}

Compiler:
C               ${CMAKE_C_COMPILER}
C++             ${CMAKE_CXX_COMPILER}

Linker:
Ld              ${CMAKE_LINKER}

Compiler flags:
C               ${CMAKE_C_FLAGS}
C++             ${CMAKE_CXX_FLAGS}

Linker flags:
Executable      ${CMAKE_EXE_LINKER_FLAGS}
Module          ${CMAKE_MODULE_LINKER_FLAGS}
Shared          ${CMAKE_SHARED_LINKER_FLAGS}\n")

foreach(p LIB BIN INCLUDE CMAKE)
  message(STATUS "CMAKE_INSTALL_${p}DIR: ${CMAKE_INSTALL_${p}DIR}")
endforeach()
