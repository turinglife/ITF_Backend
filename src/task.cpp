//
// ITF Task Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "task.hpp"

template <typename Dtype>
bool CTask<Dtype>::InitCapture(const std::string& task_name) {
    // camera_ cannot be initialzed twice or deleted if it is alreaday initialized
    if (camera_ != 0) {
        std::cerr << "Camera is already initialized" << std::endl;
        return false;
    }

    // Connect database
    CDbi db = ConnectDB();
    if (!db.isConnected()) {
        std::cerr << "Cannot connect to database" << std::endl;
        return false;
    }

    config_.setTaskName(task_name);

    // Check camera type
    std::vector<std::map<std::string, std::string> > res = db.Query("select camera_type from Tasks where task_name='"+task_name+"';");
    if (res.size() != 1) {
        std::cout << "No Such Task" << std::endl;
       return false;
    }

    // Instantiate a concrete camera.
    if (res[0]["camera_type"].compare("HTTP") == 0) {
        // RemoteCameraHttp
        config_.setCameraType(0);
        std::vector<std::map<std::string, std::string> > camera = db.Query("select * from Cameras where camera_name=(select camera_name from Task_Camera where task_name='"+task_name+"');");
        config_.setFrameWidth(std::atoi(camera[0]["width"].c_str()));
        config_.setFrameHeight(std::atoi(camera[0]["height"].c_str()));
        config_.setHost(camera[0]["host"]);
        config_.setIPAddress(camera[0]["address"]);
        config_.setPort(std::atoi(camera[0]["port"].c_str()));
        config_.setUsername(camera[0]["username"]);
        config_.setPassword(camera[0]["password"]);
        camera_.reset(new CRemoteCameraHttp(config_.getHost(), config_.getPort(), config_.getIPAddress(), config_.getUsername(), config_.getPassword()));
    } else if (res[0]["camera_type"].compare("FILE") == 0) {
        // FileCamera
        config_.setCameraType(3);
        std::vector<std::map<std::string, std::string> > file = db.Query("select * from Files where task_name='"+task_name+"';");
        config_.setFrameWidth(std::atoi(file[0]["width"].c_str()));
        config_.setFrameHeight(std::atoi(file[0]["height"].c_str()));
        config_.setIPAddress(file[0]["file_url"]);
        camera_.reset(new CFileCamera(config_.getIPAddress()));
    } else {
        // not supporting.
        std::cerr << "To Be Continued" << std::endl;
        return false;
    }

    // Create buffer
    cv::Mat frame(config_.getFrameHeight(), config_.getFrameWidth(), CV_8UC3);
    int imgSize = frame.total() * frame.elemSize();
    if (!buffer_.Init(config_.getFrameWidth(), config_.getFrameHeight(), imgSize, 50, 30, config_.getTaskName())) {
        std::cerr << "Cannot create buffer" << std::endl;
        return false;
    }

    return camera_->Connect();
}

template <typename Dtype>
bool CTask<Dtype>::InitAnalyzer(const std::string& task_name) {
    // analyzer_ cannot be initialzed twice or deleted if it is alreaday initialized
    if (analyzer_ != 0) {
        std::cerr << "Analyer is alrady initialized" << std::endl;
        return false;
    }

    // Connect database
    CDbi db = ConnectDB();
    if (!db.isConnected()) {
        std::cerr << "Cannot connect to database" << std::endl;
        return false;
    }

    config_.setTaskName(task_name);

    if (!buffer_.Init(config_.getTaskName())) {
        std::cerr << "Cannot create buffer" << std::endl;
        return false;
    }

    int w = 0;
    int h = 0;
    buffer_.frame_size(w, h);

    config_.setFrameWidth(w);
    config_.setFrameHeight(h);

    // Check task type
    std::vector<std::map<std::string, std::string> > res = db.Query("select task_type, task_path from Tasks where task_name='"+task_name+"';");

    // Instantiate a concrete analyzer.
    if (res[0]["task_type"].compare("COUNTING") == 0) {
        // Instantiate Counting analyzer
        config_.setTaskType(0);
        std::vector<std::map<std::string, std::string> > density_detail = db.Query("select * from DensityDetail where task_name='" +task_name+ "';");
        config_.setPmapPath(res[0]["task_path"] + "PMap/" + density_detail[0]["pers_file"]);
        config_.setROIPath(res[0]["task_path"] + "ROI/" + density_detail[0]["roi_file"]);
        analyzer_.reset(new CDPAnalyzerDensity<Dtype>(config_.getPmapPath(), config_.getROIPath(), config_.getFrameWidth(), config_.getFrameHeight()));
    } else if (res[0]["task_type"].compare("SEGMENTATION") == 0) {
        // Segmentation
        config_.setTaskType(1);
        analyzer_.reset(new CDPAnalyzerSegmentation<Dtype>);
    } else {
        // not supporting.
        std::cerr << "To Be Continued" << std::endl;
        return false;
    }

    predicted_value_ = 0;

    return analyzer_->Init();
}

