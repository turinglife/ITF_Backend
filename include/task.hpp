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
#include "dp_analyzer_stationary.hpp"
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
        SEGMENTATION,
        STATIONARY
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
    // object for grabing frames into buffer.
    std::unique_ptr<CCamera> camera_;
    // object for analyzing frames from buffer.
    std::unique_ptr<CAnalyzer<Dtype> > analyzer_;
    CBuffer buffer_;
    // object for generating alarm information.
    CAlarm alarmer_;
    // configuration for the task object.
    CConfig config_;

    // default value is TERMINAL state.
    int funcstatus_;
    int camerastatus_;

    cv::Mat frame_;
    cv::Mat dst_;

    /**
     *
     * @brief Write predicted number into MySQL db.
     * @param interval Specifiy how often (seconds) to write to disk.
     *
     */

    void record(int interval);
};

#endif  // ITF_TASK_H_
