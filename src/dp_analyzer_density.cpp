//
// ITF Deep Learning-based density analyzer, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//


#include "dp_analyzer_density.hpp"

#include <string>
#include <vector>


template <typename Dtype>
CDPAnalyzerDensity<Dtype>::CDPAnalyzerDensity(const std::string &pmap_path, const std::string &roi_path, const int &framewidth, const int &frameheight) :
                    CDPAnalyzer<Dtype>(pmap_path, roi_path, framewidth, frameheight) { }

template <typename Dtype>
CDPAnalyzerDensity<Dtype>::~CDPAnalyzerDensity() { delete iextracter_; }

template <typename Dtype>
bool CDPAnalyzerDensity<Dtype>::Init() {
    if (!InitNet()) {
        std::cout << "Density Analyzer init failed! " << std::endl;
        return false;
    }
    return true;
}

template <typename Dtype>
bool CDPAnalyzerDensity<Dtype>::InitNet() {
    cv::Mat pmap;
    // Setup Extracter
    itf::ExtracterParameter ep;
    // Read configuration file
    if (!itf::Util::ReadProtoFromTextFile("/home/itf/Desktop/ITF_Integrated/lib/config/density_extracter.prototxt", &ep)) {
        std::cout << "Cannot read .prototxt file!" << std::endl;
        return false;
    }

    // Factory instantiates an object of the specific type of extracter
    itf::CExtracterFactory ef;
    iextracter_ = ef.SpawnExtracter(itf::Density);
    iextracter_->SetExtracterParameters(ep);

    iextracter_->SetImagesDim(this->frameheight_, this->framewidth_);
    iextracter_->LoadPerspectiveMap(this->pers_path_, &pmap);
    iextracter_->LoadROI(this->roi_path_);

    return true;
}

template <typename Dtype>
std::vector<Dtype> CDPAnalyzerDensity<Dtype>::Analyze(IN cv::Mat frame) {
    iextracter_->LoadImages(frame, frame);

    // Extract density feature from a frame loaded above
    std::vector<float> feature = iextracter_->ExtractFeatures();

    return feature;
}



INSTANTIATE_MYCLASS(CDPAnalyzerDensity);