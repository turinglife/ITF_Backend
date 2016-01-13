//
// ITF Task Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_TASK_H_
#define ITF_TASK_H_

#include "dbi.hpp"
#include "camera.hpp"
#include "analyzer.hpp"
#include "buffer.hpp"
#include "alarm.hpp"
#include "config.hpp"
#include "remote_camera_http.hpp"
#include "file_camera.hpp"
#include "dp_analyzer_density.hpp"
#include "dp_analyzer_segmentation.hpp"
#include "dp_analyzer_stationary.hpp"
#include "dp_analyzer_crossline.hpp"
#include "common.hpp"

template <typename Dtype>
class CTask {
 public:
    enum TaskType_t {
        COUNTING,
        SEGMENTATION,
        STATIONARY,
        CROSSLINE
    };
    // The state of specific function affiliated to the current task
    enum Status_t {
        TERMINATE = 0,
        RUNNING
    };

    bool InitCapturer(const std::string& task_name);
    bool InitAnalyzer(const std::string& task_name);
    bool InitTrainer(const std::string& task_name);
    bool InitAlarmer(const std::string& task_name);

    bool DestroyCapturer();
    bool DestroyAnalyzer();
    bool DestroyTrainer();
    bool DestroyAlarmer();

    void Capture(int fps);
    void Analyze();
    void Train(std::string filename);
    void Alarm(int interval);

    inline std::string getTaskName() { return config_.getTaskName(); }
    inline int getFrameWidth() { return config_.getFrameWidth(); }
    inline int getFrameHeight() { return config_.getFrameHeight(); }
    inline TaskType_t getTaskType() { return static_cast<TaskType_t>(config_.getTaskType()); }
    // Each task object is associate with one specific daemon process.
    // And each daemon process (i.e. CD, AD, MD and RD) is under the control of state
    // Since daemon processes are independent from each other, we can rely on just one state variable.
    inline void setState(int state) { state_ = state; }
    inline int getState() { return state_; }

 private:
    CDbi ConnectDB();
    void Counting();
    void Segmentation();
    void Stationary();
    void CrossLine();
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
    int state_;
};

#endif  // ITF_TASK_H_
