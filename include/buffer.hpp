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

    unsigned char *p_header_;   // Starting address for storing header information of buffer.
    unsigned char *p_last_r_w_; // Starting address for storing buffer index information for control buffer;
    unsigned char *p_src_;    // Starting address for storing source frame.
    unsigned char *p_dst_map_;  // Starting address for storing output map;
    unsigned char *p_dst_val_; // Starting address for storing output value;

    int *p_last_r_src_; // Address for storing last read index of source image buffer;
    int *p_last_w_src_; // Adrress for storing last write index of source image buffer;
    int *p_last_r_dst_; // Address for storing last read index of destination buffer;
    int *p_last_w_dst_; // Address for storing last write index of destination buffer;
};




#endif // ITF_BUFFER_H
