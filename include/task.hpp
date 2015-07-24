//
// ITF Task Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_TASK_H
#define ITF_TASK_H


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

#include "buffer.hpp"
#include "alarm.hpp"
#include "dbi.hpp"
#include "config.hpp"


template <typename Dtype>
class CTask {
 public:
    enum CameraType_t {
        HTTP,
        RTSP,
        LOCAL,
        FILE
    };
    enum TaskType_t {
        DENSITY,
        SEGMENTATION
    };
    enum FunType_t {
        COUNT,
        SEGMENT
    };
    enum TaskStatus_t {     // Task state
        STOP = 0,
        START
    };
    enum Status_t {      // The state of specific function affiliated to the current task
        TERMINATE = 0,
        RUNNING
    };

    bool LoadTask(const std::string& task_name);
    
    bool InitCapture();
    bool InitAnalyzer();
    
    int Capture(int fps);
    int Analyze();
    
    void ShowDetails();

    bool setTaskStatus(TaskStatus_t status);

    void getCurrentCameraType();

    inline std::string getCurrentTaskName() { return config_.getTaskName(); }
    inline int getCurrentFrameWidth() { return config_.getFrameWidth(); }
    inline int getCurrentFrameHeight() { return config_.getFrameHeight(); }
    inline TaskType_t getCurrentTaskType() { return static_cast<TaskType_t>(config_.getTaskType()); };
    inline int getTaskStatus() { return config_.getTaskStatus(); }
    
    inline void setFuncStatus(int funcstatus) { funcstatus_ = funcstatus; }
    inline int getFuncStatus() { return funcstatus_; }
    inline void setCameraStatus(int camerastatus) { camerastatus_ = camerastatus; }
    inline int getCameraStatus() { return camerastatus_; }

 private:
    std::unique_ptr<CCamera> camera_;  // object for grabing frames into buffer.
    std::unique_ptr<CAnalyzer<Dtype> > analyzer_;  // object for analyzing frames from buffer.
    CAlarm *alarmer_;              // object for generating alarm information.
    CConfig config_;               // configuration for the task object.

    int funcstatus_;               // default value is TERMINAL state.
    int camerastatus_;
};

#endif  // ITF_TASK_H
