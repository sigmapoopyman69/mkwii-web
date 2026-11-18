cmake_minimum_required(VERSION 3.20)
project(mkwii_web LANGUAGES CXX)

if(NOT EMSCRIPTEN)
  message(FATAL_ERROR "This project is currently configured for Emscripten. Use emcmake cmake.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(WII_COMPILED_UPSTREAM_ROOT "" CACHE PATH "Checkout of upstream WiiCompiled used for translator-generated code")

add_executable(mkwii-web
  src/main.cpp
  src/platform/browser_platform.cpp
  src/platform/browser_host_context.cpp
  src/platform/browser_guest_memory.cpp
  src/platform/browser_runtime_memory.cpp
  src/platform/browser_translated_memory.cpp
)
target_include_directories(mkwii-web PRIVATE src src/generated)

set(TRANSLATED_SMOKE_SOURCE "${CMAKE_SOURCE_DIR}/generated/translated_addi_smoke.cpp")
if(WII_COMPILED_UPSTREAM_ROOT AND EXISTS "${TRANSLATED_SMOKE_SOURCE}")
  target_sources(mkwii-web PRIVATE
    "${TRANSLATED_SMOKE_SOURCE}"
    src/generated/translator_smoke_wrapper.cpp
  )
  target_include_directories(mkwii-web PRIVATE
    "${WII_COMPILED_UPSTREAM_ROOT}/runtime/include"
  )
  target_compile_definitions(mkwii-web PRIVATE WII_COMPILED_HAS_TRANSLATOR_SMOKE=1)
  message(STATUS "Building actual upstream-translator-generated PowerPC smoke function")
else()
  message(WARNING "Upstream translator smoke source is absent; this build will not execute translated code")
endif()

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
