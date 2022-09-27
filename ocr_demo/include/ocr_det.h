

#pragma once

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <sys/time.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <vector>
#include "rknn_api.h"
#include <cstring>
#include <fstream>
#include <numeric>
#include<memory>
#include <postprocess_op.h>
#include <preprocess_op.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
using namespace std;

namespace OCR {

class DBDetector {
public:
  explicit DBDetector(const std::string &model_dir,
                     const int &max_side_len,
                      const double &det_db_thresh,
                      const double &det_db_box_thresh,
                      const double &det_db_unclip_ratio,
                      const bool &use_polygon_score, const bool &visualize,
                      const std::string &precision)
    {

    
    

    this->max_side_len_ = max_side_len;
    //this->model_len = model_len;
    this->det_db_thresh_ = det_db_thresh;
    this->det_db_box_thresh_ = det_db_box_thresh;
    this->det_db_unclip_ratio_ = det_db_unclip_ratio;
    this->use_polygon_score_ = use_polygon_score;
    this->visualize_ = visualize;
    this->precision_ = precision;
	  

  }

  
  

  // Run predictor
  void Run(cv::Mat &img, std::vector<std::vector<std::vector<int>>> &boxes, std::vector<double> *times);

private:
  


  

  int max_side_len_ = 2000;
  
  double det_db_thresh_ = 0.3;
  double det_db_box_thresh_ = 0.5;
  double det_db_unclip_ratio_ = 1.6;
  bool use_polygon_score_ = true;
  //int model_len = 0;
  bool visualize_ = false;
  std::string precision_ = "fp16";

  std::vector<float> mean_ = {0.485f, 0.456f, 0.406f};
  std::vector<float> scale_ = {1 / 0.229f, 1 / 0.224f, 1 / 0.225f};
  bool is_scale_ = true;

  // pre-process
  ResizeImgType0 resize_op_;
  Normalize normalize_op_;
  Permute permute_op_;

  // post-process
  PostProcessor post_processor_;
  };
}