//
// ITF Config Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_CONFIG_H
#define ITF_CONFIG_H

#include <string>

#include "common.hpp"

class CConfig {
 public:
    inline void setTaskName(const std::string &task_name) { task_name_ = task_name; }
    inline void setTaskType(const int &task_type) { task_type_ = task_type; }
    inline void setTaskStatus(const std::string &task_status) { task_status_ = task_status; }
    inline void setCameraType(const int &camera_type) { camera_type_ = camera_type; }
    inline void setFrameWidth(const int &width) { width_ = width; }
    inline void setFrameHeight(const int &height) { height_ = height; }
    inline void setIPAddress(const std::string &address) { address_ = address; }
    inline void setPort(const int &port) { port_ = port; }
    inline void setHost(const std::string &host) { host_ = host; }
    inline void setUsername(const std::string &username) { username_ = username; }
    inline void setPassword(const std::string &password) { password_ = password; }
    inline void setPmapPath(const std::string &pers_path) { pers_path_ = pers_path; }
    inline void setROIPath(const std::string &roi_path) { roi_path_ = roi_path; }

    inline std::string getTaskName() { return task_name_; }
    inline int getTaskType() { return task_type_; }
    inline int getTaskStatus() { return task_status_; }
    inline int getCameraType() { return camera_type_; }
    inline int getFrameWidth() { return width_; }
    inline int getFrameHeight() { return height_; }
    inline std::string getIPAddress() { return address_; }
    inline unsigned int getPort() { return port_; }
    inline std::string getHost() { return host_; }
    inline std::string getUsername() { return username_; }
    inline std::string getPassword() { return password_; }
    inline std::string getPmapPath() { return pers_path_; }
    inline std::string getROIPath() { return roi_path_; }

 private:
    std::string task_name_;
    int task_type_ = 0;
    int task_status_ = 0;

    int camera_type_ = 0;

    int width_ = 0;
    int height_ = 0;
    int framesize = 0;

    std::string address_;
    unsigned int port_ = 0;
    std::string host_;
    std::string username_;
    std::string password_;

    std::string pers_path_;
    std::string roi_path_;
};


#endif  // ITF_CONFIG_H
