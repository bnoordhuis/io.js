cmake_minimum_required(VERSION 3.0)
find_package(PythonInterp)

include(cmake/common.cmake)
project(node)

# Reset this number to 0 on major V8 upgrades.
# Increment by one for each non-official patch applied to deps/v8.
set(v8_embedder_string "-node.11")
set(v8_postmortem_support $<BOOL:NOT win32>)

set(CMAKE_VERBOSE_MAKEFILE ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

if(NOT MSVC AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_C_FLAGS "-g -Og ${CMAKE_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "-g -Og ${CMAKE_CXX_FLAGS}")
endif()

if(MSVC)
  #add_definitions(BUILDING_UV_SHARED=1)
  #add_definitions(BUILDING_V8_SHARED=1)
  add_definitions(_HAS_EXCEPTIONS=0)
  add_definitions(/W2)
else()
  add_definitions(-Wall -Wextra -Wno-unused-parameter)
  set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
endif()

if(asan)
  if(mac)
    set(CMAKE_CXX_FLAGS "-gline-tables-only ${CMAKE_CXX_FLAGS}")
  endif()
  set(CMAKE_CXX_FLAGS "-fno-omit-frame-pointer ${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "-fsanitize=address ${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "-DLEAK_SANITIZER ${CMAKE_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "-fsanitize=address ${CMAKE_EXE_LINKER_FLAGS}")
endif()

if(node_target_type STREQUAL "shared_library")
  set(SHARED SHARED)
else()
  set(SHARED)
endif()

if(shlib_suffix)
  set(CMAKE_SHARED_LIBRARY_SUFFIX ${shlib_suffix})
endif()

if(NOT node_shared_cares)
  add_subdirectory(cmake/cares)
endif()

if(NOT node_shared_http_parser)
  add_subdirectory(cmake/http_parser)
endif()

if(NOT node_shared_libuv)
  add_subdirectory(cmake/uv)
endif()

if(NOT node_shared_nghttp2)
  add_subdirectory(cmake/nghttp2)
endif()

if(NOT node_shared_openssl)
  add_subdirectory(cmake/openssl)
endif()

if(node_use_bundled_v8)
  add_subdirectory(cmake/v8)
endif()

if(NOT node_shared_zlib)
  add_subdirectory(cmake/zlib)
endif()

add_subdirectory(cmake/gtest)
add_subdirectory(cmake/icu)
add_subdirectory(cmake/node)

# Silence cmake warnings about unused variables that are set by configure.
# These can't be removed outright because they are in use by the GYP build.
if(coverage OR
   force_dynamic_crt OR
   gas_version OR
   host_arch OR
   icu_gyp_path OR
   icu_path OR
   llvm_version OR
   node_install_npm OR
   node_module_version OR
   node_prefix OR
   target_arch OR
   v8_no_strict_aliasing OR
   v8_optimized_debug OR
   v8_random_seed OR
   v8_typed_array_max_size_in_heap OR
   want_separate_host_toolset)
endif()
