include(../common.cmake)
project(nghttp2)

set(BASE ${CMAKE_SOURCE_DIR}/deps/nghttp2)

set(SOURCES
    ${BASE}/lib/nghttp2_buf.c
    ${BASE}/lib/nghttp2_callbacks.c
    ${BASE}/lib/nghttp2_debug.c
    ${BASE}/lib/nghttp2_frame.c
    ${BASE}/lib/nghttp2_hd.c
    ${BASE}/lib/nghttp2_hd_huffman.c
    ${BASE}/lib/nghttp2_hd_huffman_data.c
    ${BASE}/lib/nghttp2_helper.c
    ${BASE}/lib/nghttp2_http.c
    ${BASE}/lib/nghttp2_map.c
    ${BASE}/lib/nghttp2_mem.c
    ${BASE}/lib/nghttp2_npn.c
    ${BASE}/lib/nghttp2_option.c
    ${BASE}/lib/nghttp2_outbound_item.c
    ${BASE}/lib/nghttp2_pq.c
    ${BASE}/lib/nghttp2_priority_spec.c
    ${BASE}/lib/nghttp2_queue.c
    ${BASE}/lib/nghttp2_rcbuf.c
    ${BASE}/lib/nghttp2_session.c
    ${BASE}/lib/nghttp2_stream.c
    ${BASE}/lib/nghttp2_submit.c
    ${BASE}/lib/nghttp2_version.c)

if(debug_nghttp2)
  add_definitions(-DDEBUGBUILD=1)
endif()

if(win32)
  add_definitions(-DWIN32 -D_WINDOWS -DHAVE_CONFIG_H -DNGHTTP2_STATICLIB)
endif()

include_directories(${BASE}/lib/includes)

add_library(nghttp2 ${SHARED} ${SOURCES})
target_include_directories(nghttp2 PUBLIC ${BASE}/lib/includes)
