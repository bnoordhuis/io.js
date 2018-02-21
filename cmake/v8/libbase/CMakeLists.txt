include(../../common.cmake)
project(v8_libbase)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DDEBUG)
endif()

set(BASE ${CMAKE_SOURCE_DIR}/deps/v8)

set(SOURCES
    ${BASE}/src/base/adapters.h
    ${BASE}/src/base/atomic-utils.h
    ${BASE}/src/base/atomicops.h
    ${BASE}/src/base/atomicops_internals_atomicword_compat.h
    ${BASE}/src/base/atomicops_internals_portable.h
    ${BASE}/src/base/atomicops_internals_std.h
    ${BASE}/src/base/base-export.h
    ${BASE}/src/base/bits.cc
    ${BASE}/src/base/bits.h
    ${BASE}/src/base/build_config.h
    ${BASE}/src/base/compiler-specific.h
    ${BASE}/src/base/cpu.cc
    ${BASE}/src/base/cpu.h
    ${BASE}/src/base/debug/stack_trace_posix.cc
    ${BASE}/src/base/debug/stack_trace.h
    ${BASE}/src/base/division-by-constant.cc
    ${BASE}/src/base/division-by-constant.h
    ${BASE}/src/base/export-template.h
    ${BASE}/src/base/file-utils.cc
    ${BASE}/src/base/file-utils.h
    ${BASE}/src/base/flags.h
    ${BASE}/src/base/format-macros.h
    ${BASE}/src/base/free_deleter.h
    ${BASE}/src/base/functional.cc
    ${BASE}/src/base/functional.h
    ${BASE}/src/base/hashmap-entry.h
    ${BASE}/src/base/hashmap.h
    ${BASE}/src/base/ieee754.cc
    ${BASE}/src/base/ieee754.h
    ${BASE}/src/base/iterator.h
    ${BASE}/src/base/lazy-instance.h
    ${BASE}/src/base/logging.cc
    ${BASE}/src/base/logging.h
    ${BASE}/src/base/macros.h
    ${BASE}/src/base/once.cc
    ${BASE}/src/base/once.h
    ${BASE}/src/base/optional.h
    ${BASE}/src/base/platform/condition-variable.cc
    ${BASE}/src/base/platform/condition-variable.h
    ${BASE}/src/base/platform/elapsed-timer.h
    ${BASE}/src/base/platform/mutex.cc
    ${BASE}/src/base/platform/mutex.h
    ${BASE}/src/base/platform/platform.h
    ${BASE}/src/base/platform/semaphore.cc
    ${BASE}/src/base/platform/semaphore.h
    ${BASE}/src/base/platform/time.cc
    ${BASE}/src/base/platform/time.h
    ${BASE}/src/base/ring-buffer.h
    ${BASE}/src/base/safe_conversions.h
    ${BASE}/src/base/safe_conversions_impl.h
    ${BASE}/src/base/safe_math.h
    ${BASE}/src/base/safe_math_impl.h
    ${BASE}/src/base/sys-info.cc
    ${BASE}/src/base/sys-info.h
    ${BASE}/src/base/template-utils.h
    ${BASE}/src/base/timezone-cache.h
    ${BASE}/src/base/tsan.h
    ${BASE}/src/base/utils/random-number-generator.cc
    ${BASE}/src/base/utils/random-number-generator.h)

if(NOT win32)
  set(LIBS ${LIBS} pthread)
  set(SOURCES ${SOURCES}
      ${BASE}/src/base/platform/platform-posix-time.cc
      ${BASE}/src/base/platform/platform-posix-time.h
      ${BASE}/src/base/platform/platform-posix.cc
      ${BASE}/src/base/platform/platform-posix.h)
  if(NOT android)
    set(SOURCES ${SOURCES} ${BASE}/src/base/debug/stack_trace_posix.cc)
  endif()
endif()

if(aix)
  set(SOURCES ${SOURCES} ${BASE}/src/base/platform/platform-aix.cc)
elseif(android)
  set(SOURCES ${SOURCES} ${BASE}/src/base/debug/stack_trace_android.cc)
elseif(freebsd)
  set(SOURCES ${SOURCES} ${BASE}/src/base/platform/platform-freebsd.cc)
elseif(linux)
  set(SOURCES ${SOURCES} ${BASE}/src/base/platform/platform-linux.cc)
  set(LIBS ${LIBS} dl rt)
elseif(mac)
  set(SOURCES ${SOURCES} ${BASE}/src/base/platform/platform-macos.cc)
elseif(netbsd OR openbsd)
  set(SOURCES ${SOURCES} ${BASE}/src/base/platform/platform-openbsd.cc)
  set(LIBS ${LIBS} execinfo)
elseif(sunos)
  set(SOURCES ${SOURCES} ${BASE}/src/base/platform/platform-solaris.cc)
  set(LIBS ${LIBS} nsl rt)
elseif(win32)
  set(SOURCES ${SOURCES}
      ${BASE}/src/base/debug/stack_trace_win.cc
      ${BASE}/src/base/platform/platform-win32.cc
      ${BASE}/src/base/win32-headers.h)
  set(LIBS ${LIBS} dbghelp shlwapi winmm ws2_32)
endif()

include_directories(${BASE})
link_libraries(${LIBS})

add_library(v8_libbase ${SHARED} ${SOURCES})
