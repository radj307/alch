# Install script for directory: X:/_Workspace/_repo/alch-cmake/307lib

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "X:/_Workspace/_repo/alch-cmake/out/install/windows-default")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("X:/_Workspace/_repo/alch-cmake/out/build/windows-default/307lib/shared/cmake_install.cmake")
  include("X:/_Workspace/_repo/alch-cmake/out/build/windows-default/307lib/str-lib/cmake_install.cmake")
  include("X:/_Workspace/_repo/alch-cmake/out/build/windows-default/307lib/TermAPI/cmake_install.cmake")
  include("X:/_Workspace/_repo/alch-cmake/out/build/windows-default/307lib/file-lib/cmake_install.cmake")
  include("X:/_Workspace/_repo/alch-cmake/out/build/windows-default/307lib/opt-lib/cmake_install.cmake")
  include("X:/_Workspace/_repo/alch-cmake/out/build/windows-default/307lib/testing/cmake_install.cmake")

endif()

