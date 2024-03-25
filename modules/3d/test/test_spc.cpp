// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
//
// Author: Zhangjie Chen <zhangjiec01@gmail.com>
#include "test_precomp.hpp"
#include <chrono>

namespace opencv_test { namespace {

TEST(SpC, FunctionTest) {
    // load point cloud
    // String test_file_path = "/home/jeffery/Desktop/Sustech/Thesis/ptclouds/cube.ply";
//     String test_file_path = "/home/jeffery/Desktop/Sustech/Thesis/data/ptclouds/two_cubes.ply";
    String test_file_path = "/home/jeffery/Desktop/Sustech/Thesis/data/scan_040_simplified_005.obj";
//    String test_file_path = "/home/jeffery/Desktop/Sustech/Thesis/data/scan_040_simplified_0005.obj";
    std::vector<cv::Point3f> vertices;
    std::vector<std::vector<int32_t>> indices;
    cv::loadPointCloud(test_file_path, vertices, cv::noArray(), cv::noArray(), indices);

    cv::SpectralCluster cluster(0.15, 0.22);

    std::vector<int> results;
    auto start = std::chrono::high_resolution_clock::now();
    cluster.cluster(results, vertices, indices, 7);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Clustering Complete! Time used: " << duration << " ms." << std::endl;
}
}
}

