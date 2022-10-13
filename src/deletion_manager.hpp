//
// Created by leking on 22-10-10.
//

#ifndef KANKU_DELETION_MANAGER_HPP
#define KANKU_DELETION_MANAGER_HPP


#include <deque>
#include <functional>

struct DeletionQueue{
    std::deque<std::function<void()>> deletors;

    void push(std::function<void()>&& function) {
        deletors.push_back(function);
    }

    void flush() {
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
            (*it)();
        }

        deletors.clear();
    }
};

#endif //KANKU_DELETION_MANAGER_HPP
