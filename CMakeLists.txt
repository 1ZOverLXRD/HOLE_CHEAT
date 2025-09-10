cmake_minimum_required(VERSION 3.20)

# This will try to solve encoding issues with MSVC compiler output
# only needed if you still have garbled text in the build output


# 项目名称和版本
project(HOLECHEAT VERSION 1.0 LANGUAGES C CXX)

# 使用最新的C++标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)



# 设置SDK路径
set(SDK_DIR "${CMAKE_SOURCE_DIR}/CPPSDK")

# 递归搜索所有 .cpp/.c/.cc/.cxx 和 .h/.hpp 文件
file(GLOB_RECURSE SDK_SOURCES
        "${SDK_DIR}/*.cpp"
        "${SDK_DIR}/*.c"
        "${SDK_DIR}/*.cc"
        "${SDK_DIR}/*.cxx"
        "${SDK_DIR}/*.h"
        "${SDK_DIR}/*.hpp"
)
#MinHook
add_library(MinHook  STATIC
        OSIMGUI/Minhook/buffer.c
        OSIMGUI/Minhook/hde32.c
        OSIMGUI/Minhook/hde64.c
        OSIMGUI/Minhook/hook.c
        OSIMGUI/Minhook/trampoline.c
)
target_include_directories(MinHook PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/OSIMGUI/Minhook
)
# 生成 DLL (MODULE = 只生成 DLL，不会生成 .lib)
add_library(coldFish MODULE ${SDK_SOURCES}
        main.cpp
        pch.h
        OSIMGUI/OS-ImGui.cpp
        OSIMGUI/OS-ImGui.h
        OSIMGUI/OS-ImGui_Base.cpp
        OSIMGUI/OS-ImGui_Base.h
        OSIMGUI/OS-ImGui_Exception.hpp
        OSIMGUI/OS-ImGui_External.cpp
        OSIMGUI/OS-ImGui_External.h
        OSIMGUI/OS-ImGui_Internal.cpp
        OSIMGUI/OS-ImGui_Internal.h
        OSIMGUI/OS-ImGui_Struct.h
        OSIMGUI/imgui/imgui.cpp
        OSIMGUI/imgui/imgui_demo.cpp
        OSIMGUI/imgui/imgui_draw.cpp
        OSIMGUI/imgui/imgui_impl_dx9.cpp
        OSIMGUI/imgui/imgui_impl_dx11.cpp
        OSIMGUI/imgui/imgui_impl_dx12.cpp
        OSIMGUI/imgui/imgui_impl_win32.cpp
        OSIMGUI/imgui/imgui_tables.cpp
        OSIMGUI/imgui/imgui_widgets.cpp
        OSIMGUI/imgui/imstb_rectpack.h
        OSIMGUI/imgui/imstb_truetype.h

        coldFish.cpp
        coldFish.h
        ActorMannager/AcotrMannager.cpp
        ActorMannager/AcotrMannager.h
        BoneESP/BoneHelper.cpp
        BoneESP/BoneHelper.h
        Aimboter/Aimboter.h
        Aimboter/Aimboter.cpp
        UI/UIState.h
        UI/UIRender.h
        UI/UIRender.h
        UI/UIRender.cpp
        UI/UIRender.cpp
)

# 包含头文件目录
target_include_directories(coldFish PUBLIC ${SDK_DIR})
# 链接 MinHook
target_link_libraries(coldFish PRIVATE
        MinHook
)
#预处理器
target_compile_definitions(coldFish PRIVATE
        OSIMGUI_INTERNAL
)

add_custom_command(TARGET coldFish POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "DLL output: $<TARGET_FILE:coldFish>"
        #COMMAND start "" "\"D:/mysteam/HOLE Build 18879807/HOLE/Binaries/Win64/HOLE-Win64-Shipping.exe\""
)


