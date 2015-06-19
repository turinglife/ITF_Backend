//
// ITF Analyzer Class, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//


#include "analyzer.hpp"

template <typename Dtype>
CAnalyzer<Dtype>::CAnalyzer(const std::string &pmap_path, const std::string &roi_path, const int &framewidth, const int &frameheight) {
    pers_path_ = pmap_path; 
    roi_path_ = roi_path; 
    framewidth_ = framewidth; 
    frameheight_ = frameheight; 
}


template <typename Dtype>
CAnalyzer<Dtype>::~CAnalyzer() {

    
    
}



INSTANTIATE_MYCLASS(CAnalyzer);