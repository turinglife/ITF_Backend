//
// ITF Deep Learning-based Analyzer Class, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_CDPANALYZER_H
#define ITF_CDPANALYZER_H


#include "common.hpp"
#include "analyzer.hpp"

#include <itf/extracters/extracter_factory.hpp>
#include <itf/segmenters/segmenter_factory.hpp>
#include <itf/util/Util.hpp>

//
// Class representing 'Deep Learning-based' analyzer.
//
template <typename Dtype>
class CDPAnalyzer : public CAnalyzer<Dtype> {
public:

    CDPAnalyzer(const std::string &pmap_path, const std::string &roi_path, const int &framewidth, const int &frameheight) : CAnalyzer<Dtype>(pmap_path, roi_path, framewidth, frameheight) {}
    virtual ~CDPAnalyzer();
    
    //virtual std::vector<Dtype> Analyze(IN cv::Mat& frame) = 0;
    
    //virtual int Init() = 0;
    //virtual int Analyze() = 0;

protected:
    
    //bool InitNet();
    
    //std::string protocol_;
    //std::string host_;
    //unsigned int port_ = 0;
    //std::string path_;
    //std::string username_;
    //std::string password_;     
    


};


#endif // ITF_CDPANALYZER_H
