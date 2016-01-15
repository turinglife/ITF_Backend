//
// ITF Deep Learning-based Cross-Line analyzer Class Interfaces
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_CDPANALYZERCROSSLINE_H_
#define ITF_CDPANALYZERCROSSLINE_H_

#include <itf/crossline/Crossline.hpp>
#include "common.hpp"
#include "dp_analyzer.hpp"
//
// Class representing 'Deep Learning-based' Cross-Line analyzer.
//
template <typename Dtype>
class CDPAnalyzerCrossLine : public CDPAnalyzer<Dtype> {
 public:
    CDPAnalyzerCrossLine(const cv::Rect& roi,
        const cv::Point2i& p1, const cv::Point2i& p2);
    bool Init();
    std::vector<Dtype> Analyze(IN cv::Mat frame);

 private:
    itf::Crossline cl_;
    cv::Rect roi_;
    cv::Mat pmap_;
    cv::Mat nextImg_;
    cv::Mat prevImg_;
    cv::Point2i p1_;
    cv::Point2i p2_;
};

#endif  // ITF_CDPANALYZERCROSSLINE_H_
