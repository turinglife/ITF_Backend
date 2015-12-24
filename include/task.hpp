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
    //enum TaskStatus_t {     // Task state
    //    OFF = 0,
    //    ON
    //};
    enum Status_t {      // The state of specific function affiliated to the current task
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
    void Alarm();

    void getCurrentCameraType();
    inline std::string getCurrentTaskName() { return config_.getTaskName(); }
    inline int getCurrentFrameWidth() { return config_.getFrameWidth(); }
    inline int getCurrentFrameHeight() { return config_.getFrameHeight(); }
    inline TaskType_t getCurrentTaskType() { return static_cast<TaskType_t>(config_.getTaskType()); }
        
    // these functions are employed to inform AD, CD, RD, MD processes to start or stop current processing procedure 
    // instead of creating or destroying these four processes. 
    // the creating and destroying procedure of AD, CD, RD, MD processes are implemented in ad.cpp, cd.cpp, rd.cpp, md.cpp respectively.
    inline void setAnalyzerStatus(int analyzerstatus) { analyzerstatus_ = analyzerstatus; }
    inline int getAnalyzerStatus() { return analyzerstatus_; }
    inline void setCameraStatus(int camerastatus) { camerastatus_ = camerastatus; }
    inline int getCameraStatus() { return camerastatus_; }
    inline void setTrainerStatus(int trainerstatus) { trainerstatus_ = trainerstatus; }
    inline int getTrainerStatus() { return trainerstatus_; }
    inline void setAlarmerStatus(int alarmerstatus) { alarmerstatus_ = alarmerstatus; }
    inline int getAlarmerStatus() { return alarmerstatus_; }

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
    // these functions are employed to inform AD, CD, RD, MD processes to start or stop current processing procedure
    int analyzerstatus_;
    int camerastatus_;
    int trainerstatus_;
    int alarmerstatus_;

};

#endif  // ITF_TASK_H_
