//
// ITF Camera Class, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_CAMERA_H
#define ITF_CAMERA_H


#include "common.hpp"


//
// Abstract base class for cameras. This is intended to express common attributes and interfaces
//
class CCamera {
public:
    virtual ~CCamera() {}
    
    virtual int Connect() = 0;
    virtual int Disconnect() = 0;
    virtual time_t Capture(cv::Mat& output) = 0;

protected:
    cv::VideoCapture cap_;
    cv::Mat curr_frame_;
    time_t rawtime_;
};


#endif // ITF_CAMERA_H
