//
// ITF Task Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "task.hpp"

template <typename Dtype>
bool CTask<Dtype>::InitCapturer(const std::string& task_name) {
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
bool CTask<Dtype>::DestroyCapturer() {
    return buffer_.destroy();
}

template <typename Dtype>
bool CTask<Dtype>::InitAnalyzer(const std::string& task_name) {
    int w = 0, h = 0;

    // analyzer_ cannot be initialzed twice or deleted if it is alreaday initialized
    if (analyzer_ != 0) {
        std::cerr << "Analyzer is alrady initialized" << std::endl;
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
        config_.setTaskPath(res[0]["task_path"]);
        analyzer_.reset(new CDPAnalyzerDensity<Dtype>(config_.getPmapPath(), config_.getROIPath(), config_.getFrameWidth(), config_.getFrameHeight()));
    } else if (res[0]["task_type"].compare("SEGMENTATION") == 0) {
        // Segmentation
        config_.setTaskType(1);
        analyzer_.reset(new CDPAnalyzerSegmentation<Dtype>);
    } else if (res[0]["task_type"].compare("STATIONARY") == 0) {
        // Stationary
        config_.setTaskType(2);
        analyzer_.reset(new CDPAnalyzerStationary<Dtype>("SK", false, config_.getROIPath(), config_.getFrameWidth(), config_.getFrameHeight()));
    } else if (res[0]["task_type"].compare("CROSSLINE") == 0) {
        // CrossLine
        config_.setTaskType(3);
        std::vector<std::map<std::string, std::string> > density_detail = db.Query("select * from DensityDetail where task_name='" +task_name+ "';");
        config_.setTaskPath(res[0]["task_path"]);
        itf::Util util;
        std::vector<std::pair<float, float> > pair_vec = util.ReadPairToVec(res[0]["task_path"] + "PMap/" + density_detail[0]["pers_file"]);
        std::string pmap_path = "tmp_pers.csv";
        util.GeneratePerspectiveMap(pair_vec, config_.getFrameHeight(), config_.getFrameWidth(), pmap_path);

        CvMLData mlData;
        mlData.read_csv((res[0]["task_path"] + "ROI/" + density_detail[0]["roi_file"]).c_str());
        const CvMat* tmp = mlData.get_values();
        cv::Mat cvroi(tmp, true);
        cv::Point2i p1(cvroi.at<float>(0, 0), cvroi.at<float>(0, 1));
        cv::Point2i p2(cvroi.at<float>(1, 0), cvroi.at<float>(1, 1));
        // Set left point as the start end for each line
        if (p1.x > p2.x) {
          cv::Point2i p3 = p2;
          p2 = p1;
          p1 = p3;
        }
        cv::Point2i p5;
        cv::Point2i p6;
        util.BrewPoints(p1, p2, 100, &roi_, &p5, &p6);
        analyzer_.reset(new CDPAnalyzerCrossLine<Dtype>(roi_, p5, p6));
    }else {
        // not supporting.
        std::cerr << "To Be Continued" << std::endl;
        return false;
    }
    return analyzer_->Init();
}

template <typename Dtype>
bool CTask<Dtype>::InitTrainer(const std::string& task_name) {

    //int width = 0, height = 0;

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
    config_.setTaskPath(tasks[0]["task_path"]);
    config_.setPmapPath(tasks[0]["task_path"] + "PMap/" + density_detail[0]["pers_file"]);
    config_.setROIPath(tasks[0]["task_path"] + "ROI/" + density_detail[0]["roi_file"]);

    std::string gt_folder = config_.getTaskPath() + "GT/";

    // access the root folder of the current task.
    if(!boost::filesystem::exists(gt_folder) || !boost::filesystem::is_directory(gt_folder)) {
        std::cerr << "Cannot connect GT folder" << std::endl;
        return false;
    }

    boost::filesystem::recursive_directory_iterator it(gt_folder);
    boost::filesystem::recursive_directory_iterator endit;

    cv::Mat gt_frame;
    // figure out the size of frames.
    while(it != endit) {
        if(boost::filesystem::is_regular_file(*it) && it->path().extension() == ".jpg") {
            gt_frame = cv::imread(it->path().string());

            break;
        }

        ++it;
    }

    cv::Size gt_size = gt_frame.size();
    config_.setFrameHeight(gt_size.height);
    config_.setFrameWidth(gt_size.width);

    // Create buffer for communicating capturer with analyzer.
    cv::Mat frame(config_.getFrameHeight(), config_.getFrameWidth(), CV_8UC3);
    //int imgSize = frame.total() * frame.elemSize();

#if 0
    if (!buffer_.Init(config_.getFrameWidth(), config_.getFrameHeight(), imgSize, 50, 30, config_.getTaskName())) {
        std::cerr << "Cannot create buffer" << std::endl;
        return false;
    }
#endif

    // analyzer_ cannot be initialzed twice or deleted if it is alreaday initialized
    if (analyzer_ != 0) {
        std::cerr << "Analyzer is alrady initialized" << std::endl;
        return false;
    }
#if 0
    // analyzer applies for buffer to use.
    if (!buffer_.Init(config_.getTaskName())) {
        std::cerr << "Cannot create buffer" << std::endl;
        return false;
    }

    buffer_.frame_size(width, height);
    config_.setFrameWidth(width);
    config_.setFrameHeight(height);

#endif
    // Check task type
    std::vector<std::map<std::string, std::string> > res = db.Query("select task_type, task_path from Tasks where task_name='"+task_name+"';");

    // Instantiate Counting analyzer
    config_.setTaskType(0);
    analyzer_.reset(new CDPAnalyzerDensity<Dtype>(config_.getPmapPath(), config_.getROIPath(), config_.getFrameWidth(), config_.getFrameHeight()));
    analyzer_->Init();

    return true;
}

template <typename Dtype>
bool CTask<Dtype>::InitAlarmer(const std::string& task_name) {
    int w = 0, h = 0;
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

    buffer_.frame_size(w, h);
    config_.setFrameWidth(w);
    config_.setFrameHeight(h);

    // Check task type
    std::vector<std::map<std::string, std::string> > res = db.Query("select task_type, task_path from Tasks where task_name='"+task_name+"';");
    if (res[0]["task_type"].compare("COUNTING") == 0) {
        std::vector<std::map<std::string, std::string> > alarm_detail = db.Query("select * from DensityAlarmStrategy where task_name='" +task_name+ "';");
        if (alarm_detail.size() > 0) {
            alarmer_.priority_high = std::atoi(alarm_detail[0]["priority_high"].c_str());
            alarmer_.priority_medium = std::atoi(alarm_detail[0]["priority_medium"].c_str());
            alarmer_.priority_low = std::atoi(alarm_detail[0]["priority_low"].c_str());

            config_.setTaskPath(res[0]["task_path"]);
        } else {
            LOG(ERROR)<<"These is no any record in table DensityAlarmStrategy";

            return false;
        }
    }

    return true;
}

template <typename Dtype>
void CTask<Dtype>::Capture(int fps) {
    buffer_.set_camera_valid(true);
    while (getState()) {
        cv::Mat frame;
        time_t timestamp = camera_->Capture(frame);
        if (frame.empty()) break;
        // Write a new frame into buffer
        buffer_.put_src(static_cast<unsigned int>(timestamp), frame);
        //cv::imshow(config_.getTaskName() + "_frame", frame);
        //cv::waitKey(1000 / fps);

        // if do not use the following code line, frame will be inserted into buffer very soon.
        // it will cause buffer overflow.
        // it will time out every 30 milliseconds
        std::this_thread::sleep_for(std::chrono::milliseconds(1000/fps));
    }
    buffer_.set_camera_valid(false);
}

template <typename Dtype>
void CTask<Dtype>::Analyze() {
    switch (getTaskType()) {
        case TaskType_t::COUNTING:
            Counting();
            break;
        case TaskType_t::SEGMENTATION:
            Segmentation();
            break;
        case TaskType_t::STATIONARY:
            Stationary();
            break;
        case TaskType_t::CROSSLINE:
            CrossLine();
            break;
        default:
            LOG(WARNING) << "No Such Task Type";
    }
}

template <typename Dtype>
void CTask<Dtype>::Train(std::string filename) {
    std::vector<float> gt;
    std::vector<float> predict;

    // generate gt vector for training linear model.
    std::cout<<"filename_ = "<<config_.getTaskPath()<<std::endl;
    std::string gt_folder = config_.getTaskPath() + "GT/";
    std::vector<boost::filesystem::path> gt_frame, gt_coordinate;
    int current_frame = 0, current_coordinate = 0;
    cv::Mat output;
    //unsigned int timestamp = 0;
    itf::Util util;
    std::vector<cv::Mat> frame_container;

    // access the root folder of the current task.
    if(!boost::filesystem::exists(gt_folder) || !boost::filesystem::is_directory(gt_folder)) {
        LOG(ERROR)<<"the folder of the current task is not exist.";

        return;
    }

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
        //buffer_.put_src(output, timestamp);
        frame_container.push_back(output);
        current_frame++;
    }

    cv::Mat roi_mask;
    roi_mask = util.ReadROItoMAT(config_.getROIPath(), config_.getFrameHeight(), config_.getFrameWidth());

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

            std::cout<<"roi_mask.at<float>(contour["<<i<<"]) = "<<roi_mask.at<float>(contour[i])<<std::endl;
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
        std::cout<<"gt["<<i<<"] = "<<gt[i]<<std::endl;
        i++;
    }

    // generate predict vector for training linear model.
    if (analyzer_ == 0) {
        std::cerr << "The analyzer has not been initialized yet." << std::endl;
        return;
    }

    int rows = config_.getFrameHeight();
    int cols = config_.getFrameWidth();
    cv::Mat frame(rows, cols, CV_8UC3);

    // Get the perspective map and square it to generate a better heat map
    //cv::Mat pmap = util.ReadPMAPtoMAT("tmp_pers.csv");
    //pmap = pmap.mul(pmap);
    int index = 0;
    while (getState()) {
        //if (!buffer_.fetch_src(frame, timestamp)) {
        //if (!frame_container.pop_back(frame)) {

        //    break;
        //}

        if (index >= frame_container.size()) {
            break;
        }

        frame = frame_container.at(index);

        vector<float> feature = analyzer_->Analyze(frame);
        cv::Mat output(rows, cols, CV_32F, feature.data());
        //cv::Mat dst;

        //dst = util.GenerateHeatMap(output, pmap);
        int predicted_value = static_cast<int>(cv::sum(output)[0]);
        if (predicted_value < 0) {
            predicted_value = 0;
        }
        //buffer_.put_dst(dst, predicted_value);
        predict.push_back(predicted_value);

        index++;
    }

    // generate linear model.
    std::string save_name = config_.getTaskPath() + "LM/" + filename + ".csv";
    std::vector<float> model = util.TrainLinearModel(gt, predict, save_name);

    return;
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

