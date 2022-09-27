import onnx
import onnx.checker
import onnx.utils
from onnx.tools import update_model_dims

model = onnx.load(r'rec_time.onnx')
# 此处可以理解为获得了一个维度 “引用”，通过该 “引用“可以修改其对应的维度
dim_proto0 = model.graph.input[0].type.tensor_type.shape.dim[0]
dim_proto2 = model.graph.input[0].type.tensor_type.shape.dim[2]
dim_proto3 = model.graph.input[0].type.tensor_type.shape.dim[3]

# dim_output0 = model.graph.output[0].type.tensor_type.shape.dim[0]
# dim_output2 = model.graph.output[0].type.tensor_type.shape.dim[2]
# dim_output3 = model.graph.output[0].type.tensor_type.shape.dim[3]
# 将该维度赋值为字符串，其维度不再为和dummy_input绑定的值


dim_proto3.dim_value = 688
# dim_proto2.dim_value = 96
# dim_output2.dim_value = 96
# dim_output3.dim_value = 864


onnx.save(model, r'./原始模型\时间\rec.onnx')
