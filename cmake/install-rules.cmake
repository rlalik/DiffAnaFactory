if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/midas-${PROJECT_VERSION}"
      CACHE PATH ""
  )
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package Midas)

install(
    DIRECTORY
        include/
        "${PROJECT_BINARY_DIR}/export/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT Midas_Development
)

install(
    TARGETS Midas
    EXPORT MidasTargets
    RUNTIME #
    COMPONENT Midas_Runtime
    LIBRARY #
    COMPONENT Midas_Runtime
    NAMELINK_COMPONENT Midas_Development
    ARCHIVE #
    COMPONENT Midas_Development
    INCLUDES #
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    Midas_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(Midas_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${Midas_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT Midas_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${Midas_INSTALL_CMAKEDIR}"
    COMPONENT Midas_Development
)

install(
    EXPORT MidasTargets
    NAMESPACE Midas::
    DESTINATION "${Midas_INSTALL_CMAKEDIR}"
    COMPONENT Midas_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