/*
 * @brief Write predicted number into MySQL db.
 * @param interval Specifiy how often (seconds) to write to disk.
*/
template <typename Dtype>
void CTask<Dtype>::Alarm(int interval) {
    CDbi db = ConnectDB();
    srand (time(NULL));

    int rows = config_.getFrameHeight();
    int cols = config_.getFrameWidth();
    cv::Mat src_frame(rows, cols, CV_8UC3);
    cv::Mat dst_frame(rows, cols, CV_8UC3);

    int predicted_value;
    unsigned int timestamp;

    while (getState()) {
        if (!buffer_.fetch_dst(timestamp, src_frame, dst_frame, predicted_value)) {
            LOG(WARNING) <<"empty frame";

            continue;
        }

        sleep(interval);
        db.RunSQL("INSERT INTO DensityPredict VALUES (DEFAULT, "
        + std::to_string(predicted_value) + ", '"
        + config_.getTaskName() + "');");

        if (predicted_value > alarmer_.priority_high) {
          //std::string random_name = std::to_string(rand() % 99999 + 10000);
          std::string file_name = std::to_string(timestamp);
          db.RunSQL("INSERT INTO DensityAlarmRecord VALUES (DEFAULT, "
            + std::to_string(predicted_value) + ", 'HIGH', '"
            + file_name
            + "', '"
            + config_.getTaskName() + "');");

          // Uncomment to save
          cv::imwrite(config_.getTaskPath() + "Alarm/" + file_name +"_src.jpg", src_frame);
          cv::imwrite(config_.getTaskPath() + "Alarm/" + file_name +"_dst.jpg", dst_frame);
        } else if (predicted_value > alarmer_.priority_medium) {
          //std::string random_name = std::to_string(rand() % 9999 + 1000);
          std::string file_name = std::to_string(timestamp);
          db.RunSQL("INSERT INTO DensityAlarmRecord VALUES (DEFAULT, "
            + std::to_string(predicted_value) + ", 'MEDIUM', '"
            + file_name
            + "', '"
            + config_.getTaskName() + "');");
          cv::imwrite(config_.getTaskPath() + "Alarm/" + file_name +"_src.jpg", src_frame);
          cv::imwrite(config_.getTaskPath() + "Alarm/" + file_name +"_dst.jpg", dst_frame);
        } else if (predicted_value > alarmer_.priority_low) {
          //std::string file_name = std::to_string(rand() % 9999 + 1000);
          std::string file_name = std::to_string(timestamp);
          db.RunSQL("INSERT INTO DensityAlarmRecord VALUES (DEFAULT, "
            + std::to_string(predicted_value) + ", 'LOW', '"
            + file_name
            + "', '"
            + config_.getTaskName() + "');");
          cv::imwrite(config_.getTaskPath() + "Alarm/" + file_name +"_src.jpg", src_frame);
          cv::imwrite(config_.getTaskPath() + "Alarm/" + file_name +"_dst.jpg", dst_frame);
        }
    }

    LOG(INFO) <<"exit successfully";

    return;
}

