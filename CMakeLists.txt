cmake_minimum_required(VERSION 3.18.0)
project(QR VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TestSources  tests/test_householder_reflector.cpp  tests/test_givens_rotator.cpp tests/test_hessenberg_reduction.cpp tests/tests.cpp tests/test_schur_decomposition.cpp tests/test_schur_decomposition_symmetric.cpp)

add_executable(exec main.cpp "${TestSources}")
