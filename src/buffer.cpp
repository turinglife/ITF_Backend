//
// ITF Buffer Class, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "buffer.hpp"

bool CBuffer::Init(int src_width, int src_height, int unit_size, int src_num, int dst_num, const std::string &buffer_id) {
    buffer_id_ = buffer_id;
    head_.frame_width = src_width;
    head_.frame_height = src_height;
    head_.frame_size = unit_size;
    head_.src_buffer_num = src_num + 1;
    head_.dst_buffer_num = dst_num + 1;
    head_.header_size = sizeof(head_);

    shm_ = boost::interprocess::shared_memory_object(boost::interprocess::open_or_create, buffer_id_.c_str(), boost::interprocess::read_write);
    /* header + index + timestamp + src + dst_map + dst_val */
    shm_.truncate(head_.header_size + 4 * sizeof(int) + head_.src_buffer_num * sizeof(unsigned int) + (head_.src_buffer_num + head_.dst_buffer_num) * head_.frame_size + head_.dst_buffer_num * sizeof(int));
    // Header region
    region_header_ = boost::interprocess::mapped_region(shm_, boost::interprocess::read_write, 0, head_.header_size);
    std::memset(region_header_.get_address(), 0, region_header_.get_size());
    p_header_ = static_cast<unsigned char*>(region_header_.get_address());
    memcpy(p_header_, &head_, head_.header_size);

    // read and write index region
    region_last_r_w_ = boost::interprocess::mapped_region(shm_, boost::interprocess::read_write, head_.header_size, 4 * sizeof(int));
    std::memset(region_last_r_w_.get_address(), 0, region_last_r_w_.get_size());
    p_last_r_w_ = static_cast<unsigned char*>(region_last_r_w_.get_address());
    p_last_r_src_ = (int *)p_last_r_w_;
    p_last_w_src_ = (int *)((char *)p_last_r_src_ + sizeof(int));
    p_last_r_dst_ = (int *)((char *)p_last_w_src_ + sizeof(int));
    p_last_w_dst_ = (int *)((char *)p_last_r_dst_ + sizeof(int));

    // Init index
    *p_last_r_src_ = 0;
    *p_last_w_src_ = 0;
    *p_last_r_dst_ = 0;
    *p_last_w_dst_ = 0;

    // Timestamp region
    region_timestamp_ = boost::interprocess::mapped_region(shm_, boost::interprocess::read_write, head_.header_size + 4 * sizeof(int), head_.src_buffer_num * sizeof(unsigned int));
    std::memset(region_timestamp_.get_address(), 0, region_timestamp_.get_size());
    p_timestamp_ = static_cast<unsigned char*>(region_timestamp_.get_address());

    // Frame region
    region_src_ = boost::interprocess::mapped_region(shm_, boost::interprocess::read_write, head_.header_size + 4 * sizeof(int) + head_.src_buffer_num * sizeof(unsigned int), head_.src_buffer_num * head_.frame_size);
    std::memset(region_src_.get_address(), 0, region_src_.get_size());
    p_src_ = static_cast<unsigned char*>(region_src_.get_address());

    // Output Map region
    region_dst_map_ = boost::interprocess::mapped_region(shm_, boost::interprocess::read_write, head_.header_size + 4 * sizeof(int) + head_.src_buffer_num * sizeof(unsigned int) + head_.src_buffer_num * head_.frame_size, head_.dst_buffer_num * head_.frame_size);
    std::memset(region_dst_map_.get_address(), 0, region_dst_map_.get_size());
    p_dst_map_ = static_cast<unsigned char*>(region_dst_map_.get_address());

    // Output value region
    region_dst_val_ = boost::interprocess::mapped_region(shm_, boost::interprocess::read_write, head_.header_size + 4 * sizeof(int) + head_.src_buffer_num * sizeof(unsigned int) + (head_.src_buffer_num +head_.dst_buffer_num) * head_.frame_size, head_.dst_buffer_num * sizeof(int));
    std::memset(region_dst_val_.get_address(), 0, region_dst_val_.get_size());
    p_dst_val_ = static_cast<unsigned char*>(region_dst_val_.get_address());

    // Init Timestamp to 0
    int init_value = 0;
    for (int i=0; i<head_.src_buffer_num; ++i) {
        memcpy(p_timestamp_ + i * sizeof(unsigned int), &init_value, sizeof(unsigned int));
    }
    // Init Output value to 0
    for (int i=0; i<head_.dst_buffer_num; ++i) {
        memcpy(p_dst_val_ + i * sizeof(int), &init_value, sizeof(int));
    }
    return true;
}

