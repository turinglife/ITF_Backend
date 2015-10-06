//
// ITF Deep Learning-based stationary analyzer Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_CDPANALYZERSTATIONARY_H
#define ITF_CDPANALYZERSTATIONARY_H

#include <opencv2/contrib/contrib.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/video/tracking.hpp>

#include <algorithm>

#include <itf/segmenters/segmenter_factory.hpp>

#include "common.hpp"
#include "dp_analyzer.hpp"

//
// Class representing 'Deep Learning-based' stationary analyzer.
//
struct Track {
    std::vector<cv::Point2f> history;
};

template <typename Dtype>
class CDPAnalyzerStationary : public CDPAnalyzer<Dtype> {
    public:
        CDPAnalyzerStationary(const std::string &mode, const bool &roi_status, const std::string &roi_path,
                const int &framewidth, const int &frameheight);

        bool Init();
        std::vector<Dtype> Analyze(IN cv::Mat frame);

    private:
        cv::Mat DrawColorMap(const cv::Mat& dens_map, int rows, int cols);

        std::string mode_;
        bool roi_status_;
        std::string roi_path_;

        std::unique_ptr<itf::ISegmenter> isegmenter_;

        cv::Mat roi_mask_;
        cv::Mat still_map_;

        cv::BackgroundSubtractorMOG2 bg_model_;
        bool update_bg_model_;

        cv::TermCriteria termcriteria_;
        cv::Size optflow_winsize_;
        cv::Size subpix_winsize_;
        int max_count_;
        int pre_;
        int curr_;
        bool need_to_init_;

        cv::Mat pre_gray_;
        cv::Mat gray_;
        std::vector<cv::Point2f> points_[2];
        //Track track_[10000];
        std::unique_ptr<Track[]> track_;

        int track_check_interval_;
        double min_distance_;
};

#endif  // ITF_CDPANALYZERSTATIONARY_H