template <typename Dtype>
void CTask<Dtype>::Counting() {
    int rows = config_.getFrameHeight();
    int cols = config_.getFrameWidth();
    itf::Util util;
    cv::Mat pmap;
    int predicted_value = 0;
    // Load the trained linear model.
    std::string lm = config_.getTaskPath() + "LM/" + "lm.csv";
    util.LoadLinearModel(lm);
    // Get the perspective map and square it to generate a better heat map
    pmap = util.ReadPMAPtoMAT("tmp_pers.csv");
    pmap = pmap.mul(pmap);
    while (getState()) {
        cv::Mat original_frame, density_frame;
        original_frame.create(rows, cols, CV_8UC3);
        unsigned int timestamp;
        if (!buffer_.fetch_src(timestamp, original_frame, true)) {
            std::cerr << "ad: No Available Frame for " << config_.getTaskName() << std::endl;
            sleep(3);  // reduce useless while loop
            continue;
        }
        vector<float> feature = analyzer_->Analyze(original_frame);
        // Post-processing
        cv::Mat output(rows, cols, CV_32F, feature.data());
        int tmp_predicted_value = static_cast<int>(cv::sum(output)[0]);
        if (tmp_predicted_value < 0) {
            tmp_predicted_value = 0;
        }
        // Predict a value using linear model.
        predicted_value = util.Predict(tmp_predicted_value);
        density_frame = util.GenerateHeatMap(output, pmap);
        if (predicted_value < 0) {
            predicted_value = 0;
        }
        buffer_.put_dst(timestamp, original_frame, density_frame, predicted_value);
        //cv::imshow(config_.getTaskName() + "_ad_result", density_frame);
        //cv::waitKey(1);
    }
}

