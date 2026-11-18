cmake_minimum_required(VERSION 3.20)
project(mkwii_web_bootstrap LANGUAGES CXX)

if(NOT EMSCRIPTEN)
  message(FATAL_ERROR "This project is currently configured for Emscripten. Use emcmake cmake.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(mkwii-web src/main.cpp)
set_target_properties(mkwii-web PROPERTIES
  OUTPUT_NAME "mkwii-web"
  SUFFIX ".js"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/dist"
)

target_link_options(mkwii-web PRIVATE
  "-sMODULARIZE=1"
  "-sEXPORT_NAME=createMkwiiModule"
  "-sENVIRONMENT=web"
  "-sALLOW_MEMORY_GROWTH=1"
  "-sEXPORTED_FUNCTIONS=['_main','_mkwii_web_status']"
  "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']"
)
