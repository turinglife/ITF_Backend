//
// ITF Task Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_TASK_H_
#define ITF_TASK_H_


#include "common.hpp"

#include "regression_camera.hpp"
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
    enum TaskType_t {
        COUNTING,
        SEGMENTATION
    };
    enum TaskStatus_t {     // Task state
        OFF = 0,
        ON
    };
    enum Status_t {      // The state of specific function affiliated to the current task
        TERMINATE = 0,
        RUNNING
    };

    bool InitCapture(const std::string& task_name);
    bool InitAnalyzer(const std::string& task_name);
    bool InitTrainer(const std::string& task_name);

    int Capture(int fps);
    int Analyze();
    int Train(std::string &);

    void ShowDetails();

    bool setTaskStatus(TaskStatus_t status);

    void getCurrentCameraType();

    inline std::string getCurrentTaskName() { return config_.getTaskName(); }
    inline int getCurrentFrameWidth() { return config_.getFrameWidth(); }
    inline int getCurrentFrameHeight() { return config_.getFrameHeight(); }
    inline TaskType_t getCurrentTaskType() { return static_cast<TaskType_t>(config_.getTaskType()); }
    inline int getTaskStatus() { return config_.getTaskStatus(); }

    inline void setFuncStatus(int funcstatus) { funcstatus_ = funcstatus; }
    inline int getFuncStatus() { return funcstatus_; }
    inline void setCameraStatus(int camerastatus) { camerastatus_ = camerastatus; }
    inline int getCameraStatus() { return camerastatus_; }

    bool FreeBuffer();

 private:
    CDbi ConnectDB();
    std::unique_ptr<CCamera> camera_;  // object for grabing frames into buffer.
    std::unique_ptr<CAnalyzer<Dtype> > analyzer_;  // object for analyzing frames from buffer.
    CBuffer buffer_;
    CAlarm *alarmer_;              // object for generating alarm information.
    CConfig config_;               // configuration for the task object.

    int funcstatus_;               // default value is TERMINAL state.
    int camerastatus_;
};

#endif  // ITF_TASK_H_