template <typename Dtype>
bool CTask<Dtype>::InitTrainer(const std::string& task_name) {

    // camera_ cannot be initialzed twice or deleted if it is alreaday initialized
    if (camera_ != 0) {
        std::cerr << "Trainer is already initialized" << std::endl;
        return false;
    }

    // Connect database
    CDbi db = ConnectDB();
    if (!db.isConnected()) {
        std::cerr << "Cannot connect to database" << std::endl;
        return false;
    }
    
    // Load configuration
    std::vector<std::map<std::string, std::string> > tasks = db.Query("select * from Tasks where task_name='"+task_name+"';");    
    std::vector<std::map<std::string, std::string> > density_detail = db.Query("select * from DensityDetail where task_name='"+task_name+"';");
        
    config_.setTaskName("trainer");
    
    config_.setIPAddress(tasks[0]["task_path"]);    
    config_.setPmapPath(tasks[0]["task_path"] + "PMap/" + density_detail[0]["pers_file"]);
    config_.setROIPath(tasks[0]["task_path"] + "ROI/" + density_detail[0]["roi_file"]);
    
    
    std::string file = config_.getIPAddress() + "GT/" + "0.jpg";
    //std::cout<<"file = "<<file<<std::endl;
        
    cv::Mat output;
    output = cv::imread(file);
    
    //camera.Connect();
    //output = camera.Capture(file);    
    
    cv::Size s = output.size();

    //std::cout<<"output.width = "<<s.width<<std::endl;
    //std::cout<<"output.height = "<<s.height<<std::endl;
    
    config_.setFrameHeight(s.height);
    config_.setFrameWidth(s.width);
    
    camera_.reset(new CRegressionCamera(config_.getIPAddress(), 0));
    //camera_->Connect();
    
    // Create buffer for communicating capturer with analyzer.
    cv::Mat frame(config_.getFrameHeight(), config_.getFrameWidth(), CV_8UC3);
    int imgSize = frame.total() * frame.elemSize();
    if (!buffer_.Init(config_.getFrameWidth(), config_.getFrameHeight(), imgSize, 50, 30, config_.getTaskName())) {
        std::cerr << "Cannot create buffer" << std::endl;
        return false;
    }    
    
    // analyzer_ cannot be initialzed twice or deleted if it is alreaday initialized
    if (analyzer_ != 0) {
        std::cerr << "Analyer is alrady initialized" << std::endl;
        return false;
    }
    
    // analyzer applies for buffer to use.
    if (!buffer_.Init(config_.getTaskName())) {
        std::cerr << "Cannot create buffer" << std::endl;
        return false;
    }

    int w = 0;
    int h = 0;
    buffer_.frame_size(w, h);

    config_.setFrameWidth(w);
    config_.setFrameHeight(h);

    // Check task type
    std::vector<std::map<std::string, std::string> > res = db.Query("select task_type, task_path from Tasks where task_name='"+task_name+"';");

    // Instantiate Counting analyzer
    config_.setTaskType(0);
    analyzer_.reset(new CDPAnalyzerDensity<Dtype>(config_.getPmapPath(), config_.getROIPath(), config_.getFrameWidth(), config_.getFrameHeight()));
    analyzer_->Init();
    
    return 1;
}

template <typename Dtype>
int CTask<Dtype>::Capture(int fps) {
    // check if camera is initialized already
    if (camera_ == 0) {
        std::cout << "The camera has not been initialized yet." << std::endl;
        return 0;
    }

    while (getCameraStatus()) {
        cv::Mat frame;
        unsigned int timestamp = camera_->Capture(frame);
        if (frame.empty()) {
            break;
        }
        // Write a new frame into buffer
        buffer_.put_src(frame, timestamp);

        cv::imshow(config_.getTaskName() + "_frame", frame);
        cv::waitKey(1000 / fps);
        
        cv::imwrite("/home/turinglife/Desktop/1/" + std::to_string(timestamp) + ".jpg", frame);
    }
    return 1;
}

