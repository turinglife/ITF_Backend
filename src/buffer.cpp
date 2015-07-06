//
// ITF Buffer Class, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "buffer.hpp"
CBuffer::CBuffer(const int &src_width, const int &src_height, const int &unit_size, const int &src_num, const int &dst_num, const std::string &buffer_id) {

    buffer_id_ = buffer_id;
    head_.frame_width = src_width;
    head_.frame_height = src_height;
    head_.frame_size = unit_size;
    head_.src_buffer_num = src_num + 1;
    head_.dst_buffer_num = dst_num + 1;
    head_.header_size = sizeof(head_);

    boost::interprocess::shared_memory_object shm(boost::interprocess::open_or_create, buffer_id_.c_str(), boost::interprocess::read_write);
    // This move() gives its target the value of its argument, but is not obliged to preserve the value of its source.
    shm_ = boost::move(shm);
    shm_.truncate(head_.header_size + 4 * sizeof(int) + (head_.src_buffer_num + head_.dst_buffer_num) * head_.frame_size + head_.dst_buffer_num * sizeof(int));

    // Header region
    boost::interprocess::mapped_region region_header(shm_, boost::interprocess::read_write, 0, head_.header_size);
    region_header_ = boost::move(region_header);
    p_header_ = static_cast<unsigned char*>(region_header_.get_address());
    memcpy(p_header_, &head_, head_.header_size);

    // read and write index region
    boost::interprocess::mapped_region region_last_r_w(shm_, boost::interprocess::read_write, head_.header_size, 4*sizeof(int));
    region_last_r_w_ = boost::move(region_last_r_w);
    p_last_r_w_ = static_cast<unsigned char*>(region_last_r_w_.get_address());
    //unsigned char *mem_ptr = static_cast<unsigned char*>(p_last_r_w_);
    p_last_r_src_ = (int *)p_last_r_w_;
    p_last_w_src_ = (int *)((char *)p_last_r_src_ + sizeof(int));
    p_last_r_dst_ = (int *)((char *)p_last_w_src_ + sizeof(int));
    p_last_w_dst_ = (int *)((char *)p_last_r_dst_ + sizeof(int));

    *p_last_r_src_ = 0;
    *p_last_w_src_ = 0;
    *p_last_r_dst_ = 0;
    *p_last_w_dst_ = 0;

    //std::cout<< "last_r_src: " << *p_last_r_src_ << " , last_w_src: " << *p_last_w_src_ << std::endl;
    //std::cout<< "last_r_dst: " << *p_last_r_dst_ << " , last_w_dst: " << *p_last_w_dst_ << std::endl;

    // Frame region
    boost::interprocess::mapped_region region_src(shm_, boost::interprocess::read_write, head_.header_size + 4*sizeof(int), head_.src_buffer_num * head_.frame_size);
    region_src_ = boost::move(region_src);
    p_src_ = static_cast<unsigned char*>(region_src_.get_address());

    // Output Map region
    boost::interprocess::mapped_region region_dst_map(shm_, boost::interprocess::read_write, head_.header_size + 4*sizeof(int) + head_.src_buffer_num * head_.frame_size, head_.dst_buffer_num * head_.frame_size);
    region_dst_map_ = boost::move(region_dst_map);
    p_dst_map_ = static_cast<unsigned char*>(region_dst_map_.get_address());

    // Output value region
    boost::interprocess::mapped_region region_dst_val(shm_, boost::interprocess::read_write, head_.header_size + 4*sizeof(int) + (head_.src_buffer_num +head_.dst_buffer_num) * head_.frame_size, head_.dst_buffer_num * sizeof(int));
    region_dst_val_ = boost::move(region_dst_val);
    p_dst_val_ = static_cast<unsigned char*>(region_dst_val_.get_address());

}

CBuffer::CBuffer(const std::string &buffer_id) {

    buffer_id_ = buffer_id;

    boost::interprocess::shared_memory_object shm(boost::interprocess::open_only, buffer_id_.c_str(), boost::interprocess::read_write);
    // This move() gives its target the value of its argument, but is not obliged to preserve the value of its source.
    shm_ = boost::move(shm);

    // Header region
    boost::interprocess::mapped_region region_header(shm_, boost::interprocess::read_write, 0, sizeof(HeaderInfo_t));
    region_header_ = boost::move(region_header);
    p_header_ = static_cast<unsigned char*>(region_header_.get_address());
    memcpy(&head_, p_header_, sizeof(HeaderInfo_t));

    //std::cout << head_.frame_width << std::endl;
    //std::cout << head_.frame_height << std::endl;
    //std::cout << head_.frame_size << std::endl;
    //std::cout << head_.src_buffer_num << std::endl;
    //std::cout << head_.dst_buffer_num << std::endl;
    //std::cout << head_.header_size << std::endl;

    // read and write index region
    boost::interprocess::mapped_region region_last_r_w(shm_, boost::interprocess::read_write, head_.header_size, 4 * sizeof(int));
    region_last_r_w_ = boost::move(region_last_r_w);
    p_last_r_w_ = static_cast<unsigned char*>(region_last_r_w_.get_address());
    //unsigned char *mem_ptr = static_cast<unsigned char*>(p_last_r_w_);
    p_last_r_src_ = (int *)p_last_r_w_;
    p_last_w_src_ = (int *)((char *)p_last_r_src_ + sizeof(int));
    p_last_r_dst_ = (int *)((char *)p_last_w_src_ + sizeof(int));
    p_last_w_dst_ = (int *)((char *)p_last_r_dst_ + sizeof(int));

    boost::interprocess::mapped_region region_src(shm_, boost::interprocess::read_only, head_.header_size + 4 * sizeof(int), head_.src_buffer_num * head_.frame_size);
    region_src_ = boost::move(region_src);
    p_src_ = static_cast<unsigned char*>(region_src_.get_address());

    // Output Map region
    boost::interprocess::mapped_region region_dst_map(shm_, boost::interprocess::read_write, head_.header_size + 4 * sizeof(int) + head_.src_buffer_num * head_.frame_size, head_.dst_buffer_num * head_.frame_size);
    region_dst_map_ = boost::move(region_dst_map);
    p_dst_map_ = static_cast<unsigned char*>(region_dst_map_.get_address());

    // Output value region
    boost::interprocess::mapped_region region_dst_val(shm_, boost::interprocess::read_write, head_.header_size + 4 * sizeof(int) + (head_.src_buffer_num + head_.dst_buffer_num) * head_.frame_size, head_.dst_buffer_num * sizeof(int));
    region_dst_val_ = boost::move(region_dst_val);
    p_dst_val_ = static_cast<unsigned char*>(region_dst_val_.get_address());

}

