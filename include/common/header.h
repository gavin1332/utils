// Copyright (c) 2010-2011, Tuji
// All rights reserved.
//
// ${license}
//
// Author: LIU Yi

#ifndef COMMON_HEADER_H_
#define COMMON_HEADER_H_

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#endif
