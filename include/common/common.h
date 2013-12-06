// Copyright (c) 2010-2011, Tuji
// All rights reserved.
// 
// ${license}
//
// Author: LIU Yi

#ifndef COMMON_COMMON_H_
#define COMMON_COMMON_H_

#include <cmath>
#include <string>
#include <vector>

class CmnUtils {
 public:
  static std::string Trim(const std::string& input);

  static void Split(const std::string& input, char delim,
                    std::vector<std::string>* output);

  // Retrieve all file names with a named suffix in the specified folder
  static void RetrieveFilenames(const std::string& dir_name,
      const std::string& suffix, std::vector<std::string>* filename_list,
      bool ignore_case);

  // Retrieve all folder names in the specified folder
  static void RetrieveDirNames(const std::string& dir_name,
      std::vector<std::string>* dir_name_list);

#ifndef __MINGW32__
  static int Utf8ToWChar(const char *utf8, int utf8_len, wchar_t *wchar,
      int wchar_len);
#endif

 private:
  static bool IsBlank(const char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
  }
};

#endif

