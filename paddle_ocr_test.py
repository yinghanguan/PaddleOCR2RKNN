from paddleocr import PaddleOCR, draw_ocr
import cv2
import os
os.environ["KMP_DUPLICATE_LIB_OK"]="TRUE"
# 模型路径下必须含有model和params文件
ocr = PaddleOCR(use_angle_cls=True,
                use_gpu=False,lang='ch')  # det_model_dir='{your_det_model_dir}', rec_model_dir='{your_rec_model_dir}'
img_path = 'test_imgs/1.jpg'

img = cv2.imread(img_path)
import time
s = time.time()
result = ocr.ocr(img, cls=True)
print("result:",result)
print("time:",time.time()-s)

# for line in result:
#     print(line)

# 显示结果
from PIL import Image

image = Image.open(img_path).convert('RGB')
boxes = [line[0] for line in result]
txts = [line[1][0] for line in result]
scores = [line[1][1] for line in result]
im_show = draw_ocr(image, boxes, txts, scores, font_path='D:/paddle_pp/PaddleOCR/doc/simfang.ttf')
im_show = Image.fromarray(im_show)
im_show.save('result.jpg')  # 结果图片保存在代码同级文件夹中。


