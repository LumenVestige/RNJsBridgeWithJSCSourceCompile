/**
 * Created by sanyinchen on 19-11-24.
 *
 * @author sanyinchen
 * @version v0.1
 * @since 19-11-24
 */

const Bridge = require('./bridge/Bridge');
const NativeModules = require('./bridge/NativeModules')

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
