// Copyright (c) 2010-2011, Tuji
// All rights reserved.
//
// ${license}
//
// Author: LIU Yi

#ifndef COMMON_CONFIG_H_
#define COMMON_CONFIG_H_

#include <cstdlib>
#include <cassert>
#include <string>
#include <map>

#include "header.h"

// '#' is recognized as the head of a comment.
// And config file only supports English by now
class Config {
 public:
  Config() {
    new (this)Config(".");
  }
  Config(const char* file_path) : file_path_(file_path) {}
  ~Config() {}

  void set_file_path(const std::string& file_path) {
    file_path_ = file_path;
  }

  bool Load();

  bool Put(std::string key, std::string value);

  void PutByForce(std::string key, std::string value) {
    properties_[key] = value;
  }

  bool Exist(std::string key) {
    return Get(key).length() > 0;
  }

  bool Change(std::string key, std::string value) {
    if (!Exist(key)) {
      return false;
    }

    PutByForce(key, value);
    return true;
  }

  std::string Get(std::string key) {
    Properties::iterator itr = properties_.find(key);

    if (itr == properties_.end()) {
      assert(false);
    }

    return itr->second;
  }

  bool GetBool(std::string key) throw (std::string);

  int GetInt(std::string key) {
    std::string value = Get(key);
    return atoi(value.c_str());
  }

  float GetFloat(std::string key) {
    std::string value = Get(key);
    return atof(value.c_str());
  }

  void Store();

 private:
  std::string file_path_;

  typedef std::map<std::string, std::string> Properties;
  Properties properties_;

  DISALLOW_COPY_AND_ASSIGN(Config);
};

#endif
