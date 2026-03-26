cmake_minimum_required(VERSION 3.5) 
project(StockMarketAnalysis CXX)
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
configure_file(
  ${CMAKE_SOURCE_DIR}/.env
  ${CMAKE_CURRENT_BINARY_DIR}/.env
  COPYONLY
)

# Main ---- 
add_executable(main ./src/main.cpp)
target_include_directories(
  main PRIVATE
  ${dotenv_SOURCE_DIR}/include
)
# Test ----
add_executable(test_json ./tests/test_json.cpp)
target_link_libraries(test_json PRIVATE nlohmann_json::nlohmann_json)
