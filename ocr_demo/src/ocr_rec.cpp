
#include <ocr_rec.h>
#include<opencv2/opencv.hpp>


using namespace cv;
double __get_us_rec(struct timeval t) { return (t.tv_sec * 1000000 + t.tv_usec); }
static unsigned char *load_model_rec(const char *filename, int *model_size)
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

void CRNNRecognizer::Run(cv::Mat &img, std::vector<double> *times) {
  cv::Mat srcimg;
  img.copyTo(srcimg);
  cv::Mat resize_img;

  float wh_ratio = float(srcimg.cols) / float(srcimg.rows);
  auto preprocess_start = std::chrono::steady_clock::now();
  this->resize_op_.Run(srcimg, resize_img, wh_ratio);

  this->normalize_op_.Run(&resize_img, this->mean_, this->scale_,
                          this->is_scale_);

  std::vector<float> input(1 * 3 * resize_img.rows * resize_img.cols, 0.0f);

  this->permute_op_.Run(&resize_img, input.data());
  auto preprocess_end = std::chrono::steady_clock::now();

  // Inference.
  struct timeval start_time, stop_time;
  rknn_context ctx;
  int ret;
  int model_len = 0;
  unsigned char *model;
  model = load_model_rec("./rknn_weights/rec_hw.rknn", &model_len);
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
  cout<<"resize_img.rows:"<<resize_img.rows<<endl;
  cout<<"resize_img.cols:"<<resize_img.cols<<endl;
  //auto input_t->Reshape({1, 3, resize_img.rows, resize_img.cols});
  auto inference_start = std::chrono::steady_clock::now();
  //input_t->CopyFromCpu(input.data());
  rknn_input inputs[1];
  memset(inputs, 0, sizeof(inputs));
  inputs[0].index = 0;
  inputs[0].type = RKNN_TENSOR_FLOAT32;
  inputs[0].size = resize_img.rows * resize_img.cols*3 * 4;
  inputs[0].fmt = RKNN_TENSOR_NCHW;
  inputs[0].buf = input.data();
  gettimeofday(&start_time, NULL);
  ret = rknn_inputs_set(ctx, 1, inputs);
  ret = rknn_run(ctx, NULL);
  cout<<ret<<endl;

  rknn_output outputs[io_num.n_output];

  memset(outputs, 0, sizeof(outputs));
  for (int i = 0; i < io_num.n_output; i++) { outputs[i].want_float = 1; }

  ret = rknn_outputs_get(ctx, io_num.n_output, outputs, NULL);

  for (int i = 0; i < io_num.n_output; i++) {
      char filename[64] = {0};
    	FILE *fp = NULL;
    	float *out_buf = (float *)outputs[i].buf;
      sprintf(filename, "rec_output_%u.txt", i);

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
  gettimeofday(&stop_time, NULL);
  printf("once run use %f ms\n",
	  (__get_us_rec(stop_time) - __get_us_rec(start_time)) / 1000);

  // std::vector<float> predict_batch;
  float *predict_batch = (float *)outputs[0].buf;

  std::vector<int> predict_shape = {1,94,6625};
  //auto output_t->CopyToCpu(outputs[0].buf);
  auto inference_end = std::chrono::steady_clock::now();

  // ctc decode
  auto postprocess_start = std::chrono::steady_clock::now();
  std::vector<std::string> str_res;
  int argmax_idx;
  int last_index = 0;
  float score = 0.f;
  int count = 0;
  float max_value = 0.0f;

  for (int n = 0; n < predict_shape[1]; n++) {
    argmax_idx =
        int(Utility::argmax(&predict_batch[n * predict_shape[2]],
                            &predict_batch[(n + 1) * predict_shape[2]]));
    max_value =
        float(*std::max_element(&predict_batch[n * predict_shape[2]],
                                &predict_batch[(n + 1) * predict_shape[2]]));

    if (argmax_idx > 0 && (!(n > 0 && argmax_idx == last_index))) {
      score += max_value;
      count += 1;
      str_res.push_back(label_list_[argmax_idx]);
    }
    last_index = argmax_idx;
  }
  auto postprocess_end = std::chrono::steady_clock::now();
  score /= count;
  for (int i = 0; i < str_res.size(); i++) {
    std::cout << str_res[i];
  }
  std::cout << "\tscore: " << score << std::endl;

  std::chrono::duration<float> preprocess_diff = preprocess_end - preprocess_start;
  times->push_back(double(preprocess_diff.count() * 1000));
  std::chrono::duration<float> inference_diff = inference_end - inference_start;
  times->push_back(double(inference_diff.count() * 1000));
  std::chrono::duration<float> postprocess_diff = postprocess_end - postprocess_start;
  times->push_back(double(postprocess_diff.count() * 1000));
  ret = rknn_outputs_release(ctx, io_num.n_output, outputs);
  ret = rknn_destroy(ctx);
  free(model);
}
}



