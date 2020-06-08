#pragma once

#include <map>
#include <string>
#include <iostream>
#include <memory>
#include <stdexcept>

template <typename T> class NamedHandle {
public:
    // "default" construct
    NamedHandle() {}
    NamedHandle(const std::string& name, const std::shared_ptr<T>& ptr) : name(name), ptr(ptr) {}

    // create new object and store handle in map for later retrieval
    template <class... Args>
    NamedHandle(const std::string& name, Args&&... args) : name(name), ptr(std::make_shared<T>(args...)) {
        if (map.count(name))
            std::cerr << "WARN: NamedHandle<" <<  typeid(T).name() << ">: overwriting previous mapping for: " << name << std::endl;
        map[name] = *this;
    }
    template <class... Args>
    static NamedHandle<T> create(const std::string& name, Args&&... args) {
        map[name] = NamedHandle<T>(name, std::make_shared<T>(args...));
        return map[name];
    }

    virtual ~NamedHandle() {}

    // copy / move
    NamedHandle(const NamedHandle<T>& other) = default;
    NamedHandle(NamedHandle<T>&& other) = default;
    inline NamedHandle<T>& operator=(const NamedHandle<T>& other) = default;
    inline NamedHandle<T>& operator=(NamedHandle<T>&& other) = default;

    // operators for pointer-like usage
    inline explicit operator bool() const { return ptr.operator bool(); }
    inline T* operator->() { return ptr.operator->(); }
    inline const T* operator->() const { return ptr.operator->(); }
    inline T& operator*() { return *ptr; }
    inline const T& operator*() const { return *ptr; }

    // check if mapping for given name exists
    static bool valid(const std::string& name) { return map.count(name); }
    // return shared pointer to mapping for given name
    static NamedHandle<T> find(const std::string& name) { return map[name]; }
    // clear saved handles to possibly free unsused memory
    static void clear() { map.clear(); }

    // iterators to iterate over all entries
    static typename std::map<std::string, NamedHandle<T>>::iterator begin() { return map.begin(); }
    static typename std::map<std::string, NamedHandle<T>>::iterator end() { return map.end(); }

    std::string name;    
    std::shared_ptr<T> ptr;
    static std::map<std::string, NamedHandle<T>> map;
};

// definition of static member (compiler magic)
template <typename T> std::map<std::string, NamedHandle<T>> NamedHandle<T>::map;
