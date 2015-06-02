//
// Copyright 2015 CUHK
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <opencv2/highgui/highgui.hpp>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <itf/extracters/extracter_factory.hpp>
#include <itf/segmenters/segmenter_factory.hpp>
#include <itf/util/Util.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <thread>

#include "type.hpp"
#include "SQLiteCpp/SQLiteCpp.h"

static bool on = true;

void density(std::string path);
void segmentation(std::string path);

int main(int argc, char* argv[]) {
    std::string task_name(argv[1]);









    std::string socket_path = "ad_" + task_name;

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR socket");
        exit(1);
    }

    struct sockaddr_un serv_addr;
    serv_addr.sun_family = AF_UNIX;
    // strcpy(serv_addr.sun_path, argv[1]);
    snprintf(serv_addr.sun_path, socket_path.length() + 1, "%s", socket_path.c_str());

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR bind");
        return -1;
    }

    listen(sockfd, 5);

    std::thread t1;

    std::cout << getpid() << ": ad is ready" << std::endl;
    while (true) {
        struct sockaddr_un cli_addr;
        socklen_t clilen = sizeof(cli_addr);

        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("ERROR accept");
            return -1;
        }

        char message[256];

        if (read(newsockfd, message, 255) < 0) {
            perror("ERROR read");
            exit(1);
        }

        printf("%d: ad receive: (%s)\n", getpid(), message);
        close(newsockfd);

        if (strncmp(message, "density", 7) == 0) {
            std::cout << "density()" << std::endl;

            on = false;
            if (t1.joinable())
                t1.join();
            on = true;
            t1 = std::thread(density, task_name);
        }

        if (strncmp(message, "segmentation", 12) == 0) {
            std::cout << "segmentation()" << std::endl;

            on = false;
            if (t1.joinable())
                t1.join();
            on = true;
            t1 = std::thread(segmentation, task_name);
        }

        if (strncmp(message, "close", 4) == 0) {
            std::cout << "close()" << std::endl;

            on = false;
        }

        if (strncmp(message, "quit", 4) == 0) {
            std::cout << "quit()" << std::endl;

            on = false;
            on = false;
            if (t1.joinable())
                t1.join();

            break;
        }
    }

    close(sockfd);

    if (unlink(socket_path.c_str()) == -1) {
        std::cout << "ERROR unlink: " << socket_path << std::endl;
        return -1;
    }

    std::cout << "ad is done" << std::endl;
    return 0;
}

void density(std::string path) {
    using boost::interprocess::shared_memory_object;
    using boost::interprocess::open_only;
    using boost::interprocess::read_only;

    shared_memory_object shm(open_only, path.c_str(), read_only);

    using boost::interprocess::mapped_region;
    mapped_region region_header(shm, read_only);
    HeaderInfo head;
    memcpy(&head, region_header.get_address(), sizeof(head));

    int rows = head.height;
    int cols = head.width;
    mapped_region region_frame(shm, read_only, sizeof(head));



    /* Setup Extracter */
    itf::ExtracterParameter ep;
    // Read configuration file
    if (!itf::Util::ReadProtoFromTextFile("./model/density_extracter.prototxt", &ep)) {
        std::cout << "Cannot read .prototxt file!" << std::endl;
        exit(-1);
    }
    // Create extracter factory
    itf::CExtracterFactory *ef = new itf::CExtracterFactory();
    // Factory instantiates an object of the specific type of extracter
    itf::IExtracter *iextracter = ef->SpawnExtracter(itf::Density);
    iextracter->SetExtracterParameters(ep);

    std::string pers_path;
    std::string roi_path;

    try {
        SQLite::Database db("./db/ITF.db");

        SQLite::Statement query(db, "SELECT pers_path, roi_path FROM Tasks WHERE task_name=?");
        query.bind(1, path);

        bool has_records = false;
        // Init a task object with appropriate parameters.
        while (query.executeStep()) {
            pers_path = query.getColumn(0).getText();
            roi_path = query.getColumn(1).getText();
            has_records = true;
        }
        if (!has_records) {
            std::cout << "No Such Pers or ROI" << std::endl;
            return;
        }
    } catch (std::exception& e) {
        std::cout << "exception: " << e.what() << std::endl;
        return;
    }


    iextracter->SetImagesDim(rows, cols);
    cv::Mat pmap;
    iextracter->LoadPerspectiveMap(pers_path, &pmap);
    iextracter->LoadROI(roi_path);

    itf::Util util;

    while (on) {
        cv::Mat frame(rows, cols, CV_8UC3);
        int imgSize = frame.total() * frame.elemSize();
        memcpy(frame.data, region_frame.get_address(), imgSize);

        iextracter->LoadImages(frame, frame);

        // Extract density feature from a frame loaded above
        std::vector<float> feature = iextracter->ExtractFeatures();

        cv::Mat density_map(rows, cols, CV_32F, feature.data());

        // std::cout << cv::sum(density_map)[0] << std::endl;

        // Generate a heat map
        cv::Mat heat = util.GenerateHeatMap(density_map, pmap);
        cv::imshow(path, heat);

        cv::waitKey(10);
    }

    std::cout << path << ": Video is Over" << std::endl;

    delete ef;
}

void segmentation(std::string path) {
    itf::SegmenterParameter sp;

    if (!itf::Util::ReadProtoFromTextFile("./model/fcnn_segmenter.prototxt", &sp)) {
        std::cout << "Cannot read .prototxt file!" << std::endl;
        exit(-1);
    }
    // Create segmenter factory
    itf::CSegmenterFactory *sf = new itf::CSegmenterFactory();
    // Factory instantiate an object of the specific type of segmenter
    itf::ISegmenter *isegmenter = sf->SpawnSegmenter(itf::FCNN);

    isegmenter->SetParameters(sp);

    int rows = 576;
    int cols = 704;

    using boost::interprocess::shared_memory_object;
    using boost::interprocess::open_only;
    using boost::interprocess::read_only;

    shared_memory_object shm(open_only, path.c_str(), read_only);

    using boost::interprocess::mapped_region;
    mapped_region region_frame(shm, read_only);

    while (on) {
        cv::Mat frame(rows, cols, CV_8UC3);
        int imgSize = frame.total() * frame.elemSize();
        memcpy(frame.data, region_frame.get_address(), imgSize);

        cv::Mat foreground;
        cv::Mat img_bgmodel;
        isegmenter->process(frame, foreground, img_bgmodel);
        foreground = foreground > 0.5;
        cv::imshow(path, foreground);

        cv::waitKey(10);
    }

    std::cout << path << ": Video is Over" << std::endl;

    delete sf;
}
