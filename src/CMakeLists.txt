
# Add source to this project's executable.
add_executable(KanKu
    main.cpp
    vk_engine.cpp
    vk_engine.h
    vk_types.h
    vk_initializers.cpp
    vk_initializers.h
    pipeline_builder.cpp
    pipeline_builder.hpp
    maze_game.cpp
    maze_game.hpp
    vk_mesh.cpp
    vk_mesh.hpp kan_window.cpp kan_window.hpp kan_swap_chain.cpp kan_swap_chain.hpp kan_device.cpp kan_device.hpp deletion_manager.cpp deletion_manager.hpp kan_camera.cpp kan_camera.hpp kan_game_object.cpp kan_game_object.hpp kan_renderer.cpp kan_renderer.hpp controller.cpp controller.hpp vk_textures.cpp vk_textures.hpp)


set_property(TARGET KanKu PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:vulkan_guide>")

target_include_directories(KanKu PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(KanKu vkbootstrap vma glm tinyobjloader imgui stb_image)

add_dependencies(KanKu Shaders)
