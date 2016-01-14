//
// ITF Buffer Class, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "buffer.hpp"

bool CBuffer::Init(int src_width, int src_height, int unit_size, int src_num, int dst_num, const std::string &buffer_id) {
    //buffer_id_ = buffer_id;
    //Init buffer name
    buffer_id_header_ = buffer_id + std::string("_header");
    buffer_id_src_ = buffer_id + std::string("_src");
    buffer_id_dst_ = buffer_id + std::string("_dst");
    buffer_id_alarm_ = buffer_id + std::string("_alarm");

    // Init head_
    head_.frame_width = src_width;
    head_.frame_height = src_height;
    head_.frame_size = unit_size;
    head_.src_buffer_num = src_num + 1;
    head_.dst_buffer_num = dst_num + 1;
    head_.header_size = sizeof(head_);

    // Create Share Memory
    //shm_ = boost::interprocess::shared_memory_object(boost::interprocess::open_or_create, buffer_id_.c_str(), boost::interprocess::read_write);
    shm_header_ = boost::interprocess::shared_memory_object(boost::interprocess::open_or_create, buffer_id_header_.c_str(), boost::interprocess::read_write);
    shm_src_ = boost::interprocess::shared_memory_object(boost::interprocess::open_or_create, buffer_id_src_.c_str(), boost::interprocess::read_write);
    shm_dst_ = boost::interprocess::shared_memory_object(boost::interprocess::open_or_create, buffer_id_dst_.c_str(), boost::interprocess::read_write);
    shm_alarm_ = boost::interprocess::shared_memory_object(boost::interprocess::open_or_create, buffer_id_alarm_.c_str(), boost::interprocess::read_write);

    /* header + camera_status + index + timestamp + src + dst_map + dst_val + alarm_unit*/
    //shm_.truncate(head_.header_size + sizeof(bool) + 4 * sizeof(int) + head_.src_buffer_num * sizeof(unsigned int) + (head_.src_buffer_num + head_.dst_buffer_num) * head_.frame_size + head_.dst_buffer_num * sizeof(int) + (2 * head_.frame_size + sizeof(int) + sizeof(unsigned int)));
    /*header + camera_status*/
    shm_header_.truncate(head_.header_size + sizeof(bool));
    /*index + timestamp + src_frame*/
    shm_src_.truncate(2*sizeof(int) + head_.src_buffer_num*(sizeof(unsigned int) + head_.frame_size));
    /*index + timestamp + dst_frame_1 + dst_frame_2 + dst_val_1 + dst_val_2*/
    shm_dst_.truncate(2*sizeof(int) + head_.dst_buffer_num*(sizeof(unsigned int) + 2*head_.frame_size + 2*sizeof(int)));
    /*alarm unit for counting include timestamp, src_frame, dst_frame, dst_val*/
    shm_alarm_.truncate(sizeof(unsigned int) + 2*head_.frame_size + sizeof(int));

    /*shm_header_*/
    // Header region
    region_header_ = boost::interprocess::mapped_region(shm_header_, boost::interprocess::read_write, 0, head_.header_size);
    std::memset(region_header_.get_address(), 0, region_header_.get_size());
    p_header_ = static_cast<unsigned char*>(region_header_.get_address());
    memcpy(p_header_, &head_, head_.header_size);

    // Camera Status region
    region_camera_status_ = boost::interprocess::mapped_region(shm_header_, boost::interprocess::read_write, head_.header_size, sizeof(bool));
    std::memset(region_camera_status_.get_address(), 0, region_camera_status_.get_size());
    p_camera_status_ = static_cast<unsigned char*>(region_camera_status_.get_address());

    /*shm_src_*/
    // read and write src index region
    region_index_src_ = boost::interprocess::mapped_region(shm_src_, boost::interprocess::read_write, 0, 2*sizeof(int));
    std::memset(region_index_src_.get_address(), 0, region_index_src_.get_size());
    p_index_src_ = static_cast<unsigned char*>(region_index_src_.get_address());
    p_last_r_src_ = (int *)p_index_src_;
    p_last_w_src_ = (int *)((char *)p_last_r_src_ + sizeof(int));

    // src timestamp region
    region_timestamp_src_ = boost::interprocess::mapped_region(shm_src_, boost::interprocess::read_write, 2*sizeof(int), head_.src_buffer_num*sizeof(unsigned int));
    std::memset(region_timestamp_src_.get_address(), 0, region_timestamp_src_.get_size());
    p_timestamp_src_ = static_cast<unsigned char*>(region_timestamp_src_.get_address());

    // src frame region
    region_src_frame_ = boost::interprocess::mapped_region(shm_src_, boost::interprocess::read_write, 2*sizeof(int) + head_.src_buffer_num*sizeof(unsigned int), head_.src_buffer_num*head_.frame_size);
    std::memset(region_src_frame_.get_address(), 0, region_src_frame_.get_size());
    p_src_frame_ = static_cast<unsigned char*>(region_src_frame_.get_address());

    /*shm_dst_*/
    // read and write dst index region
    region_index_dst_ = boost::interprocess::mapped_region(shm_dst_, boost::interprocess::read_write, 0, 2*sizeof(int));
    std::memset(region_index_dst_.get_address(), 0, region_index_dst_.get_size());
    p_index_dst_ = static_cast<unsigned char*>(region_index_dst_.get_address());
    p_last_r_dst_ = (int *)p_index_dst_;
    p_last_w_dst_ = (int *)((char *)p_last_r_dst_ + sizeof(int));

    // dst timestamp region
    region_timestamp_dst_ = boost::interprocess::mapped_region(shm_dst_, boost::interprocess::read_write, 2*sizeof(int), head_.dst_buffer_num*sizeof(unsigned int));
    std::memset(region_timestamp_dst_.get_address(), 0, region_timestamp_dst_.get_size());
    p_timestamp_dst_ = static_cast<unsigned char*>(region_timestamp_dst_.get_address());

    // dst frame 1 region
    region_dst_frame_1_ = boost::interprocess::mapped_region(shm_dst_, boost::interprocess::read_write, 2*sizeof(int) + head_.dst_buffer_num*sizeof(unsigned int), head_.dst_buffer_num*head_.frame_size);
    std::memset(region_dst_frame_1_.get_address(), 0, region_dst_frame_1_.get_size());
    p_dst_frame_1_ = static_cast<unsigned char*>(region_dst_frame_1_.get_address());

    // dst frame 2 region
    region_dst_frame_2_ = boost::interprocess::mapped_region(shm_dst_, boost::interprocess::read_write, 2*sizeof(int) + head_.dst_buffer_num*(sizeof(unsigned int) + head_.frame_size), head_.dst_buffer_num*head_.frame_size);
    std::memset(region_dst_frame_2_.get_address(), 0, region_dst_frame_2_.get_size());
    p_dst_frame_2_ = static_cast<unsigned char*>(region_dst_frame_2_.get_address());

    // dst val 1 region
    region_dst_val_1_ = boost::interprocess::mapped_region(shm_dst_, boost::interprocess::read_write, 2*sizeof(int) + head_.dst_buffer_num*(sizeof(unsigned int) + 2*head_.frame_size), head_.dst_buffer_num*sizeof(int));
    std::memset(region_dst_val_1_.get_address(), 0, region_dst_val_1_.get_size());
    p_dst_val_1_ = static_cast<unsigned char*>(region_dst_val_1_.get_address());

    // dst val 2 region
    region_dst_val_2_ = boost::interprocess::mapped_region(shm_dst_, boost::interprocess::read_write, 2*sizeof(int) + head_.dst_buffer_num*(sizeof(unsigned int) + 2*head_.frame_size + sizeof(int)), head_.dst_buffer_num*sizeof(int));
    std::memset(region_dst_val_2_.get_address(), 0, region_dst_val_2_.get_size());
    p_dst_val_2_ = static_cast<unsigned char*>(region_dst_val_2_.get_address());

    /*shm_alarm_*/
    // Counint Alarm Unit region
    region_alarm_unit_ = boost::interprocess::mapped_region(shm_alarm_, boost::interprocess::read_write, 0, sizeof(unsigned int) + 2*head_.frame_size + sizeof(int));
    std::memset(region_alarm_unit_.get_address(), 0, region_alarm_unit_.get_size());
    p_alarm_unit_ = static_cast<unsigned char*>(region_alarm_unit_.get_address());

    /*INIT*/
    // Init camera_status
    bool init_camera_status = true;
    memcpy(p_camera_status_, &init_camera_status, sizeof(bool));

    // Init src timestamp to 0
    int init_value = 0;
    for (int i=0; i<head_.src_buffer_num; ++i) {
        memcpy(p_timestamp_src_ + i*sizeof(unsigned int), &init_value, sizeof(unsigned int));
    }

    // Init index
    *p_last_r_src_ = 0;
    *p_last_w_src_ = 0;
    *p_last_r_dst_ = 0;
    *p_last_w_dst_ = 0;

    // Init dst timestamp and value to 0
    for (int i=0; i<head_.dst_buffer_num; ++i) {
        memcpy(p_timestamp_dst_ + i*sizeof(unsigned int), &init_value, sizeof(unsigned int));
        memcpy(p_dst_val_1_ + i*sizeof(int), &init_value, sizeof(int));
        memcpy(p_dst_val_2_ + i*sizeof(int), &init_value, sizeof(int));
    }

    // Init Alarm Unit
    memcpy(p_alarm_unit_ + 2*head_.frame_size, &init_value, sizeof(int));
    memcpy(p_alarm_unit_ + 2*head_.frame_size + sizeof(int), &init_value, sizeof(unsigned int));

    return true;
}

