set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)

# CMake 4.x removed support for the very old cmake_minimum_required()
# versions some vcpkg ports (e.g. gperf) still declare. This tells every
# port's own configure step to treat those as if they had asked for 3.5.
set(VCPKG_CMAKE_CONFIGURE_OPTIONS "-DCMAKE_POLICY_VERSION_MINIMUM=3.5")
