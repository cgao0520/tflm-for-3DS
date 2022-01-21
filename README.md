# tflm-for-3DS
tflm for 3DS is a project aims at porting tensorflow lite micro to Nintendo 3DS.

This repo mainly includes the tensorflow lite micro library, i.e., [tensorflowlitemicro_lib](https://github.com/cgao0520/tflm-for-3DS/tree/494e07eba73e51e75617a4444101ab6dfb826303/tensorflowlitemicro_lib) and adapted examples (currently only has [tflm_hello_world](https://github.com/cgao0520/tflm-for-3DS/tree/494e07eba73e51e75617a4444101ab6dfb826303/tflm_hello_world)) for 3DS.

The source code of the library is obtained by running a python script within tensorflow repo, check [here](https://github.com/tensorflow/tflite-micro/blob/main/tensorflow/lite/micro/docs/new_platform_support.md) for details. I created a Makefile which applies [devkitPro](https://devkitpro.org) to compile the code into a static .a lib file. This lib file can be further linked into any 3DS apps when needed.

Examples are going to be ported to 3DS as well, while currently the hello world example which demonstrate the sine wave fitting has been fully ported. The following is a picture taken when the example was running on a real 3DS hardware.

![sine wave fitting on 3DS](/screenshots/hello_world.jpg)

Note:
    - To run the binary of the example, you will need a homebrew capable 3DS.
    - Check [my blog article](http://vgao.ddns.net/wordpress/2022/01/12/build-tensorflow-lite-micro-for-nintendo-3ds/) for more details.

