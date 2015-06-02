//
// ITF File Camera Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_FC_H
#define ITF_FC_H

#include "common.hpp"
#include "camera.hpp"

class CFileCamera : public CCamera {
public:
    CFileCamera(const std::string& filename);
    ~CFileCamera();

    int Connect();
    int Disconnect();
    int Capture(cv::Mat& output);

private:
    std::string filename_;

};

#endif // ITF_FC_H
