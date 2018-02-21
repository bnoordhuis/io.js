include(../common.cmake)
project(uv)

set(BASE ${CMAKE_SOURCE_DIR}/deps/uv)

set(SOURCES
    ${BASE}/include/tree.h
    ${BASE}/include/uv-errno.h
    ${BASE}/include/uv-threadpool.h
    ${BASE}/include/uv-version.h
    ${BASE}/include/uv.h
    ${BASE}/src/fs-poll.c
    ${BASE}/src/heap-inl.h
    ${BASE}/src/inet.c
    ${BASE}/src/queue.h
    ${BASE}/src/threadpool.c
    ${BASE}/src/uv-common.c
    ${BASE}/src/uv-common.h
    ${BASE}/src/version.c)

if(win32)
  set(DEFINES ${DEFINES} _GNU_SOURCE _WIN32_WINNT=0x0600)
  set(LIBS ${LIBS} advapi32 iphlpapi psapi shell32 user32 userenv ws2_32)
  set(SOURCES ${SOURCES}
      ${BASE}/include/uv-win.h
      ${BASE}/src/win/async.c
      ${BASE}/src/win/atomicops-inl.h
      ${BASE}/src/win/core.c
      ${BASE}/src/win/detect-wakeup.c
      ${BASE}/src/win/dl.c
      ${BASE}/src/win/error.c
      ${BASE}/src/win/fs.c
      ${BASE}/src/win/fs-event.c
      ${BASE}/src/win/getaddrinfo.c
      ${BASE}/src/win/getnameinfo.c
      ${BASE}/src/win/handle.c
      ${BASE}/src/win/handle-inl.h
      ${BASE}/src/win/internal.h
      ${BASE}/src/win/loop-watcher.c
      ${BASE}/src/win/pipe.c
      ${BASE}/src/win/thread.c
      ${BASE}/src/win/poll.c
      ${BASE}/src/win/process.c
      ${BASE}/src/win/process-stdio.c
      ${BASE}/src/win/req.c
      ${BASE}/src/win/req-inl.h
      ${BASE}/src/win/signal.c
      ${BASE}/src/win/snprintf.c
      ${BASE}/src/win/stream.c
      ${BASE}/src/win/stream-inl.h
      ${BASE}/src/win/tcp.c
      ${BASE}/src/win/tty.c
      ${BASE}/src/win/timer.c
      ${BASE}/src/win/udp.c
      ${BASE}/src/win/util.c
      ${BASE}/src/win/winapi.c
      ${BASE}/src/win/winapi.h
      ${BASE}/src/win/winsock.c
      ${BASE}/src/win/winsock.h)
else()
  set(DEFINES ${DEFINES} _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE)
  set(LIBS ${LIBS} pthread)
  set(SOURCES
      ${SOURCES}
      ${BASE}/include/uv-aix.h
      ${BASE}/include/uv-bsd.h
      ${BASE}/include/uv-darwin.h
      ${BASE}/include/uv-linux.h
      ${BASE}/include/uv-sunos.h
      ${BASE}/include/uv-unix.h
      ${BASE}/src/unix/async.c
      ${BASE}/src/unix/atomic-ops.h
      ${BASE}/src/unix/core.c
      ${BASE}/src/unix/dl.c
      ${BASE}/src/unix/fs.c
      ${BASE}/src/unix/getaddrinfo.c
      ${BASE}/src/unix/getnameinfo.c
      ${BASE}/src/unix/internal.h
      ${BASE}/src/unix/loop-watcher.c
      ${BASE}/src/unix/loop.c
      ${BASE}/src/unix/pipe.c
      ${BASE}/src/unix/poll.c
      ${BASE}/src/unix/process.c
      ${BASE}/src/unix/signal.c
      ${BASE}/src/unix/spinlock.h
      ${BASE}/src/unix/stream.c
      ${BASE}/src/unix/tcp.c
      ${BASE}/src/unix/thread.c
      ${BASE}/src/unix/timer.c
      ${BASE}/src/unix/tty.c
      ${BASE}/src/unix/udp.c)
endif()

if(aix)
  set(DEFINES ${DEFINES} _ALL_SOURCE)
  set(DEFINES ${DEFINES} _LINUX_SOURCE_COMPAT)
  set(DEFINES ${DEFINES} _THREAD_SAFE)
  set(DEFINES ${DEFINES} _XOPEN_SOURCE=500)
  set(LIBS ${LIBS} perfstat)
  set(SOURCES ${SOURCES} ${BASE}/src/unix/aix.c)
endif()

