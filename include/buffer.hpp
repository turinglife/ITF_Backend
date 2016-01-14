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

    bool put_src(IN unsigned int timestamp, IN const cv::Mat &src);
    bool fetch_src(OUT unsigned int &timestamp, OUT cv::Mat &src, IN bool flag);  // This function fetch the frame and discard it;

    /* for segmentation and stationary */
    //bool put_dst(IN unsigned int timestamp, IN const cv::Mat &src, IN const cv::Mat &dst_1);
    //bool fetch_dst(OUT unsigned int timestamp, OUT cv::Mat &src, OUT cv::Mat &dst_1, IN bool flag);  // This function fetch the destination;
    bool put_dst(IN unsigned int timestamp, IN const cv::Mat &dst_1);
    bool fetch_dst(OUT unsigned int timestamp, OUT cv::Mat &dst_1, IN bool flag);  // This function fetch the destination;

    /* for counting */
    bool put_dst(IN unsigned int timestamp, IN const cv::Mat &dst_1, IN int val_1); // This Function put alarm_unit(src, dst, value, timestamp) to buffer
    bool fetch_dst(OUT unsigned int timestamp, OUT cv::Mat &dst_1, OUT int &val_1, IN bool flag);  // This function fetch the destination;
    /* for counting alarm */
    bool put_dst(IN unsigned int timestamp, IN const cv::Mat &src, IN const cv::Mat &dst, IN int val); // This Function put alarm_unit(src, dst, value, timestamp) to buffer
    bool fetch_dst(OUT unsigned int timestamp, OUT cv::Mat &src, OUT cv::Mat &dst, OUT int &val);  // This function fetch the destination;

    /* for crossline */
    //bool put_dst(IN unsigned int timestamp, IN const cv::Mat &src, IN const cv::Mat &dst_1, IN const cv::Mat &dst_2, IN int val_1, IN int val_2); // This Function put alarm_unit(src, dst, value, timestamp) to buffer
    //bool fetch_dst(OUT unsigned int timestamp, OUT cv::Mat &src, OUT cv::Mat &dst_1, OUT cv::Mat &dst_2, OUT int &val_1, OUT int &val_2, IN bool flag);  // This function fetch the destination;
    bool put_dst(IN unsigned int timestamp, IN const cv::Mat &dst_1, IN const cv::Mat &dst_2, IN int val_1, IN int val_2); // This Function put alarm_unit(src, dst, value, timestamp) to buffer
    bool fetch_dst(OUT unsigned int timestamp, OUT cv::Mat &dst_1, OUT cv::Mat &dst_2, OUT int &val_1, OUT int &val_2, IN bool flag);  // This function fetch the destination;

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

    std::string buffer_id_header_;  // Buffer unique id for communicating with other processes.
    std::string buffer_id_src_;  // Buffer unique id for communicating with other processes.
    std::string buffer_id_dst_;  // Buffer unique id for communicating with other processes.
    std::string buffer_id_alarm_;  // Buffer unique id for communicating with other processes.

    boost::interprocess::shared_memory_object shm_header_;
    boost::interprocess::mapped_region region_header_;
    boost::interprocess::mapped_region region_camera_status_;

    boost::interprocess::shared_memory_object shm_src_;
    boost::interprocess::mapped_region region_index_src_;
    boost::interprocess::mapped_region region_timestamp_src_;
    boost::interprocess::mapped_region region_src_frame_;

    boost::interprocess::shared_memory_object shm_dst_;
    boost::interprocess::mapped_region region_index_dst_;
    boost::interprocess::mapped_region region_timestamp_dst_;
    boost::interprocess::mapped_region region_dst_frame_1_;
    boost::interprocess::mapped_region region_dst_frame_2_;
    boost::interprocess::mapped_region region_dst_val_1_;
    boost::interprocess::mapped_region region_dst_val_2_;

    boost::interprocess::shared_memory_object shm_alarm_;
    boost::interprocess::mapped_region region_alarm_unit_;

    unsigned char *p_header_;  // Starting address for storing header information of buffer.
    unsigned char *p_camera_status_; // Starting address for storing camera status (valid or invalid).

    unsigned char *p_index_src_;  // Starting address for storing buffer index information for control source frame buffer;
    unsigned char *p_timestamp_src_; // Starting address for storing source frame timestamp
    unsigned char *p_src_frame_;  // Starting address for storing source frame.

    unsigned char *p_index_dst_;  // Starting address for storing buffer index information for control dst buffer;
    unsigned char *p_timestamp_dst_; // Starting address for storing dst timestamp
    unsigned char *p_dst_frame_1_;  // Starting address for storing output map;
    unsigned char *p_dst_frame_2_;  // Starting address for storing output map;
    unsigned char *p_dst_val_1_;  // Starting address for storing output value;
    unsigned char *p_dst_val_2_;  // Starting address for storing output value;

    unsigned char *p_alarm_unit_; // Starting address for storing alarm_unit(src, dst, value, timestamp);

    int *p_last_r_src_;  // Address for storing last read index of source image buffer;
    int *p_last_w_src_;  // Adrress for storing last write index of source image buffer;
    int *p_last_r_dst_;  // Address for storing last read index of destination buffer;
    int *p_last_w_dst_;  // Address for storing last write index of destination buffer;
};


#endif  // ITF_BUFFER_H_