bool CBuffer::Init(const std::string &buffer_id) {
    //buffer_id_ = buffer_id;
    //Init buffer name
    buffer_id_header_ = buffer_id + std::string("_header");
    buffer_id_src_ = buffer_id + std::string("_src");
    buffer_id_dst_ = buffer_id + std::string("_dst");
    buffer_id_alarm_ = buffer_id + std::string("_alarm");

    // Create Share Memory
    //shm_ = boost::interprocess::shared_memory_object(boost::interprocess::open_or_create, buffer_id_.c_str(), boost::interprocess::read_write);
    shm_header_ = boost::interprocess::shared_memory_object(boost::interprocess::open_or_create, buffer_id_header_.c_str(), boost::interprocess::read_write);
    shm_src_ = boost::interprocess::shared_memory_object(boost::interprocess::open_or_create, buffer_id_src_.c_str(), boost::interprocess::read_write);
    shm_dst_ = boost::interprocess::shared_memory_object(boost::interprocess::open_or_create, buffer_id_dst_.c_str(), boost::interprocess::read_write);
    shm_alarm_ = boost::interprocess::shared_memory_object(boost::interprocess::open_or_create, buffer_id_alarm_.c_str(), boost::interprocess::read_write);

    /*shm_header_*/
    // Header region
    region_header_ = boost::interprocess::mapped_region(shm_header_, boost::interprocess::read_write, 0, head_.header_size);
    p_header_ = static_cast<unsigned char*>(region_header_.get_address());
    memcpy(&head_, p_header_, sizeof(HeaderInfo_t));

    // Camera Status region
    region_camera_status_ = boost::interprocess::mapped_region(shm_header_, boost::interprocess::read_write, head_.header_size, sizeof(bool));
    p_camera_status_ = static_cast<unsigned char*>(region_camera_status_.get_address());

    /*shm_src_*/
    // read and write src index region
    region_index_src_ = boost::interprocess::mapped_region(shm_src_, boost::interprocess::read_write, 0, 2*sizeof(int));
    p_index_src_ = static_cast<unsigned char*>(region_index_src_.get_address());
    p_last_r_src_ = (int *)p_index_src_;
    p_last_w_src_ = (int *)((char *)p_last_r_src_ + sizeof(int));

    // src timestamp region
    region_timestamp_src_ = boost::interprocess::mapped_region(shm_src_, boost::interprocess::read_write, 2*sizeof(int), head_.src_buffer_num*sizeof(unsigned int));
    p_timestamp_src_ = static_cast<unsigned char*>(region_timestamp_src_.get_address());

    // src frame region
    region_src_frame_ = boost::interprocess::mapped_region(shm_src_, boost::interprocess::read_write, 2*sizeof(int) + head_.src_buffer_num*sizeof(unsigned int), head_.src_buffer_num*head_.frame_size);
    p_src_frame_ = static_cast<unsigned char*>(region_src_frame_.get_address());

    /*shm_dst_*/
    // read and write dst index region
    region_index_dst_ = boost::interprocess::mapped_region(shm_dst_, boost::interprocess::read_write, 0, 2*sizeof(int));
    p_index_dst_ = static_cast<unsigned char*>(region_index_dst_.get_address());
    p_last_r_dst_ = (int *)p_index_dst_;
    p_last_w_dst_ = (int *)((char *)p_last_r_dst_ + sizeof(int));

    // dst timestamp region
    region_timestamp_dst_ = boost::interprocess::mapped_region(shm_dst_, boost::interprocess::read_write, 2*sizeof(int), head_.dst_buffer_num*sizeof(unsigned int));
    p_timestamp_dst_ = static_cast<unsigned char*>(region_timestamp_dst_.get_address());

    // dst frame 1 region
    region_dst_frame_1_ = boost::interprocess::mapped_region(shm_dst_, boost::interprocess::read_write, 2*sizeof(int) + head_.dst_buffer_num*sizeof(unsigned int), head_.dst_buffer_num*head_.frame_size);
    p_dst_frame_1_ = static_cast<unsigned char*>(region_dst_frame_1_.get_address());

    // dst frame 2 region
    region_dst_frame_2_ = boost::interprocess::mapped_region(shm_dst_, boost::interprocess::read_write, 2*sizeof(int) + head_.dst_buffer_num*(sizeof(unsigned int) + head_.frame_size), head_.dst_buffer_num*head_.frame_size);
    p_dst_frame_2_ = static_cast<unsigned char*>(region_dst_frame_2_.get_address());

    // dst val 1 region
    region_dst_val_1_ = boost::interprocess::mapped_region(shm_dst_, boost::interprocess::read_write, 2*sizeof(int) + head_.dst_buffer_num*(sizeof(unsigned int) + 2*head_.frame_size), head_.dst_buffer_num*sizeof(int));
    p_dst_val_1_ = static_cast<unsigned char*>(region_dst_val_1_.get_address());

    // dst val 2 region
    region_dst_val_2_ = boost::interprocess::mapped_region(shm_dst_, boost::interprocess::read_write, 2*sizeof(int) + head_.dst_buffer_num*(sizeof(unsigned int) + 2*head_.frame_size + sizeof(int)), head_.dst_buffer_num*sizeof(int));
    p_dst_val_2_ = static_cast<unsigned char*>(region_dst_val_2_.get_address());

    /*shm_alarm_*/
    // Counint Alarm Unit region
    region_alarm_unit_ = boost::interprocess::mapped_region(shm_alarm_, boost::interprocess::read_write, 0, sizeof(unsigned int) + 2*head_.frame_size + sizeof(int));
    p_alarm_unit_ = static_cast<unsigned char*>(region_alarm_unit_.get_address());

    return true;
}

