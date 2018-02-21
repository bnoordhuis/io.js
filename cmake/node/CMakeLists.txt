include(../common.cmake)
project(node)

set(BASE ${CMAKE_SOURCE_DIR})

set(LIBS cares http_parser icu_common icu_i18n icu_common_data nghttp2 uv v8)

# TODO(bnoordhuis) Make ICU and icu-small support configurable.
add_definitions(
    -DNODE_ARCH="${NODE_ARCH}"
    -DNODE_HAVE_I18N_SUPPORT=1
    # -DNODE_HAVE_SMALL_ICU=1
    -DNODE_OPENSSL_SYSTEM_CERT_PATH="${openssl_system_ca_path}"
    -DNODE_PLATFORM="${NODE_PLATFORM}"
    -DNODE_USE_V8_PLATFORM=$<BOOL:node_use_v8_platform>
    -DNODE_WANT_INTERNALS=1
    -DV8_DEPRECATION_WARNINGS=1)

set(JAVASCRIPT_SOURCES
    ${BASE}/config.gypi
    ${BASE}/deps/acorn/dist/acorn.js
    ${BASE}/deps/acorn/dist/walk.js
    ${BASE}/deps/node-inspect/lib/_inspect.js
    ${BASE}/deps/node-inspect/lib/internal/inspect_client.js
    ${BASE}/deps/node-inspect/lib/internal/inspect_repl.js
    ${BASE}/deps/v8/tools/SourceMap.js
    ${BASE}/deps/v8/tools/codemap.js
    ${BASE}/deps/v8/tools/consarray.js
    ${BASE}/deps/v8/tools/csvparser.js
    ${BASE}/deps/v8/tools/logreader.js
    ${BASE}/deps/v8/tools/profile.js
    ${BASE}/deps/v8/tools/profile_view.js
    ${BASE}/deps/v8/tools/splaytree.js
    ${BASE}/deps/v8/tools/tickprocessor-driver.js
    ${BASE}/deps/v8/tools/tickprocessor.js
    ${BASE}/lib/_http_agent.js
    ${BASE}/lib/_http_client.js
    ${BASE}/lib/_http_common.js
    ${BASE}/lib/_http_incoming.js
    ${BASE}/lib/_http_outgoing.js
    ${BASE}/lib/_http_server.js
    ${BASE}/lib/_stream_duplex.js
    ${BASE}/lib/_stream_passthrough.js
    ${BASE}/lib/_stream_readable.js
    ${BASE}/lib/_stream_transform.js
    ${BASE}/lib/_stream_wrap.js
    ${BASE}/lib/_stream_writable.js
    ${BASE}/lib/_tls_common.js
    ${BASE}/lib/_tls_legacy.js
    ${BASE}/lib/_tls_wrap.js
    ${BASE}/lib/assert.js
    ${BASE}/lib/async_hooks.js
    ${BASE}/lib/buffer.js
    ${BASE}/lib/child_process.js
    ${BASE}/lib/cluster.js
    ${BASE}/lib/console.js
    ${BASE}/lib/constants.js
    ${BASE}/lib/crypto.js
    ${BASE}/lib/dgram.js
    ${BASE}/lib/dns.js
    ${BASE}/lib/domain.js
    ${BASE}/lib/events.js
    ${BASE}/lib/fs.js
    ${BASE}/lib/http.js
    ${BASE}/lib/http2.js
    ${BASE}/lib/https.js
    ${BASE}/lib/inspector.js
    ${BASE}/lib/internal/async_hooks.js
    ${BASE}/lib/internal/bootstrap_node.js
    ${BASE}/lib/internal/buffer.js
    ${BASE}/lib/internal/child_process.js
    ${BASE}/lib/internal/cluster/child.js
    ${BASE}/lib/internal/cluster/master.js
    ${BASE}/lib/internal/cluster/round_robin_handle.js
    ${BASE}/lib/internal/cluster/shared_handle.js
    ${BASE}/lib/internal/cluster/utils.js
    ${BASE}/lib/internal/cluster/worker.js
    ${BASE}/lib/internal/crypto/certificate.js
    ${BASE}/lib/internal/crypto/cipher.js
    ${BASE}/lib/internal/crypto/diffiehellman.js
    ${BASE}/lib/internal/crypto/hash.js
    ${BASE}/lib/internal/crypto/pbkdf2.js
    ${BASE}/lib/internal/crypto/random.js
    ${BASE}/lib/internal/crypto/sig.js
    ${BASE}/lib/internal/crypto/util.js
    ${BASE}/lib/internal/encoding.js
    ${BASE}/lib/internal/errors.js
    ${BASE}/lib/internal/freelist.js
    ${BASE}/lib/internal/fs.js
    ${BASE}/lib/internal/http.js
    ${BASE}/lib/internal/http2/compat.js
    ${BASE}/lib/internal/http2/core.js
    ${BASE}/lib/internal/http2/util.js
    ${BASE}/lib/internal/inspector_async_hook.js
    ${BASE}/lib/internal/linkedlist.js
    ${BASE}/lib/internal/loader/Loader.js
    ${BASE}/lib/internal/loader/ModuleJob.js
    ${BASE}/lib/internal/loader/ModuleMap.js
    ${BASE}/lib/internal/loader/ModuleRequest.js
    ${BASE}/lib/internal/loader/ModuleWrap.js
    ${BASE}/lib/internal/loader/search.js
    ${BASE}/lib/internal/module.js
    ${BASE}/lib/internal/net.js
    ${BASE}/lib/internal/os.js
    ${BASE}/lib/internal/process.js
    ${BASE}/lib/internal/process/next_tick.js
    ${BASE}/lib/internal/process/promises.js
    ${BASE}/lib/internal/process/stdio.js
    ${BASE}/lib/internal/process/warning.js
    ${BASE}/lib/internal/process/write-coverage.js
    ${BASE}/lib/internal/querystring.js
    ${BASE}/lib/internal/readline.js
    ${BASE}/lib/internal/repl.js
    ${BASE}/lib/internal/repl/await.js
    ${BASE}/lib/internal/safe_globals.js
    ${BASE}/lib/internal/socket_list.js
    ${BASE}/lib/internal/streams/BufferList.js
    ${BASE}/lib/internal/streams/destroy.js
    ${BASE}/lib/internal/streams/lazy_transform.js
    ${BASE}/lib/internal/streams/legacy.js
    ${BASE}/lib/internal/test/unicode.js
    ${BASE}/lib/internal/timers.js
    ${BASE}/lib/internal/tls.js
    ${BASE}/lib/internal/trace_events_async_hooks.js
    ${BASE}/lib/internal/url.js
    ${BASE}/lib/internal/util.js
    ${BASE}/lib/internal/util/comparisons.js
    ${BASE}/lib/internal/util/inspector.js
    ${BASE}/lib/internal/util/types.js
    ${BASE}/lib/internal/v8.js
    ${BASE}/lib/internal/v8_prof_polyfill.js
    ${BASE}/lib/internal/v8_prof_processor.js
    ${BASE}/lib/internal/wrap_js_stream.js
    ${BASE}/lib/module.js
    ${BASE}/lib/net.js
    ${BASE}/lib/os.js
    ${BASE}/lib/path.js
    ${BASE}/lib/perf_hooks.js
    ${BASE}/lib/process.js
    ${BASE}/lib/punycode.js
    ${BASE}/lib/querystring.js
    ${BASE}/lib/readline.js
    ${BASE}/lib/repl.js
    ${BASE}/lib/stream.js
    ${BASE}/lib/string_decoder.js
    ${BASE}/lib/sys.js
    ${BASE}/lib/timers.js
    ${BASE}/lib/tls.js
    ${BASE}/lib/tty.js
    ${BASE}/lib/url.js
    ${BASE}/lib/util.js
    ${BASE}/lib/v8.js
    ${BASE}/lib/vm.js
    ${BASE}/lib/zlib.js)

