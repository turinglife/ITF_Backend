//
// ITF Deep Learning-based density analyzer Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_CDPANALYZERDENSITY_H
#define ITF_CDPANALYZERDENSITY_H

#include <itf/extracters/extracter_factory.hpp>
#include <itf/segmenters/segmenter_factory.hpp>
#include "common.hpp"
#include "dp_analyzer.hpp"

//
// Class representing 'Deep Learning-based' density analyzer.
//
template <typename Dtype>
class CDPAnalyzerDensity: public CDPAnalyzer<Dtype> {
 public:
    CDPAnalyzerDensity(const std::string &pmap_path, const std::string &roi_path,
        const int &framewidth, const int &frameheight);
    bool Init();
    std::vector<Dtype> Analyze(IN cv::Mat frame);

 protected:
    std::unique_ptr<itf::IExtracter> iextracter_;
    std::unique_ptr<itf::ISegmenter> fcn_extracter_;
    std::string pmap_path_;
    std::string roi_path_;
    cv::Mat roi_mask_;
    int patch_based_;
};

#endif  // ITF_CDPANALYZERDENSITY_H
