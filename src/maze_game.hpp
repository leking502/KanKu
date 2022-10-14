//
// Created by leking on 22-10-1.
//

#ifndef KanENGINE_MAZE_GAME_HPP
#define KanENGINE_MAZE_GAME_HPP


#include <vector>
#include <GLFW/glfw3.h>
#include "kan_game_object.hpp"
#include "kan_device.hpp"

namespace leking {
    struct MazePos {
        int x{};
        int y{};
    };
    struct MazeKeyMapping {
        int resetMaze = GLFW_KEY_R;
        int solveMaze = GLFW_KEY_L;
        int autoSolveMaze = GLFW_KEY_O;
    };
    struct MazeRouteStack {
        void Push(MazePos pos);
        MazePos Pop();
        MazePos Peek();
        int Size();
    private:
        std::vector<MazePos> data{};
    };

    class MazeGameManager {
    public:


        MazeGameManager(KanDevice& device, std::vector<KanGameObject>& gameObjects, std::unordered_map<std::string,std::shared_ptr<Mesh>> meshes,std::unordered_map<std::string,std::shared_ptr<leking::Material>> materials, int& width, int& height);

        void CreateMaze();

        void RefreshMaze();

        int StepMaze();

        void Update(GLFWwindow *window, float dt);
    private:

        void GenerateMaze();

        void AddWall(int x, int y);

        void AddWalkThrough(int x, int y);

        void AddRoad(int x, int y);

        KanDevice& device;

        std::vector<KanGameObject>& gameObjects;

        std::shared_ptr<Mesh> wallMesh;
        std::shared_ptr<Mesh> roadMesh;
        std::shared_ptr<Material> wallMaterial;
        std::shared_ptr<Material> roadMaterial;

        MazePos currentPos;

        MazeRouteStack mazeRouteStack{};

        bool canSolveMaze{false};

        bool startSolveMaze{false};

        bool noSolve{ false };

        bool tooBig{ false };

        bool solvedSuccessfully{false};

        bool autoMod{false};

        bool onRefresh{false};
        char* data;
        int width;
        int height;
        int& srcWidth;
        int& srcHeight;

        MazeKeyMapping keys{};

        float count{0};

        void PopWall();

        void SolveMaze();
    };

} // leking

#endif //KanENGINE_MAZE_GAME_HPP