template<typename Dtype>
void CTask<Dtype>::Segmentation() {
    int rows = config_.getFrameHeight();
    int cols = config_.getFrameWidth();
    while (getState()) {
        cv::Mat original_frame, density_frame;
        original_frame.create(rows, cols, CV_8UC3);
        unsigned int timestamp;
        if (!buffer_.fetch_src(timestamp, original_frame, true)) {
            std::cerr << "ad: No Available Frame for " << config_.getTaskName() << std::endl;
            sleep(3);  // reduce useless while loop
            continue;
        }
        vector<float> feature = analyzer_->Analyze(original_frame);
        // Post-processing
        cv::Mat output(rows, cols, CV_32F, feature.data());
        original_frame.copyTo(density_frame, output > 0.5);
        buffer_.put_dst(timestamp, density_frame);
        //cv::imshow(config_.getTaskName() + "_ad_result", density_frame);
        //cv::waitKey(1);
    }
}

template<typename Dtype>
void CTask<Dtype>::Stationary() {
    int rows = config_.getFrameHeight();
    int cols = config_.getFrameWidth();
    while (getState()) {
        cv::Mat original_frame, density_frame;
        original_frame.create(rows, cols, CV_8UC3);
        unsigned int timestamp;
        if (!buffer_.fetch_src(timestamp, original_frame, true)) {
            std::cerr << "ad: No Available Frame for " << config_.getTaskName() << std::endl;
            sleep(3);  // reduce useless while loop
            continue;
        }
        vector<float> feature = analyzer_->Analyze(original_frame);
        // Post-processing
        cv::Mat output(rows, cols, CV_8UC3, feature.data());
        density_frame = output + original_frame;
        buffer_.put_dst(timestamp, density_frame);
        // cv::imshow(config_.getTaskName() + "_ad_result", density_frame);
        // cv::waitKey(1);
    }
}

