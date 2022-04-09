cmake_minimum_required(VERSION 3.0.0)
project(jserdes_test VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(jserdes_test jserdes_test.cc)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
