//
// ITF Deep Learning-based segmentation analyzer, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//


#include "dp_analyzer_segmentation.hpp"



template <typename Dtype>
CDPAnalyzerSegmentation<Dtype>::CDPAnalyzerSegmentation(const std::string &pmap_path, const std::string &roi_path, const int &framewidth, const int &frameheight) : 
                                CDPAnalyzer<Dtype>(pmap_path, roi_path, framewidth, frameheight) {
    
    
}



template <typename Dtype>
CDPAnalyzerSegmentation<Dtype>::~CDPAnalyzerSegmentation() { }


template <typename Dtype>
bool CDPAnalyzerSegmentation<Dtype>::Init()
{

    
    return true;
}



template <typename Dtype>
bool CDPAnalyzerSegmentation<Dtype>::InitNet()
{

    
    return true;
}



template <typename Dtype>
std::vector<Dtype> CDPAnalyzerSegmentation<Dtype>::Analyze(IN cv::Mat frame) {
    
    vector<Dtype> feature;
    
    return feature;
    
}



INSTANTIATE_MYCLASS(CDPAnalyzerSegmentation);

