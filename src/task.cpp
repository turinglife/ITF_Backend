//
// ITF Task Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "task.hpp"


template <typename Dtype>
CTask<Dtype>::CTask() { }

template <typename Dtype>
CTask<Dtype>::~CTask() {
    // Free camera and analyzer objects
    delete camera_;
    delete analyzer_;
}

template <typename Dtype>
bool CTask<Dtype>::LoadTask(const std::string& task_name) {
     /* Prepare DB Infomation*/
    const std::string server = "localhost";
    const std::string user = "itf";
    const std::string pass = "itf";
    const std::string db_name = "ITF";

    CDbi db;

    if (!db.Connect(server, user, pass)) {
        std::cout << "Fail to connect mysql" << std::endl;
        return false;
    }

    if (!db.UseDB(db_name)) {
        std::cout << "Fail to use database" << std::endl;
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
    // camera_ cannot be initialzed twice or deleted if it is alreaday initialized
    if (camera_ != 0) {
        std::cerr << "Camera is already initialized" << std::endl;
        return false;
    }
    // Instantiate a concrete camera.
    CameraType_t CameraType = static_cast<CameraType_t>(config_.getCameraType());

    if (CameraType == HTTP) {
        // RemoteCameraHttp
        camera_ = new CRemoteCameraHttp(config_.getHost(), config_.getPort(), config_.getIPAddress(), config_.getUsername(), config_.getPassword());
    } else if (CameraType == FILE) {
        // FileCamera
        camera_ = new CFileCamera(config_.getIPAddress());
    } else {
        // not supporting.
        std::cout << "To Be Continued" << std::endl;
        return false;
    }
    return camera_->Connect();
}

template <typename Dtype>
bool CTask<Dtype>::InitAnalyzer() {
    // analyzer_ cannot be initialzed twice or deleted if it is alreaday initialized
    if (analyzer_ != 0) {
        std::cerr << "Analyer is alrady initialized" << std::endl;
        return false;
    }
    // Instantiate a concrete analyzer.
    FunType_t FunType = static_cast<FunType_t>(config_.getTaskType());

    if (FunType == COUNT) {
        // Instantiate Counting analyzer
        analyzer_ = new CDPAnalyzerDensity<Dtype>(config_.getPmapPath(), config_.getROIPath(), config_.getFrameWidth(), config_.getFrameHeight());
    } else if (FunType == SEGMENT) {
        // Segmentation
        analyzer_ = new CDPAnalyzerSegmentation<Dtype>();
    } else {
        // not supporting.
        std::cout << "To Be Continued" << std::endl;
        return false;
    }
    return analyzer_->Init();
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
std::vector<Dtype> CTask<Dtype>::Analyze(const cv::Mat& frame) {
    // check if camera is initialized already
    if (analyzer_ == 0) {
        std::cerr << "The camera has not been initialized yet." << std::endl;
        std::vector<Dtype> empty_vec;
        return empty_vec;  // return empty vector if analyzer_ is not initialized
    } else {
        return analyzer_->Analyze(frame);
    }
}

template <typename Dtype>
bool CTask<Dtype>::setTaskStatus(TaskStatus_t status) {
    /* Prepare DB Infomation*/
    const std::string server = "localhost";
    const std::string user = "itf";
    const std::string pass = "itf";
    const std::string db_name = "ITF";

    CDbi db;

    std::cout << "\nConnect  ...  ";
    if (!db.Connect(server, user, pass)) {
        std::cout << "Fail" << std::endl;
        return false;
    } else {
        std::cout << "OK" << std::endl;
    }

     std::cout << "Select DB  ...  ";
    if (!db.UseDB(db_name)) {
        std::cout << "Fail" << std::endl;
        return false;
    } else {
        std::cout << "OK" << std::endl;
    }

    std::string str_status;
    if (status == TaskStatus_t::START)
        str_status = "START";
    else
        str_status = "STOP";

    bool ok = db.RunSQL("UPDATE Tasks SET task_status='"+str_status+"' WHERE task_name='"+getCurrentTaskName()+"';");
    if (!ok) {
        std::cerr << "UPDATE DB ... Fail" << std::endl;
        return false;
    } else {
        std::cout << "UPDATE DB ... OK" << std::endl;
    }
    return true;
}

INSTANTIATE_MYCLASS(CTask);
