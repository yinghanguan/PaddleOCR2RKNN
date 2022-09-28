# PaddleOCR2RKNN

*转换官方paddleocr模型到瑞芯微板子上部署推理，提供全套部署流程以及c++推理程序。*

> **注意**: 目前只测试过PP-OVRv2 由于我的应用场景不需要方向分类模型，故不包括分类模型相关。

[![Downloads](https://img.shields.io/npm/dm/eslint-config-airbnb.svg)](https://www.npmjs.com/package/eslint-config-airbnb)
[![Downloads](https://img.shields.io/npm/dm/eslint-config-airbnb-base.svg)](https://www.npmjs.com/package/eslint-config-airbnb-base)

## 目录

  1. [PC端](#PC) 
  1. [板端](#C++)

## PC

  <a name="1.1"></a>
  <a name="PC--installation"></a>

  - [1.1](#PC--installation) installation
    
    ```sh
    pip install -r requirements.txt
    ```

​	rknn_toolkit-1.7.2.dev12229317-cp36-cp36m-linux_x86_64.whl下载地址请发邮件联系我，yinghan_guan@163.com

```sh
pip install rknn_toolkit-1.7.2.dev12229317-cp36-cp36m-linux_x86_64.whl
```

  <a name="1.2"></a>
  <a name="PC--shape"></a>

  - [1.2](#PC--shape)  更改paddleocr源码,得到输出形状。
    
    - `RKNN不支持动态输入所以要固定输入`
    
      ![1](https://github.com/yinghanguan/PaddleOCR2RKNN/blob/main/test_imgs/1.jpg)
    
    ```python
    '''anaconda3\envs\paddle\Lib\site-packages\paddleocr\tools\infer\predict_det.py的212行加入'''
    print("img:",img.shape)
    '''anaconda3\envs\paddle\Lib\site-packages\paddleocr\tools\infer\predict_rec.py的260行加入'''
    print("norm_img:",norm_img.shape)
    ```
    
    - 运行`paddle_ocr_test.py`
    
    ```sh
    python paddle_ocr_test.py
    ```
    
    ![7](https://github.com/yinghanguan/images/blob/main/7.png)
    
    得到要识别文字的检测模型输入大小和识别模型的输入大小（1，3，64，512）和（3，32，377）。

​	   <a name="PC--ONNX"></a>

- [1.3](#PC--ONNX) Paddle推理模型转换ONNX模型并修改节点、ONNX推理

  ```sh
  paddle2onnx --model_dir det/ --model_filename inference.pdmodel --params_filename inference.pdiparams --save_file det.onnx --opset_version 11 --enable_onnx_checker True 
  ```

  ```sh
  paddle2onnx --model_dir rec/ --model_filename inference.pdmodel --params_filename inference.pdiparams --save_file rec.onnx --opset_version 11 --enable_onnx_checker True 
  ```

  得到了ONNX模型后需要对模型的输入节点和输出节点进行修改。

  初始的det.onnx和rec.onnx分别为

  ![det](https://github.com/yinghanguan/images/blob/main/det.png)

  ![rec](https://github.com/yinghanguan/images/blob/main/rec.png)

  ```sh
  python onnx_trans.py --onnx_path det.onnx --output_path det_test.onnx --type det --det_h 64 --det_w 512
  ```

  ```sh
  python onnx_trans.py --onnx_path rec.onnx --output_path rec_test.onnx --type rec --rec_shape 377
  ```

  得到的ONNX模型进行简化

  ```sh
  python -m onnxsim det_test.onnx det_sim.onnx
  ```

  ```sh
  python -m onnxsim rec_test.onnx rec_sim.onnx
  ```

  得到ONNX模型可视化

  ![det_sim](https://github.com/yinghanguan/images/blob/main/det_sim.png)

  ![rec_sim](https://github.com/yinghanguan/images/blob/main/rec_sim.png)

  测试修改节点并简化后的ONNX模型推理结果是否对应

  ```sh
  python paddleocr_onnx_test.py
  ```

  注：

  - 第312、313行为ONNX模型路径，根据自己的路径进行更改，第318行是词典路径。
  - 第450、451、549行中的数字根据自己的图像在1.2中得出的结果进行更改。
  - 推理产生的两个npy文件会在后面的量化时用到。

![0](https://github.com/yinghanguan/images/blob/main/0.png)

​	<a name="PC--ONNX2RKNN"></a>

- [1.4](#PC--ONNX2RKNN) 模型转换RKNN

​	将得到的npy文件分别放入det_time和rec_time文件夹内，更改datasets.txt和model_config.yml。然后进行量化。

```sh
python rknn_convert.py det_time/ rknn_weights/ 0
```

识别模型由于包含了BiLSTM算子，只能进行fp16量化，并且转换模型时不要用离线预编译功能，一定要在板子上在线预编译。

```sh
python rknn_convert.py rec_time/ rknn_weights/ 0
```

​	在rknn_weights文件夹中得到量化后的RKNN模型，在PC上的simulator运行python进行推理测试。注意：PC推理结果如果是错的，并不能证明模型不可用，可	能会编译之后会变好。亲测！

```sh
python paddle_ocr_rknn.py
```

![14](https://github.com/yinghanguan/images/blob/main/14.png)

注：修改方法与1.3中paddleocr_onnx_test.py一致，注意替换的是rknn模型的路径。

**[⬆ 回到顶部](#目录)**

## C++

  <a name="2.1"></a>
  <a name="references--prefer-const"></a>

  - [2.1](#references--prefer-const) 板子上更新驱动

    > 驱动更新后可以运行一下rknpu里的demo测试是否更新成功。

    可能会出现这个，（借用一下交流群中群友的报错）。
    
    ![bfc283b45a4fe496ec4bc6d2b9a1fb4](https://github.com/yinghanguan/images/blob/main/bfc283b45a4fe496ec4bc6d2b9a1fb4.jpg)

  		出现这个错误是因为在更新驱动的时候，没有给galcore.ko权限。

```sh
sudo chmod +x galcore.ko
```

​		这样做的话，只能在root用户下才能调用npu，如果想要方便的话就chown改个所有者。RV1126固件缺少几个库,同时为了支持导出预编译模型的功能,需要更新librknn_runtime.so。

<a name="2.2"></a>
  <a name="references--disallow-var"></a>

  - [2.2](#references--disallow-var)  预编译RKNN模型

    > 预编译模型和没有预编译模型的区别只在于加载速度，经过我的测试，可能会存在未预编译推理结果是混乱的，预编译后恢复正常的情况。

    ```sh
    python3 precompile_rknn.py rknn_weights/det_time_sim.rknn det_hw.rknn
    ```

```sh
python3 precompile_rknn.py rknn_weights/rec_time_sim.rknn rec_hw.rknn
```

至此，就得到了预编译后的RKNN模型。 

 <a name="2.3"></a>
  <a name="references--block-scope"></a>

  - [2.3](#references--block-scope) C++代码修改（在服务器（PC）中进行）

    RKNN不支持动态输入，所以要固定输入，除了需要在1.2中得到的3个数字，还需要用netron.app打开rec_time_sim.rknn（未进行预编译的rknn模型）。可以看到LSTM算子已经编译成为了RKNN的OP。我们需要最后输出的第二维度，这里是94。
    
    ![59](https://github.com/yinghanguan/images/blob/main/59.png)
    
    接下来修改代码：
    
    - [ ] main.cpp第80行的检测模型输入形状。
    
    - [ ] ocr_det.cpp第50行的模型路径。第120行的检测模型输入形状。
    
    - [ ] ocr_rec.cpp第58行的模型路径。第122行识别模型的输出形状（这里就是上面得到的94）。
    
    - [ ] include/preprocess_op.h中第43行的形状改为识别模型的输入形状
    
      接下来进行代码编译（在build.sh中指定交叉编译工具链的路径）
    
    ```sh
    sh build.sh
    ```

- [2.4] C++板端推理

  将编译后的可执行程序（也可编译成动态链接库，使用自己的代码调用）install/ocr_demo拖到板子上。

  ```sh
  sudo chmod +x ./ocr_demo
  ```

  ```sh
  ./ocr_demo system test_imgs/cut_time.jpg
  ```

  ![50](https://github.com/yinghanguan/images/blob/main/50.png)

  这样就得到了板端的推理结果，总体来说优化空间还很大。

  |          | 检测模型 | 识别模型   |
  | -------- | -------- | ---------- |
  | 推理耗时 | 29.273ms | 1421.446ms |

**[⬆ back to top](#目录)**

## 合作联系

  - https://guyuanjie.com

