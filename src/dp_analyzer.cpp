//
// ITF Deep Learning-based Analyzer Class, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//


#include "dp_analyzer.hpp"


template <typename Dtype>
CDPAnalyzer<Dtype>::CDPAnalyzer() { }

template <typename Dtype>
CDPAnalyzer<Dtype>::CDPAnalyzer(int framewidth, int frameheight) {
    framewidth_ = framewidth;
    frameheight_ = frameheight;
}


INSTANTIATE_MYCLASS(CDPAnalyzer);
