include(../../common.cmake)
project(icu_common)

set(BASE ${CMAKE_SOURCE_DIR}/deps/icu-small/source/common)

set(DEFINES
    -DUCONFIG_NO_BREAK_ITERATION=0
    -DUCONFIG_NO_LEGACY_CONVERSION=1
    -DUCONFIG_NO_REGULAR_EXPRESSIONS=1
    -DUCONFIG_NO_SERVICE=1
    -DU_ENABLE_DYLOAD=0
    -DU_HAVE_STD_STRING=0)

add_definitions(${DEFINES})
include_directories(${BASE})

set(SOURCES
    ${BASE}/appendable.cpp
    ${BASE}/bmpset.cpp
    ${BASE}/bmpset.h
    ${BASE}/brkeng.cpp
    ${BASE}/brkeng.h
    ${BASE}/brkiter.cpp
    ${BASE}/bytesinkutil.cpp
    ${BASE}/bytesinkutil.h
    ${BASE}/bytestream.cpp
    ${BASE}/bytestrie.cpp
    ${BASE}/bytestriebuilder.cpp
    ${BASE}/bytestrieiterator.cpp
    ${BASE}/caniter.cpp
    ${BASE}/chariter.cpp
    ${BASE}/charstr.cpp
    ${BASE}/charstr.h
    ${BASE}/cmemory.cpp
    ${BASE}/cmemory.h
    ${BASE}/cpputils.h
    ${BASE}/cstr.cpp
    ${BASE}/cstr.h
    ${BASE}/cstring.cpp
    ${BASE}/cstring.h
    ${BASE}/cwchar.cpp
    ${BASE}/cwchar.h
    ${BASE}/dictbe.cpp
    ${BASE}/dictbe.h
    ${BASE}/dictionarydata.cpp
    ${BASE}/dictionarydata.h
    ${BASE}/dtintrv.cpp
    ${BASE}/edits.cpp
    ${BASE}/errorcode.cpp
    ${BASE}/filteredbrk.cpp
    ${BASE}/filterednormalizer2.cpp
    ${BASE}/hash.h
    ${BASE}/icudataver.cpp
    ${BASE}/icuplug.cpp
    ${BASE}/icuplugimp.h
    ${BASE}/listformatter.cpp
    ${BASE}/loadednormalizer2impl.cpp
    ${BASE}/localsvc.h
    ${BASE}/locavailable.cpp
    ${BASE}/locbased.cpp
    ${BASE}/locbased.h
    ${BASE}/locdispnames.cpp
    ${BASE}/locdspnm.cpp
    ${BASE}/locid.cpp
    ${BASE}/loclikely.cpp
    ${BASE}/locmap.cpp
    ${BASE}/locmap.h
    ${BASE}/locresdata.cpp
    ${BASE}/locutil.cpp
    ${BASE}/locutil.h
    ${BASE}/messageimpl.h
    ${BASE}/messagepattern.cpp
    ${BASE}/msvcres.h
    ${BASE}/mutex.h
    ${BASE}/norm2_nfc_data.h
    ${BASE}/norm2allmodes.h
    ${BASE}/normalizer2.cpp
    ${BASE}/normalizer2impl.cpp
    ${BASE}/normalizer2impl.h
    ${BASE}/normlzr.cpp
    ${BASE}/parsepos.cpp
    ${BASE}/patternprops.cpp
    ${BASE}/patternprops.h
    ${BASE}/pluralmap.cpp
    ${BASE}/pluralmap.h
    ${BASE}/propname.cpp
    ${BASE}/propname.h
    ${BASE}/propname_data.h
    ${BASE}/propsvec.cpp
    ${BASE}/propsvec.h
    ${BASE}/punycode.cpp
    ${BASE}/punycode.h
    ${BASE}/putil.cpp
    ${BASE}/putilimp.h
    ${BASE}/rbbi.cpp
    ${BASE}/rbbi_cache.cpp
    ${BASE}/rbbidata.cpp
    ${BASE}/rbbidata.h
    ${BASE}/rbbinode.cpp
    ${BASE}/rbbinode.h
    ${BASE}/rbbirb.cpp
    ${BASE}/rbbirb.h
    ${BASE}/rbbirpt.h
    ${BASE}/rbbiscan.cpp
    ${BASE}/rbbiscan.h
    ${BASE}/rbbisetb.cpp
    ${BASE}/rbbisetb.h
    ${BASE}/rbbistbl.cpp
    ${BASE}/rbbitblb.cpp
    ${BASE}/rbbitblb.h
    ${BASE}/resbund.cpp
    ${BASE}/resbund_cnv.cpp
    ${BASE}/resource.cpp
    ${BASE}/resource.h
    ${BASE}/ruleiter.cpp
    ${BASE}/ruleiter.h
    ${BASE}/schriter.cpp
    ${BASE}/serv.cpp
    ${BASE}/serv.h
    ${BASE}/servlk.cpp
    ${BASE}/servlkf.cpp
    ${BASE}/servloc.h
    ${BASE}/servls.cpp
    ${BASE}/servnotf.cpp
    ${BASE}/servnotf.h
    ${BASE}/servrbf.cpp
    ${BASE}/servslkf.cpp
    ${BASE}/sharedobject.cpp
    ${BASE}/sharedobject.h
    ${BASE}/simpleformatter.cpp
    ${BASE}/sprpimpl.h
    ${BASE}/stringpiece.cpp
    ${BASE}/stringtriebuilder.cpp
    ${BASE}/uarrsort.cpp
    ${BASE}/uarrsort.h
    ${BASE}/uassert.h
    ${BASE}/ubidi.cpp
    ${BASE}/ubidi_props.cpp
    ${BASE}/ubidi_props.h
    ${BASE}/ubidi_props_data.h
    ${BASE}/ubidiimp.h
    ${BASE}/ubidiln.cpp
    ${BASE}/ubiditransform.cpp
    ${BASE}/ubidiwrt.cpp
    ${BASE}/ubrk.cpp
    ${BASE}/ubrkimpl.h
    ${BASE}/ucase.cpp
    ${BASE}/ucase.h
    ${BASE}/ucase_props_data.h
    ${BASE}/ucasemap.cpp
    ${BASE}/ucasemap_imp.h
    ${BASE}/ucasemap_titlecase_brkiter.cpp
    ${BASE}/ucat.cpp
    ${BASE}/uchar.cpp
    ${BASE}/uchar_props_data.h
    ${BASE}/ucharstrie.cpp
    ${BASE}/ucharstriebuilder.cpp
    ${BASE}/ucharstrieiterator.cpp
    ${BASE}/uchriter.cpp
    ${BASE}/ucln.h
    ${BASE}/ucln_cmn.cpp
    ${BASE}/ucln_cmn.h
    ${BASE}/ucln_imp.h
    ${BASE}/ucmndata.cpp
    ${BASE}/ucmndata.h
    ${BASE}/ucnv.cpp
    ${BASE}/ucnv2022.cpp
    ${BASE}/ucnv_bld.cpp
    ${BASE}/ucnv_bld.h
    ${BASE}/ucnv_cb.cpp
    ${BASE}/ucnv_cnv.cpp
    ${BASE}/ucnv_cnv.h
    ${BASE}/ucnv_ct.cpp
    ${BASE}/ucnv_err.cpp
    ${BASE}/ucnv_ext.cpp
    ${BASE}/ucnv_ext.h
    ${BASE}/ucnv_imp.h
    ${BASE}/ucnv_io.cpp
    ${BASE}/ucnv_io.h
    ${BASE}/ucnv_lmb.cpp
    ${BASE}/ucnv_set.cpp
    ${BASE}/ucnv_u16.cpp
    ${BASE}/ucnv_u32.cpp
    ${BASE}/ucnv_u7.cpp
    ${BASE}/ucnv_u8.cpp
    ${BASE}/ucnvbocu.cpp
    ${BASE}/ucnvdisp.cpp
    ${BASE}/ucnvhz.cpp
    ${BASE}/ucnvisci.cpp
    ${BASE}/ucnvlat1.cpp
    ${BASE}/ucnvmbcs.cpp
    ${BASE}/ucnvmbcs.h
    ${BASE}/ucnvscsu.cpp
    ${BASE}/ucnvsel.cpp
    ${BASE}/ucol_data.h
    ${BASE}/ucol_swp.cpp
    ${BASE}/ucol_swp.h
    ${BASE}/ucurr.cpp
    ${BASE}/ucurrimp.h
    ${BASE}/udata.cpp
    ${BASE}/udatamem.cpp
    ${BASE}/udatamem.h
    ${BASE}/udataswp.cpp
    ${BASE}/udataswp.h
    ${BASE}/uelement.h
    ${BASE}/uenum.cpp
    ${BASE}/uenumimp.h
    ${BASE}/uhash.cpp
    ${BASE}/uhash.h
    ${BASE}/uhash_us.cpp
    ${BASE}/uidna.cpp
    ${BASE}/uinit.cpp
    ${BASE}/uinvchar.cpp
    ${BASE}/uinvchar.h
    ${BASE}/uiter.cpp
    ${BASE}/ulist.cpp
    ${BASE}/ulist.h
    ${BASE}/ulistformatter.cpp
    ${BASE}/uloc.cpp
    ${BASE}/uloc_keytype.cpp
    ${BASE}/uloc_tag.cpp
    ${BASE}/ulocimp.h
    ${BASE}/umapfile.cpp
    ${BASE}/umapfile.h
    ${BASE}/umath.cpp
    ${BASE}/umutex.cpp
    ${BASE}/umutex.h
    ${BASE}/unames.cpp
    ${BASE}/unifiedcache.cpp
    ${BASE}/unifiedcache.h
    ${BASE}/unifilt.cpp
    ${BASE}/unifunct.cpp
    ${BASE}/uniset.cpp
    ${BASE}/uniset_closure.cpp
    ${BASE}/uniset_props.cpp
    ${BASE}/unisetspan.cpp
    ${BASE}/unisetspan.h
    ${BASE}/unistr.cpp
    ${BASE}/unistr_case.cpp
    ${BASE}/unistr_case_locale.cpp
    ${BASE}/unistr_cnv.cpp
    ${BASE}/unistr_props.cpp
    ${BASE}/unistr_titlecase_brkiter.cpp
    ${BASE}/unistrappender.h
    ${BASE}/unorm.cpp
    ${BASE}/unormcmp.cpp
    ${BASE}/unormimp.h
    ${BASE}/uobject.cpp
    ${BASE}/uposixdefs.h
    ${BASE}/uprops.cpp
    ${BASE}/uprops.h
    ${BASE}/ures_cnv.cpp
    ${BASE}/uresbund.cpp
    ${BASE}/uresdata.cpp
    ${BASE}/uresdata.h
    ${BASE}/uresimp.h
    ${BASE}/ureslocs.h
    ${BASE}/usc_impl.cpp
    ${BASE}/usc_impl.h
    ${BASE}/uscript.cpp
    ${BASE}/uscript_props.cpp
    ${BASE}/uset.cpp
    ${BASE}/uset_imp.h
    ${BASE}/uset_props.cpp
    ${BASE}/usetiter.cpp
    ${BASE}/ushape.cpp
    ${BASE}/usprep.cpp
    ${BASE}/ustack.cpp
    ${BASE}/ustr_cnv.cpp
    ${BASE}/ustr_cnv.h
    ${BASE}/ustr_imp.h
    ${BASE}/ustr_titlecase_brkiter.cpp
    ${BASE}/ustr_wcs.cpp
    ${BASE}/ustrcase.cpp
    ${BASE}/ustrcase_locale.cpp
    ${BASE}/ustrenum.cpp
    ${BASE}/ustrenum.h
    ${BASE}/ustrfmt.cpp
    ${BASE}/ustrfmt.h
    ${BASE}/ustring.cpp
    ${BASE}/ustrtrns.cpp
    ${BASE}/utext.cpp
    ${BASE}/utf_impl.cpp
    ${BASE}/util.cpp
    ${BASE}/util.h
    ${BASE}/util_props.cpp
    ${BASE}/utrace.cpp
    ${BASE}/utracimp.h
    ${BASE}/utrie.cpp
    ${BASE}/utrie.h
    ${BASE}/utrie2.cpp
    ${BASE}/utrie2.h
    ${BASE}/utrie2_builder.cpp
    ${BASE}/utrie2_impl.h
    ${BASE}/uts46.cpp
    ${BASE}/utypeinfo.h
    ${BASE}/utypes.cpp
    ${BASE}/uvector.cpp
    ${BASE}/uvector.h
    ${BASE}/uvectr32.cpp
    ${BASE}/uvectr32.h
    ${BASE}/uvectr64.cpp
    ${BASE}/uvectr64.h
    ${BASE}/wintz.cpp
    ${BASE}/wintz.h)