bool CBuffer::Init(const std::string &buffer_id) {
    buffer_id_ = buffer_id;

    shm_ = boost::interprocess::shared_memory_object(boost::interprocess::open_only, buffer_id_.c_str(), boost::interprocess::read_write);
    // Header region
    region_header_ = boost::interprocess::mapped_region(shm_, boost::interprocess::read_write, 0, sizeof(HeaderInfo_t));
    p_header_ = static_cast<unsigned char*>(region_header_.get_address());
    memcpy(&head_, p_header_, sizeof(HeaderInfo_t));

    // read and write index region
    region_last_r_w_ = boost::interprocess::mapped_region(shm_, boost::interprocess::read_write, head_.header_size, 4 * sizeof(int));
    p_last_r_w_ = static_cast<unsigned char*>(region_last_r_w_.get_address());
    p_last_r_src_ = (int *)p_last_r_w_;
    p_last_w_src_ = (int *)((char *)p_last_r_src_ + sizeof(int));
    p_last_r_dst_ = (int *)((char *)p_last_w_src_ + sizeof(int));
    p_last_w_dst_ = (int *)((char *)p_last_r_dst_ + sizeof(int));

    // Timestamp region
    region_timestamp_ = boost::interprocess::mapped_region(shm_, boost::interprocess::read_write, head_.header_size + 4 * sizeof(int), head_.src_buffer_num * sizeof(unsigned int));
    p_timestamp_ = static_cast<unsigned char*>(region_timestamp_.get_address());

    // Frame region
    region_src_ = boost::interprocess::mapped_region(shm_, boost::interprocess::read_write, head_.header_size + 4 * sizeof(int) + head_.src_buffer_num * sizeof(unsigned int), head_.src_buffer_num * head_.frame_size);
    p_src_ = static_cast<unsigned char*>(region_src_.get_address());

    // Output Map region
    region_dst_map_ = boost::interprocess::mapped_region(shm_, boost::interprocess::read_write, head_.header_size + 4 * sizeof(int) + head_.src_buffer_num * sizeof(unsigned int) + head_.src_buffer_num * head_.frame_size, head_.dst_buffer_num * head_.frame_size);
    p_dst_map_ = static_cast<unsigned char*>(region_dst_map_.get_address());

    // Output value region
    region_dst_val_ = boost::interprocess::mapped_region(shm_, boost::interprocess::read_write, head_.header_size + 4 * sizeof(int) + head_.src_buffer_num * sizeof(unsigned int) + (head_.src_buffer_num +head_.dst_buffer_num) * head_.frame_size, head_.dst_buffer_num * sizeof(int));
    p_dst_val_ = static_cast<unsigned char*>(region_dst_val_.get_address());

    return true;
}

bool CBuffer::frame_size(OUT int &width, OUT int &height) {
    width = head_.frame_width;
    height = head_.frame_height;

    if(width<=0 || height <=0) {
        return false;
    }

    return true;
}

bool CBuffer::put_src(IN const cv::Mat &frame, IN unsigned int timestamp) {
    // check whether the input buffer is full or not;
    int curr_w_src = (*p_last_w_src_ + 1) % head_.src_buffer_num;
    if (curr_w_src == *p_last_r_src_) {
        return false;
    }
    memcpy(p_src_ + curr_w_src * head_.frame_size, frame.data, head_.frame_size);
    memcpy(p_timestamp_ + curr_w_src * sizeof(unsigned int), &timestamp, sizeof(unsigned int));
    *p_last_w_src_ = curr_w_src;

    return true;
}

bool CBuffer::put_dst(IN const cv::Mat &frame, int predicted_value) {
    // check whether the output buffer is full or not;
    int curr_w_dst = (*p_last_w_dst_ + 1) % head_.dst_buffer_num;
    if (curr_w_dst == *p_last_r_dst_) {
        return false;
    }
    memcpy(p_dst_map_ + curr_w_dst * head_.frame_size, frame.data, head_.frame_size);
    memcpy(p_dst_val_ + curr_w_dst * sizeof(int), &predicted_value, sizeof(int));
    *p_last_w_dst_ = curr_w_dst;

    return true;
}

bool CBuffer::fetch_frame(OUT cv::Mat &frame, OUT unsigned int &timestamp) {
    // check whether the input buffer is empty or not;
    //if (*p_last_r_src_ == *p_last_w_src_) {
    //    return false;
    //}
    //int curr_r_src = (*p_last_r_src_ + 1) % head_.src_buffer_num;
    //memcpy(frame.data, p_src_ + curr_r_src * head_.frame_size, head_.frame_size);
    //memcpy(&timestamp, p_timestamp_ + curr_r_src * sizeof(unsigned int), sizeof(unsigned int));

    int curr_r_src = *p_last_r_src_;
    memcpy(frame.data, p_src_ + curr_r_src * head_.frame_size, head_.frame_size);
    memcpy(&timestamp, p_timestamp_ + curr_r_src * sizeof(unsigned int), sizeof(unsigned int));

    return true;
}

bool CBuffer::fetch_src(OUT cv::Mat &frame, OUT unsigned int &timestamp) {
    // check whether the input buffer is empty or not;
    if (*p_last_r_src_ == *p_last_w_src_) {
        return false;
    }

    int curr_r_src = (*p_last_r_src_ + 1) % head_.src_buffer_num;
    memcpy(frame.data, p_src_ + curr_r_src * head_.frame_size, head_.frame_size);
    memcpy(&timestamp, p_timestamp_ + curr_r_src * sizeof(unsigned int), sizeof(unsigned int));
    *p_last_r_src_ = curr_r_src;

    return true;
}

bool CBuffer::fetch_dst(OUT cv::Mat &frame, OUT int &predicted_value) {
    // check whether the output buffer is empty or not;
    if (*p_last_r_dst_ == *p_last_w_dst_) {
        return false;
    }
    int curr_r_dst = (*p_last_r_dst_ + 1) % head_.dst_buffer_num;
    memcpy(frame.data, p_dst_map_ + curr_r_dst * head_.frame_size, head_.frame_size);
    memcpy(&predicted_value, p_dst_val_ + curr_r_dst * sizeof(int), sizeof(int));
    *p_last_r_dst_ = curr_r_dst;
    return true;
}

bool CBuffer::destroy() {
    return boost::interprocess::shared_memory_object::remove(buffer_id_.c_str());
}
