/*
 * interface.h
 *
 *  Created on: Jan 9, 2013
 *      Author: liuyi
 */

#ifndef UTILS_SVM_INTERFACE_H_
#define UTILS_SVM_INTERFACE_H_

#include "svm.h"

class SvmClassifiable {
 public:
	virtual ~SvmClassifiable() {};

	virtual struct svm_node* GetSvmNode() = 0;

	virtual int Dimension() const = 0;

 protected:
	SvmClassifiable() {}

};


#endif
