include(CMakeFindDependencyMacro)
find_dependency(OpenCV REQUIRED)
find_dependency(Threads REQUIRED)
include("${CMAKE_CURRENT_LIST_DIR}/rapidAfTargets.cmake")
