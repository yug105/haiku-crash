cmake_minimum_required(VERSION 3.10)
project(haiku_tls_crash CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The shared library carrying the thread_local with a non-trivial destructor.
add_library(tlslib SHARED tlslib.cpp)

# The driver: dlopen -> touch -> dlclose -> exit.
add_executable(main main.cpp)
target_link_libraries(main ${CMAKE_DL_LIBS})

# So dlopen("./libtlslib.so") resolves when running from the build dir, and
# $ORIGIN keeps it working regardless of cwd.
set_target_properties(main PROPERTIES
    BUILD_RPATH "$ORIGIN"
    INSTALL_RPATH "$ORIGIN")
