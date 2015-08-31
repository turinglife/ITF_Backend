//
// ITF Deep Learning-based segmentation analyzer, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "dp_analyzer_segmentation.hpp"

template <typename Dtype>
bool CDPAnalyzerSegmentation<Dtype>::Init() {
    // Setup Extracter
    itf::SegmenterParameter sp;

    // Read configuration file
    //if (!itf::Util::ReadProtoFromTextFile("./config/fcnn_segmenter.prototxt", &sp)) {
    //    std::cout << "Cannot read .prototxt file!" << std::endl;
    //    return false;
    //}

    itf::CSegmenterFactory sf;
    isegmenter_.reset(sf.SpawnSegmenter(itf::CSegmenterFactory::FCNN));

    isegmenter_->SetParameters("/home/lpzhang/Desktop/ITF_Backend/config/fcnn_segmenter.prototxt");

    return true;
}

template <typename Dtype>
std::vector<Dtype> CDPAnalyzerSegmentation<Dtype>::Analyze(IN cv::Mat frame) {
    cv::Mat foreground, dummy;
    isegmenter_->process(frame, foreground, dummy);

    vector<Dtype> feature;
    feature.assign(reinterpret_cast<Dtype*>(foreground.datastart), reinterpret_cast<Dtype*>(foreground.dataend));
    return feature;
}


INSTANTIATE_MYCLASS(CDPAnalyzerSegmentation);
