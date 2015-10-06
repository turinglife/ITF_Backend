//
// ITF Deep Learning-based segmentation analyzer, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "dp_analyzer_stationary.hpp"

template <typename Dtype>
CDPAnalyzerStationary<Dtype>::CDPAnalyzerStationary(const std::string &mode, const bool &roi_status, const std::string &roi_path, const int &framewidth, const int &frameheight)
    :CDPAnalyzer<Dtype>(framewidth, frameheight) {

    mode_ = mode;
    roi_status_ = roi_status;
    roi_path_ = roi_path;

    optflow_winsize_ = cv::Size(31, 31);
    subpix_winsize_ = cv::Size(10, 10);
    max_count_ = 10000;
    pre_ = 0;
    curr_ = 1;
    need_to_init_ = true;

    track_check_interval_ = 25;
    min_distance_ = 20;

    track_.reset(new Track[max_count_]);
}

template <typename Dtype>
bool CDPAnalyzerStationary<Dtype>::Init() {
    // Setup Extracter
    itf::CSegmenterFactory sf;
    isegmenter_.reset(sf.SpawnSegmenter(itf::CSegmenterFactory::FCNN));
    isegmenter_->SetParameters("./config/fcnn_segmenter.prototxt");

    // Prepare roi_mask_
    if (roi_status_) {
        roi_mask_ = cv::imread(roi_path_, CV_LOAD_IMAGE_GRAYSCALE);
        roi_mask_ = roi_mask_ / 255;
    }

    // Prepare GMM
    update_bg_model_ = true;

    // Prepare tracker
    termcriteria_ = cv::TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03);

    still_map_ = cv::Mat::zeros(this->frameheight_, this->framewidth_, CV_8U);

    return true;
}

template <typename Dtype>
std::vector<Dtype> CDPAnalyzerStationary<Dtype>::Analyze(IN cv::Mat frame) {
    cv::Mat curr_still, second_mask;

    /* Base */
    cv::Mat crowd_mask;
    cv::Mat probs, dummy;
    isegmenter_->process(frame, probs, dummy);
    crowd_mask = probs > 0.5f;
    crowd_mask.convertTo(crowd_mask, CV_8U, 0.004);

    if (std::string(mode_).compare("S") == 0) {
        /* Mode -- "S" */
        curr_still = crowd_mask;
        second_mask = crowd_mask;
    } else if (std::string(mode_).compare("SGS") == 0) {
        /* Mode -- "SGS" */
        cv::Mat fgmask, bgimg;
        bg_model_(frame, fgmask, update_bg_model_ ? -1 : 0);
        bg_model_.getBackgroundImage(bgimg);

        cv::Mat probs, dummy;
        isegmenter_->process(bgimg, probs, dummy);
        second_mask = probs > 0.5f;
        second_mask.convertTo(second_mask, CV_8U, 0.004);

        curr_still = crowd_mask.mul(second_mask);
    } else if (std::string(mode_).compare("SK") == 0) {
        /* Mode -- "SK" */
        cvtColor(frame, gray_, cv::COLOR_BGR2GRAY);
        second_mask = cv::Mat::zeros(frame.rows, frame.cols, CV_8U);
        if (need_to_init_) {
            // Determines strong corners on an image
            const cv::Mat mask;   // Optional ROI
            goodFeaturesToTrack(gray_, points_[curr_], max_count_, 0.01, 10.0, mask, 3, 0, 0.04);
            cornerSubPix(gray_, points_[curr_], subpix_winsize_, cv::Size(-1, -1), termcriteria_);
        } else if (!points_[pre_].empty()) {
            std::vector<uchar> status;
            std::vector<float> err;
            if (pre_gray_.empty())
                gray_.copyTo(pre_gray_);
            // Calculates an optical flow for a sparse feature set using the iterative Lucas-Kanade method with pyramids.
            calcOpticalFlowPyrLK(pre_gray_, gray_, points_[pre_], points_[curr_], status, err, optflow_winsize_, 3, termcriteria_, 0, 0.001);
            for (size_t i=0; i<points_[curr_].size(); ++i) {
                if (!status[i]) {  // if it is not ok, regenerate a point
                    double x = rand() % frame.cols;
                    double y = rand() % frame.rows;
                    points_[curr_][i] = cv::Point2f(x, y);
                    track_[i].history.clear();
                }

                track_[i].history.push_back(points_[curr_][i]);

                if (track_[i].history.size() > track_check_interval_) {
                    if (norm(track_[i].history[track_[i].history.size() - track_check_interval_] - track_[i].history.back()) > min_distance_) {
                        circle(second_mask, track_[i].history.back(), 5, cv::Scalar(1), -1, 8);
                    } else {
                        double x = rand() % frame.cols;
                        double y = rand() % frame.rows;
                        points_[curr_][i] = cv::Point2f(x, y);
                        track_[i].history.clear();
                        track_[i].history.push_back(points_[curr_][i]);
                    }
                }
            }
        }

        std::swap(points_[curr_], points_[pre_]);
        cv::swap(pre_gray_, gray_);
        need_to_init_ = false;

        const cv::Mat dilate_element = getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(15, 15));
        cv::dilate(second_mask, second_mask, dilate_element);

        curr_still = crowd_mask - second_mask;
    }

    // get still_map_
    still_map_ = still_map_ + curr_still - (1-curr_still) * 10;

    if (roi_status_)
        still_map_ = still_map_.mul(roi_mask_);

    // crowd_mask * 20 is used to draw blue mask on walking people
    cv::Mat color_map = DrawColorMap(still_map_ + crowd_mask * 20, frame.rows, frame.cols);

    //cv::imshow("color_map", color_map + frame);
    //cv::waitKey(1);

    vector<Dtype> feature;

    feature.assign(reinterpret_cast<Dtype*>(color_map.datastart), reinterpret_cast<Dtype*>(color_map.dataend));

    return feature;
}

template <typename Dtype>
cv::Mat CDPAnalyzerStationary<Dtype>::DrawColorMap(const cv::Mat& dens_map, int rows, int cols) {
    cv::Mat color_map;
    cv::blur(dens_map, color_map, cv::Size(10, 10));
    cv::applyColorMap(color_map, color_map, cv::COLORMAP_JET);
    color_map = color_map - cv::Mat(rows, cols, CV_8UC3, cv::Scalar(128, 0, 0));

    return color_map;
}


INSTANTIATE_MYCLASS(CDPAnalyzerStationary);