bool CBuffer::set_camera_valid(IN bool flag) {
    memcpy(p_camera_status_, &flag, sizeof(bool));
    return true;
}

bool CBuffer::is_camera_valid() {
    bool *p_flag = (bool *)p_camera_status_;
    return *p_flag;
}

bool CBuffer::frame_size(OUT int &width, OUT int &height) {
    width = head_.frame_width;
    height = head_.frame_height;

    if(width<=0 || height <=0) {
        return false;
    }

    return true;
}

//bool CBuffer::put_src(IN const cv::Mat &frame, IN unsigned int timestamp) {
//    // check whether the input buffer is full or not;
//    int curr_w_src = (*p_last_w_src_ + 1) % head_.src_buffer_num;
//    if (curr_w_src == *p_last_r_src_) {
//        return false;
//    }
//    memcpy(p_src_ + curr_w_src * head_.frame_size, frame.data, head_.frame_size);
//    memcpy(p_timestamp_ + curr_w_src * sizeof(unsigned int), &timestamp, sizeof(unsigned int));
//    *p_last_w_src_ = curr_w_src;
//
//    return true;
//}
bool CBuffer::put_src(IN unsigned int timestamp, IN const cv::Mat &src) {
    // check whether the input buffer is full or not;
    int curr_w_src = (*p_last_w_src_ + 1) % head_.src_buffer_num;
    if (curr_w_src == *p_last_r_src_) {
        return false;
    }

    memcpy(p_timestamp_src_ + curr_w_src*sizeof(unsigned int), &timestamp, sizeof(unsigned int));
    memcpy(p_src_frame_ + curr_w_src*head_.frame_size, src.data, head_.frame_size);
    *p_last_w_src_ = curr_w_src;

    return true;
}

