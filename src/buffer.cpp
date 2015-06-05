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


void CBuffer::init(const int &unit_size) {
    
    //buffer_id_ = buffer_id;
    
    //head_.frame_size = unit_size;
    //head_.header_size = sizeof(head_);

    //using boost::interprocess::shared_memory_object;
    //using boost::interprocess::open_or_create;
    //using boost::interprocess::read_write;

    //struct shm_remove {
    //    shm_remove() { boost::interprocess::shared_memory_object::remove(buffer_id_.c_str()); }
    //    ~shm_remove() { boost::interprocess::shared_memory_object::remove(buffer_id_.c_str()); }
    //};

    //boost::interprocess::shared_memory_object shm(open_or_create, buffer_id_.c_str(), read_write);
    //shm_ = shm;
    //p_shm_ = new boost::interprocess::shared_memory_object(open_or_create, buffer_id_.c_str(), read_write);
    //shm_.truncate(head_.frame_size + head_.header_size);
    
    // Header region
    //boost::interprocess::mapped_region region_header(shm_, read_write, 0, head_.header_size);
    //region_header_ = region_header;
    //p_header_ = region_header_.get_address();
    //region_header_ = new boost::interprocess::mapped_region(*p_shm_, read_write, 0, head_.header_size);
    //p_header_ = region_header_.get_address();
    //memcpy(p_header_, &head_, head_.header_size);
    
    // Frame region
    //boost::interprocess::mapped_region region_frame(shm_, read_write, head_.header_size);
    //region_frame_ = region_frame;
    //p_region_frame_ = new boost::interprocess::mapped_region(*p_shm_, read_write, head_.header_size);
    //p_frame_ = region_frame.get_address();

    return;
}

void CBuffer::put(const cv::Mat &frame) {
    
    memcpy(p_frame_, frame.data, head_.frame_size);
    
    return;
}

bool CBuffer::destroy() {
    
    return boost::interprocess::shared_memory_object::remove(buffer_id_.c_str());
   
}



