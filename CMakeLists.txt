cmake_minimum_required(VERSION 3.20)

project(DevTerm
  VERSION 0.1.0
  LANGUAGES CXX
)

if(NOT WIN32)
  message(FATAL_ERROR "DevTerm currently requires Windows.")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui")
set(IMPLOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui-plot")

add_library(imgui STATIC
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
  ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
  ${IMGUI_DIR}/backends/imgui_impl_opengl2.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)

add_library(implot STATIC
  ${IMPLOT_DIR}/implot.cpp
  ${IMPLOT_DIR}/implot_items.cpp
)
target_include_directories(implot PUBLIC ${IMPLOT_DIR})
target_link_libraries(implot PUBLIC imgui)

find_package(Threads REQUIRED)

add_executable(DevTerm WIN32
  src/CApplication.cpp
  src/CCommunicationWorker.cpp
  src/CConnection.cpp
  src/CScpiResponse.cpp
  src/CWindowsFont.cpp
  src/theme/spectrum.cpp
  src/main.cpp
)

target_link_libraries(DevTerm
  PRIVATE
    imgui
    implot
    Threads::Threads
    gdi32
    opengl32
    user32
    dwmapi
    ws2_32
)

include(CTest)
if(BUILD_TESTING)
  add_executable(DevTermThemeTest
    tests/theme_test.cpp
    src/theme/spectrum.cpp
    src/CCommunicationWorker.cpp
    src/CConnection.cpp
    src/CScpiResponse.cpp
    src/CWindowsFont.cpp
  )
  target_include_directories(DevTermThemeTest PRIVATE src)
  target_link_libraries(DevTermThemeTest PRIVATE imgui Threads::Threads ws2_32 dwmapi)
  add_test(NAME DevTermTheme COMMAND DevTermThemeTest)
endif()
