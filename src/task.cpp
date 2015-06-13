//
// ITF Task Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "task.hpp"

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <itf/extracters/extracter_factory.hpp>
#include <itf/segmenters/segmenter_factory.hpp>
#include <itf/util/Util.hpp>

#include <string>
#include <vector>

CTask::CTask() {}

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

std::thread CTask::Analyze(FunType_t ft, std::string path) {
    switch (ft) {
        case COUNT:
            return std::thread(&CTask::Do_Count, this, path);
        case SEGMENT:
            return std::thread(&CTask::Do_Segment, this, path);
        default:
            std::cout << "No Such Function!" << std::endl;
            exit(-1);
    }
}

void CTask::Do_Count(std::string path) {
    using boost::interprocess::shared_memory_object;
    using boost::interprocess::open_only;
    using boost::interprocess::read_only;

    shared_memory_object shm(open_only, path.c_str(), read_only);

    using boost::interprocess::mapped_region;

    mapped_region region_frame(shm, read_only);

    /* Setup Extracter */
    itf::ExtracterParameter ep;
    // Read configuration file
    if (!itf::Util::ReadProtoFromTextFile("./model/density_extracter.prototxt", &ep)) {
        std::cout << "Cannot read .prototxt file!" << std::endl;
        exit(-1);
    }
    // Create extracter factory
    itf::CExtracterFactory *ef = new itf::CExtracterFactory();
    // Factory instantiates an object of the specific type of extracter
    itf::IExtracter *iextracter = ef->SpawnExtracter(itf::Density);
    iextracter->SetExtracterParameters(ep);

    std::string pers_path;
    std::string roi_path;

    try {
        SQLite::Database db("./db/ITF.db");

        SQLite::Statement query(db, "SELECT pers_path, roi_path FROM Tasks WHERE task_name=?");
        query.bind(1, path);

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
        cv::imshow(path, heat);

        cv::waitKey(10);
    }

    std::cout << path << ": Video is Over" << std::endl;

    delete ef;
}

void CTask::Do_Segment(std::string path) {
     itf::SegmenterParameter sp;

    if (!itf::Util::ReadProtoFromTextFile("./model/fcnn_segmenter.prototxt", &sp)) {
        std::cout << "Cannot read .prototxt file!" << std::endl;
        exit(-1);
    }
    // Create segmenter factory
    itf::CSegmenterFactory *sf = new itf::CSegmenterFactory();
    // Factory instantiate an object of the specific type of segmenter
    itf::ISegmenter *isegmenter = sf->SpawnSegmenter(itf::FCNN);

    isegmenter->SetParameters(sp);

    using boost::interprocess::shared_memory_object;
    using boost::interprocess::open_only;
    using boost::interprocess::read_only;

    shared_memory_object shm(open_only, path.c_str(), read_only);

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
        cv::imshow(path, foreground);

        cv::waitKey(10);
    }

    std::cout << path << ": Video is Over" << std::endl;

    delete sf;
}
