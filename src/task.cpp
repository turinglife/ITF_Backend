//
// ITF Task Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "task.hpp"



CTask::CTask() { }

CTask::~CTask() {
    delete camera_;
}

bool CTask::LoadTask(const std::string& task_name, const std::string& db_name) {
    // Retrieve information from database according to task id.
    try {
        SQLite::Database db(db_name);

        SQLite::Statement query(db, "SELECT * FROM Tasks WHERE task_name=?");
        query.bind(1, task_name);

        bool has_records = false;
        // Init a task object with appropriate parameters.
        while (query.executeStep()) {
            task_name_ = task_name;
            type_ = query.getColumn(1).getInt();
            width_ = query.getColumn(2).getInt();
            height_ = query.getColumn(3).getInt();
            address_ = query.getColumn(4).getText();
            port_ = query.getColumn(5).getInt();
            host_ = query.getColumn(6).getText();
            username_ = query.getColumn(7).getText();
            password_ = query.getColumn(8).getText();
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

    // Instantiate a concrete camera.
    CameraType_t CameraType = static_cast<CameraType_t>(type_);

    if (CameraType == REMOTE_CAMERA_HTTP) {
        // RemoteCameraHttp
        camera_ = new CRemoteCameraHttp(host_, port_, address_, username_, password_);
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
        camera_ = new CFileCamera(address_);
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

int CTask::Capture(cv::Mat& frame) {
    // check if camera is initialized already
    if (camera_ == 0) {
        std::cout << "The camera has not been initialized yet." << std::endl;
        return 0;
    }
    camera_->Capture(frame);

    return 1;
}

void CTask::ShowDetails() {
    std::cout << std::endl;
    std::cout << getpid() << ": Task Info ------" << std::endl;
    std::cout << " Name: " << task_name_ << std::endl;
    std::cout << " Type: " << type_ << std::endl;
    std::cout << " Width: " << width_ << std::endl;
    std::cout << " Height: " << height_ << std::endl;
    std::cout << " Address: " << address_ << std::endl;
    std::cout << " Port: " << port_ << std::endl;
    std::cout << " Host: " << host_ << std::endl;
    std::cout << " User: " << username_ << std::endl;
    std::cout << " Password: " << password_ << std::endl;
    std::cout << std::endl;
}
