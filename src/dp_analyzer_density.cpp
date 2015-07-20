//
// ITF Deep Learning-based density analyzer, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "dp_analyzer_density.hpp"


template <typename Dtype>
CDPAnalyzerDensity<Dtype>::CDPAnalyzerDensity(const std::string &pmap_path, const std::string &roi_path, const int &framewidth, const int &frameheight) : CDPAnalyzer<Dtype>(framewidth, frameheight) {
    pmap_path_ = pmap_path;
    roi_path_ = roi_path;
}

template <typename Dtype>
CDPAnalyzerDensity<Dtype>::~CDPAnalyzerDensity() { delete iextracter_; }

template <typename Dtype>
bool CDPAnalyzerDensity<Dtype>::Init() {
    cv::Mat pmap;
    // Setup Extracter
    itf::ExtracterParameter ep;
    // Read configuration file
    if (!itf::Util::ReadProtoFromTextFile("./config/density_extracter.prototxt", &ep)) {
        std::cout << "Cannot read .prototxt file!" << std::endl;
        return false;
    }

    // Factory instantiates an object of the specific type of extracter
    itf::CExtracterFactory ef;
    iextracter_ = ef.SpawnExtracter(itf::Density);
    iextracter_->SetExtracterParameters(ep);

    iextracter_->SetImagesDim(this->frameheight_, this->framewidth_);
    iextracter_->LoadPerspectiveMap(pmap_path_, &pmap);
    iextracter_->LoadROI(roi_path_);

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