# Needs to be compiled *without* -DU_COMMON_IMPLEMENTATION=1.
add_library(
    icu_common_data STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/icudt${icu_ver_major}_dat.c)

if(SHARED STREQUAL SHARED)
  # Compile with -fPIC when building icu_common as a shared library.
  set_property(TARGET icu_common_data PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

add_library(icu_common ${SHARED} ${SOURCES})
target_link_libraries(icu_common icu_common_data)

add_library(
    icu_common_stubdata ${SHARED}
    ${BASE}/../stubdata/stubdata.cpp
    ${SOURCES})

target_compile_definitions(icu_common PUBLIC ${DEFINES})
target_compile_definitions(icu_common_stubdata PUBLIC ${DEFINES})

target_compile_definitions(icu_common PRIVATE -DU_COMMON_IMPLEMENTATION=1)
target_compile_definitions(
    icu_common_stubdata PRIVATE -DU_COMMON_IMPLEMENTATION=1)

target_include_directories(icu_common PUBLIC ${BASE})
target_include_directories(icu_common_stubdata PUBLIC ${BASE})

if(win32)
  target_link_libraries(advapi32 user32)
endif()

set(icu_data_out
    ${CMAKE_CURRENT_BINARY_DIR}/icudt${icu_ver_major}${icu_endianness}.dat)

# Note that both icutrim and icupkg byte-swap the data if necessary.
if(icu_small)
  add_custom_command(
      COMMAND
          ${PYTHON_EXECUTABLE}
          ${CMAKE_SOURCE_DIR}/tools/icu/icutrim.py
          -v
          -P ${CMAKE_CURRENT_BINARY_DIR}/../tools
          -D ${icu_data_in}
          --delete-tmp
          -T ${CMAKE_CURRENT_BINARY_DIR}/icutmp
          -F ${CMAKE_SOURCE_DIR}/tools/icu/icu_small.json
          -O ${icu_data_out}
          -L ${icu_locales}
      DEPENDS
          genrb iculslocs icupkg  # icutrim.py calls out to these.
          ${CMAKE_SOURCE_DIR}/tools/icu/icu_small.json
          ${CMAKE_SOURCE_DIR}/tools/icu/icutrim.py
          ${icu_data_in}
      OUTPUT
          ${icu_data_out})
  if(icu_endianness)
    # Silence unused variable warning.
  endif()
else()
  add_custom_command(
      COMMAND
          icupkg -t${icu_endianness} ${icu_data_in} ${icu_data_out}
      DEPENDS
          icupkg ${icu_data_in}
      OUTPUT
          ${icu_data_out})
endif()

# Note that the filename determines the name of the symbol; i.e.,
# icudt42_dat.c is mapped by genccode to `char icudt42_dat[]`.
add_custom_command(
    COMMAND
        genccode
        -d ${CMAKE_CURRENT_BINARY_DIR}
        -f icudt${icu_ver_major}_dat
        ${icu_data_out}
    DEPENDS
        genccode
        ${icu_data_out}
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/icudt${icu_ver_major}_dat.c)
