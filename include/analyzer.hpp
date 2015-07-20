//
// ITF Analyzer Class, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_ANALYZER_H
#define ITF_ANALYZER_H

#include <string>
#include <vector>

#include "common.hpp"

//
// Abstract base class for analyzers. This is intended to express common attributes and interfaces
//
template <typename Dtype>
class CAnalyzer {
 public:
    // Make destructor virtual if  the class has virtual methods
    virtual ~CAnalyzer() {};

    virtual std::vector<Dtype> Analyze(IN cv::Mat frame) = 0;
    virtual bool Init() = 0;
};

#endif  // ITF_ANALYZER_H
