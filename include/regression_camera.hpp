//
// ITF File Camera Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_REGRESSIONC_H
#define ITF_REGRESSIONC_H

#include "camera.hpp"


class CRegressionCamera : public CCamera {
 public:
    CRegressionCamera(const std::string& filename, const int &start_index);

    int Connect();
    int Disconnect();
    time_t Capture(cv::Mat& output);
    
 private:
    std::string filename_;
    
    std::vector<boost::filesystem::path> ret_;
    int numofiles_;
    int currentfile_;
    
};

#endif  // ITF_REGRESSIONC_H
