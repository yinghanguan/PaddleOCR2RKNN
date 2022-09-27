
#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <stdlib.h>
#include <vector>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <numeric>

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

namespace OCR {

class Utility {
public:
  static std::vector<std::string> ReadDict(const std::string &path);

  static void
  VisualizeBboxes(const cv::Mat &srcimg,
                  const std::vector<std::vector<std::vector<int>>> &boxes);

  template <class ForwardIterator>
  inline static size_t argmax(ForwardIterator first, ForwardIterator last) {
    return std::distance(first, std::max_element(first, last));
  }

  static void GetAllFiles(const char *dir_name,
                          std::vector<std::string> &all_inputs);
    
  static cv::Mat GetRotateCropImage(const cv::Mat &srcimage,
                          std::vector<std::vector<int>> box);
};

} 