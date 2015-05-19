//
// ITF Task Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_TASK_H
#define ITF_TASK_H


#include "common.hpp"
#include "camera.hpp"


class CTask {


public:
    
    typedef enum { REMOTE_CAMERA_HTTP, REMOTE_CAMERA_RTSP, LOCAL_CAMERA, FILE_CAMERA } CameraType_t;
    
    
    void LoadTask(unsigned int task_id);
    int Capture();

private:
    typedef struct {
        unsigned int size;
        unsigned int last_write_index;
        unsigned int last_read_index;
    } BufferHead;

    int shm_id;
    BufferHead *pbufferhead_;

    
    CCamera *camera;


};









#endif // ITF_TASK_H
