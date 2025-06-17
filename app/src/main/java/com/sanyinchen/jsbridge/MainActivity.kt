package com.sanyinchen.jsbridge

import android.Manifest
import android.annotation.SuppressLint
import android.app.Activity
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.os.Handler
import android.os.Looper
import android.provider.Settings
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity

import com.facebook.soloader.SoLoader
import com.sanyinchen.jsbridge.business.BusinessPackages
import com.sanyinchen.jsbridge.business.jsmodule.HelloJavaScriptModule


class MainActivity : AppCompatActivity() {
    val mainHandle = Handler(Looper.getMainLooper())
    var jsBridgeInstanceManager: JsBridgeManager? = null


    /** Android 11+ 的“All files access” */
    private val allFilesLauncher =
        registerForActivityResult(ActivityResultContracts.StartActivityForResult()) {

        }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        SoLoader.init(this, false)
        val logTextView = findViewById<TextView>(R.id.log)
        val initButton = findViewById<Button>(R.id.init)?.apply {
            this.setOnClickListener {
                bridgeInit(logTextView)
            }
        }
        val invokeJs = findViewById<Button>(R.id.invoke)?.apply {
            this.setOnClickListener {
                helloMsg()
            }
        }
        val cleanLog = findViewById<Button>(R.id.clean)?.apply {
            this.setOnClickListener {
                logTextView.text = ""
            }
        }
    }


    private fun helloMsg() {
        val helloJavaScriptModule =
            jsBridgeInstanceManager?.currentReactContext?.getJSModule(
                HelloJavaScriptModule::class.java
            )
        helloJavaScriptModule?.showMessage("test")
    }

    @SuppressLint("SetTextI18n")
    private fun bridgeInit(logTextView: TextView) {

        jsBridgeInstanceManager = JsBridgeManagerBuilder()
            .setApplication(application)
            .setBundleAssetName("js-bridge-bundle.js")
            .setNativeModuleCallExceptionHandler { e -> e.printStackTrace() }
            .addPackage(BusinessPackages() {
                mainHandle.post {
                    logTextView.text = (logTextView.text.toString() + "\n $it")
                }
            })
            .build()
        jsBridgeInstanceManager?.run()

    }


}


