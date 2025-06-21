
<p>
  <strong>👀</strong>
  <img src="https://profile-counter.glitch.me/LumenVestige.RNJsBridgeWithJSCSourceCompile/count.svg" alt="Visitor Count"  width="120"/>
</p>

### 基于RN V0.6x版本JS-Bridge通讯框架，集成JSC以及ATrace源码编译


#### 文件/目录清单： 
+ [buildSrc](buildSrc)：Trace插桩插件（from https://github.com/aidaole/EasyTrace）
+ [js-bridge](js-bridge)：JS层Bridge
+ [js-bridge-lib](js-bridge-lib): Native层Bridge
  + [bridge](js-bridge-lib/src/main/cpp/bridge) BridgeCore
  + [jsc](js-bridge-lib/src/main/cpp/jsc) JSC（JavaScriptCore）源码
  + [trace](js-bridge-lib/src/main/cpp/trace) aTrace
+ [perfetto.sh](perfetto.sh) trace 执行脚本（sh ./perfetto.sh ）
+ icu buid from:[https://github.com/LumenVestige/android-jsc-icu-build](https://github.com/LumenVestige/android-jsc-icu-build)
#### API Test：
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
#### Trace预览
<img src="source/trace.png" alt="trace"  height="700">

#### Demo预览
<img src="source/demo.png" alt="demo"  height="700">



