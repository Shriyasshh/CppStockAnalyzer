cmake_minimum_required(VERSION 3.16)

project(StockAnalyser VERSION 1.0)

# -----------------------------
# C++ Standard
# -----------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Target Windows 10
add_definitions(-D_WIN32_WINNT=0x0A00)

# -----------------------------
# Include Directories
# -----------------------------

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/cpp-dotenv/include
)

# -----------------------------
# cpp-dotenv Library
# -----------------------------

add_subdirectory(cpp-dotenv)

# -----------------------------
# Source Files
# -----------------------------

set(SOURCES
    src/main.cpp
)

add_executable(StockAnalyser ${SOURCES})

# Add CURL include directories to target
target_include_directories(StockAnalyser PRIVATE ${CURL_INCLUDE_DIRS})

# -----------------------------
# OpenSSL Configuration
# (MSYS2 auto-detect usually works,
# but this keeps it explicit)
# -----------------------------

set(OPENSSL_ROOT_DIR "C:/msys64/mingw64")

find_package(OpenSSL REQUIRED)

# -----------------------------
# libcurl Configuration
# -----------------------------

find_package(CURL REQUIRED)

# -----------------------------
# Link Libraries
# -----------------------------

target_link_libraries(StockAnalyser
    PRIVATE

    cpp_dotenv

    CURL::libcurl

    OpenSSL::SSL
    OpenSSL::Crypto

    ws2_32
    crypt32
    jsoncpp
)