set(SOURCES
    ${BASE}/src/CNNICHashWhitelist.inc
    ${BASE}/src/StartComAndWoSignData.inc
    ${BASE}/src/async_wrap-inl.h
    ${BASE}/src/async_wrap.cc
    ${BASE}/src/async_wrap.h
    ${BASE}/src/base_object-inl.h
    ${BASE}/src/base_object.h
    ${BASE}/src/cares_wrap.cc
    ${BASE}/src/connect_wrap.cc
    ${BASE}/src/connect_wrap.h
    ${BASE}/src/connection_wrap.cc
    ${BASE}/src/connection_wrap.h
    ${BASE}/src/env-inl.h
    ${BASE}/src/env.cc
    ${BASE}/src/env.h
    ${BASE}/src/freelist.h
    ${BASE}/src/fs_event_wrap.cc
    ${BASE}/src/handle_wrap.cc
    ${BASE}/src/handle_wrap.h
    ${BASE}/src/js_stream.cc
    ${BASE}/src/js_stream.h
    ${BASE}/src/module_wrap.cc
    ${BASE}/src/module_wrap.h
    ${BASE}/src/node.cc
    ${BASE}/src/node.h
    ${BASE}/src/node_api.cc
    ${BASE}/src/node_api.h
    ${BASE}/src/node_api_types.h
    ${BASE}/src/node_buffer.cc
    ${BASE}/src/node_buffer.h
    ${BASE}/src/node_config.cc
    ${BASE}/src/node_constants.cc
    ${BASE}/src/node_constants.h
    ${BASE}/src/node_contextify.cc
    ${BASE}/src/node_debug_options.cc
    ${BASE}/src/node_debug_options.h
    ${BASE}/src/node_file.cc
    ${BASE}/src/node_http2.cc
    ${BASE}/src/node_http2.h
    ${BASE}/src/node_http2_state.h
    ${BASE}/src/node_http_parser.cc
    ${BASE}/src/node_i18n.cc
    ${BASE}/src/node_i18n.h
    ${BASE}/src/node_internals.h
    ${BASE}/src/node_javascript.h
    ${BASE}/src/node_mutex.h
    ${BASE}/src/node_os.cc
    ${BASE}/src/node_perf.cc
    ${BASE}/src/node_perf_common.h
    ${BASE}/src/node_platform.cc
    ${BASE}/src/node_platform.h
    ${BASE}/src/node_revert.h
    ${BASE}/src/node_root_certs.h
    ${BASE}/src/node_serdes.cc
    ${BASE}/src/node_stat_watcher.cc
    ${BASE}/src/node_trace_events.cc
    ${BASE}/src/node_url.cc
    ${BASE}/src/node_util.cc
    ${BASE}/src/node_v8.cc
    ${BASE}/src/node_version.h
    ${BASE}/src/node_watchdog.cc
    ${BASE}/src/node_watchdog.h
    ${BASE}/src/node_wrap.h
    ${BASE}/src/node_zlib.cc
    ${BASE}/src/pipe_wrap.cc
    ${BASE}/src/pipe_wrap.h
    ${BASE}/src/process_wrap.cc
    ${BASE}/src/req_wrap-inl.h
    ${BASE}/src/req_wrap.h
    ${BASE}/src/signal_wrap.cc
    ${BASE}/src/spawn_sync.cc
    ${BASE}/src/stream_base-inl.h
    ${BASE}/src/stream_base.cc
    ${BASE}/src/stream_base.h
    ${BASE}/src/stream_wrap.cc
    ${BASE}/src/stream_wrap.h
    ${BASE}/src/string_bytes.cc
    ${BASE}/src/string_bytes.h
    ${BASE}/src/string_search.cc
    ${BASE}/src/tcp_wrap.cc
    ${BASE}/src/tcp_wrap.h
    ${BASE}/src/timer_wrap.cc
    ${BASE}/src/tracing/agent.cc
    ${BASE}/src/tracing/agent.h
    ${BASE}/src/tracing/node_trace_buffer.cc
    ${BASE}/src/tracing/node_trace_buffer.h
    ${BASE}/src/tracing/node_trace_writer.cc
    ${BASE}/src/tracing/node_trace_writer.h
    ${BASE}/src/tracing/trace_event.cc
    ${BASE}/src/tracing/trace_event.h
    ${BASE}/src/tracing/trace_event_common.h
    ${BASE}/src/tty_wrap.cc
    ${BASE}/src/tty_wrap.h
    ${BASE}/src/udp_wrap.cc
    ${BASE}/src/udp_wrap.h
    ${BASE}/src/util-inl.h
    ${BASE}/src/util.cc
    ${BASE}/src/util.h
    ${BASE}/src/uv.cc
    ${CMAKE_CURRENT_BINARY_DIR}/node_javascript.cc)

