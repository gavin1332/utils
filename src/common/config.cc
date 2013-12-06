// Copyright (c) 2010-2011, Tuji
// All rights reserved.
//
// ${license}
//
// Author: LIU Yi

#include "common/config.h"

#include <string>
#include <fstream>
#include <sstream>

#include "common/common.h"

using namespace std;

bool Config::Load() {
  ifstream in(file_path_.c_str());
  if (!in) {
    return false;
  }

  string line;
  while (getline(in, line)) {
    line = CmnUtils::Trim(line);
    if (line.length() == 0 || line[0] == '#') {
      continue;
    }

    string::size_type pos = line.find_first_of("=");
    if (pos == string::npos) {
      return false;
    }

    string key = line.substr(0, pos);
    key = CmnUtils::Trim(key);
    string value = line.substr(pos + 1, line.length() - pos - 1);
    value = CmnUtils::Trim(value);

    PutByForce(key, value);
  }

  return true;
}

bool Config::Put(string key, string value) {
  pair<Properties::iterator, bool> res;

  res = properties_.insert(make_pair(key, value));

  return res.second;
}

void Config::Store() {
  ofstream ofs(file_path_.c_str());

  for (Properties::iterator itr = properties_.begin(); itr != properties_.end();
      itr++) {
    ofs << itr->first << "=" << itr->second << endl;
  }
}

bool Config::GetBool(std::string key) throw (std::string) {
  std::string value = Get(key);

  if (value == "true") {
    return true;
  } else if (value == "false") {
    return false;
  } else {
    std::string str("ERROR: the value of key \"");
    str += key + "\" is not bool";

    throw str;
  }
}
