include(../common.cmake)
project(gtest)
set(BASE ${CMAKE_SOURCE_DIR}/deps/gtest)
add_library(
    gtest ${SHARED}
    ${BASE}/include/gtest/gtest-death-test.h
    ${BASE}/include/gtest/gtest-message.h
    ${BASE}/include/gtest/gtest-param-test.h
    ${BASE}/include/gtest/gtest-printers.h
    ${BASE}/include/gtest/gtest-spi.h
    ${BASE}/include/gtest/gtest-test-part.h
    ${BASE}/include/gtest/gtest-typed-test.h
    ${BASE}/include/gtest/gtest.h
    ${BASE}/include/gtest/gtest_pred_impl.h
    ${BASE}/include/gtest/gtest_prod.h
    ${BASE}/include/gtest/internal/gtest-death-test-internal.h
    ${BASE}/include/gtest/internal/gtest-filepath.h
    ${BASE}/include/gtest/internal/gtest-internal.h
    ${BASE}/include/gtest/internal/gtest-linked_ptr.h
    ${BASE}/include/gtest/internal/gtest-param-util-generated.h
    ${BASE}/include/gtest/internal/gtest-param-util.h
    ${BASE}/include/gtest/internal/gtest-port.h
    ${BASE}/include/gtest/internal/gtest-string.h
    ${BASE}/include/gtest/internal/gtest-tuple.h
    ${BASE}/include/gtest/internal/gtest-type-util.h
    ${BASE}/src/gtest-death-test.cc
    ${BASE}/src/gtest-filepath.cc
    ${BASE}/src/gtest-internal-inl.h
    ${BASE}/src/gtest-port.cc
    ${BASE}/src/gtest-printers.cc
    ${BASE}/src/gtest-test-part.cc
    ${BASE}/src/gtest-typed-test.cc
    ${BASE}/src/gtest.cc
    ${BASE}/src/gtest_main.cc)
include_directories(${BASE} ${BASE}/include)
target_include_directories(gtest PUBLIC ${BASE}/include)
