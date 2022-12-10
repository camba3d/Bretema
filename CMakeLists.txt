
cmake_minimum_required(VERSION 3.20)

# ------------------------- #
# - PROJECT SETTINGS
# ------------------------- #

# Name
project(Bretema VERSION 1.0)
# set(CMAKE_CXX_STANDARD 20)

# Configure file
# configure_file(BuildInfo.in BuildInfo.hpp)

# Show only warning messages
#set(CMAKE_MESSAGE_LOG_LEVEL "WARNING")

# Export compile commands 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# ------------------------- #
# - OPTIONS
# ------------------------- #

option(OPT_TESTS "Compile tests instead of main app" OFF)


# ------------------------- #
# - THIRD PARTY
# ------------------------- #

# Modern Printf
set(FMT_DOC OFF)
add_subdirectory(Modules/fmt)
list(APPEND EXTRA_LIBS fmt)

# Glsl-Style Maths
add_subdirectory(Modules/glm)
list(APPEND EXTRA_LIBS glm)

# Cross-Platform Window Management
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_VULKAN_STATIC  ON  CACHE BOOL "" FORCE)
add_subdirectory(Modules/glfw)
list(APPEND EXTRA_LIBS glfw)

# Vulkan
find_package(Vulkan REQUIRED COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)
#list(APPEND EXTRA_LIB_INCL ${GLFW_DIR}/deps)
list(APPEND EXTRA_LIBS Vulkan::Vulkan)
add_subdirectory(Modules/vk-bootstrap)
list(APPEND EXTRA_LIBS vk-bootstrap)

