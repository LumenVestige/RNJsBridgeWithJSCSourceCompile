### 基于RN V0.6x版本JS-Bridge通讯框架，集成JSC以及ATrace源码编译
#### 文件/目录清单： 
+ js-bridge：JS层Bridge
+ js-bridge-lib: Native层Bridge
  + [bridge](js-bridge-lib/src/main/cpp/bridge) BridgeCore
  + [jsc](js-bridge-lib/src/main/cpp/jsc) JSC（JavaScriptCore）源码
#### Test：
```
// test
// async invoke NativeModule:TestSum
NativeModules.TestSum.sum(1, 2)
    .then(result => {
        NAConsole.log("TestSum.sum(1, 2) test:" + result);
    })
    .catch(error => {
        NAConsole.log("Error" + error);
    });

// Js HelloJavaScriptModule define
global.HelloJavaScriptModule = {
    showMessage: (message) => {
        NativeModules.NativeLog.log('HelloJavaScriptModule:showMessage:' + message);
    }
};
Bridge.registerCallableModule('HelloJavaScriptModule', global.HelloJavaScriptModule)

// NALog NativeModule test
NAConsole.log("Js bridge inited");
// NativeModule c++ module invoke test
NativeModules.HelloCxxModule.foo((r) => {
    NAConsole.log("js HelloCxxModule invoke test:" + r);
});

```
+ 点击init初始化jsBridge以及加载js-bridge-bundle.js
+ 点击invoke js测试native调用js module

<img src="https://github.com/sanyinchen/jsbridge/blob/master/source/demo.png" alt="demo"  height="700">


