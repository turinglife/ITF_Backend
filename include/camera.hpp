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

protected:
    unsigned int width;
    unsigned int height;
    unsigned int channels;
    unsigned int framesize;
    
public:
    CCamera();
    virtual ~CCamera();
    
    virtual int Capture() = 0;

};


#endif // ITF_CAMERA_H
