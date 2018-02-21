include(../common.cmake)
project(zlib)

set(BASE ${CMAKE_SOURCE_DIR}/deps/zlib)

add_library(
    zlib ${SHARED}
    ${BASE}/adler32.c
    ${BASE}/compress.c
    ${BASE}/crc32.c
    ${BASE}/crc32.h
    ${BASE}/deflate.c
    ${BASE}/deflate.h
    ${BASE}/gzclose.c
    ${BASE}/gzguts.h
    ${BASE}/gzlib.c
    ${BASE}/gzread.c
    ${BASE}/gzwrite.c
    ${BASE}/infback.c
    ${BASE}/inffast.c
    ${BASE}/inffast.h
    ${BASE}/inffixed.h
    ${BASE}/inflate.c
    ${BASE}/inflate.h
    ${BASE}/inftrees.c
    ${BASE}/inftrees.h
    ${BASE}/trees.c
    ${BASE}/trees.h
    ${BASE}/uncompr.c
    ${BASE}/zconf.h
    ${BASE}/zlib.h
    ${BASE}/zutil.c
    ${BASE}/zutil.h)

if(NOT MSVC)
  target_compile_definitions(zlib PRIVATE Z_HAVE_UNISTD_H HAVE_HIDDEN)
endif()
