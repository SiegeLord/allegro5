# - Find libdrm
# Find the libdrm includes
#
#  DRM_INCLUDE_DIR - where to find libdrm/drm.h, etc.
#  DRM_FOUND       - True if libdrm is found.

if(DRM_INCLUDE_DIR)
    # Already in cache, be silent
    set(DRM_FIND_QUIETLY TRUE)
endif(DRM_INCLUDE_DIR)

find_path(DRM_INCLUDE_DIR drm.h PATH_SUFFIXES libdrm)

# Handle the QUIETLY and REQUIRED arguments and set DRM_FOUND
# to TRUE if all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DRM DEFAULT_MSG
    DRM_INCLUDE_DIR)

mark_as_advanced(DRM_INCLUDE_DIR)