template <typename Dtype>
int CTask<Dtype>::Analyze() {
    // check if camera is initialized already
    if (analyzer_ == 0) {
        std::cerr << "The analyzer has not been initialized yet." << std::endl;
        return 0;
    }

    int rows = config_.getFrameHeight();
    int cols = config_.getFrameWidth();
    cur_frame_.create(rows, cols, CV_8UC3);
    unsigned int timestamp;
    
    // Write predicted number into disk every second
    const int interval = 1;
    itf::Util util;
    cv::Mat pmap;
    if (getCurrentTaskType() == TaskType_t::COUNTING) {
        // Get the perspective map and square it to generate a better heat map
        pmap = util.ReadPMAPtoMAT("tmp_pers.csv");
        pmap = pmap.mul(pmap);
        tdb_ = std::thread(&CTask::record, this, interval);
    }

    while (getFuncStatus()) {
        if (!buffer_.fetch_frame(cur_frame_, timestamp)) {
            std::cerr << "ad: No Available Frame for " << config_.getTaskName() << std::endl;
            sleep(3);  // reduce useless while loop
            continue;
        }
        vector<float> feature = analyzer_->Analyze(cur_frame_);
        cv::Mat output(rows, cols, CV_32F, feature.data());

        // Post-processing
        cv::Mat dst;
        if (getCurrentTaskType() == TaskType_t::COUNTING) {
            predicted_value_ = static_cast<int>(cv::sum(output)[0]);
            dst = util.GenerateHeatMap(output, pmap);
            cv::applyColorMap(dst, dst, cv::COLORMAP_JET);
            buffer_.put_dst(dst, predicted_value_);
        } else if (getCurrentTaskType() == TaskType_t::SEGMENTATION) {
            cur_frame_.copyTo(dst, output > 0.5);
            buffer_.put_dst(dst, 0);
        }
        
        // Uncomment following two lins to debug
        cv::imshow(config_.getTaskName() + "_ad_result", dst);
        cv::waitKey(1);
    }
    if (tdb_.joinable()) tdb_.join();
    return  1;
}

