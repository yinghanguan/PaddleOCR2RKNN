
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <vector>

#include <cstring>
#include <fstream>
#include <numeric>
#include<opencv2/opencv.hpp>

#include <ocr_det.h>

#include <ocr_rec.h>
#include <utility.h>
#include <sys/stat.h>



using namespace std;
using namespace cv;
using namespace OCR;

void swap(std::vector<std::vector<std::vector<int>>>& arr,int i,int j) {
		std::vector<std::vector<int>> temp = arr[j];
		arr[j] = arr[i];
		arr[i] = temp;
}
static bool PathExists(const std::string& path){
#ifdef _WIN32
  struct _stat buffer;
  return (_stat(path.c_str(), &buffer) == 0);
#else
  struct stat buffer;
  return (stat(path.c_str(), &buffer) == 0);
#endif  // !_WIN32
}

int main_system(std::vector<cv::String> cv_all_img_names) {
    // parameters for detection model
    const std::string det_model_dir = "./rknn_weights/det_hw.rknn";
    int max_side_len = 960;
    float det_db_thresh = 0.3;
    float det_db_box_thresh = 0.5;
    float det_db_unclip_ratio = 1.6;
    bool use_polygon_score = false;
    bool visualize = true;
    const std::string precision = "fp16";

    // paramters for recognition model
    const std::string rec_model_dir = "./rknn_weights/rec_hw.rknn";
    const std::string char_list_file = "ppocr_keys_v1.txt";

    
    DBDetector det(det_model_dir, 
                    max_side_len, det_db_thresh,
                   det_db_box_thresh, det_db_unclip_ratio,
                   use_polygon_score, visualize,
                    precision);


    CRNNRecognizer rec(rec_model_dir, 
                       char_list_file,
                       precision);

    auto start = std::chrono::system_clock::now();

    for (int i = 0; i < cv_all_img_names.size(); ++i) {
      std::cout << "The predict img: " << cv_all_img_names[i] << endl;
      cv::Mat srcimg = cv::imread(cv_all_img_names[i], cv::IMREAD_COLOR);
      if (!srcimg.data) {
        std::cerr << "[ERROR] image read failed! image path: " << cv_all_img_names[i] << endl;
        exit(1);
      }
      
      cv::resize(srcimg, srcimg, cv::Size(512, 64));
      std::vector<std::vector<std::vector<int>>> boxes;
      std::vector<double> det_times;
      std::vector<double> rec_times;
        
      det.Run(srcimg, boxes, &det_times);
    
      cv::Mat crop_img;
      int min_num = 999999;
      int min_index = 0;
      for (int k=0;k<boxes.size();k++){
          if (min_num >boxes[k][0][0]){
              min_num = boxes[k][0][0];
              min_index = k;
          }
          
      }
      swap(boxes,0,min_index);
      cout<<min_num<<endl;
      cout<<min_index<<endl;
      for (int j = 0; j < boxes.size(); j++) {
        crop_img = Utility::GetRotateCropImage(srcimg, boxes[j]);
        cv::imwrite("./crop_img.png", crop_img);
        std::cout << "The crop visualized image saved in ./crop_img.png"
                  << std::endl;

        rec.Run(crop_img, &rec_times);
      }
        
      auto end = std::chrono::system_clock::now();
      auto duration =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start);
      std::cout << "Cost  "
                << double(duration.count()) *
                       std::chrono::microseconds::period::num /
                       std::chrono::microseconds::period::den
                << "s" << std::endl;
    }
      
    return 0;
}



int main(int argc, char **argv) {
    if (argc<=1 || (strcmp(argv[1], "det")!=0 && strcmp(argv[1], "rec")!=0 && strcmp(argv[1], "system")!=0)) {
        std::cout << "Please choose one mode of [det, rec, system] !" << std::endl;
        return -1;
    }
    std::cout << "mode: " << argv[1] << endl;

    string img = argv[2];
    
    std::vector<cv::String> cv_all_img_names;

    cv::String test_img = img;
    cv_all_img_names.push_back(test_img);

    return main_system(cv_all_img_names);
    

}
