//
// Copyright 2015 CUHK
//

#include "regression_camera.hpp"


CRegressionCamera::CRegressionCamera(const std::string& filename, const int &start_index) : filename_(filename), currentfile_(start_index) {}




int CRegressionCamera::Connect() {
    rawtime_ = 0;
    
#if 0
    cap_.open(filename_);
    if (cap_.isOpened())
        return 1;
    else
        return 0;
#endif
    
    
    std::cout<<"filename_ = "<<filename_<<std::endl;
    std::string gt_folder = filename_ + "GT/";
    
    //std::vector<boost::filesystem::path> ret;
    
    if(!boost::filesystem::exists(gt_folder) || !boost::filesystem::is_directory(gt_folder)) 
        return 0;
    
    boost::filesystem::recursive_directory_iterator it(gt_folder);
    boost::filesystem::recursive_directory_iterator endit;

    while(it != endit)
    {
        if(boost::filesystem::is_regular_file(*it) && it->path().extension() == ".jpg")
            //ret_.push_back(it->path().filename());
            ret_.push_back(it->path());

        ++it;
    }
    
    numofiles_ = ret_.size();
    
//     for(int i = 0; i < ret_.size(); i++) {
//         std::cout<<"ret = "<<ret_[i]<<std::endl;
//     }

    return 1;
}

int CRegressionCamera::Disconnect() {
    
    return 1;
}

time_t CRegressionCamera::Capture(cv::Mat& output) {    
    if(currentfile_ < numofiles_) {
        output = cv::imread(ret_[currentfile_].string());
        std::cout<<"ret_ = "<<ret_[currentfile_]<<std::endl;
        
        currentfile_++;
    }
        
    rawtime_ += 1;
    
    return rawtime_;
}