//bool CBuffer::fetch_frame(OUT cv::Mat &frame, OUT unsigned int &timestamp) {
//    // check whether the input buffer is empty or not;
//    //if (*p_last_r_src_ == *p_last_w_src_) {
//    //    return false;
//    //}
//    //int curr_r_src = (*p_last_r_src_ + 1) % head_.src_buffer_num;
//    //memcpy(frame.data, p_src_ + curr_r_src * head_.frame_size, head_.frame_size);
//    //memcpy(&timestamp, p_timestamp_ + curr_r_src * sizeof(unsigned int), sizeof(unsigned int));
//
//    int curr_r_src = *p_last_r_src_;
//    memcpy(frame.data, p_src_ + curr_r_src * head_.frame_size, head_.frame_size);
//    memcpy(&timestamp, p_timestamp_ + curr_r_src * sizeof(unsigned int), sizeof(unsigned int));
//
//    return true;
//}
//bool CBuffer::fetch_src(OUT cv::Mat &frame, OUT unsigned int &timestamp) {
//    // check whether the input buffer is empty or not;
//    if (*p_last_r_src_ == *p_last_w_src_) {
//        return false;
//    }
//
//    int curr_r_src = (*p_last_r_src_ + 1) % head_.src_buffer_num;
//    memcpy(frame.data, p_src_ + curr_r_src * head_.frame_size, head_.frame_size);
//    memcpy(&timestamp, p_timestamp_ + curr_r_src * sizeof(unsigned int), sizeof(unsigned int));
//    *p_last_r_src_ = curr_r_src;
//
//    return true;
//}
bool CBuffer::fetch_src(OUT unsigned int &timestamp, OUT cv::Mat &src, IN bool flag) {
    // flag determine whether keep the src after fetching
    // flag is true, then keep the src (buffer index unchange)
    if (flag) {
        int curr_r_src = *p_last_r_src_;
        memcpy(&timestamp, p_timestamp_src_ + curr_r_src*sizeof(unsigned int), sizeof(unsigned int));
        memcpy(src.data, p_src_frame_ + curr_r_src*head_.frame_size, head_.frame_size);

        return true;
    }
    // flag is false, then discard the src after feaching (buffer index change)
    // check whether the input buffer is empty or not;
    if (*p_last_r_src_ == *p_last_w_src_) {
        return false;
    }
    int curr_r_src = (*p_last_r_src_ + 1) % head_.src_buffer_num;
    memcpy(&timestamp, p_timestamp_src_ + curr_r_src*sizeof(unsigned int), sizeof(unsigned int));
    memcpy(src.data, p_src_frame_ + curr_r_src*head_.frame_size, head_.frame_size);
    *p_last_r_src_ = curr_r_src;

    return true;
}

