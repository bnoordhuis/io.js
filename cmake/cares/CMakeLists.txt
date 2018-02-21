include(../common.cmake)
project(cares)

set(BASE ${CMAKE_SOURCE_DIR}/deps/cares)

set(DEFINES CARES_STATICLIB)

set(INCLUDES ${BASE}/include ${BASE}/src)

set(SOURCES
    ${BASE}/include/ares.h
    ${BASE}/include/ares_rules.h
    ${BASE}/include/ares_version.h
    ${BASE}/include/nameser.h
    ${BASE}/src/ares__close_sockets.c
    ${BASE}/src/ares__get_hostent.c
    ${BASE}/src/ares__read_line.c
    ${BASE}/src/ares__timeval.c
    ${BASE}/src/ares_cancel.c
    ${BASE}/src/ares_create_query.c
    ${BASE}/src/ares_data.c
    ${BASE}/src/ares_data.h
    ${BASE}/src/ares_destroy.c
    ${BASE}/src/ares_dns.h
    ${BASE}/src/ares_expand_name.c
    ${BASE}/src/ares_expand_string.c
    ${BASE}/src/ares_fds.c
    ${BASE}/src/ares_free_hostent.c
    ${BASE}/src/ares_free_string.c
    ${BASE}/src/ares_getenv.h
    ${BASE}/src/ares_gethostbyaddr.c
    ${BASE}/src/ares_gethostbyname.c
    ${BASE}/src/ares_getnameinfo.c
    ${BASE}/src/ares_getopt.c
    ${BASE}/src/ares_getopt.h
    ${BASE}/src/ares_getsock.c
    ${BASE}/src/ares_inet_net_pton.h
    ${BASE}/src/ares_init.c
    ${BASE}/src/ares_ipv6.h
    ${BASE}/src/ares_library_init.c
    ${BASE}/src/ares_library_init.h
    ${BASE}/src/ares_llist.c
    ${BASE}/src/ares_llist.h
    ${BASE}/src/ares_mkquery.c
    ${BASE}/src/ares_nowarn.c
    ${BASE}/src/ares_nowarn.h
    ${BASE}/src/ares_options.c
    ${BASE}/src/ares_parse_a_reply.c
    ${BASE}/src/ares_parse_aaaa_reply.c
    ${BASE}/src/ares_parse_mx_reply.c
    ${BASE}/src/ares_parse_naptr_reply.c
    ${BASE}/src/ares_parse_ns_reply.c
    ${BASE}/src/ares_parse_ptr_reply.c
    ${BASE}/src/ares_parse_soa_reply.c
    ${BASE}/src/ares_parse_srv_reply.c
    ${BASE}/src/ares_parse_txt_reply.c
    ${BASE}/src/ares_platform.h
    ${BASE}/src/ares_private.h
    ${BASE}/src/ares_process.c
    ${BASE}/src/ares_query.c
    ${BASE}/src/ares_search.c
    ${BASE}/src/ares_send.c
    ${BASE}/src/ares_setup.h
    ${BASE}/src/ares_strcasecmp.c
    ${BASE}/src/ares_strcasecmp.h
    ${BASE}/src/ares_strdup.c
    ${BASE}/src/ares_strdup.h
    ${BASE}/src/ares_strerror.c
    ${BASE}/src/ares_timeout.c
    ${BASE}/src/ares_version.c
    ${BASE}/src/ares_writev.c
    ${BASE}/src/ares_writev.h
    ${BASE}/src/bitncmp.c
    ${BASE}/src/bitncmp.h
    ${BASE}/src/inet_net_pton.c
    ${BASE}/src/inet_ntop.c
    ${BASE}/src/setup_once.h)

if(win32)
  set(LIBS ${LIBS} iphlpapi ws2_32)
  set(SOURCES ${SOURCES}
      ${BASE}/src/src/ares_getenv.c
      ${BASE}/src/src/ares_iphlpapi.h
      ${BASE}/src/src/ares_platform.c
      ${BASE}/src/src/config-win32.h
      ${BASE}/src/src/windows_port.c)
else()
  set(DEFINES ${DEFINES} HAVE_CONFIG_H)
endif()

if(android)
  set(INCLUDES ${INCLUDES} ${BASE}/config/android)
  set(SOURCES ${SOURCES} ${BASE}/config/android/ares_config.h)
endif()

if(dragonflybsd OR freebsd)
  set(INCLUDES ${INCLUDES} ${BASE}/config/freebsd)
  set(SOURCES ${SOURCES} ${BASE}/config/freebsd/ares_config.h)
endif()

if(mac)
  set(INCLUDES ${INCLUDES} ${BASE}/config/darwin)
  set(SOURCES ${SOURCES} ${BASE}/config/darwin/ares_config.h)
endif()

if(openbsd)
  set(INCLUDES ${INCLUDES} ${BASE}/config/openbsd)
  set(SOURCES ${SOURCES} ${BASE}/config/openbsd/ares_config.h)
endif()

if(linux)
  set(INCLUDES ${INCLUDES} ${BASE}/config/linux)
  set(SOURCES ${SOURCES} ${BASE}/config/linux/ares_config.h)
endif()

if(sunos)
  set(INCLUDES ${INCLUDES} ${BASE}/config/sunos)
  set(LIBS ${LIBS} nsl socket)
  set(SOURCES ${SOURCES} ${BASE}/config/sunos/ares_config.h)
endif()

add_library(cares ${SHARED} ${SOURCES})
target_compile_definitions(cares PRIVATE ${DEFINES})
target_include_directories(cares PRIVATE ${INCLUDES})
target_include_directories(cares PUBLIC ${BASE}/include)
target_link_libraries(cares ${LIBS})
