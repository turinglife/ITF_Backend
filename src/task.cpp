//
// ITF Task Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "task.hpp"

#include <string>
#include <map>
#include <vector>

#include "dbi.hpp"

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
     /* Prepare DB Infomation*/
    const std::string server = "localhost";
    const std::string user = "itf";
    const std::string pass = "itf";

    CDbi db;

    if (!db.Connect(server, user, pass)) {
        std::cout << "Fail" << std::endl;
        return false;
    }

    if (!db.UseDB(db_name)) {
        std::cout << "Fail" << std::endl;
        return false;
    }

    const std::string sql = "select * from (select Tasks.*,TaskDetail.host, TaskDetail.port, TaskDetail.username, TaskDetail.password from Tasks left join TaskDetail on Tasks.task_name=TaskDetail.task_name) as temp where task_name='"+task_name+"';";
    std::vector<std::map<std::string, std::string> > res = db.Query(sql);

   if (res.size() != 1) {
        std::cout << "No Such Task" << std::endl;
       return false;
   } else {
        config_.setTaskName(task_name);
        if (res[0]["task_type"].compare("DENSITY") == 0) {
            config_.setTaskType(0);
        } else if (res[0]["task_type"].compare("SEGMENTATION") == 0) {
            config_.setTaskType(1);
        }
        if (res[0]["camera_type"].compare("HTTP") == 0) {
            config_.setCameraType(0);
            config_.setPort(std::atoi(res[0]["port"].c_str()));
            config_.setHost(res[0]["host"]);
            config_.setUsername(res[0]["username"]);
            config_.setPassword(res[0]["password"]);
        } else if (res[0]["campera_type"].compare("RTSP") == 0) {
            config_.setCameraType(1);
        } else if (res[0]["campera_type"].compare("LOCAL") == 0) {
            config_.setCameraType(2);
        } else if (res[0]["campera_type"].compare("FILE") == 0) {
            config_.setCameraType(3);
        }
        config_.setFrameWidth(std::atoi(res[0]["width"].c_str()));
        config_.setFrameHeight(std::atoi(res[0]["height"].c_str()));
        config_.setIPAddress(res[0]["address"]);
        config_.setPmapPath(res[0]["task_path"] + res[0]["pers_dir"] + res[0]["pers_file"]);
        config_.setROIPath(res[0]["task_path"] + res[0]["roi_dir"] + res[0]["roi_file"]);
        return true;
    }
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