void CBuffer::init(const int &unit_size) {
    //head_.frame_size = unit_size;

}

bool CBuffer::put_src(IN const cv::Mat &frame) {
    // check whether the input buffer is full or not;

    int curr_w_src = (*p_last_w_src_ + 1) % head_.src_buffer_num;
    if (curr_w_src == *p_last_r_src_) {
        return false;
    }

    //std::cout << "**************put_src*****************" << std::endl;
    //std::cout<< "last_r_src: " << *p_last_r_src_ << " , last_w_src: " << *p_last_w_src_ << std::endl;
    //std::cout<< "last_r_dst: " << *p_last_r_dst_ << " , last_w_dst: " << *p_last_w_dst_ << std::endl;

    memcpy(p_src_ + curr_w_src * head_.frame_size, frame.data, head_.frame_size);

    *p_last_w_src_ = curr_w_src;

    return true;

}

bool CBuffer::put_dst(IN const cv::Mat &frame, IN const int &predicted_value) {
    // check whether the output buffer is full or not;

    int curr_w_dst = (*p_last_w_dst_ + 1) % head_.dst_buffer_num;
    if (curr_w_dst == *p_last_r_dst_) {
        return false;
    }

    //std::cout << "**************put_dst*****************" << std::endl;
    //std::cout<< "last_r_dst: " << *p_last_r_dst_ << " , last_w_dst: " << *p_last_w_dst_ << std::endl;

    memcpy(p_dst_map_ + curr_w_dst * head_.frame_size, frame.data, head_.frame_size);

    memcpy(p_dst_val_ + curr_w_dst * sizeof(int), &predicted_value, sizeof(int));

    *p_last_w_dst_ = curr_w_dst;

    return true;

}

bool CBuffer::fetch_frame(OUT cv::Mat &frame) {
    // check whether the input buffer is empty or not;

    if (*p_last_r_src_ == *p_last_w_src_) {
        return false;
    }

    int curr_r_src = (*p_last_r_src_ + 1) % head_.src_buffer_num;

    memcpy(frame.data, p_src_ + curr_r_src * head_.frame_size, head_.frame_size);

    return true;
}

bool CBuffer::fetch_src(OUT cv::Mat &frame) {
    // check whether the input buffer is empty or not;

    if (*p_last_r_src_ == *p_last_w_src_) {
        return false;
    }

    int curr_r_src = (*p_last_r_src_ + 1) % head_.src_buffer_num;

    //std::cout << "__________fetch_src______________" << std::endl;
    //std::cout<< "last_r_src: " << *p_last_r_src_ << " , last_w_src: " << *p_last_w_src_ << std::endl;

    memcpy(frame.data, p_src_ + curr_r_src * head_.frame_size, head_.frame_size);

    *p_last_r_src_ = curr_r_src;

    return true;

}

bool CBuffer::fetch_dst(OUT cv::Mat &frame, OUT int &predicted_value) {
    // check whether the output buffer is empty or not;

    if (*p_last_r_dst_ == *p_last_w_dst_) {
        return false;
    }

    int curr_r_dst = (*p_last_r_dst_ + 1) % head_.dst_buffer_num;

    //std::cout << "__________fetch_dst______________" << std::endl;
    //std::cout<< "last_r_dst: " << *p_last_r_dst_ << " , last_w_dst: " << *p_last_w_dst_ << std::endl;

    memcpy(frame.data, p_dst_map_ + curr_r_dst * head_.frame_size, head_.frame_size);

    memcpy(&predicted_value, p_dst_val_ + curr_r_dst * sizeof(int), sizeof(int));

    *p_last_r_dst_ = curr_r_dst;

    return true;

}

//void CBuffer::unlock_buffer() {
//    // src buffer index add 1 after Front_End read src img;
//    *p_last_r_src_ = (*p_last_r_src_ + 1) % head_.src_buffer_num;
//
//    //std::cout << "__________unlock_buffer_______________" << std::endl;
//    //std::cout<< "last_r_src: " << *p_last_r_src_ << " , last_w_src: " << *p_last_w_src_ << std::endl;
//    //std::cout<< "last_r_dst: " << *p_last_r_dst_ << " , last_w_dst: " << *p_last_w_dst_ << std::endl;
//}

bool CBuffer::destroy() {

    return boost::interprocess::shared_memory_object::remove(buffer_id_.c_str());

}

