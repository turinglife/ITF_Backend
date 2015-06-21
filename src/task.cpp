//
// ITF Task Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "task.hpp"

#include <string>
#include <vector>

template <typename Dtype>
CTask<Dtype>::CTask() : camera_(), analyzer_(), alarmer_(), config_() { }

template <typename Dtype>
CTask<Dtype>::~CTask() {
    // Free camera and analyzer objects
    delete camera_;
    delete analyzer_;
}

template <typename Dtype>
bool CTask<Dtype>::LoadTask(const std::string& task_name, const std::string& db_name) {
    // Retrieve information from database according to task id.
    try {
        SQLite::Database db(db_name);

        SQLite::Statement query(db, "SELECT * FROM Tasks WHERE task_name=?");
        query.bind(1, task_name);

        bool has_records = false;
        // Init a task object with appropriate parameters.
        while (query.executeStep()) {
            config_.setTaskName(task_name);
            config_.setTaskType(query.getColumn(1).getInt());
            config_.setCameraType(query.getColumn(2).getInt());
            config_.setFrameWidth(query.getColumn(3).getInt());
            config_.setFrameHeight(query.getColumn(4).getInt());
            config_.setIPAddress(query.getColumn(5).getText());
            config_.setPort(query.getColumn(6).getInt());
            config_.setHost(query.getColumn(7).getText());
            config_.setUsername(query.getColumn(8).getText());
            config_.setPassword(query.getColumn(9).getText());
            config_.setPmapPath(query.getColumn(10).getText());
            config_.setROIPath(query.getColumn(11).getText());

            has_records = true;
        }
        if (!has_records) {
            std::cout << "No Such Task" << std::endl;
            return false;
        }
    } catch (std::exception& e) {
        std::cout << "exception: " << e.what() << std::endl;
        return false;
    }

    return true;
}


template <typename Dtype>
bool CTask<Dtype>::InitCapture() {
    // Instantiate a concrete camera.
    CameraType_t CameraType = static_cast<CameraType_t>(config_.getCameraType());

    if (CameraType == REMOTE_CAMERA_HTTP) {
        // RemoteCameraHttp
        camera_ = new CRemoteCameraHttp(config_.getHost(), config_.getPort(), config_.getIPAddress(), config_.getUsername(), config_.getPassword());
        if (!camera_->Connect()) {
            std::cout << "Camera Connect Fail" << std::endl;
            return false;
        }
        std::cout << getpid() << ": REMOTE_CAMERA_HTTP is initialized" << std::endl;

    } else if (CameraType == REMOTE_CAMERA_RTSP) {
        // RemoteCameraRtsp
        std::cout << "To Be Continued" << std::endl;
        return false;
    } else if (CameraType == LOCAL_CAMERA) {
        // LocalCamera
        std::cout << "To Be Continued" << std::endl;
        return false;
    } else if (CameraType == FILE_CAMERA) {
        // FileCamera
        camera_ = new CFileCamera(config_.getIPAddress());
        if (!camera_->Connect()) {
            std::cout << "Camera Connect Fail" << std::endl;
            return false;
        }

        std::cout << getpid() << ": FILE_CAMERA is initialized" << std::endl;
    } else {
        // not supporting.
        std::cout << "To Be Continued" << std::endl;

        return false;
    }
    return true;
}

template <typename Dtype>
bool CTask<Dtype>::InitAnalyzer() {
    // Instantiate a concrete analyzer.
    FunType_t FunType = static_cast<FunType_t>(config_.getTaskType());

    if (FunType == COUNT) {
        // Instantiate Counting analyzer
        analyzer_ = new CDPAnalyzerDensity<Dtype>(config_.getPmapPath(), config_.getROIPath(), config_.getFrameWidth(), config_.getFrameHeight());

        std::cout << getpid() << ": CDPAnalyzerDensity is initialized" << std::endl;

        analyzer_->Init();
    } else if (FunType == SEGMENT) {
        // Segmentation
        // Note: All parameters in constructor of CDPAnalyzerSegmentation is no needed
        analyzer_ = new CDPAnalyzerSegmentation<Dtype>(config_.getPmapPath(), config_.getROIPath(), config_.getFrameWidth(), config_.getFrameHeight());

        std::cout << getpid() << ": CDPAnalyzerSegmentation is initialized" << std::endl;

        analyzer_->Init();
    } else {
        // not supporting.
        std::cout << "To Be Continued" << std::endl;

        return false;
    }

    return true;
}

template <typename Dtype>
int CTask<Dtype>::Capture(cv::Mat& frame) {
    // check if camera is initialized already
    if (camera_ == 0) {
        std::cout << "The camera has not been initialized yet." << std::endl;
        return 0;
    }
    camera_->Capture(frame);

    return 1;
}

template <typename Dtype>
void CTask<Dtype>::ShowDetails() {
    std::cout << std::endl;
    std::cout << getpid() << ": Task Info ------" << std::endl;
    std::cout << " Task Name: " << config_.getTaskName() << std::endl;
    std::cout << " Task Type: " << config_.getTaskType() << std::endl;
    std::cout << " Camera Type: " << config_.getCameraType() << std::endl;
    std::cout << " Width: " << config_.getFrameWidth() << std::endl;
    std::cout << " Height: " << config_.getFrameHeight() << std::endl;
    std::cout << " Address: " << config_.getIPAddress() << std::endl;
    std::cout << " Port: " << config_.getPort() << std::endl;
    std::cout << " Host: " << config_.getHost() << std::endl;
    std::cout << " User: " << config_.getUsername() << std::endl;
    std::cout << " Password: " << config_.getPassword() << std::endl;
    std::cout << std::endl;
}

template <typename Dtype>
std::vector<Dtype> CTask<Dtype>::Analyze(cv::Mat& frame) {
    vector<Dtype> feature;
    // check if camera is initialized already
    if (analyzer_ == 0) {
        std::cout << "The camera has not been initialized yet." << std::endl;

        return feature;
    }

    feature = analyzer_->Analyze(frame);

    return feature;
}

INSTANTIATE_MYCLASS(CTask);