template<typename Dtype>
void CTask<Dtype>::CrossLine() {
    int rows = config_.getFrameHeight();
    int cols = config_.getFrameWidth();

    int h = abs(roi_.tl().y - roi_.br().y);
    int w = abs(roi_.tl().x - roi_.br().x);
    float predicted_value_1 = 0;
    float predicted_value_2 = 0;
    // load linear model
    itf::Util util;
    std::vector<float> linear_model = util.ReadCSV(config_.getTaskPath() + "LM/lm.csv");
    while (getState()) {
        cv::Mat frame;
        frame.create(rows, cols, CV_8UC3);
        unsigned int timestamp;
        if (!buffer_.fetch_src(timestamp, frame, true)) {
            std::cerr << "ad: No Available Frame for " << config_.getTaskName() << std::endl;
            sleep(3);  // reduce useless while loop
            continue;
        }
        vector<float> feature = analyzer_->Analyze(frame);
        // Post-processing
        // Get the size of output map. Note: the last two elements are predicted values
        size_t map_size = (feature.size() - 2) / 2;
        // Recover flow map
        std::vector<float> flow(feature.begin(), feature.begin() + map_size);
        cv::Mat flow_map(h, w, CV_8UC3, flow.data());
        cv::Mat flow_padding(rows, cols, CV_8UC3, cv::Scalar(255, 255, 255));
        flow_map.copyTo(flow_padding(roi_));
        // Recover density map
        std::vector<float> density(feature.begin() + map_size , feature.end() - 2);
        cv::Mat density_map(h, w, CV_8UC3, density.data());
        cv::Mat density_padding(rows, cols, CV_8UC3, cv::Scalar(128, 0, 0));
        density_map.copyTo(density_padding(roi_));
        // Retrive predicted values (two directions)
        std::vector<float> predicted_value (feature.end() - 2, feature.end());
        predicted_value_1 += predicted_value[0] * linear_model[0];
        predicted_value_2 += predicted_value[1] * linear_model[1];
        buffer_.put_dst(timestamp, flow_padding, density_padding,
            abs(predicted_value_1), abs(predicted_value_2));
        // cv::imshow(config_.getTaskName() + "_ad_flow_padding", flow_padding);
        // cv::imshow(config_.getTaskName() + "_ad_density_padding", density_padding);
        // cv::waitKey(1);
    }
}

INSTANTIATE_MYCLASS(CTask);
