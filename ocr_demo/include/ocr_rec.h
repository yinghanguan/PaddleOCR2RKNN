

#pragma once
#include "rknn_api.h"
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <vector>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <ostream>


#include <cstring>
#include <fstream>
#include <numeric>
#include<memory>
#include <postprocess_op.h>
#include <preprocess_op.h>
#include <utility.h>
#include <sys/time.h>

namespace OCR {

class CRNNRecognizer {
public:
  explicit CRNNRecognizer(const std::string &model_dir,      
                           const string &label_path,const std::string &precision) 
  {

    
    this->precision_ = precision;

    this->label_list_ = Utility::ReadDict(label_path);
    this->label_list_.insert(this->label_list_.begin(),
                             "#"); // blank char for ctc
    this->label_list_.push_back(" ");
	

  }




  void Run(cv::Mat &img, std::vector<double> *times);

private:
  


  std::vector<std::string> label_list_;

  std::vector<float> mean_ = {0.5f, 0.5f, 0.5f};
  std::vector<float> scale_ = {1 / 0.5f, 1 / 0.5f, 1 / 0.5f};
  bool is_scale_ = true;
  std::string precision_ = "fp32";
  // pre-process
  CrnnResizeImg resize_op_;
  Normalize normalize_op_;
  Permute permute_op_;

  // post-process
  PostProcessor post_processor_;
};
} 
