cmake_minimum_required(VERSION 3.15) 
project(StockMarketAnalysis CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# MSYS2's CMake will find this automatically! No paths needed.
find_package(Qt6 COMPONENTS Widgets Charts REQUIRED)

include(FetchContent)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git 
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
  dotenv
  GIT_REPOSITORY https://github.com/laserpants/dotenv-cpp.git
  GIT_TAG master
)
FetchContent_MakeAvailable(dotenv)

FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG 1.10.5
)
FetchContent_MakeAvailable(cpr)

configure_file(
  ${CMAKE_SOURCE_DIR}/.env
  ${CMAKE_CURRENT_BINARY_DIR}/.env
  COPYONLY
)

# WIN32 still hides the black console window
add_executable(main WIN32 ./src/main.cpp)

target_include_directories(main PRIVATE ${dotenv_SOURCE_DIR}/include)
target_include_directories(main PRIVATE ${dotenv_SOURCE_DIR}/include/laserpants/dotenv)

target_link_libraries(main PRIVATE 
    cpr::cpr 
    nlohmann_json::nlohmann_json 
    Qt6::Widgets
    Qt6::Charts
)

add_executable(test_json ./tests/test_json.cpp)
target_link_libraries(test_json PRIVATE nlohmann_json::nlohmann_json)
