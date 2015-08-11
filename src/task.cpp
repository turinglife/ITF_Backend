//
// ITF Task Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "task.hpp"


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
    std::vector<std::map<std::string, std::string> > res = db.Query("select task_type, camera_type, task_path from Tasks where task_name='"+task_name+"';");

    if (res.size() != 1) {
        std::cout << "No Such Task" << std::endl;
       return false;
    }

    config_.setTaskName(task_name);
    if (res[0]["task_type"].compare("DENSITY") == 0) {
        config_.setTaskType(0);
        std::vector<std::map<std::string, std::string> > density_detail = db.Query("select * from DensityDetail where task_name='" +task_name+ "';");
        config_.setPmapPath(res[0]["task_path"] + "PMap/" + density_detail[0]["pers_file"]);
        config_.setROIPath(res[0]["task_path"] + "ROI/" + density_detail[0]["roi_file"]);
    } else if (res[0]["task_type"].compare("SEGMENTATION") == 0) {
        config_.setTaskType(1);
    } else {
        std::cerr << "No such task type" << std::endl;
        return false;
    }

    if (res[0]["camera_type"].compare("HTTP") == 0) {
        config_.setCameraType(0);
        std::vector<std::map<std::string, std::string> > camera = db.Query("select * from Cameras where camera_name=(select camera_name from Task_Camera where task_name='" +task_name+ "');");
        config_.setPort(std::atoi(camera[0]["port"].c_str()));
        config_.setHost(camera[0]["host"]);
        config_.setIPAddress(camera[0]["address"]);
        config_.setUsername(camera[0]["username"]);
        config_.setPassword(camera[0]["password"]);
        config_.setFrameWidth(std::atoi(camera[0]["width"].c_str()));
        config_.setFrameHeight(std::atoi(camera[0]["height"].c_str()));
    } else if (res[0]["campera_type"].compare("RTSP") == 0) {
        config_.setCameraType(1);
    } else if (res[0]["campera_type"].compare("LOCAL") == 0) {
        config_.setCameraType(2);
    } else if (res[0]["campera_type"].compare("FILE") == 0) {
        config_.setCameraType(3);
        std::vector<std::map<std::string, std::string> > file = db.Query("select * from Cameras where camera_name=(select camera_name from Task_Camera where task_name='" +task_name+ "');");
        config_.setFrameWidth(std::atoi(file[0]["width"].c_str()));
        config_.setFrameHeight(std::atoi(file[0]["height"].c_str()));
        config_.setIPAddress(file[0]["file_url"]);
    } else {
        std::cerr << "No such camera type" << std::endl;
        return false;
    }

    return true;
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
        camera_.reset(new CRemoteCameraHttp(config_.getHost(), config_.getPort(), config_.getIPAddress(), config_.getUsername(), config_.getPassword()));
    } else if (CameraType == FILE) {
        // FileCamera
        camera_.reset(new CFileCamera(config_.getIPAddress()));
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
        analyzer_.reset(new CDPAnalyzerDensity<Dtype>(config_.getPmapPath(), config_.getROIPath(), config_.getFrameWidth(), config_.getFrameHeight()));
    } else if (FunType == SEGMENT) {
        // Segmentation
        analyzer_.reset(new CDPAnalyzerSegmentation<Dtype>());
    } else {
        // not supporting.
        std::cout << "To Be Continued" << std::endl;
        return false;
    }
    return analyzer_->Init();
}

template <typename Dtype>
int CTask<Dtype>::Capture(int fps) {
    // check if camera is initialized already
    if (camera_ == 0) {
        std::cout << "The camera has not been initialized yet." << std::endl;
        return 0;
    }    

    cv::Mat frame(config_.getFrameHeight(), config_.getFrameWidth(), CV_8UC3);
    int imgSize = frame.total() * frame.elemSize();

    CBuffer buffer(config_.getFrameWidth(), config_.getFrameHeight(), imgSize, 50, 30, config_.getTaskName());

    while (getCameraStatus()) {
        cv::Mat frame;
        camera_->Capture(frame);
        if (frame.empty()) {
            break;
        }
        // Write a new frame into buffer
        buffer.put_src(frame);

        cv::imshow(config_.getTaskName() + "_frame", frame);
        cv::waitKey(1000 / fps);
    }
    return 1;
}

template <typename Dtype>
int CTask<Dtype>::Analyze() {
    // check if camera is initialized already
    if (analyzer_ == 0) {
        std::cerr << "The camera has not been initialized yet." << std::endl;
        return 0;
    }

    int rows = config_.getFrameHeight();
    int cols = config_.getFrameWidth();
    cv::Mat frame(rows, cols, CV_8UC3);

    CBuffer buffer(config_.getTaskName());

    itf::Util util;
    // Get the perspective map and square it to generate a better heat map
    cv::Mat pmap = util.ReadPMAPtoMAT(config_.getPmapPath());
    pmap = pmap.mul(pmap);

    while (getFuncStatus()) {
        if (!buffer.fetch_src(frame)) {
            std::cerr << "ad: No Available Frame for " << config_.getTaskName() << std::endl;
            sleep(3);  // reduce useless while loop
            continue;
        }
        vector<float> feature = analyzer_->Analyze(frame);
        cv::Mat output(rows, cols, CV_32F, feature.data());

        if (getCurrentTaskType() == TaskType_t::DENSITY) {
            output = util.GenerateHeatMap(output, pmap);
            int predicted_value = static_cast<int>(cv::sum(output)[0]);
            buffer.put_dst(output, predicted_value);
        } else if (getCurrentTaskType() == TaskType_t::SEGMENTATION) {
            /* code */
        }
        cv::imshow(config_.getTaskName() + "_ad_result", output);
        cv::waitKey(1);
    }
    return  1;
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

INSTANTIATE_MYCLASS(CTask);
