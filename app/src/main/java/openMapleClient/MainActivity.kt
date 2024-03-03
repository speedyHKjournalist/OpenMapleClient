package openMapleClient

import android.content.Intent
import android.os.Bundle
import android.widget.EditText
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import java.io.File
import java.io.IOException

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        showTextBoxDialog()
    }

    override fun onDestroy() {
        super.onDestroy()
        finish()
    }

    private fun showTextBoxDialog() {
        val editText = EditText(this)
        val dialog = AlertDialog.Builder(this)
            .setTitle("Specify HOST IP Address")
            .setView(editText)
            .setPositiveButton("OK") { _, _ ->
                val userInput = editText.text.toString()
                if (userInput.isNotEmpty()) {
                    saveUserInputToFile(userInput)
                    startOpenMapleClient()
                }
            }
            .setNegativeButton("Cancel") { dialog, _ ->
                dialog.dismiss()
                finish()
            }
            .create()

        dialog.show()
    }

    private fun startOpenMapleClient() {
        val intent = Intent(this, MapleActivity::class.java)
        startActivity(intent)
    }

    private fun saveUserInputToFile(userInput: String) {
        try {
            val externalDir = getExternalFilesDir(null)
            val file = File(externalDir, "hostip.txt")
            file.writeText(userInput)
        } catch (e: IOException) {
            e.printStackTrace()
        }
    }
}