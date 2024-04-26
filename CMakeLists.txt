cmake_minimum_required(VERSION 3.20)
project(vkHook)

set(FUNCHOOK_INSTALL OFF)
set(FUNCHOOK_BUILD_TESTS OFF)

add_subdirectory(deps/funchook)
find_package(Vulkan REQUIRED)
add_library(vkHook SHARED src/vkhook.cpp src/vulkaninit.cpp ${FUNCHOOK_SOURCES})
target_link_libraries(vkHook funchook-shared Vulkan::Vulkan)
