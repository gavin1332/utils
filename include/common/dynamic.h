/*
 * dynamic.h
 *
 *  Created on: Jul 20, 2012
 *      Author: liuyi
 */

#ifndef COMMON_DYNAMIC_H_
#define COMMON_DYNAMIC_H_

#include <string>
#include <map>

#include "header.h"

class DynamicObject {};

class DynamicFactory {
 public:

  // Call back function
  typedef DynamicObject* (*ObjectCreateFunc)();

  static DynamicFactory& Instance() {
    static DynamicFactory instance;
    return instance;
  }

  // Register DynamicObject create function along with its class name
  //
  // @param class_name The name of the class needing register
  // @param func The function to create a DynamicObject
  //
  // @return True if the class_name has not been registered yet, false otherwise
  bool RegisterCreateFunc(const std::string& class_name, ObjectCreateFunc func);

  // Dynamically create a DynamicObject specified by its class name.
  // Make sure the specified class has been registered by RegisterCreateFunc,
  // or you will get a NULL object after calling
  //
  // @param class_name The name of the class needing create
  //
  // @return The newly create DynamicObject
  DynamicObject* Create(const std::string& class_name) const;

  private:
  typedef std::map<std::string, ObjectCreateFunc> FuncRegistry;

  // The registry table
  FuncRegistry registry_;

  DynamicFactory() {}
  ~DynamicFactory() {}
  DISALLOW_COPY_AND_ASSIGN(DynamicFactory);
};

// A trick to implements dynamic initialization
//
// The class which needs dynamic initialization must do such two steps to
// make it work:
// First, write DECLARE_CLASS_CREATE(parent) within its class declaration;
// Second, write IMPL_AND_REG_CLASS_CREATE(parent, class_name) in its
// corresponding cpp file.
// After these two steps, the class can be registered in DynamicFactory
#define DECLARE_CLASS_CREATE()  \
static DynamicObject* Create()

#define IMPL_AND_REG_CLASS_CREATE(class_name) \
    DynamicObject* class_name::Create() {  \
  return new class_name(); \
} \
namespace { \
bool class_name##Dummy = DynamicFactory::Instance() \
    .RegisterCreateFunc(#class_name, class_name::Create);  \
}

#endif
