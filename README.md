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
    
      ![1](F:\211环境部署\README-master\test_imgs\1.jpg)
    
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
    
    ![7](F:\211环境部署\README-master\7.png)
    
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

  ![det](F:\211环境部署\README-master\det.png)

  ![rec](F:\211环境部署\README-master\rec.png)

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

  ![det_sim](F:\211环境部署\README-master\det_sim.png)

  ![rec_sim](F:\211环境部署\README-master\rec_sim.png)

  测试修改节点并简化后的ONNX模型推理结果是否对应

  ```sh
  python paddleocr_onnx_test.py
  ```

  注：

  - 第312、313行为ONNX模型路径，根据自己的路径进行更改，第318行是词典路径。
  - 第450、451、549行中的数字根据自己的图像在1.2中得出的结果进行更改。
  - 推理产生的两个npy文件会在后面的量化时用到。

![0](F:\211环境部署\README-master\0.png)

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

![14](F:\211环境部署\README-master\14.png)

注：修改方法与1.3中paddleocr_onnx_test.py一致，注意替换的是rknn模型的路径。

**[⬆ 回到顶部](#目录)**

## C++

  <a name="2.1"></a>
  <a name="references--prefer-const"></a>

  - [2.1](#references--prefer-const) 板子上更新驱动

    > 驱动更新后可以运行一下rknpu里的demo测试是否更新成功。

    可能会出现这个，（借用一下交流群中群友的报错）。
    
    ![bfc283b45a4fe496ec4bc6d2b9a1fb4](F:\211环境部署\README-master\bfc283b45a4fe496ec4bc6d2b9a1fb4.jpg)

  		出现这个错误是因为在更新驱动的时候，没有给galcore.ko权限。

```sh
sudo chmod +x galcore.ko
```

​		这样做的话，只能在root用户下才能调用npu，如果想要方便的话就chown改个所有者。RV1126固件缺少几个库,同时为了支持导出预编译模型的功能,需要更新librknn_runtime.so。



<a name="2.2"></a>
  <a name="references--disallow-var"></a>

  - [2.2](#references--disallow-var) 

    > Why? 因为`let`是块级作用域，而`var`是函数级作用域

    ```javascript
    // bad
    var count = 1;
    if (true) {
      count += 1;
    }
    
    // good, use the let.
    let count = 1;
    if (true) {
      count += 1;
    }
    ```

  <a name="2.3"></a>
  <a name="references--block-scope"></a>
  - [2.3](#references--block-scope) 注意： `let`、`const`都是块级作用域

    ```javascript
    // const 和 let 都只存在于它定义的那个块级作用域
    {
      let a = 1;
      const b = 1;
    }
    console.log(a); // ReferenceError
    console.log(b); // ReferenceError
    ```

**[⬆ back to top](#目录)**

## Objects

  <a name="3.1"></a>
  <a name="objects--no-new"></a>
  - [3.1](#objects--no-new) 使用字面值创建对象. eslint: [`no-new-object`](http://eslint.org/docs/rules/no-new-object.html)

    ```javascript
    // bad
    const item = new Object();
    
    // good
    const item = {};
    ```

  <a name="3.2"></a>
  <a name="es6-computed-properties"></a>
  - [3.2](#es6-computed-properties) 当创建一个带有动态属性名的对象时，用计算后属性名

    > Why? 这可以使你将定义的所有属性放在对象的一个地方.

    ```javascript
    
    function getKey(k) {
      return `a key named ${k}`;
    }
    
    // bad
    const obj = {
      id: 5,
      name: 'San Francisco',
    };
    obj[getKey('enabled')] = true;
    
    // good getKey('enabled')是动态属性名
    const obj = {
      id: 5,
      name: 'San Francisco',
      [getKey('enabled')]: true,
    };
    ```

  <a name="3.3"></a>
  <a name="es6-object-shorthand"></a>
  - [3.3](#es6-object-shorthand) 用对象方法简写. eslint: [`object-shorthand`](http://eslint.org/docs/rules/object-shorthand.html)

    ```javascript
    // bad
    const atom = {
      value: 1,
    
      addValue: function (value) {
        return atom.value + value;
      },
    };
    
    // good
    const atom = {
      value: 1,
    
      // 对象的方法
      addValue(value) {
        return atom.value + value;
      },
    };
    ```

  <a name="3.4"></a>
  <a name="es6-object-concise"></a>
  - [3.4](#es6-object-concise) 用属性值缩写. eslint: [`object-shorthand`](http://eslint.org/docs/rules/object-shorthand.html)

    > Why? 这样写的更少且更可读

    ```javascript
    const lukeSkywalker = 'Luke Skywalker';
    
    // bad
    const obj = {
      lukeSkywalker: lukeSkywalker,
    };
    
    // good
    const obj = {
      lukeSkywalker,
    };
    ```

  <a name="3.5"></a>
  <a name="objects--grouped-shorthand"></a>
  - [3.5](#objects--grouped-shorthand) 将你的所有缩写放在对象声明的开始.

    > Why? 这样也是为了更方便的知道有哪些属性用了缩写.

    ```javascript
    const anakinSkywalker = 'Anakin Skywalker';
    const lukeSkywalker = 'Luke Skywalker';
    
    // bad
    const obj = {
      episodeOne: 1,
      twoJediWalkIntoACantina: 2,
      lukeSkywalker,
      episodeThree: 3,
      mayTheFourth: 4,
      anakinSkywalker,
    };
    
    // good
    const obj = {
      lukeSkywalker,
      anakinSkywalker,
      episodeOne: 1,
      twoJediWalkIntoACantina: 2,
      episodeThree: 3,
      mayTheFourth: 4,
    };
    ```

  <a name="3.6"></a>
  <a name="objects--quoted-props"></a>
  - [3.6](#objects--quoted-props) 只对那些无效的标示使用引号 `''`. eslint: [`quote-props`](http://eslint.org/docs/rules/quote-props.html)

    > Why? 通常我们认为这种方式主观上易读。他优化了代码高亮，并且页更容易被许多JS引擎压缩。

    ```javascript
    // bad
    const bad = {
      'foo': 3,
      'bar': 4,
      'data-blah': 5,
    };
    
    // good
    const good = {
      foo: 3,
      bar: 4,
      'data-blah': 5,
    };
    ```

  <a name="3.7"></a>
  <a name="objects--prototype-builtins"></a>
  - [3.7](#objects--prototype-builtins) 不要直接调用`Object.prototype`上的方法，如`hasOwnProperty`, `propertyIsEnumerable`, `isPrototypeOf`。

    > Why? 在一些有问题的对象上， 这些方法可能会被屏蔽掉 - 如：`{ hasOwnProperty: false }` - 或这是一个空对象`Object.create(null)`

    ```javascript
    // bad
    console.log(object.hasOwnProperty(key));
    
    // good
    console.log(Object.prototype.hasOwnProperty.call(object, key));
    
    // best
    const has = Object.prototype.hasOwnProperty; // 在模块作用内做一次缓存
    /* or */
    import has from 'has'; // https://www.npmjs.com/package/has
    // ...
    console.log(has.call(object, key));
    ```

  <a name="3.8"></a>
  <a name="objects--rest-spread"></a>
  - [3.8](#objects--rest-spread) 对象浅拷贝时，更推荐使用扩展运算符[就是`...`运算符]，而不是[`Object.assign`](https://developer.mozilla.org/en/docs/Web/JavaScript/Reference/Global_Objects/Object/assign)。获取对象指定的几个属性时，用对象的rest解构运算符[也是`...`运算符]更好。
    + 这一段不太好翻译出来， 大家看下面的例子就懂了。^.^

  ```javascript
  // very bad
  const original = { a: 1, b: 2 };
  const copy = Object.assign(original, { c: 3 }); // this mutates `original` ಠ_ಠ
  delete copy.a; // so does this

  // bad
  const original = { a: 1, b: 2 };
  const copy = Object.assign({}, original, { c: 3 }); // copy => { a: 1, b: 2, c: 3 }

  // good es6扩展运算符 ...
  const original = { a: 1, b: 2 };
  // 浅拷贝
  const copy = { ...original, c: 3 }; // copy => { a: 1, b: 2, c: 3 }

  // rest 赋值运算符
  const { a, ...noA } = copy; // noA => { b: 2, c: 3 }
  ```

**[⬆ back to top](#目录)**

## Resources

**Learning ES6**

  - [Draft ECMA 2015 (ES6) Spec](https://people.mozilla.org/~jorendorff/es6-draft.html)
  - [ExploringJS](http://exploringjs.com/)
  - [ES6 Compatibility Table](https://kangax.github.io/compat-table/es6/)
  - [Comprehensive Overview of ES6 Features](http://es6-features.org/)

**Read This**

  - [Standard ECMA-262](http://www.ecma-international.org/ecma-262/6.0/index.html)

**Tools**

  - Code Style Linters
    + [ESlint](http://eslint.org/) - [Airbnb Style .eslintrc](https://github.com/airbnb/javascript/blob/master/linters/.eslintrc)
    + [JSHint](http://jshint.com/) - [Airbnb Style .jshintrc](https://github.com/airbnb/javascript/blob/master/linters/.jshintrc)
    + [JSCS](https://github.com/jscs-dev/node-jscs) - [Airbnb Style Preset](https://github.com/jscs-dev/node-jscs/blob/master/presets/airbnb.json)

**Other Style Guides**

  - [Google JavaScript Style Guide](https://google.github.io/styleguide/javascriptguide.xml)
  - [jQuery Core Style Guidelines](https://contribute.jquery.org/style-guide/js/)
  - [Principles of Writing Consistent, Idiomatic JavaScript](https://github.com/rwaldron/idiomatic.js)

**Other Styles**

  - [Naming this in nested functions](https://gist.github.com/cjohansen/4135065) - Christian Johansen
  - [Conditional Callbacks](https://github.com/airbnb/javascript/issues/52) - Ross Allen
  - [Popular JavaScript Coding Conventions on GitHub](http://sideeffect.kr/popularconvention/#javascript) - JeongHoon Byun
  - [Multiple var statements in JavaScript, not superfluous](http://benalman.com/news/2012/05/multiple-var-statements-javascript/) - Ben Alman

**Further Reading**

  - [Understanding JavaScript Closures](https://javascriptweblog.wordpress.com/2010/10/25/understanding-javascript-closures/) - Angus Croll
  - [Basic JavaScript for the impatient programmer](http://www.2ality.com/2013/06/basic-javascript.html) - Dr. Axel Rauschmayer
  - [You Might Not Need jQuery](http://youmightnotneedjquery.com/) - Zack Bloom & Adam Schwartz
  - [ES6 Features](https://github.com/lukehoban/es6features) - Luke Hoban
  - [Frontend Guidelines](https://github.com/bendc/frontend-guidelines) - Benjamin De Cock

**Books**

  - [JavaScript: The Good Parts](https://www.amazon.com/JavaScript-Good-Parts-Douglas-Crockford/dp/0596517742) - Douglas Crockford
  - [JavaScript Patterns](https://www.amazon.com/JavaScript-Patterns-Stoyan-Stefanov/dp/0596806752) - Stoyan Stefanov
  - [Pro JavaScript Design Patterns](https://www.amazon.com/JavaScript-Design-Patterns-Recipes-Problem-Solution/dp/159059908X)  - Ross Harmes and Dustin Diaz
  - [High Performance Web Sites: Essential Knowledge for Front-End Engineers](https://www.amazon.com/High-Performance-Web-Sites-Essential/dp/0596529309) - Steve Souders
  - [Maintainable JavaScript](https://www.amazon.com/Maintainable-JavaScript-Nicholas-C-Zakas/dp/1449327680) - Nicholas C. Zakas
  - [JavaScript Web Applications](https://www.amazon.com/JavaScript-Web-Applications-Alex-MacCaw/dp/144930351X) - Alex MacCaw
  - [Pro JavaScript Techniques](https://www.amazon.com/Pro-JavaScript-Techniques-John-Resig/dp/1590597273) - John Resig
  - [Smashing Node.js: JavaScript Everywhere](https://www.amazon.com/Smashing-Node-js-JavaScript-Everywhere-Magazine/dp/1119962595) - Guillermo Rauch
  - [Secrets of the JavaScript Ninja](https://www.amazon.com/Secrets-JavaScript-Ninja-John-Resig/dp/193398869X) - John Resig and Bear Bibeault
  - [Human JavaScript](http://humanjavascript.com/) - Henrik Joreteg
  - [Superhero.js](http://superherojs.com/) - Kim Joar Bekkelund, Mads Mobæk, & Olav Bjorkoy
  - [JSBooks](http://jsbooks.revolunet.com/) - Julien Bouquillon
  - [Third Party JavaScript](https://www.manning.com/books/third-party-javascript) - Ben Vinegar and Anton Kovalyov
  - [Effective JavaScript: 68 Specific Ways to Harness the Power of JavaScript](http://amzn.com/0321812182) - David Herman
  - [Eloquent JavaScript](http://eloquentjavascript.net/) - Marijn Haverbeke
  - [You Don't Know JS: ES6 & Beyond](http://shop.oreilly.com/product/0636920033769.do) - Kyle Simpson

**Blogs**

  - [JavaScript Weekly](http://javascriptweekly.com/)
  - [JavaScript, JavaScript...](https://javascriptweblog.wordpress.com/)
  - [Bocoup Weblog](https://bocoup.com/weblog)
  - [Adequately Good](http://www.adequatelygood.com/)
  - [NCZOnline](https://www.nczonline.net/)
  - [Perfection Kills](http://perfectionkills.com/)
  - [Ben Alman](http://benalman.com/)
  - [Dmitry Baranovskiy](http://dmitry.baranovskiy.com/)
  - [Dustin Diaz](http://dustindiaz.com/)
  - [nettuts](http://code.tutsplus.com/?s=javascript)

**Podcasts**

  - [JavaScript Air](https://javascriptair.com/)
  - [JavaScript Jabber](https://devchat.tv/js-jabber/)

