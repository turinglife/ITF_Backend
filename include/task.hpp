//
// ITF Task Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_TASK_H
#define ITF_TASK_H

#include <itf/extracters/extracter_factory.hpp>
#include <itf/segmenters/segmenter_factory.hpp>
#include <itf/util/Util.hpp>

#include <string>
#include <vector>

#include "common.hpp"

#include "remote_camera_http.hpp"
#include "remote_camera_rtsp.hpp"
#include "local_camera.hpp"
#include "file_camera.hpp"
#include "camera.hpp"

#include "ndp_analyzer_tracking.hpp"
#include "dp_analyzer_segmentation.hpp"
#include "dp_analyzer_density.hpp"
#include "ndp_analyzer.hpp"
#include "dp_analyzer.hpp"
#include "analyzer.hpp"

#include "alarm.hpp"

#include "config.hpp"


template <typename Dtype>
class CTask {
 public:
    enum CameraType_t {
        REMOTE_CAMERA_HTTP,
        REMOTE_CAMERA_RTSP,
        LOCAL_CAMERA,
        FILE_CAMERA
    };
    enum FunType_t {
        COUNT,
        SEGMENT
    };

    CTask();
    ~CTask();

    bool LoadTask(const std::string& task_name, const std::string& db_name);
    bool InitCapture();
    int Capture(OUT cv::Mat& frame);
    bool InitAnalyzer();
    std::vector<Dtype> Analyze(IN cv::Mat& frame);
    void ShowDetails();
    inline std::string getCurrentTaskName() { return config_.getTaskName(); }
    void getCurrentTaskType();
    void getCurrentCameraType();
    inline int getCurrentFrameWidth() { return config_.getFrameWidth(); }
    inline int getCurrentFrameHeight() { return config_.getFrameHeight(); }

    bool on = false;

 private:
    CCamera *camera_;              // object for grabing frames into buffer.
    CAnalyzer<Dtype> *analyzer_;   // object for analyzing frames from buffer.
    CAlarm *alarmer_;              // object for generating alarm information.
    CConfig config_;               // configuration for the task object
};

#endif  // ITF_TASK_H
