//
// ITF Deep Learning-based segmentation analyzer, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "dp_analyzer_crossline.hpp"

template <typename Dtype>
CDPAnalyzerCrossLine<Dtype>::CDPAnalyzerCrossLine(const cv::Rect& roi,
        const cv::Point2i& p1, const cv::Point2i& p2) {
    roi_ = roi;
    p1_ = p1;
    p2_ = p2;
}

template <typename Dtype>
bool CDPAnalyzerCrossLine<Dtype>::Init() {
    std::string home_path(std::getenv("HOME"));
    std::string path = home_path + "/ITF_SmartClient/config/";
    itf::ExtracterParameter ep;
    // Read configuration file
    std::string protofile = path + "crossline_extracter.prototxt";
    if (!itf::Util::ReadProtoFromTextFile(protofile.c_str(), &ep)) {
        std::cout << "Cannot read .prototxt file!" << std::endl;
        return false;
    }
    itf::Util util;
    pmap_ = util.ReadPMAPtoMAT("tmp_pers.csv");
    // Initialize crossline object
    cl_.Init(ep.model(), ep.prototxt(), pmap_(roi_),
        abs(roi_.tl().y - roi_.br().y), abs(roi_.br().x - roi_.tl().x));
    return true;
}

template <typename Dtype>
std::vector<Dtype> CDPAnalyzerCrossLine<Dtype>::Analyze(IN cv::Mat frame) {
    frame.copyTo(nextImg_);
    nextImg_ = nextImg_(roi_);
    if (prevImg_.empty()) {
        nextImg_.copyTo(prevImg_);
    }
    float* density_feature;
    float* x_feature;
    float* y_feature;
    // Extract features
    cl_.Process(prevImg_, nextImg_, &density_feature, &x_feature, &y_feature);
    // Calculate predicted values (two directions)
    std::vector<float> predicted = cl_.CalcPredict(p1_, p2_,
        density_feature, x_feature, y_feature);
    cv::swap(prevImg_, nextImg_);
    // Get flow map and store as a vector
    cv::Mat flow_map = cl_.VisualizeFlow(x_feature, y_feature);
    vector<Dtype> flow_vec(reinterpret_cast<Dtype*>(flow_map.datastart),
        reinterpret_cast<Dtype*>(flow_map.dataend));
    // Get density map and store as a vector
    cv::Mat density_map = cl_.VisualizeDensity(density_feature);
    vector<Dtype> density_vec(reinterpret_cast<Dtype*>(density_map.datastart),
        reinterpret_cast<Dtype*>(density_map.dataend));
    // Concate vectors
    std::vector<Dtype> feature;
    feature.insert(feature.end(), flow_vec.begin(), flow_vec.end());
    feature.insert(feature.end(), density_vec.begin(), density_vec.end());
    // Add predicted values at end of the vector
    feature.push_back(predicted[0]);
    feature.push_back(predicted[1]);

    return feature;
}

INSTANTIATE_MYCLASS(CDPAnalyzerCrossLine);
