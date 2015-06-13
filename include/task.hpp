//
// ITF Task Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_TASK_H
#define ITF_TASK_H

#include <string>

#include "common.hpp"

#include "SQLiteCpp.h"

#include "remote_camera_http.hpp"
#include "remote_camera_rtsp.hpp"
#include "local_camera.hpp"
#include "file_camera.hpp"
#include "camera.hpp"

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
    int Capture(cv::Mat& frame);
    std::thread Analyze(FunType_t ft, std::string path);
    void Do_Count(std::string path);
    void Do_Segment(std::string path);
    void ShowDetails();

    inline std::string task_name() const { return task_name_; }

    bool on = false;

 private:
    CCamera *camera_;

    std::string task_name_;
    int type_ = 0;
    int width_ = 0;
    int height_ = 0;
    std::string address_;
    unsigned int port_ = 0;
    std::string host_;
    std::string username_;
    std::string password_;

    int action_;  // capture, segmentation, counting

    typedef struct {
        struct timeval *timestamp;
        cv::Mat *frame;
    }Snapshot_t;
    Snapshot_t snapshot;
};


#endif  // ITF_TASK_H
