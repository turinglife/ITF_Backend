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
    CAnalyzer(const std::string &pmap_path, const std::string &roi_path, const int &framewidth, const int &frameheight);
    virtual ~CAnalyzer();
    virtual std::vector<Dtype> Analyze(IN cv::Mat frame) = 0;
    virtual bool Init() = 0;

 protected:
    std::string buffer_id_;  // Buffer unique id for communicating with other processes.
    int framewidth_ = 0;
    int frameheight_ = 0;
    std::string pers_path_;
    std::string roi_path_;
};


#endif  // ITF_ANALYZER_H
