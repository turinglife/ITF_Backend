//
// ITF Buffer Class, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "buffer.hpp"

CBuffer::CBuffer(const int &unit_size, const std::string &buffer_id) {

    buffer_id_ = buffer_id;
    head_.frame_size = unit_size;
    head_.header_size = sizeof(head_);
    
    boost::interprocess::shared_memory_object shm(boost::interprocess::open_or_create, buffer_id_.c_str(), boost::interprocess::read_write);
    // This move() gives its target the value of its argument, but is not obliged to preserve the value of its source.
    shm_ = boost::move(shm);
    shm_.truncate(head_.frame_size + head_.header_size);
    
    // Header region
    boost::interprocess::mapped_region region_header(shm_, boost::interprocess::read_write, 0, head_.header_size);
    region_header_ = boost::move(region_header);
    p_header_ = region_header_.get_address();
    memcpy(p_header_, &head_, head_.header_size);
    
    // Frame region
    boost::interprocess::mapped_region region_frame(shm_, boost::interprocess::read_write, head_.header_size);
    region_frame_ = boost::move(region_frame);
    p_frame_ = region_frame_.get_address();

}

CBuffer::CBuffer(const std::string &buffer_id) {
    
    buffer_id_ = buffer_id;
    
    boost::interprocess::shared_memory_object shm(boost::interprocess::open_only, buffer_id_.c_str(), boost::interprocess::read_only);
    // This move() gives its target the value of its argument, but is not obliged to preserve the value of its source.
    shm_ = boost::move(shm);
    
    // Frame region
    boost::interprocess::mapped_region region_frame(shm_, boost::interprocess::read_only);
    region_frame_ = boost::move(region_frame);
    p_frame_ = region_frame_.get_address();
    
}

void CBuffer::init(const int &unit_size) {
    
    head_.frame_size = unit_size;

}

void CBuffer::put(IN const cv::Mat &frame) {
    
    memcpy(p_frame_, frame.data, head_.frame_size);
    
    return;
}

void CBuffer::fetch(OUT cv::Mat &frame) {
    
    memcpy(frame.data, p_frame_, head_.frame_size);
    
}

bool CBuffer::destroy() {
    
    return boost::interprocess::shared_memory_object::remove(buffer_id_.c_str());
   
}



