#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

template <typename T>
class SingleTon {
protected:
    SingleTon() {}
    ~SingleTon() {}
public:
    static T* getInstance() {
        static T p_instance;// = new T();
        return &p_instance;
    }
};
