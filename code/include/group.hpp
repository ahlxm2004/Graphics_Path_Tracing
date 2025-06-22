#ifndef GROUP_H
#define GROUP_H


#include "object3d.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include <iostream>
#include <vector>


class Group : public Object3D {

public:

    Group() {
        array = nullptr;
        size = 0;
    }

    explicit Group (int num_objects) {
        array = new Object3D *[num_objects];
        size = num_objects;
    }

    ~Group() override {
        for (int i = 0; i < size; i++)
            delete array[i];
        delete array;
    }

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        bool ans = false;
        for (int i = 0; i < size; i++)
            ans |= array[i]->intersect(r, h, tmin);
        return ans;
    }

    void addObject(int index, Object3D *obj) {
        array[index] = obj;
    }

    int getGroupSize() {
        return size;
    }

private:

    Object3D **array;
    int size;
};

#endif
	
