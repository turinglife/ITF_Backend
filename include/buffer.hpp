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
    CBuffer(const int &src_width, const int &src_height, const int &unit_size, const int &src_num, const int &dst_num, const std::string &buffer_id);
    CBuffer(const std::string &buffer_id);

    void init(const int &);
    bool put_src(IN const cv::Mat &);
    bool put_dst(IN const cv::Mat &, IN const int &);
    bool fetch_src(OUT cv::Mat &);
    bool fetch_dst(OUT cv::Mat &, OUT int &);
    void unlock_buffer();
    bool destroy();


private:
    typedef struct {
        int frame_width;
        int frame_height;
        int frame_size;
        int src_buffer_num;
        int dst_buffer_num;
        int header_size;
    }HeaderInfo_t;
    HeaderInfo_t head_;

    std::string buffer_id_; // Buffer unique id for communicating with other processes.

    boost::interprocess::shared_memory_object shm_;
    boost::interprocess::mapped_region region_header_;
    boost::interprocess::mapped_region region_last_r_w_;
    boost::interprocess::mapped_region region_src_;
    boost::interprocess::mapped_region region_dst_map_;
    boost::interprocess::mapped_region region_dst_val_;

    void *p_header_;   // Starting address for storing header information of buffer.
    void *p_last_r_w_;
    void *p_src_;    // Starting address for storing source frame.
    void *p_dst_map_;  // Starting address for storing output map;
    void *p_dst_val_; // Starting address for storing output value;

    int *p_last_r_src_;
    int *p_last_w_src_;
    int *p_last_r_dst_;
    int *p_last_w_dst_;
};




#endif // ITF_BUFFER_H
