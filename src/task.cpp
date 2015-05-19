//
// ITF Task Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "task.hpp"

#include "remote_camera_http.hpp"
#include "remote_camera_rtsp.hpp"
#include "local_camera.hpp"
#include "file_camera.hpp"

void CTask::LoadTask(unsigned int task_id) {

    // Retrieve information from database according to task id.
    
    
    // Init a task object with appropriate parameters.
    
    
    // Instantiate a concrete camera.
    std::string host, port, path;
    CameraType_t CameraType = REMOTE_CAMERA_HTTP;
    
    if (CameraType == REMOTE_CAMERA_HTTP) {
        // RemoteCameraHttp
        camera = new CRemoteCameraHttp(host, port, path);
    }
    else if (CameraType == REMOTE_CAMERA_RTSP) {
        // RemoteCameraRtsp
        
        
        
    }
    else if (CameraType == LOCAL_CAMERA) {
        // LocalCamera 
        
        
        
    }
    else if (CameraType == FILE_CAMERA) {
        // FileCamera
        
    }
    else {
        // not supporting.
        
    }

    return;

}

int CTask::Capture() {
    
    
    
    // Capture a new next image
    camera->Capture();
    
    
    return 0;

}

