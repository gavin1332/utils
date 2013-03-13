// Copyright (c) 2010-2011, Tuji
// All rights reserved.
// 
// ${license}
//
// Author: LIU Yi

#include "utils/common/common.h"

#include <sys/stat.h>
#include <dirent.h>
#include <cstring>
#include <cmath>
#include <cassert>

#ifndef __MINGW32__
#include <iconv.h>
#endif

#include <iostream>
#include <algorithm>

using namespace std;

string CmnUtils::Trim(const string& input) {
  if (input.length() == 0) {
    return string();
  }

  string::const_iterator it = input.begin();
  string::const_iterator end = input.end();
  for (; it != end && isBlank(*it); ++it) {}
  if (it == end) {
    return string();
  }
  string::const_iterator start = it;

  for (it = input.end() - 1; it != start && isBlank(*it); --it) {}

  return string(start, it + 1);
}

void CmnUtils::Split(const std::string& input, char sep, vector<string>* output) {
  string::size_type start = 0;
  string::size_type end;
  while ((end = input.find_first_of(sep, start)) != string::npos) {
    output->push_back(input.substr(start, end - start));

    start = end + 1;
  }

  output->push_back(input.substr(start, input.length() - start));
}

void CmnUtils::RetrieveFilenames(const string& dir_name, const string& suffix,
      vector<string>* filename_list, bool ignore_case) {
  string complete_dir = dir_name;
  if(complete_dir.at(complete_dir.length() - 1) != '/') {
    complete_dir += '/';
  }

  DIR* dir = opendir(complete_dir.c_str());
  if (dir == NULL) {
    cerr << "Cannot open directory: " << complete_dir << endl;
    return;
  }

  struct stat file_stat;
  while (true) {
    struct dirent* dirent = readdir(dir);
    if (dirent == NULL) {
      break;
    }

    string path = complete_dir + dirent->d_name;
    int res = stat(path.c_str(), &file_stat);
    assert(res == 0);

    if (S_ISREG(file_stat.st_mode)) { // regular file
      string filename(dirent->d_name);

      string temp_suffix = filename.substr(filename.length() - suffix.length(),
          suffix.length());

      string suffix_target = suffix;
      if(ignore_case) {
        std::transform(temp_suffix.begin(), temp_suffix.end(),
            temp_suffix.begin(), ::tolower);

        std::transform(suffix_target.begin(), suffix_target.end(),
            suffix_target.begin(), ::tolower);
      }

      if (temp_suffix.compare(suffix_target) == 0) {
        filename_list->push_back(filename);
      }
    }
  }

  closedir(dir);
}

void CmnUtils::RetrieveDirNames(const string& dir_name,
      vector<std::string>* dir_name_list) {
  string complete_dir = dir_name;
  if(complete_dir.at(complete_dir.length() - 1) != '/') {
    complete_dir += '/';
  }

  DIR* dir = opendir(complete_dir.c_str());
  if (dir == NULL) {
    cerr << "Cannot open directory: " << complete_dir << endl;
    return;
  }

  struct stat file_stat;
  while (true) {
    struct dirent* dirent = readdir(dir);
    if (dirent == NULL) {
      break;
    }

    string path = complete_dir + dirent->d_name;
    int res = stat(path.c_str(), &file_stat);
    assert(res == 0);

    if (S_ISDIR(file_stat.st_mode)) {
      string name(dirent->d_name);
      if (name != "." && name != "..") {
        dir_name_list->push_back(name);
      }
    }
  }

  closedir(dir);
}

#ifndef __MINGW32__
int CmnUtils::Utf8ToWChar(const char* src, int src_len, wchar_t* dst,
    int dst_len) {
  const int utf8_unit_len = 3;
  int char_num = src_len / utf8_unit_len;

  assert(char_num <= dst_len);

  const char* from_code = "UTF8";
  const char* to_code = "WCHAR_T";
  iconv_t cnv_type = iconv_open(to_code, from_code);
  if (cnv_type == (iconv_t) - 1) {
    assert(false);
  }

  // Have to malloc space, for input src is const and iconv funcion does not
  // allow char[]
  char* from_char = new char[src_len];
  memcpy(from_char, src, src_len * sizeof(char));
  char* to_char = (char*) dst;

  size_t s_len = static_cast<size_t>(src_len);
  size_t d_len = static_cast<size_t>(char_num * sizeof(wchar_t));

  char* bk = from_char;
  int res = iconv(cnv_type, &from_char, &s_len, &to_char, &d_len);

  delete[] bk;

  return (res == 0) ? char_num : 0;
}
#endif

