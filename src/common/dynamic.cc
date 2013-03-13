/*
 * dynamic.cc
 *
 *  Created on: Jul 20, 2012
 *      Author: liuyi
 */

#include "utils/common/dynamic.h"

#include <cstdlib>

using namespace std;

bool DynamicFactory::RegisterCreateFunc(const string & class_name,
    ObjectCreateFunc func) {
  pair<FuncRegistry::iterator, bool> res = registry_.insert(make_pair(class_name, func));
  return res.second;
}

DynamicObject* DynamicFactory::Create(const string& class_name) const {
  DynamicObject* object = NULL;

  FuncRegistry::const_iterator entry = registry_.find(class_name);
  if (entry != registry_.end()) {
    object = entry->second();
  }

  return object;
}
