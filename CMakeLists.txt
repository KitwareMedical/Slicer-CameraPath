cmake_minimum_required(VERSION 3.10)

project(ivsplan)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "")
set(EXTENSION_CATEGORY "IVSPlan")
set(EXTENSION_CONTRIBUTORS "Alexis Girault (Kitware)")
set(EXTENSION_DESCRIPTION "IVSPlan: the Intelligent Virtual Surgical Planning application software")
set(EXTENSION_ICONURL "")
set(EXTENSION_SCREENSHOTURLS "")
set(EXTENSION_DEPENDS "NA") # Specified as a space separated string, a list or 'NA' if any

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(Modules/Loadable/CameraPath)
add_subdirectory(Modules/Scripted/testCameraPath)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})
