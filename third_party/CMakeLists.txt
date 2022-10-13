find_package(Vulkan REQUIRED)

add_library(vkbootstrap STATIC)
add_library(glm INTERFACE)
add_library(vma INTERFACE)

add_library(stb_image INTERFACE)

add_library(tinyobjloader STATIC)

target_sources(vkbootstrap PRIVATE 
    vkbootstrap/VkBootstrap.h
    vkbootstrap/VkBootstrap.cpp
    )

target_include_directories(vkbootstrap PUBLIC vkbootstrap)
target_link_libraries(vkbootstrap PUBLIC Vulkan::Vulkan $<$<BOOL:UNIX>:${CMAKE_DL_LIBS}>)

#both vma and glm and header only libs so we only need the include path
target_include_directories(vma INTERFACE vma)
target_include_directories(glm INTERFACE glm)

target_sources(tinyobjloader PRIVATE 
    tinyobjloader/tiny_obj_loader.h
    tinyobjloader/tiny_obj_loader.cc
    )

target_include_directories(tinyobjloader PUBLIC tinyobjloader)


if (WIN32)
    # Vulkan SDK on Windows
    set(VULKAN_SDK_PATH c:/VulkanSDK/1.2.135.0)
    include_directories(${VULKAN_SDK_PATH}/include)
    link_directories(${VULKAN_SDK_PATH}/Lib)
    link_libraries(vulkan-1)

    # GLFW on Windows, without the help of pkg-config
    set(GLFW_PATH c:/GLFW_Libs)
    include_directories(${GLFW_PATH}/include)
    link_directories(${GLFW_PATH}/lib-vc2019)
    link_libraries(glfw3)
elseif (UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW3 REQUIRED glfw3)
    message(GLFW3_STATIC_LIBRARIES: ${GLFW3_STATIC_LIBRARIES})
    link_directories(/usr/local/lib)
    link_libraries(${GLFW3_STATIC_LIBRARIES})
endif()


add_library(imgui STATIC)

target_include_directories(imgui PUBLIC imgui)

target_sources(imgui PRIVATE
    imgui/imgui.h
    imgui/imgui.cpp

    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp

    imgui/imgui_tables.cpp
    imgui/imgui_impl_vulkan.cpp
    imgui/imgui_impl_glfw.cpp
    )

target_link_libraries(imgui PUBLIC Vulkan::Vulkan glfw)

target_include_directories(stb_image INTERFACE stb_image)