# ImGui: Platform agnostic GUI
set(IMGUI_DIR Modules/ImGui)
list(APPEND EXTRA_LIB_INCL ${IMGUI_DIR})
list(APPEND EXTRA_LIB_SOURCES
    ${IMGUI_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/imgui_impl_vulkan.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)
list(APPEND EXTRA_LIB_HEADERS
    ${IMGUI_DIR}/imgui_impl_glfw.h
    ${IMGUI_DIR}/imgui_impl_vulkan.h
    ${IMGUI_DIR}/imgui.h
    ${IMGUI_DIR}/imgui_internal.h
    ${IMGUI_DIR}/imstb_rectpack.h
    ${IMGUI_DIR}/imstb_textedit.h
    ${IMGUI_DIR}/imstb_truetype.h
)


# ------------------------- #
# - TESTING
# ------------------------- #

if (OPT_TESTS)
    add_subdirectory(Modules/Catch2)
    list(APPEND EXTRA_LIBS Catch2::Catch2WithMain)
    include(CTest)
    include(Catch)
endif()


# ------------------------- #
# - SYSTEM GUI RESOURCES
# ------------------------- #

# if(WIN32)
#     # WINDOWS :: Tested
#     set(CLIENT_RESOURCES WIN32 Clients/Windows/Resources.rc)
# elseif(NOT UNIX)
#     # MACOS :: NOT Tested
#     set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
#     set(MACOSX_BUNDLE_ICON_FILE Icon.icns) # <- Just name, not abs-path.
#     set(CLIENT_RESOURCES MACOSX_BUNDLE Clients/MacOS/Icon.icns)
#     set_source_files_properties(Clients/MacOS/Icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
# else()
#     # LINUX :: NOT Tested
# endif()


# ------------------------- #
# - SHADERS COMPILE AND COPY (@review: improve vars naming)
# ------------------------- #

set(shaders_src_dir "Assets/Shaders")
set(shaders_bin_dir "Assets/Shaders")

file(GLOB shaders_list ${shaders_src_dir}/*)

add_custom_command(COMMAND
    ${CMAKE_COMMAND} -E remove_directory ${shaders_bin_dir}
    ${CMAKE_COMMAND} -E make_directory   ${shaders_bin_dir}
    OUTPUT ${shaders_bin_dir}
)

foreach(shader_src IN LISTS shaders_list)
    get_filename_component(filename_and_ext ${shader_src} NAME)
    set(shader_dst ${shaders_bin_dir}/${filename_and_ext}.spv)
    # MESSAGE(STATUS "[SHADERS] - Compiling:\n  From: ${shader_src}\n  To  : ${shader_dst}")
    add_custom_command(COMMAND ${glslc_executable} -o ${shader_dst} ${shader_src} OUTPUT ${shader_dst})

    list(APPEND EXTRA_SHADERS ${shaders_bin_dir}/${filename_and_ext}.spv)
endforeach()

add_custom_target(shaders ALL DEPENDS ${EXTRA_SHADERS}) # Seems like unneeded


# ------------------------- #
# - PROJECT
# ------------------------- #

set(SRC "Bretema")

# file(GLOB EXTRA_HEADERS ${SRC}/*.h)
list(APPEND EXTRA_HEADERS
    ${SRC}/Base.hpp
    ${SRC}/Window.hpp

    ${SRC}/Vulkan/Base.hpp
    ${SRC}/Vulkan/Manager.hpp
    ${SRC}/Vulkan/ToStr.hpp
    ${SRC}/Vulkan/Initializers.hpp
    )
    
    # file(GLOB EXTRA_SOURCES ${SRC}/*.cpp)
list(APPEND EXTRA_SOURCES
    ${SRC}/Window.cpp

    ${SRC}/Vulkan/Manager.cpp
)

add_library(${PROJECT_NAME}
    STATIC
        ${EXTRA_SHADERS}
        ${EXTRA_HEADERS}
        ${EXTRA_SOURCES}
        ${EXTRA_LIB_HEADERS} # @review
        ${EXTRA_LIB_SOURCES} # @review
)

# Standard
target_compile_features(${PROJECT_NAME}
    PUBLIC
        cxx_std_20
)
set_property(TARGET ${PROJECT_NAME} 
    PROPERTY 
        CXX_STANDARD 20
)

# Defines
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        -DImTextureID=ImU64
)

# Compile Flags [ Windows ]
if(MSVC)
    if (CMAKE_BUILD_TYPE EQUAL "RELEASE")
        target_compile_options(${PROJECT_NAME} PRIVATE /O2 /MD)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX /MDd /Zi)
    endif()

# Compile Flags [ Unix / Linux / MacOS ]
else()
    if (CMAKE_BUILD_TYPE EQUAL "RELEASE")
        target_compile_options(${PROJECT_NAME} PRIVATE -O3)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -O0 -g3 -ggdb -Wall -Wpedantic -Wextra -Wrange-loop-construct -Wno-missing-field-initializers)
    endif()
endif() 

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        Modules
        ${EXTRA_LIB_INCL}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${EXTRA_LIBS}
)


# ------------------------- #
# - HELPERS
# ------------------------- #

function(NEW_EXE exe_name exe_sources)
    add_executable(${exe_name} ${exe_sources} ${EXTRA_LIB_HEADERS})
    target_include_directories(${exe_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/build ${EXTRA_LIB_INCL})
    target_link_libraries(${exe_name} PRIVATE ${EXTRA_LIBS} ${PROJECT_NAME})
endfunction()

function(NEW_TEST test_name test_sources)
    set(aux_test_name test_${test_name})
    NEW_EXE(${aux_test_name} ${test_sources})
    catch_discover_tests(${aux_test_name})
endfunction()


# ------------------------- #
# - EXE and TESTING
# ------------------------- #

# if (OPT_TESTS)
# NEW_TEST( Sum1 Tests/Sum.cpp  )
# NEW_TEST( Sum2 Tests/Sum2.cpp )
# endif()

# NEW_EXE( VkDemo Tests/VkDemo.cpp )
NEW_EXE( BasicDemo Tests/BasicDemo.cpp )
NEW_EXE( ImGuiDemo Tests/ImGuiDemo.cpp )
