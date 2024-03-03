package openMapleClient

import android.app.NativeActivity
import android.os.Bundle
import android.view.Gravity


class MapleActivity : NativeActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        SetLayoutMapleClient()
    }

    private fun SetLayoutMapleClient() {
        val windowManager = windowManager
        val display = windowManager.defaultDisplay
        val screenHeight = display.height
        val screenWidth = display.width
        val params = window.attributes
        params.width = screenWidth * 800 / screenHeight
        params.gravity = Gravity.CENTER
        window.attributes = params
    }
}