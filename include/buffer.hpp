//
// ITF Buffer Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_BUFFER_H_
#define ITF_BUFFER_H_

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "common.hpp"

class CBuffer {
 public:
    /**
    * @brief This Init() will make  buffer a producer
    */
    bool Init(int src_width, int src_height, int unit_size, int src_num, int dst_num, const std::string &buffer_id);

    /**
    * @brief This Init() will make  buffer a consumer
    */
    bool Init(const std::string &buffer_id);

    bool set_camera_valid(IN bool flag);
    bool is_camera_valid();

    bool frame_size(OUT int &width, OUT int &height);
    bool put_src(IN const cv::Mat &frame, IN unsigned int timestamp);
    bool put_dst(IN const cv::Mat &frame, IN int predicted_value);
    bool fetch_frame(OUT cv::Mat &frame, OUT unsigned int &timestamp);  // This function fetch the frame without discarding it;
    bool fetch_src(OUT cv::Mat &frame, OUT unsigned int &timestamp);  // This function fetch the frame and discard it;
    bool fetch_dst(OUT cv::Mat &frame, OUT int &predicted_value);  // This function fetch the destination;

    bool put_dst(IN const cv::Mat &src_img, IN const cv::Mat &dst_img, IN int predicted_value, IN unsigned int timestamp); // This Function put alarm_unit(src, dst, value, timestamp) to buffer
    bool fetch_dst(OUT cv::Mat &src_img, OUT cv::Mat &dst_img, OUT int &predicted_value, OUT unsigned int &timestamp);// This Function fetch alarm_unit(src, dst, value, timestamp) from buffer.

    bool destroy();

 private:
    struct HeaderInfo_t{
        int frame_width;
        int frame_height;
        int frame_size;
        int src_buffer_num;
        int dst_buffer_num;
        int header_size;
    };
    HeaderInfo_t head_;  //headerinfo keep the static info of buffer and frame;

    std::string buffer_id_;  // Buffer unique id for communicating with other processes.

    boost::interprocess::shared_memory_object shm_;
    boost::interprocess::mapped_region region_header_;
    boost::interprocess::mapped_region region_camera_status_;
    boost::interprocess::mapped_region region_last_r_w_;
    boost::interprocess::mapped_region region_timestamp_;
    boost::interprocess::mapped_region region_src_;
    boost::interprocess::mapped_region region_dst_map_;
    boost::interprocess::mapped_region region_dst_val_;
    boost::interprocess::mapped_region region_alarm_unit_;

    unsigned char *p_header_;  // Starting address for storing header information of buffer.
    unsigned char *p_camera_status_; // Starting address for storing camera status (valid or invalid).
    unsigned char *p_last_r_w_;  // Starting address for storing buffer index information for control buffer;
    unsigned char *p_timestamp_; // Starting address for storing timestamp
    unsigned char *p_src_;  // Starting address for storing source frame.
    unsigned char *p_dst_map_;  // Starting address for storing output map;
    unsigned char *p_dst_val_;  // Starting address for storing output value;
    unsigned char *p_alarm_unit_; // Starting address for storing alarm_unit(src, dst, value, timestamp);

    int *p_last_r_src_;  // Address for storing last read index of source image buffer;
    int *p_last_w_src_;  // Adrress for storing last write index of source image buffer;
    int *p_last_r_dst_;  // Address for storing last read index of destination buffer;
    int *p_last_w_dst_;  // Address for storing last write index of destination buffer;
};


#endif  // ITF_BUFFER_H_
