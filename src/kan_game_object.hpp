//
// Created by leking on 22-10-11.
//

#ifndef KANKU_KAN_GAME_OBJECT_HPP
#define KANKU_KAN_GAME_OBJECT_HPP

#include <vector>
#include <memory>
#include <string>
#include "vk_mesh.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <memory>

namespace leking {

    struct Material {
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
        VkDescriptorSet textureSet{VK_NULL_HANDLE}; //texture defaulted to null
        Material(VkPipeline pipeline, VkPipelineLayout pipelineLayout);
    };
    struct  TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        glm::vec3  rotation{};

        glm::mat4 mat4();
        glm::mat3 normalMatrix();
    };
    class KanGameObject {
    public:
        using id_t = unsigned int;
        int test{0};

        static KanGameObject createGameObject() {
            static  id_t currentId = 0;
            return  KanGameObject{currentId++};
        }

        KanGameObject(const KanGameObject &) = delete;
        KanGameObject& operator=(const KanGameObject&) = delete;
        KanGameObject(KanGameObject&&) = default;
        KanGameObject& operator=(KanGameObject&&) = default;

        static void Destroy(KanGameObject& gameObject);
        static void GameObjectGC(std::vector<KanGameObject>& gameObjects);

        bool CanDraw(){return canDraw;}

        id_t getId() {return id;}

        std::shared_ptr<Mesh> mesh{};
        std::shared_ptr<Material> material{};
        glm::vec3 color{};
        TransformComponent transform{};
        std::string name{"gameObject"};

    private:
        bool canDraw{true};
        bool onDestroy{false};

        KanGameObject(id_t objId) : id{objId} {}

        id_t id;
    };

} // leking

#endif //KANKU_KAN_GAME_OBJECT_HPP
