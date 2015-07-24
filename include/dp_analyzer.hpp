//
// ITF Deep Learning-based Analyzer Class, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_CDPANALYZER_H
#define ITF_CDPANALYZER_H

#include <itf/util/Util.hpp>

#include "analyzer.hpp"
#include "common.hpp"

//
// Class representing 'Deep Learning-based' analyzer.
//
template <typename Dtype>
class CDPAnalyzer : public CAnalyzer<Dtype> {
 public:
    CDPAnalyzer();
    CDPAnalyzer(int framewidth, int frameheight);

 protected:
    int framewidth_ = 0;
    int frameheight_ = 0;
};


#endif  // ITF_CDPANALYZER_H
