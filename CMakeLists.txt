cmake_minimum_required(VERSION 3.20)
project(mkwii_web LANGUAGES CXX)

if(NOT EMSCRIPTEN)
  message(FATAL_ERROR "This project is currently configured for Emscripten. Use emcmake cmake.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(mkwii-web
  src/main.cpp
  src/platform/browser_platform.cpp
  src/platform/browser_host_context.cpp
  src/platform/browser_guest_memory.cpp
)
target_include_directories(mkwii-web PRIVATE src)

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
  "-sASYNCIFY=1"
  "-sSTACK_SIZE=1048576"
  "-sEXPORTED_FUNCTIONS=['_main','_mkwii_web_status','_mkwii_web_capabilities','_mkwii_web_uptime_ms']"
  "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','FS']"
)
