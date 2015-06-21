//
// ITF Deep Learning-based segmentation analyzer Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_CDPANALYZERSEGMENTATION_H
#define ITF_CDPANALYZERSEGMENTATION_H


#include "common.hpp"
#include "dp_analyzer.hpp"

//
// Class representing 'Deep Learning-based' segmentation analyzer.
//
template <typename Dtype>
class CDPAnalyzerSegmentation : public CDPAnalyzer<Dtype> {
 public:
    CDPAnalyzerSegmentation(const std::string &pmap_path, const std::string &roi_path, const int &framewidth, const int &frameheight);
    ~CDPAnalyzerSegmentation();
    
    bool Init();                                      
    std::vector<Dtype> Analyze(IN cv::Mat frame);

 protected:
    bool InitNet();

    itf::ISegmenter *isegmenter_;
};


#endif // ITF_CDPANALYZERSEGMENTATION_H
