//
// Created by Kaning on 22-10-1.
//

#include <cstring>
#include <iostream>
#include <chrono>
#include <unordered_map>
#include "maze_game.hpp"
#include "imgui.h"

#define OPENNESS 3

namespace leking {
    MazeGameManager::MazeGameManager(KanDevice& device, std::vector<KanGameObject>& gameObjects, std::unordered_map<std::string,std::shared_ptr<Mesh>> meshes,std::unordered_map<std::string,std::shared_ptr<leking::Material>> materials, int& width, int& height) : width(width),height(height),device(device), gameObjects(gameObjects), srcWidth(width), srcHeight(height){
        srand((unsigned)time(NULL));
        if(width < 3 || height < 3) {
            throw std::runtime_error("At least 3 in length or width");
        }
        if(width > 100 || height > 100) {
            throw std::runtime_error("At big 100 in length or width");
        }
        wallMesh = meshes["cube"];
        roadMesh = meshes["cube"];
        wallMaterial = materials["defaultMesh"];
        roadMaterial = materials["defaultMesh"];
        GenerateMaze();
    }

    void MazeGameManager::GenerateMaze() {
        data = (char*)malloc(sizeof(char)*width*height);
        memset(data,'o',sizeof(char)*width*height);
        for(int i = 0; i < width; i++) {
            for(int j = 0; j < height; j++) {
                if(i == 0 || i == width - 1 || j == 0 || j == height - 1) {
                    data[height*i + j] = 'x';
                    continue;
                }
                if(rand()%OPENNESS == 0) data[height*i + j] = 'x';
                else data[height*i + j] = 'o';
            }
            //std::cout << endl;
        }
        data[height*0+1] = 'o';
        data[height*(width-1)+(height-2)] = 'o';
        for(int i = 0; i < width; i++) {
            for(int j = 0; j < height; j++) {
                //std::cout << data[height*i + j];
            }
            //std::cout << endl;
        }
    }

    void MazeGameManager::CreateMaze() {

        for(int i = 0;i< gameObjects.size();i++) {
            if(gameObjects[i].name.substr(0,4) == "wall" || gameObjects[i].name.substr(0,4) == "road") {
                KanGameObject::Destroy(gameObjects[i]);
            }
        }

        for(int i = 0; i < width; i++) {
            for(int j = 0; j < height; j++) {
                if(data[height*i+j] == 'x')
                {
                    AddWall(i, j);
                }
            }
        }
    }
    static float canSolveCount = 0;
    static float cantSolveCount = 0;
    void MazeGameManager::Update(GLFWwindow *window, float dt) {
        if(ImGui::Button(u8"重置迷宫")) {
            width = srcWidth;
            height = srcHeight;
            if(width > 100 || height > 100) {
                return;
            }
            RefreshMaze();
            CreateMaze();
            mazeRouteStack = {};
            solvedSuccessfully = false;
            startSolveMaze = false;
            autoMod = false;
        }
        if(ImGui::Button(u8"自动求解")) {
            autoMod = true;
        }

        if(glfwGetKey(window, keys.resetMaze) == GLFW_PRESS && !onRefresh && !autoMod) {
            onRefresh = true;
            RefreshMaze();
            CreateMaze();
            //std::cout<<gameObjects.size()<<endl;
        }
        else if(glfwGetKey(window, keys.resetMaze) == GLFW_RELEASE && onRefresh && !autoMod) {
            mazeRouteStack = {};
            solvedSuccessfully = false;
            startSolveMaze = false;
            onRefresh = false;
        }
//        if(glfwGetKey(window, keys.autoSolveMaze) == GLFW_PRESS && !autoMod) {
//            autoMod = true;
//        }else if(glfwGetKey(window, keys.autoSolveMaze) == GLFW_RELEASE && autoMod) {
//            autoMod = false;
//        }
        if(autoMod){
            if(count>0.125f){
                SolveMaze();
                count = 0;
                return;
            }
            count += dt;
        }
        if(glfwGetKey(window, keys.solveMaze) == GLFW_PRESS && !canSolveMaze && !autoMod) {
            canSolveMaze = true;
            SolveMaze();
        }
        else if(glfwGetKey(window, keys.solveMaze) == GLFW_RELEASE && canSolveMaze && !autoMod) {
            canSolveMaze = false;
        }
    }

    void MazeGameManager::RefreshMaze() {
        if(data != nullptr){
            free(data);
        }
        GenerateMaze();
    }


