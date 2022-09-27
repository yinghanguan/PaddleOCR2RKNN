# PaddleOCR time demo
## 环境
### 硬件
RV1109/RV1126
### 软件
RKNN Toolkit: 1.7.1
NPU DRV: 1.7.1 (0cfd4a1 build: 2021-11-24 09:33:04)
## demo编译
在pc上执行build.sh脚本进行编译
**注意事项**
**在执行该脚本前,需要将build.sh中的交叉编译器所在目录的路径GCC_COMPILER改成实际的路径**
脚本中所用的交叉编译工具可以从以下网站下载:
https://developer.arm.com/-/media/Files/downloads/gnu-a/8.3-2019.03/binrel/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf.tar.xz?revision=e09a1c45-0ed3-4a8e-b06b-db3978fd8d56&hash=8BB29A0584746D97A027A45B813916DD
## 将demo推到板子上
执行push_demo.sh脚本将demo推到1109/1126板子上的根目录
## 在板子上运行demo
在推到板子上的demo里有个run_rv1109_rv1126.sh脚本,执行这个脚本会对cut_time.jpg这张图片进行测试.  
```
chmod +x run_rv1109_rv1126.sh
./run_rv1109_rv1126.sh
```
推到板子上第一次运行这个脚本时可能没有权限,需要执行chmod +x run_rv1109_rv1126.sh命令增加可执行权限.
## 将结果拉回PC端进行对比
执行pull_results.sh可以将推理得到的结果拉回pc端以进行比较.  
results目录结构说明:
```
results/
├── crop_img.png
├── ocr_vis.png
├── output_0.txt
├── python_det_out_0.txt
├── python_rec_out_0.txt
└── rec_output_0.txt
```
- crop_img.png: det推理结果进行裁剪后的图像
- ocr_vis.png: det结果可视化图像
- output_0.txt: det模型推理结果保存成tensor的形式,用来跟python demo跑的结果进行对比
- python_det_out_0.txt: python demo跑det模型的推理结果,这个是一个基准结果,板子上跑的时候不会有这个文件
- rec_output_0.txt: rec模型推理结果保存成tensor的形式,用来跟python demo跑的结果进行对比
- python_rec_out_0.txt: python demo跑rec模型的推理结果,这个是一个基准结果,板子上跑的时候不会有这个文件
这里几个文件都是我的环境上跑出来的结果,从ocr_vis.png可以看到det模型跑的结果是对的,但是rec的后处理时候是不完整的,默认不该的话会core dump,我直接用outputs buf替换predict_batch,处理的结果是错的,这个建议你们再检查下.我现在是直接用c跑的rec_output_0.txt和python_rec_out_0.txt做比较,是非常接近的,余弦距离0.9997以上,基本可以认为是一致的.说明rknn端的处理是没问题的.  
