#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "MathLib::math_lib" for configuration ""
set_property(TARGET MathLib::math_lib APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(MathLib::math_lib PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libmath_lib.a"
  )

list(APPEND _cmake_import_check_targets MathLib::math_lib )
list(APPEND _cmake_import_check_files_for_MathLib::math_lib "${_IMPORT_PREFIX}/lib/libmath_lib.a" )

# Import target "MathLib::demo" for configuration ""
set_property(TARGET MathLib::demo APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(MathLib::demo PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/bin/demo"
  )

list(APPEND _cmake_import_check_targets MathLib::demo )
list(APPEND _cmake_import_check_files_for_MathLib::demo "${_IMPORT_PREFIX}/bin/demo" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