    void MazeGameManager::SolveMaze() {
        if(solvedSuccessfully) {
            autoMod = false;
            return;
        }
        if(mazeRouteStack.Size() == 0 && !startSolveMaze) {
            startSolveMaze = true;
            if(data[height*0+1] == 'z') {
                startSolveMaze = false;
                autoMod = false;
                std::cout<<"no solve"<<std::endl;
                return;
            }
            mazeRouteStack.Push({0,1});
            AddWalkThrough(0,1);
            data[height*0+1] = 'z';
            return;
        }
        switch(StepMaze()){
            case 0:
                mazeRouteStack.Pop();
                PopWall();
                break;
            case 1:
                break;
            case 2:
                solvedSuccessfully = true;
                for(int i = 0;i<mazeRouteStack.Size();i++){
                    gameObjects.pop_back();
                }
                while (mazeRouteStack.Size() != 0) {
                    MazePos curr = mazeRouteStack.Pop();
                    AddRoad(curr.x,curr.y);
                }
                return;
        }
        if(mazeRouteStack.Size() == 0 && startSolveMaze) {
            mazeRouteStack = {};
            startSolveMaze = false;
            std::cout<<"no solve"<<std::endl;
            autoMod = false;
        }
    }


    int MazeGameManager::StepMaze() {
        MazePos curr = mazeRouteStack.Peek();
        //终点
        if(curr.x == width - 1 && curr.y == height - 2) {
            return 2;
        }
        //向左寻路
        if(data[ height * curr.x + curr.y+ 1 ] == 'o') {
            mazeRouteStack.Push({curr.x,curr.y+ 1});
            data[ height * (curr.x) + curr.y + 1 ] = 'z';
            AddWalkThrough(curr.x,curr.y + 1);
            return 1;
        }
        //向前寻路
        if(data[ height * (curr.x + 1) + curr.y ] == 'o') {
            mazeRouteStack.Push({curr.x + 1,curr.y});
            data[ height * (curr.x + 1) + curr.y ] = 'z';
            AddWalkThrough(curr.x + 1,curr.y);
            return 1;
        }
        //向右寻路
        if(data[ height * curr.x + curr.y - 1 ] == 'o') {
            mazeRouteStack.Push({curr.x,curr.y - 1});
            data[ height * (curr.x) + curr.y - 1 ] = 'z';
            AddWalkThrough(curr.x,curr.y - 1);
            return 1;
        }
        //向后寻路
        if(data[ height * (curr.x - 1) + curr.y ] == 'o') {
            mazeRouteStack.Push({curr.x - 1,curr.y});
            data[ height * (curr.x - 1) + curr.y  ] = 'z';
            AddWalkThrough(curr.x - 1,curr.y);
            return 1;
        }
        return 0;
    }

    void MazeGameManager::AddWall(int x, int y) {
        auto wall = KanGameObject::createGameObject();
        wall.mesh = wallMesh;
        wall.material = wallMaterial;
        wall.name = "wall("+ std::to_string(x) + std::to_string(y)+")";
        wall.transform.translation = {x, 0.0f, y};
        wall.transform.scale = {0.5f, 0.5f, 0.5f};
        wall.color = {0.890196f,0.929411f,0.803921f};
        gameObjects.push_back(std::move(wall));
    }
    void MazeGameManager::PopWall() {
        gameObjects.pop_back();
    }

    void MazeGameManager::AddWalkThrough(int x, int y) {
        auto wall = KanGameObject::createGameObject();
        wall.mesh = wallMesh;
        wall.material = wallMaterial;
        wall.name = "wall("+ std::to_string(x) + std::to_string(y)+")";
        wall.transform.translation = {x, 0.0f, y};
        wall.transform.scale = {0.25f, 0.25f, 0.25f};
        wall.color = {0.0f,1.0f,0.0f};
        gameObjects.push_back(std::move(wall));
    }

    void MazeGameManager::AddRoad(int x, int y) {
        auto road = KanGameObject::createGameObject();
        road.mesh = roadMesh;
        road.material = roadMaterial;
        road.name = "road(" + std::to_string(x) + std::to_string(y) + ")";
        road.transform.translation = {x, 0.0f, y};
        road.transform.scale = {0.25f, 0.25f, 0.25f};
        road.color = {0.0f,1.0f,1.0f};
        gameObjects.push_back(std::move(road));
    }

    void MazeRouteStack::Push(MazePos pos) {
        data.push_back(pos);
    }

    MazePos MazeRouteStack::Pop() {
        MazePos res = data.back();
        data.pop_back();
        return res;
    }

    MazePos MazeRouteStack::Peek() {
        return data.back();
    }

    int MazeRouteStack::Size() {
        return data.size();
    }
} // leking