//
// ITF Task Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_TASK_H
#define ITF_TASK_H


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

    CTask();
    ~CTask();
    bool LoadTask(const std::string& task_name, const std::string& db_name);
    int Capture(cv::Mat& frame);
    int Analyze();
    void ShowDetails();

    inline std::string task_name() const { return task_name_; }

private:
    //typedef struct {
    //    unsigned int size;
    //    unsigned int last_write_index;
    //    unsigned int last_read_index;
    //} BufferHead;

    //int shm_id;
    //BufferHead *pbufferhead_;

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



#endif // ITF_TASK_H
