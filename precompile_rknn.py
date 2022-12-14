import sys

if __name__ == '__main__':

    if len(sys.argv) != 3:
        print('Usage: python {} xxx.rknn xxx.hw.rknn'.format(sys.argv[0]))
        print('Such as: python {} mobilenet_v1.rknn mobilenet_v1.hw.rknn'.format(sys.argv[0]))
        exit(1)

    from rknnlite.api import RKNNLite as RKNN

    orig_rknn = sys.argv[1]
    hw_rknn = sys.argv[2]

    # Create RKNN object
    rknn = RKNN()

    # Load rknn model
    print('--> Loading RKNN model')
    ret = rknn.load_rknn(orig_rknn)
    if ret != 0:
        print('Load RKNN model failed!')
        exit(ret)
    print('done')

    # Init runtime environment
    print('--> Init runtime environment')

    # Note: you must set rknn2precompile=True when call rknn.init_runtime()
    #       RK3399Pro with android system does not support this function.
    ret = rknn.init_runtime(rknn2precompile=True)
    if ret != 0:
        print('Init runtime environment failed')
        exit(ret)
    print('done')

    ret = rknn.export_rknn_precompile_model(hw_rknn)

    rknn.release()

