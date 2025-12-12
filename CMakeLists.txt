cmake_minimum_required(VERSION 3.15)
project(terrain)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Project sources + glad
file(GLOB_RECURSE SRC_FILES
    "src/*.cpp"
    "include/glad.c"
)

# ImGui sources (only needed ones)
set(IMGUI_SOURCES
    include/imgui/imgui.cpp
    include/imgui/imgui_draw.cpp
    include/imgui/imgui_tables.cpp
    include/imgui/imgui_widgets.cpp
    include/imgui/backends/imgui_impl_glfw.cpp
    include/imgui/backends/imgui_impl_opengl3.cpp
)

# Combine sources
list(APPEND SRC_FILES ${IMGUI_SOURCES})

# Include directories
include_directories(
    include
    include/glad
    include/imgui
    include/imgui/backends
    C:/msys64/mingw64/include
    C:/msys64/mingw64/include/LinearMath # Bullet LinearMath
)

# Add executable
add_executable(terrain ${SRC_FILES})


# --- 🟢 OpenMP Support ---
target_compile_options(terrain PRIVATE -fopenmp)

# Put the executable in parent folder of build
set_target_properties(terrain PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# Link libraries
target_link_libraries(terrain
    glfw3
    LinearMath
    opengl32
    gdi32
    -fopenmp
)