set(CCTEST_SOURCES
    ${BASE}/test/cctest/node_test_fixture.cc
    ${BASE}/test/cctest/test_aliased_buffer.cc
    ${BASE}/test/cctest/test_base64.cc
    ${BASE}/test/cctest/test_environment.cc
    ${BASE}/test/cctest/test_url.cc
    ${BASE}/test/cctest/test_util.cc)

if(debug_http2)  # Poorly named flag.
  add_definitions(-DNODE_DEBUG_HTTP2=1)
endif()

if(node_enable_v8_vtunejit)
  add_definitions(-DNODE_ENABLE_VTUNE_PROFILING)
  set(LIBS ${LIBS} v8_vtune)
endif()

if(node_no_browser_globals)
  add_definitions(-DNODE_NO_BROWSER_GLOBALS)
endif()

if(node_release_urlbase)
  add_definitions(-DNODE_RELEASE_URLBASE="${node_release_urlbase}")
endif()

if(node_shared)
  add_definitions(-DNODE_SHARED_MODE)
endif()

if(node_tag)
  add_definitions(-DNODE_TAG="${node_tag}")
endif()

if(node_use_openssl)
  add_definitions(-DHAVE_OPENSSL=1)
  set(SOURCES ${SOURCES}
      ${BASE}/src/node_crypto.cc
      ${BASE}/src/node_crypto.h
      ${BASE}/src/node_crypto_bio.cc
      ${BASE}/src/node_crypto_bio.h
      ${BASE}/src/node_crypto_clienthello.cc
      ${BASE}/src/node_crypto_clienthello.h
      ${BASE}/src/tls_wrap.cc
      ${BASE}/src/tls_wrap.h)
  set(LIBS ${LIBS} crypto ssl)
