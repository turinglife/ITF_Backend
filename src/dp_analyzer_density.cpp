//
// ITF Deep Learning-based density analyzer, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "dp_analyzer_density.hpp"

template <typename Dtype>
CDPAnalyzerDensity<Dtype>::CDPAnalyzerDensity(const std::string &roi_path, int framewidth, int frameheight)
        : CDPAnalyzer<Dtype>(framewidth, frameheight) {
    patch_based_ = 0;
    roi_path_ = roi_path;
}

template <typename Dtype>
CDPAnalyzerDensity<Dtype>::CDPAnalyzerDensity(const std::string &pmap_path, const std::string &roi_path, const int &framewidth, const int &frameheight) : CDPAnalyzer<Dtype>(framewidth, frameheight) {
    patch_based_ = 1;
    pmap_path_ = pmap_path;
    roi_path_ = roi_path;
}

template <typename Dtype>
bool CDPAnalyzerDensity<Dtype>::Init() {
    std::string home_path(std::getenv("HOME"));
    std::string path = home_path + "/ITF_SmartClient/config/";
    itf::Util util;
    if (patch_based_) {
        std::string protofile = path + "density_extracter.prototxt";
        // Setup Extracter
        itf::ExtracterParameter ep;
        // Read configuration file
        if (!itf::Util::ReadProtoFromTextFile(protofile.c_str(), &ep)) {
            std::cout << "Cannot read .prototxt file!" << std::endl;
            return false;
        }
        // Factory instantiates an object of the specific type of extracter
        itf::CExtracterFactory ef;
        iextracter_.reset(ef.SpawnExtracter(itf::CExtracterFactory::Density));
        iextracter_->SetExtracterParameters(ep);
        iextracter_->SetImagesDim(this->frameheight_, this->framewidth_);
        std::vector<std::pair<float, float> > pair_vec = util.ReadPairToVec(pmap_path_);
        std::string pmap_path = "tmp_pers.csv";  // We need to think of a way to get a real path to the perspective map, now I just fake it.
        util.GeneratePerspectiveMap(pair_vec, this->frameheight_, this->framewidth_, pmap_path);
        iextracter_->LoadPerspectiveMap(pmap_path);
        iextracter_->LoadROI(roi_path_);
        return true;
    } else {
        std::string protofile = path + "vgg_density_extracter.prototxt";
        itf::CSegmenterFactory sf;
        fcn_extracter_.reset(sf.SpawnSegmenter(itf::CSegmenterFactory::FCNN));
        fcn_extracter_->SetParameters(protofile);
        roi_mask_ = util.ReadROItoMAT(roi_path_, this->frameheight_, this->framewidth_);
        return true;
    }
}

template <typename Dtype>
std::vector<Dtype> CDPAnalyzerDensity<Dtype>::Analyze(IN cv::Mat frame) {
    // Extract density feature from a frame loaded above
    std::vector<float> feature;
    if (patch_based_) {
        feature = iextracter_->ExtractFeatures(frame);
    } else {
        cv::Mat foreground, dummy;
        fcn_extracter_->process(frame, foreground, dummy);
        foreground = foreground.mul(roi_mask_);
        // CUHK_density_v3.caffemodel is trained with 10000x response of ground-truth
        foreground *= 0.0001;
        feature.assign(reinterpret_cast<Dtype*>(foreground.datastart), reinterpret_cast<Dtype*>(foreground.dataend));
    }
    return feature;
}

INSTANTIATE_MYCLASS(CDPAnalyzerDensity);