template <typename Dtype>
int CTask<Dtype>::Train(std::string &filename) {
    
    std::vector<double> gt;
    std::vector<double> predict;
    
   
    /************************************************************************************/
    // generate gt vector for training linear model.
  
    std::cout<<"filename_ = "<<config_.getIPAddress()<<std::endl;
    std::string gt_folder = config_.getIPAddress() + "GT/";
    std::vector<boost::filesystem::path> gt_frame, gt_coordinate;
    int current_frame = 0, current_coordinate = 0;
    cv::Mat output;
    unsigned int timestamp = 0;
    itf::Util util;
    
    // access the root folder of the current task.
    if(!boost::filesystem::exists(gt_folder) || !boost::filesystem::is_directory(gt_folder)) 
        return 0;
    
    boost::filesystem::recursive_directory_iterator it(gt_folder);
    boost::filesystem::recursive_directory_iterator endit;
    
    // figure out gt frames and gt coordinates.
    while(it != endit) {
        if(boost::filesystem::is_regular_file(*it) && it->path().extension() == ".jpg") {
            gt_frame.push_back(it->path());
            
            std::string csv_path = it->path().parent_path().string() + "/" + it->path().stem().string() + ".csv";
            gt_coordinate.push_back(csv_path);
        }
        
        ++it;
    }
    
    // gt frames are put into buffer.
    while(current_frame < gt_frame.size()) {
        output = cv::imread(gt_frame[current_frame].string());
        //std::cout<<"ret_ = "<<gt_frame[current_frame]<<std::endl;
        //std::cout<<"csv_ = "<<gt_coordinate[current_frame]<<std::endl;

        buffer_.put_src(output, timestamp);
        
        cv::imwrite("/home/turinglife/Desktop/1/" + std::to_string(current_frame + 1) + ".jpg", output);
        
        current_frame++;
    }
    
    cv::Mat roi_mask;
    roi_mask = util.ReadROItoMAT(config_.getROIPath(), config_.getFrameHeight(), config_.getFrameWidth());
    //std::ofstream myfile("/home/turinglife/Desktop/some_name.csv");
    //if (myfile.is_open()) {
    //    myfile << cv::format(roi_mask, "csv");
    //    myfile.close();
    //}

    // figure out the number of people in ROI.
    while(current_coordinate < gt_coordinate.size()) {
        CvMLData mlData;
        mlData.read_csv(gt_coordinate[current_coordinate].c_str());
        const CvMat* tmp = mlData.get_values();
        
        cv::Mat cvroi(tmp, true);

        //std::vector< std::vector<cv::Point> > contours;
        std::vector<cv::Point> contour;
        int counts = 0;
        
        cv::Size s = roi_mask.size();

        std::cout<<"roi_mask.width = "<<s.width<<std::endl;
        std::cout<<"roi_mask.height = "<<s.height<<std::endl;
        std::cout<<"cvroi = "<<cvroi<<std::endl;
        
        for (int i = 0; i < cvroi.rows; ++i) {
            contour.push_back(cv::Point(cvroi.at<float>(i, 0), cvroi.at<float>(i, 1)));
            
            //std::cout<<"cvroi"<<cv::Point(cvroi.at<int>(i, 0), cvroi.at<int>(i, 1))<<std::endl;
            
            
            std::cout<<"roi_mask.at<int>(contour[i] = "<<roi_mask.at<float>(contour[i])<<std::endl;
            if(roi_mask.at<float>(contour[i]) == 1) {
                counts++;
            }
        }
        
        std::cout<<" "<<std::endl;
        
        
        
        gt.push_back(counts);
        
        current_coordinate++;
    }
    
    int i = 0;
    while(i < gt.size()) {
        std::cout<<"gt[i] = "<<gt[i]<<std::endl;
        i++;
    }
    
    /************************************************************************************/
    // generate predict vector for training linear model.
        
    if (analyzer_ == 0) {
        std::cerr << "The analyzer has not been initialized yet." << std::endl;
        return 0;
    }

    int rows = config_.getFrameHeight();
    int cols = config_.getFrameWidth();
    cv::Mat frame(rows, cols, CV_8UC3);
    //unsigned int timestamp;

    //itf::Util util;
    // Get the perspective map and square it to generate a better heat map
    cv::Mat pmap = util.ReadPMAPtoMAT("tmp_pers.csv");
    pmap = pmap.mul(pmap);
    int index = 1;
    while (getFuncStatus()) {
        if (!buffer_.fetch_src(frame, timestamp)) {
            //std::cerr << "ad: No Available Frame for " << config_.getTaskName() << std::endl;
            //sleep(3);  // reduce useless while loop
            //continue;
            
            break;
        }
        
        vector<float> feature = analyzer_->Analyze(frame);
        cv::Mat output(rows, cols, CV_32F, feature.data());
        cv::Mat dst;
        
        dst = util.GenerateHeatMap(output, pmap);
        int predicted_value = static_cast<int>(cv::sum(output)[0]);
        buffer_.put_dst(dst, predicted_value);
        predict.push_back(predicted_value);
        
        //cv::imshow(config_.getTaskName() + "_ad_result", dst);
        //cv::waitKey(1);
        
        cv::imwrite("/home/turinglife/Desktop/1/density/"+ std::to_string(index) + ".jpg", dst);
        std::cout<<std::to_string(index)<<", predicted_value = "<<predicted_value<<std::endl;
        
        index++;
    }
    
    /************************************************************************************/
    // generate linear model.
    std::string save_name = config_.getIPAddress() + "LM/" + filename + ".csv";
    std::vector<double> model = util.TrainLinearModel(gt, predict, save_name);
    
    return  1;   
}

template <typename Dtype>
bool CTask<Dtype>::setTaskStatus(TaskStatus_t status) {
    CDbi db = ConnectDB();
    if (!db.isConnected()) {
        std::cerr << "Cannot connect to database" << std::endl;
        return false;
    }

    std::string str_status;
    if (status == TaskStatus_t::ON)
        str_status = "ON";
    else
        str_status = "OFF";

    bool ok = db.RunSQL("UPDATE Tasks SET task_status='"+str_status+"' WHERE task_name='"+getCurrentTaskName()+"';");
    if (!ok) {
        std::cerr << "UPDATE DB ... Fail" << std::endl;
        return false;
    } else {
        std::cout << "UPDATE DB ... OK" << std::endl;
    }
    return true;
}

template <typename Dtype>
CDbi CTask<Dtype>::ConnectDB() {
    // Prepare DB Infomation
    const std::string server = "localhost";
    const std::string user = "itf";
    const std::string pass = "itf";
    const std::string db_name = "ITF";

    CDbi db;
    db.Connect(server, user, pass);
    db.UseDB(db_name);
    return db;
}

template <typename Dtype>
bool CTask<Dtype>::FreeBuffer() {
    return buffer_.destroy();
}

template <typename Dtype>
void CTask<Dtype>::record(int interval) {
    CDbi db = ConnectDB();
    while (getFuncStatus()) {
        sleep(interval);
        db.RunSQL("INSERT INTO DensityPredict VALUES (DEFAULT, "
            + std::to_string(predicted_value_) + ", '"
            + config_.getTaskName() + "');");
    }
}

INSTANTIATE_MYCLASS(CTask);