else()
  add_definitions(-DHAVE_OPENSSL=0)
endif()

if(node_use_openssl AND v8_enable_inspector)
  add_definitions(-DHAVE_INSPECTOR=1)

  set(SOURCES ${SOURCES}
      ${BASE}/src/inspector_agent.cc
      ${BASE}/src/inspector_agent.h
      ${BASE}/src/inspector_js_api.cc
      ${BASE}/src/inspector_io.cc
      ${BASE}/src/inspector_io.h
      ${BASE}/src/inspector_socket.cc
      ${BASE}/src/inspector_socket.h
      ${BASE}/src/inspector_socket_server.cc
      ${BASE}/src/inspector_socket_server.h
      ${CMAKE_CURRENT_BINARY_DIR}/v8_inspector_protocol_json.h)

  set(CCTEST_SOURCES ${CCTEST_SOURCES}
      ${BASE}/test/cctest/test_inspector_socket.cc
      ${BASE}/test/cctest/test_inspector_socket_server.cc)
else()
  add_definitions(-DHAVE_INSPECTOR=0)
endif()

if(node_use_openssl_ca_store)
  add_definitions(-DNODE_OPENSSL_CERT_STORE)
endif()

if(node_use_v8_platform)
  add_definitions(-DNODE_USE_V8_PLATFORM=1)
else()
  add_definitions(-DNODE_USE_V8_PLATFORM=0)
endif()

if(node_shared_zlib)
  set(LIBS ${LIBS} z)
else()
  set(LIBS ${LIBS} zlib)
endif()

if(node_v8_options)
  add_definitions(-DNODE_V8_OPTIONS="${node_v8_options}")
endif()

if(node_without_node_options)
  add_definitions(-DNODE_WITHOUT_NODE_OPTIONS)
endif()

if(win32)
  set(SOURCES ${SOURCES} ${BASE}/src/backtrace_win32.cc)
else()
  add_definitions(-D__POSIX__=1)
  set(SOURCES ${SOURCES} ${BASE}/src/backtrace_posix.cc)
endif()

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "-fno-exceptions -fno-rtti ${CMAKE_CXX_FLAGS}")
endif()

if(NOT node_use_dtrace AND NOT node_use_etw)
  set(JAVASCRIPT_SOURCES ${JAVASCRIPT_SOURCES} ${BASE}/src/notrace_macros.py)
endif()

if(NOT node_use_lttng)
  set(JAVASCRIPT_SOURCES ${JAVASCRIPT_SOURCES} ${BASE}/src/nolttng_macros.py)
endif()

if(NOT node_use_perfctr)
  set(JAVASCRIPT_SOURCES ${JAVASCRIPT_SOURCES} ${BASE}/src/noperfctr_macros.py)
endif()

add_custom_command(
    COMMAND
        ${PYTHON_EXECUTABLE}
        ${BASE}/tools/js2c.py
        ${BASE}
        ${CMAKE_CURRENT_BINARY_DIR}/node_javascript.cc
        ${JAVASCRIPT_SOURCES}
    DEPENDS
        ${BASE}/tools/js2c.py
        ${JAVASCRIPT_SOURCES}
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/node_javascript.cc)

add_custom_command(
    COMMAND
        ${PYTHON_EXECUTABLE}
        ${BASE}/tools/compress_json.py
        ${BASE}/deps/v8/src/inspector/js_protocol.json
        ${CMAKE_CURRENT_BINARY_DIR}/v8_inspector_protocol_json.h
    DEPENDS
        ${BASE}/tools/compress_json.py
        ${BASE}/deps/v8/src/inspector/js_protocol.json
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/v8_inspector_protocol_json.h)

include_directories(${BASE}/src ${CMAKE_CURRENT_BINARY_DIR})

add_executable(node ${SOURCES} ${BASE}/src/node_main.cc)
target_link_libraries(node ${LIBS})

add_executable(cctest ${SOURCES} ${CCTEST_SOURCES})
target_compile_definitions(cctest PRIVATE NODE_WANT_INTERNALS=1)
target_link_libraries(cctest gtest ${LIBS})
