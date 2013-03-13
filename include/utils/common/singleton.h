// Copyright (c) 2010-2011, Tuji
// All rights reserved.
//
// ${license}
//
// Author: LIU Yi

#ifndef UTILS_COMMON_SINGLETON_H
#define UTILS_COMMON_SINGLETON_H

#include <cstdlib>
#include <cassert>

template <typename T>
class Singleton {
 protected:
  static T* singleton_;
public:
  Singleton() {
    assert(!singleton_);
    singleton_ = static_cast<T*>(this);
  }

  ~Singleton() {
    assert(singleton_);
    singleton_ = NULL;
  }

  static T& GetSingleton() {
    assert(singleton_);
    return *singleton_;
  }

  static T* GetSingletonPtr() {
    return singleton_;
  }

 private:
  Singleton& operator=(const Singleton&);
  Singleton(const Singleton&);
};

#endif
