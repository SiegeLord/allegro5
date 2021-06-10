# - Find libgbm
# Find the libgbm includes and libraries
#
#  GBM_INCLUDE_DIR - where to find libgbm/gbm.h, etc.
#  GBM_LIBRARY     - The libgbm library.
#  GBM_FOUND       - True if libgbm is found.

if(GBM_INCLUDE_DIR)
    # Already in cache, be silent
    set(GBM_FIND_QUIETLY TRUE)
endif(GBM_INCLUDE_DIR)

find_path(GBM_INCLUDE_DIR gbm.h)

find_library(GBM_LIBRARY NAMES gbm)

# Handle the QUIETLY and REQUIRED arguments and set GBM_FOUND
# to TRUE if all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GBM DEFAULT_MSG
    GBM_INCLUDE_DIR GBM_LIBRARY)

mark_as_advanced(GBM_INCLUDE_DIR)
mark_as_advanced(GBM_LIBRARY)
