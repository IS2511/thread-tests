cmake_minimum_required(VERSION 3.17)
project(thread-tests)

set(CMAKE_CXX_STANDARD 17)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(thread-tests
        main.cpp)

target_link_libraries(thread-tests PRIVATE Threads::Threads)