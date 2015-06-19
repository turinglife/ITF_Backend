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
class CDNPAnalyzer : public CAnalyzer<Dtype> {
public:
    //CDPAnalyzer(const std::string &p_protocol, const std::string &p_host, unsigned int p_port, const std::string &p_path, const std::string& p_username, const std::string& p_password);
    CDNPAnalyzer(const std::string &pmap_path, const std::string &roi_path, const int &framewidth, const int &frameheight) : CAnalyzer<Dtype>(pmap_path, roi_path, framewidth, frameheight) {}
    virtual ~CDNPAnalyzer();
    
    //virtual int Init() = 0;
    //virtual int Analyze() = 0;

protected:
    //std::string protocol_;
    //std::string host_;
    //unsigned int port_ = 0;
    //std::string path_;
    //std::string username_;
    //std::string password_;      

};


#endif // ITF_CNDPANALYZER_H