//bool CBuffer::put_dst(IN const cv::Mat &frame, IN int predicted_value) {
//    // check whether the output buffer is full or not;
//    int curr_w_dst = (*p_last_w_dst_ + 1) % head_.dst_buffer_num;
//    if (curr_w_dst == *p_last_r_dst_) {
//        return false;
//    }
//    memcpy(p_dst_map_ + curr_w_dst * head_.frame_size, frame.data, head_.frame_size);
//    memcpy(p_dst_val_ + curr_w_dst * sizeof(int), &predicted_value, sizeof(int));
//    *p_last_w_dst_ = curr_w_dst;
//
//    return true;
//}
//bool CBuffer::fetch_dst(OUT cv::Mat &frame, OUT int &predicted_value) {
//    // check whether the output buffer is empty or not;
//    if (*p_last_r_dst_ == *p_last_w_dst_) {
//        return false;
//    }
//    int curr_r_dst = (*p_last_r_dst_ + 1) % head_.dst_buffer_num;
//    memcpy(frame.data, p_dst_map_ + curr_r_dst * head_.frame_size, head_.frame_size);
//    memcpy(&predicted_value, p_dst_val_ + curr_r_dst * sizeof(int), sizeof(int));
//    *p_last_r_dst_ = curr_r_dst;
//    return true;
//}
//
/* for segmentation and stationary */
bool CBuffer::put_dst(IN unsigned int timestamp, IN const cv::Mat &dst_1) {
    // check whether the output buffer is full or not;
    int curr_w_dst = (*p_last_w_dst_ + 1) % head_.dst_buffer_num;
    if (curr_w_dst == *p_last_r_dst_) {
        return false;
    }
    memcpy(p_timestamp_dst_ + curr_w_dst*sizeof(unsigned int), &timestamp, sizeof(unsigned int));
    memcpy(p_dst_frame_1_ + curr_w_dst*head_.frame_size, dst_1.data, head_.frame_size);
    *p_last_w_dst_ = curr_w_dst;

    return true;
}
bool CBuffer::fetch_dst(OUT unsigned int timestamp, OUT cv::Mat &dst_1, IN bool flag) {
    // flag determine whether keep the dst frame after fetching
    // flag is true, then keep the dst frame (buffer index unchange)
    if (flag) {
        int curr_r_dst = *p_last_r_dst_;
        memcpy(&timestamp, p_timestamp_dst_ + curr_r_dst*sizeof(unsigned int), sizeof(unsigned int));
        memcpy(dst_1.data, p_dst_frame_1_ + curr_r_dst*head_.frame_size, head_.frame_size);

        return true;
    }
    // flag is false, then discard the dst frame after feaching (buffer index change)
    // check whether the dst buffer is empty or not;
    if (*p_last_r_dst_ == *p_last_w_dst_) {
        return false;
    }
    int curr_r_dst = (*p_last_r_dst_ + 1) % head_.dst_buffer_num;
    memcpy(&timestamp, p_timestamp_dst_ + curr_r_dst*sizeof(unsigned int), sizeof(unsigned int));
    memcpy(dst_1.data, p_dst_frame_1_ + curr_r_dst*head_.frame_size, head_.frame_size);
    *p_last_r_dst_ = curr_r_dst;

    return true;
}
/* for counting */
bool CBuffer::put_dst(IN unsigned int timestamp, IN const cv::Mat &dst_1, IN int val_1) {
    // check whether the dst buffer is full or not;
    int curr_w_dst = (*p_last_w_dst_ + 1) % head_.dst_buffer_num;
    if (curr_w_dst == *p_last_r_dst_) {
        return false;
    }
    memcpy(p_timestamp_dst_ + curr_w_dst*sizeof(unsigned int), &timestamp, sizeof(unsigned int));
    memcpy(p_dst_frame_1_ + curr_w_dst*head_.frame_size, dst_1.data, head_.frame_size);
    memcpy(p_dst_val_1_ + curr_w_dst*sizeof(int), &val_1, sizeof(int));
    *p_last_w_dst_ = curr_w_dst;

    return true;
}
bool CBuffer::fetch_dst(OUT unsigned int timestamp, OUT cv::Mat &dst_1, OUT int &val_1, IN bool flag) {
    // flag determine whether keep the dst frame after fetching
    // flag is true, then keep the dst (buffer index unchange)
    if (flag) {
        int curr_r_dst = *p_last_r_dst_;
        memcpy(&timestamp, p_timestamp_dst_ + curr_r_dst*sizeof(unsigned int), sizeof(unsigned int));
        memcpy(dst_1.data, p_dst_frame_1_ + curr_r_dst*head_.frame_size, head_.frame_size);
        memcpy(&val_1, p_dst_val_1_ + curr_r_dst*sizeof(int), sizeof(int));

        return true;
    }
    // flag is false, then discard the dst frame after feaching (buffer index change)
    // check whether the dst buffer is empty or not;
    if (*p_last_r_dst_ == *p_last_w_dst_) {
        return false;
    }
    int curr_r_dst = (*p_last_r_dst_ + 1) % head_.dst_buffer_num;
    memcpy(&timestamp, p_timestamp_dst_ + curr_r_dst*sizeof(unsigned int), sizeof(unsigned int));
    memcpy(dst_1.data, p_dst_frame_1_ + curr_r_dst*head_.frame_size, head_.frame_size);
    memcpy(&val_1, p_dst_val_1_ + curr_r_dst*sizeof(int), sizeof(int));
    *p_last_r_dst_ = curr_r_dst;

    return true;
}
/* for counting alarm */
bool CBuffer::put_dst(IN unsigned int timestamp, IN const cv::Mat &src, IN const cv::Mat &dst, IN int val) {
    memcpy(p_alarm_unit_, &timestamp, sizeof(unsigned int));
    memcpy(p_alarm_unit_ + sizeof(unsigned int), src.data, head_.frame_size);
    memcpy(p_alarm_unit_ + sizeof(unsigned int) + head_.frame_size, dst.data, head_.frame_size);
    memcpy(p_alarm_unit_ + sizeof(unsigned int) + 2*head_.frame_size, &val, sizeof(int));

    //return true;
    return put_dst(timestamp, dst, val);
}
bool CBuffer::fetch_dst(OUT unsigned int timestamp, OUT cv::Mat &src, OUT cv::Mat &dst, OUT int &val) {
    memcpy(&timestamp, p_alarm_unit_, sizeof(unsigned int));
    memcpy(src.data, p_alarm_unit_ + sizeof(unsigned int), head_.frame_size);
    memcpy(dst.data, p_alarm_unit_ + sizeof(unsigned int) + head_.frame_size, head_.frame_size);
    memcpy(&val, p_alarm_unit_ + sizeof(unsigned int) + 2*head_.frame_size, sizeof(int));

    return true;
}
/* for crossline */
bool CBuffer::put_dst(IN unsigned int timestamp, IN const cv::Mat &dst_1, IN const cv::Mat &dst_2, IN int val_1, IN int val_2) {
    // check whether the output buffer is full or not;
    int curr_w_dst = (*p_last_w_dst_ + 1) % head_.dst_buffer_num;
    if (curr_w_dst == *p_last_r_dst_) {
        return false;
    }
    memcpy(p_timestamp_dst_ + curr_w_dst*sizeof(unsigned int), &timestamp, sizeof(unsigned int));
    memcpy(p_dst_frame_1_ + curr_w_dst*head_.frame_size, dst_1.data, head_.frame_size);
    memcpy(p_dst_frame_2_ + curr_w_dst*head_.frame_size, dst_2.data, head_.frame_size);
    memcpy(p_dst_val_1_ + curr_w_dst*sizeof(int), &val_1, sizeof(int));
    memcpy(p_dst_val_2_ + curr_w_dst*sizeof(int), &val_2, sizeof(int));
    *p_last_w_dst_ = curr_w_dst;

    return true;;
}
bool CBuffer::fetch_dst(OUT unsigned int timestamp, OUT cv::Mat &dst_1, OUT cv::Mat &dst_2, OUT int &val_1, OUT int &val_2, IN bool flag) {
    // flag determine whether keep the dst frame after fetching
    // flag is true, then keep the dst (buffer index unchange)
    if (flag) {
        int curr_r_dst = *p_last_r_dst_;
        memcpy(&timestamp, p_timestamp_dst_ + curr_r_dst*sizeof(unsigned int), sizeof(unsigned int));
        memcpy(dst_1.data, p_dst_frame_1_ + curr_r_dst*head_.frame_size, head_.frame_size);
        memcpy(dst_2.data, p_dst_frame_2_ + curr_r_dst*head_.frame_size, head_.frame_size);
        memcpy(&val_1, p_dst_val_1_ + curr_r_dst*sizeof(int), sizeof(int));
        memcpy(&val_2, p_dst_val_2_ + curr_r_dst*sizeof(int), sizeof(int));

        return true;
    }
    // flag is false, then discard the dst frame after feaching (buffer index change)
    // check whether the dst buffer is empty or not;
    if (*p_last_r_dst_ == *p_last_w_dst_) {
        return false;
    }
    int curr_r_dst = (*p_last_r_dst_ + 1) % head_.dst_buffer_num;
    memcpy(&timestamp, p_timestamp_dst_ + curr_r_dst*sizeof(unsigned int), sizeof(unsigned int));
    memcpy(dst_1.data, p_dst_frame_1_ + curr_r_dst*head_.frame_size, head_.frame_size);
    memcpy(dst_2.data, p_dst_frame_2_ + curr_r_dst*head_.frame_size, head_.frame_size);
    memcpy(&val_1, p_dst_val_1_ + curr_r_dst*sizeof(int), sizeof(int));
    memcpy(&val_2, p_dst_val_2_ + curr_r_dst*sizeof(int), sizeof(int));
    *p_last_r_dst_ = curr_r_dst;

    return true;
}


bool CBuffer::destroy() {
    return ((boost::interprocess::shared_memory_object::remove(buffer_id_header_.c_str())) &&
            (boost::interprocess::shared_memory_object::remove(buffer_id_src_.c_str())) &&
            (boost::interprocess::shared_memory_object::remove(buffer_id_dst_.c_str())) &&
            (boost::interprocess::shared_memory_object::remove(buffer_id_alarm_.c_str())));
}
