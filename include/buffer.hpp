//
// ITF Buffer Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_BUFFER_H
#define ITF_BUFFER_H


#include "common.hpp"

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>



class CBuffer {

    
public:
    CBuffer(const int &unit_size, const std::string &buffer_id);
    CBuffer(const std::string &buffer_id);
    
    void init(const int &);
    void put(IN const cv::Mat &);
    void fetch(OUT cv::Mat &);
    bool destroy();


private:        
    typedef struct {
        int frame_size;
        int header_size;
    }HeaderInfo_t;
    HeaderInfo_t head_;
    
    std::string buffer_id_; // Buffer unique id for communicating with other processes.
    
    boost::interprocess::shared_memory_object shm_;
    boost::interprocess::mapped_region region_header_;
    boost::interprocess::mapped_region region_frame_;
    
    void *p_header_;   // Starting address for storing header information of buffer.
    void *p_frame_;    // Starting address for storing frame.


    
  
};




#endif // ITF_BUFFER_H
