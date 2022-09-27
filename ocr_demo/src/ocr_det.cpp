

#include <ocr_det.h>
#include<opencv2/opencv.hpp>
using namespace cv;
using namespace std;
double __get_us_det(struct timeval t) { return (t.tv_sec * 1000000 + t.tv_usec); }

static unsigned char *load_model_det(const char *filename, int *model_size)
{
    FILE *fp = fopen(filename, "rb");
    if (fp == nullptr)
    {
        printf("fopen %s fail!\n", filename);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    int model_len = ftell(fp);
    unsigned char *model = (unsigned char *)malloc(model_len);
    fseek(fp, 0, SEEK_SET);
    if (model_len != fread(model, 1, model_len, fp))
    {
        printf("fread %s fail!\n", filename);
        free(model);
        return NULL;
    }
    *model_size = model_len;
    if (fp)
    {
        fclose(fp);
    }
    return model;
}

namespace OCR {



void DBDetector::Run(cv::Mat &img,
                     std::vector<std::vector<std::vector<int>>> &boxes,
                     std::vector<double> *times) {
  float ratio_h{};
  float ratio_w{};

  rknn_context ctx;
  struct timeval start_time, stop_time;
  unsigned char *model;
  int ret;
  int model_len = 0;
  model = load_model_det("./rknn_weights/det_hw.rknn", &model_len);
  ret = rknn_init(&ctx, model, model_len, 0);
  if (ret < 0)
  {
	  printf("rknn_init fail! ret=%d\n", ret);

  }
  rknn_input_output_num io_num;
  ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
  if (ret != RKNN_SUCC)
  {
	  printf("rknn_query fail! ret=%d\n", ret);
	 
  }
  printf("model input num: %d, output num: %d\n", io_num.n_input, io_num.n_output);


  cv::Mat srcimg;
  cv::Mat resize_img;
  img.copyTo(srcimg);
  
  auto preprocess_start = std::chrono::steady_clock::now();
  this->resize_op_.Run(img, resize_img, this->max_side_len_, ratio_h, ratio_w);
  img.copyTo(resize_img);                 
  this->normalize_op_.Run(&resize_img, this->mean_, this->scale_,
                          this->is_scale_);

  std::vector<float> input(1 * 3 * resize_img.rows * resize_img.cols, 0.0f);
  this->permute_op_.Run(&resize_img, input.data());
  auto preprocess_end = std::chrono::steady_clock::now();
  // Inference.
  auto inference_start = std::chrono::steady_clock::now();
  rknn_input inputs[1];
  memset(inputs, 0, sizeof(inputs));

  inputs[0].index = 0;
  inputs[0].type = RKNN_TENSOR_FLOAT32;
  inputs[0].size = resize_img.rows*resize_img.cols*3*4;
  inputs[0].fmt = RKNN_TENSOR_NCHW;
  inputs[0].buf = input.data();
  gettimeofday(&start_time, NULL);
  ret = rknn_inputs_set(ctx, 1, inputs);
  ret = rknn_run(ctx, NULL);
  rknn_output outputs[io_num.n_output];
  memset(outputs, 0, sizeof(outputs));
  for (int i = 0; i < io_num.n_output; i++) { outputs[i].want_float = 1; }
  ret = rknn_outputs_get(ctx, io_num.n_output, outputs, NULL);
  gettimeofday(&stop_time, NULL);
  printf("once run use %f ms\n",
	  (__get_us_det(stop_time) - __get_us_det(start_time)) / 1000);
  float *buffer = (float *)outputs[0].buf;
  uint32_t sz = outputs[0].size/4;
  // dump outputs
  for (int i = 0; i < io_num.n_output; i++) {
      char filename[64] = {0};
    	FILE *fp = NULL;
    	float *out_buf = (float *)outputs[i].buf;
      sprintf(filename, "output_%u.txt", i);

      fp = fopen(filename, "wb+");
      if (fp != NULL)
      {
			    for (int j=0; j<outputs[i].size/4; ++j) {
				      fprintf(fp, "%f\n", out_buf[j]);
			    }
      }
      fclose(fp);

      printf("Dump %s ...\n", filename);
  }
  std::vector<int> output_shape = {1,1,64,512};
  auto inference_end = std::chrono::steady_clock::now();
  auto postprocess_start = std::chrono::steady_clock::now();
  int n2 = output_shape[2];
  int n3 = output_shape[3];
  int n = n2 * n3;

  std::vector<float> pred(n, 0.0);
  std::vector<unsigned char> cbuf(n, ' ');

  for (int i = 0; i < n; i++) {
    pred[i] = float(buffer[i]);
    cbuf[i] = (unsigned char)((buffer[i]) * 255);
  }

  cv::Mat cbuf_map(n2, n3, CV_8UC1, (unsigned char *)cbuf.data());
  cv::Mat pred_map(n2, n3, CV_32F, (float *)pred.data());

  const double threshold = this->det_db_thresh_*255 ;
  const double maxvalue = 255;
  cv::Mat bit_map;
  cv::threshold(cbuf_map, bit_map, threshold, maxvalue, cv::THRESH_BINARY);
  cv::Mat dilation_map;
  cv::Mat dila_ele = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
  cv::dilate(bit_map, dilation_map, dila_ele);
  boxes = post_processor_.BoxesFromBitmap(
      pred_map, dilation_map, this->det_db_box_thresh_,
      this->det_db_unclip_ratio_, this->use_polygon_score_);

  boxes = post_processor_.FilterTagDetRes(boxes, ratio_h, ratio_w, srcimg);
  auto postprocess_end = std::chrono::steady_clock::now();
  std::cout << "Detected boxes num: " << boxes.size() << endl;

  std::chrono::duration<float> preprocess_diff = preprocess_end - preprocess_start;
  times->push_back(double(preprocess_diff.count() * 1000));
  std::chrono::duration<float> inference_diff = inference_end - inference_start;
  times->push_back(double(inference_diff.count() * 1000));
  std::chrono::duration<float> postprocess_diff = postprocess_end - postprocess_start;
  times->push_back(double(postprocess_diff.count() * 1000));
  ret = rknn_outputs_release(ctx, io_num.n_output, outputs);
  ret = rknn_destroy(ctx);
  free(model);
  //// visualization
  if (this->visualize_) {
    Utility::VisualizeBboxes(srcimg, boxes);
  }
}

} 