if(android)
  set(LIBS ${LIBS} dl)
  set(SOURCES ${SOURCES}
      ${BASE}/src/unix/android-ifaddrs.c
      ${BASE}/src/unix/linux-core.c
      ${BASE}/src/unix/linux-inotify.c
      ${BASE}/src/unix/linux-syscalls.c
      ${BASE}/src/unix/linux-syscalls.h
      ${BASE}/src/unix/procfs-exepath.c
      ${BASE}/src/unix/pthread-barrier.c
      ${BASE}/src/unix/pthread-fixes.c
      ${BASE}/src/unix/sysinfo-loadavg.c
      ${BASE}/src/unix/sysinfo-memory.c)
endif()

if(android OR ios OR linux OR mac OR os390)
  set(SOURCES ${SOURCES} ${BASE}/src/unix/proctitle.c)
endif()

if(dragonflybsd OR freebsd)
  set(SOURCES ${SOURCES} ${BASE}/src/unix/freebsd.c)
endif()

if(dragonflybsd OR freebsd OR netbsd OR openbsd)
  set(SOURCES ${SOURCES} ${BASE}/src/unix/posix-hrtime.c)
  set(LIBS ${LIBS} kvm)
endif()

if(dragonflybsd OR freebsd OR ios OR mac OR netbsd OR openbsd)
  set(SOURCES ${SOURCES}
      ${BASE}/src/unix/bsd-ifaddrs.c
      ${BASE}/src/unix/kqueue.c)
endif()

if(ios OR mac)
  set(DEFINES ${DEFINES} _DARWIN_UNLIMITED_SELECT=1 _DARWIN_USE_64_BIT_INODE=1)
  set(SOURCES ${SOURCES}
      ${BASE}/src/unix/darwin-proctitle.c
      ${BASE}/src/unix/darwin.c
      ${BASE}/src/unix/fsevents.c
      ${BASE}/src/unix/pthread-barrier.c)
endif()

if(linux)
  set(DEFINES ${DEFINES} _GNU_SOURCE _POSIX_C_SOURCE=200112)
  set(LIBS ${LIBS} dl rt)
  set(SOURCES ${SOURCES}
      ${BASE}/src/unix/linux-core.c
      ${BASE}/src/unix/linux-inotify.c
      ${BASE}/src/unix/linux-syscalls.c
      ${BASE}/src/unix/linux-syscalls.h
      ${BASE}/src/unix/procfs-exepath.c
      ${BASE}/src/unix/sysinfo-loadavg.c
      ${BASE}/src/unix/sysinfo-memory.c)
endif()

if(netbsd)
  set(SOURCES ${SOURCES} ${BASE}/src/unix/netbsd.c)
endif()

if(openbsd)
  set(SOURCES ${SOURCES} ${BASE}/src/unix/openbsd.c)
endif()

if(os390)
  set(DEFINES ${DEFINES} PATH_MAX=255)
  set(DEFINES ${DEFINES} _AE_BIMODAL)
  set(DEFINES ${DEFINES} _ALL_SOURCE)
  set(DEFINES ${DEFINES} _LARGE_TIME_API)
  set(DEFINES ${DEFINES} _OPEN_MSGQ_EXT)
  set(DEFINES ${DEFINES} _OPEN_SYS_FILE_EXT)
  set(DEFINES ${DEFINES} _OPEN_SYS_IF_EXT)
  set(DEFINES ${DEFINES} _OPEN_SYS_SOCK_IPV6)
  set(DEFINES ${DEFINES} _UNIX03_SOURCE)
  set(DEFINES ${DEFINES} _UNIX03_THREADS)
  set(DEFINES ${DEFINES} _UNIX03_WITHDRAWN)
  set(DEFINES ${DEFINES} _XOPEN_SOURCE_EXTENDED)
  set(SOURCES ${SOURCES}
      ${BASE}/src/unix/pthread-fixes.c
      ${BASE}/src/unix/pthread-barrier.c
      ${BASE}/src/unix/os390.c
      ${BASE}/src/unix/os390-syscalls.c)
endif()

if(sunos)
  set(DEFINES ${DEFINES} __EXTENSIONS__ _XOPEN_SOURCE=500)
  set(LIBS ${LIBS} kstat nsl sendfile socket)
  set(SOURCES ${SOURCES}
      ${BASE}/src/unix/no-proctitle.c
      ${BASE}/src/unix/sunos.c)
endif()

add_library(uv ${SHARED} ${SOURCES})
target_compile_definitions(uv PRIVATE ${DEFINES})
target_include_directories(uv PRIVATE ${BASE}/src)
target_include_directories(uv PUBLIC ${BASE}/include)
target_link_libraries(uv ${LIBS})

if(NOT win32)
  target_compile_definitions(uv PUBLIC _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE)
endif()

if(linux)
  target_compile_definitions(uv PUBLIC _POSIX_C_SOURCE=200112)
endif()

if(mac)
  target_compile_definitions(uv PUBLIC _DARWIN_USE_64_BIT_INODE=1)
endif()
