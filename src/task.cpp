//
// ITF Task Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "task.hpp"


template <typename Dtype>
CTask<Dtype>::CTask() : camera_(), analyzer_(), alarmer_(), config_() {



}

template <typename Dtype>
CTask<Dtype>::~CTask() {
    
    // Free camera and analyzer objects
    
    delete camera_;
    delete analyzer_;
}

template <typename Dtype>
bool CTask<Dtype>::LoadTask(const std::string& task_name, const std::string& db_name) {
    // Retrieve information from database according to task id.
    try {
        SQLite::Database db(db_name);

        SQLite::Statement query(db, "SELECT * FROM Tasks WHERE task_name=?");
        query.bind(1, task_name);

        bool has_records = false;
        // Init a task object with appropriate parameters.
        while (query.executeStep()) {
            config_.setTaskName(task_name);
            config_.setTaskType(query.getColumn(1).getInt());
            config_.setCameraType(query.getColumn(2).getInt());
            config_.setFrameWidth(query.getColumn(3).getInt());
            config_.setFrameHeight(query.getColumn(4).getInt());
            config_.setIPAddress(query.getColumn(5).getText());
            config_.setPort(query.getColumn(6).getInt());
            config_.setHost(query.getColumn(7).getText());
            config_.setUsername(query.getColumn(8).getText());
            config_.setPassword(query.getColumn(9).getText());
            config_.setPmapPath(query.getColumn(10).getText());
            config_.setROIPath(query.getColumn(11).getText());

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

    return true;
}


template <typename Dtype>
bool CTask<Dtype>::InitCapture()
{
    // Instantiate a concrete camera.
    CameraType_t CameraType = static_cast<CameraType_t>(config_.getCameraType());

    if (CameraType == REMOTE_CAMERA_HTTP) {
        // RemoteCameraHttp
        camera_ = new CRemoteCameraHttp(config_.getHost(), config_.getPort(), config_.getIPAddress(), config_.getUsername(), config_.getPassword());
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
        camera_ = new CFileCamera(config_.getIPAddress());
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

template <typename Dtype>
bool CTask<Dtype>::InitAnalyzer()
{
    // Instantiate a concrete analyzer.
    FunType_t FunType = static_cast<FunType_t>(config_.getTaskType());
    
    if (FunType == COUNT) {
        // Instantiate Counting analyzer
        analyzer_ = new CDPAnalyzerDensity<Dtype>(config_.getPmapPath(), config_.getROIPath(), config_.getFrameWidth(), config_.getFrameHeight());
        
        std::cout << getpid() << ": CDPAnalyzerDensity is initialized" << std::endl;
        
        analyzer_->Init();
        
    } else if (FunType == SEGMENT) {
        // Segmentation
        analyzer_ = new CDPAnalyzerSegmentation<Dtype>(config_.getPmapPath(), config_.getROIPath(), config_.getFrameWidth(), config_.getFrameHeight());
        
        std::cout << getpid() << ": CDPAnalyzerSegmentation is initialized" << std::endl;

        analyzer_->Init();

    } else {
        // not supporting.
        std::cout << "To Be Continued" << std::endl;

        return false;
    }
    
    return true;
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
std::vector<Dtype> CTask<Dtype>::Analyze(cv::Mat& frame)
{
    vector<Dtype> feature;
    
    // check if camera is initialized already
    if (analyzer_ == 0) {
        std::cout << "The camera has not been initialized yet." << std::endl;

        return feature;
    }
    
    feature = analyzer_->Analyze(frame);

    return feature;
}


#if 0

std::thread CTask::Analyze(FunType_t ft) {
    switch (ft) {
        case COUNT:
            return std::thread(&CTask::Do_Count, this);
        case SEGMENT:
            return std::thread(&CTask::Do_Segment, this);
        default:
            std::cout << "No Such Function!" << std::endl;
            exit(-1);
    }
}


void CTask::Do_Count() {
    using boost::interprocess::shared_memory_object;
    using boost::interprocess::open_only;
    using boost::interprocess::read_only;

    shared_memory_object shm(open_only, task_name_.c_str(), read_only);

    using boost::interprocess::mapped_region;

    mapped_region region_frame(shm, read_only);

    /* Setup Extracter */
    itf::ExtracterParameter ep;
    // Read configuration file
    if (!itf::Util::ReadProtoFromTextFile("/home/turinglife/Desktop/SDK/lib/config/density_extracter.prototxt", &ep)) {
        std::cout << "Cannot read .prototxt file!" << std::endl;
        exit(-1);
    }
    // Create extracter factory
    itf::CExtracterFactory ef;
    // Factory instantiates an object of the specific type of extracter
    itf::IExtracter *iextracter = ef.SpawnExtracter(itf::Density);
    iextracter->SetExtracterParameters(ep);

    std::string pers_path;
    std::string roi_path;

    try {
        SQLite::Database db("./db/ITF.db");

        SQLite::Statement query(db, "SELECT pers_path, roi_path FROM Tasks WHERE task_name=?");
        query.bind(1, task_name_);

        bool has_records = false;
        // Init a task object with appropriate parameters.
        while (query.executeStep()) {
            pers_path = query.getColumn(0).getText();
            roi_path = query.getColumn(1).getText();
            has_records = true;
        }
        if (!has_records) {
            std::cout << "No Such Pers or ROI" << std::endl;
            return;
        }
    } catch (std::exception& e) {
        std::cout << "exception: " << e.what() << std::endl;
        return;
    }


    iextracter->SetImagesDim(height_, width_);
    cv::Mat pmap;
    iextracter->LoadPerspectiveMap(pers_path, &pmap);
    iextracter->LoadROI(roi_path);

    itf::Util util;

    while (on) {
        cv::Mat frame(height_, width_, CV_8UC3);
        int imgSize = frame.total() * frame.elemSize();
        memcpy(frame.data, region_frame.get_address(), imgSize);

        iextracter->LoadImages(frame, frame);

        // Extract density feature from a frame loaded above
        std::vector<float> feature = iextracter->ExtractFeatures();

        cv::Mat density_map(height_, width_, CV_32F, feature.data());

        // Generate a heat map
        cv::Mat heat = util.GenerateHeatMap(density_map, pmap, 2000);
        cv::imshow(task_name_ + "_heat", heat);

        cv::waitKey(10);
    }

    std::cout << task_name_ << ": Video is Over" << std::endl;

    delete iextracter;
}

void CTask::Do_Segment() {
     itf::SegmenterParameter sp;

    if (!itf::Util::ReadProtoFromTextFile("./model/fcnn_segmenter.prototxt", &sp)) {
        std::cout << "Cannot read .prototxt file!" << std::endl;
        exit(-1);
    }
    // Create segmenter factory
    itf::CSegmenterFactory sf;
    // Factory instantiate an object of the specific type of segmenter
    itf::ISegmenter *isegmenter = sf.SpawnSegmenter(itf::FCNN);

    isegmenter->SetParameters(sp);

    using boost::interprocess::shared_memory_object;
    using boost::interprocess::open_only;
    using boost::interprocess::read_only;

    shared_memory_object shm(open_only, task_name_.c_str(), read_only);

    using boost::interprocess::mapped_region;
    mapped_region region_frame(shm, read_only);

    while (on) {
        cv::Mat frame(height_, width_, CV_8UC3);
        int imgSize = frame.total() * frame.elemSize();
        memcpy(frame.data, region_frame.get_address(), imgSize);

        cv::Mat foreground;
        cv::Mat img_bgmodel;
        isegmenter->process(frame, foreground, img_bgmodel);
        foreground = foreground > 0.5;
        cv::imshow(task_name_ + "_seg", foreground);

        cv::waitKey(10);
    }

    std::cout << task_name_ << ": Video is Over" << std::endl;

    delete isegmenter;
}



#endif


INSTANTIATE_MYCLASS(CTask);

