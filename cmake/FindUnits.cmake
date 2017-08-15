# This file is provided under the "BSD-style" License

# Find Units
#
# This sets the following variables:
# Units_FOUND
# Units_INCLUDE_DIRS
# Units_LIBRARIES
# Units_DEFINITIONS
# Units_VERSION

find_package(PkgConfig QUIET)

# Check to see if pkgconfig is installed.
pkg_check_modules(PC_Units Units QUIET)

# Definitions
set(Units_DEFINITIONS ${PC_Units_CFLAGS_OTHER})

# Include directories
find_path(Units_INCLUDE_DIRS
    NAMES units.hpp
    HINTS ${PC_Units_INCLUDEDIR}
    PATHS "${CMAKE_INSTALL_PREFIX}/include")

# Libraries
find_library(Units_LIBRARIES
    NAMES units
    HINTS ${PC_Units_LIBDIR})

# Version
set(Units_VERSION ${PC_Units_VERSION})

# Set (NAME)_FOUND if all the variables and the version are satisfied.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Units
    FAIL_MESSAGE  DEFAULT_MSG
    REQUIRED_VARS Units_INCLUDE_DIRS Units_LIBRARIES
    VERSION_VAR   Units_VERSION)

