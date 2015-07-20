//
// ITF Conventional Analyzer Class, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_CNDPANALYZER_H
#define ITF_CNDPANALYZER_H


#include "common.hpp"
#include "analyzer.hpp"

//
// Class representing 'Conventional' analyzer.
//
template <typename Dtype>
class CNDPAnalyzer : public CAnalyzer<Dtype> {
public:
    CNDPAnalyzer();
};

#endif // ITF_CNDPANALYZER_H
