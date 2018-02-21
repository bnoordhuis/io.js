include(../../common.cmake)
project(icu_tools)

set(BASE ${CMAKE_SOURCE_DIR}/deps/icu-small/source/tools)

include_directories(${BASE}/toolutil)

add_library(
    icu_toolutil ${SHARED}
    ${BASE}/toolutil/collationinfo.cpp
    ${BASE}/toolutil/collationinfo.h
    ${BASE}/toolutil/dbgutil.cpp
    ${BASE}/toolutil/dbgutil.h
    ${BASE}/toolutil/denseranges.cpp
    ${BASE}/toolutil/denseranges.h
    ${BASE}/toolutil/filestrm.cpp
    ${BASE}/toolutil/filestrm.h
    ${BASE}/toolutil/filetools.cpp
    ${BASE}/toolutil/filetools.h
    ${BASE}/toolutil/flagparser.cpp
    ${BASE}/toolutil/flagparser.h
    ${BASE}/toolutil/package.cpp
    ${BASE}/toolutil/package.h
    ${BASE}/toolutil/pkg_genc.cpp
    ${BASE}/toolutil/pkg_genc.h
    ${BASE}/toolutil/pkg_gencmn.cpp
    ${BASE}/toolutil/pkg_gencmn.h
    ${BASE}/toolutil/pkg_icu.cpp
    ${BASE}/toolutil/pkg_icu.h
    ${BASE}/toolutil/pkg_imp.h
    ${BASE}/toolutil/pkgitems.cpp
    ${BASE}/toolutil/ppucd.cpp
    ${BASE}/toolutil/ppucd.h
    ${BASE}/toolutil/swapimpl.cpp
    ${BASE}/toolutil/swapimpl.h
    ${BASE}/toolutil/toolutil.cpp
    ${BASE}/toolutil/toolutil.h
    ${BASE}/toolutil/ucbuf.cpp
    ${BASE}/toolutil/ucbuf.h
    ${BASE}/toolutil/ucln_tu.cpp
    ${BASE}/toolutil/ucm.cpp
    ${BASE}/toolutil/ucm.h
    ${BASE}/toolutil/ucmstate.cpp
    ${BASE}/toolutil/udbgutil.cpp
    ${BASE}/toolutil/udbgutil.h
    ${BASE}/toolutil/unewdata.cpp
    ${BASE}/toolutil/unewdata.h
    ${BASE}/toolutil/uoptions.cpp
    ${BASE}/toolutil/uoptions.h
    ${BASE}/toolutil/uparse.cpp
    ${BASE}/toolutil/uparse.h
    ${BASE}/toolutil/writesrc.cpp
    ${BASE}/toolutil/writesrc.h
    ${BASE}/toolutil/xmlparser.cpp
    ${BASE}/toolutil/xmlparser.h)

target_compile_definitions(icu_toolutil PRIVATE U_TOOLUTIL_IMPLEMENTATION=1)
target_link_libraries(icu_toolutil icu_common_stubdata icu_i18n_stubdata)

add_executable(
    genrb
    ${BASE}/genrb/errmsg.c
    ${BASE}/genrb/errmsg.h
    ${BASE}/genrb/genrb.cpp
    ${BASE}/genrb/genrb.h
    ${BASE}/genrb/parse.cpp
    ${BASE}/genrb/parse.h
    ${BASE}/genrb/prscmnts.cpp
    ${BASE}/genrb/prscmnts.h
    ${BASE}/genrb/rbutil.c
    ${BASE}/genrb/rbutil.h
    ${BASE}/genrb/read.c
    ${BASE}/genrb/read.h
    ${BASE}/genrb/reslist.cpp
    ${BASE}/genrb/reslist.h
    ${BASE}/genrb/rle.c
    ${BASE}/genrb/rle.h
    ${BASE}/genrb/ustr.c
    ${BASE}/genrb/ustr.h
    ${BASE}/genrb/wrtjava.cpp
    ${BASE}/genrb/wrtxml.cpp)
target_link_libraries(genrb icu_toolutil)

add_executable(
    genccode
    ${BASE}/genccode/genccode.c
    ${BASE}/../stubdata/stubdata.cpp)
target_link_libraries(genccode icu_toolutil)

add_executable(icupkg ${BASE}/icupkg/icupkg.cpp)
target_link_libraries(icupkg icu_toolutil)

add_executable(iculslocs ${CMAKE_SOURCE_DIR}/tools/icu/iculslocs.cc)
target_link_libraries(iculslocs icu_toolutil)
