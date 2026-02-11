package com.example.myapplication

import android.content.Context
import android.os.Bundle
import android.view.View
import com.google.androidgamesdk.GameActivity
import com.google.firebase.FirebaseApp
import com.google.firebase.auth.FirebaseAuth
import com.google.firebase.auth.ktx.auth
import com.google.firebase.firestore.FirebaseFirestore
import com.google.firebase.firestore.ktx.firestore
import com.google.firebase.ktx.Firebase

class MainActivity : GameActivity() {
    companion object {
        init {
            System.loadLibrary("myapplication")
        }
        private const val PREFS_NAME = "TowerDefensePrefs"
        private const val KEY_MENU_COINS = "menu_coins"
        private const val KEY_CARDS = "cards"
        private const val KEY_ARCHER_LEVEL = "archer_level"
        private const val KEY_SHERIFF_LEVEL = "sheriff_level"
        private const val KEY_ALLY_LEVEL = "ally_level"
        private const val KEY_IS_LOGGED_IN = "is_logged_in"
        private const val KEY_USER_EMAIL = "user_email"
        private const val KEY_USER_UID = "user_uid"
    }

    private lateinit var auth: FirebaseAuth
    private lateinit var db: FirebaseFirestore

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        
        // Initialize Firebase
        FirebaseApp.initializeApp(this)
        auth = Firebase.auth
        db = Firebase.firestore
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) {
            hideSystemUi()
        }
    }

    private fun hideSystemUi() {
        val decorView = window.decorView
        decorView.systemUiVisibility = (View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_FULLSCREEN)
    }

    // ==================== LOCAL STORAGE ====================
    
    fun saveMenuCoins(coins: Int) {
        val prefs = getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
        prefs.edit().putInt(KEY_MENU_COINS, coins).apply()
    }

    fun loadMenuCoins(): Int {
        val prefs = getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
        return prefs.getInt(KEY_MENU_COINS, 0)
    }
    
    fun saveLocalProgress(coins: Int, cards: Int, archerLvl: Int, sheriffLvl: Int, allyLvl: Int) {
        val prefs = getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
        prefs.edit()
            .putInt(KEY_MENU_COINS, coins)
            .putInt(KEY_CARDS, cards)
            .putInt(KEY_ARCHER_LEVEL, archerLvl)
            .putInt(KEY_SHERIFF_LEVEL, sheriffLvl)
            .putInt(KEY_ALLY_LEVEL, allyLvl)
            .apply()
    }
    
    fun loadLocalProgress(): GameProgress {
        val prefs = getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
        return GameProgress(
            menuCoins = prefs.getInt(KEY_MENU_COINS, 0),
            cards = prefs.getInt(KEY_CARDS, 0),
            archerLevel = prefs.getInt(KEY_ARCHER_LEVEL, 1),
            sheriffLevel = prefs.getInt(KEY_SHERIFF_LEVEL, 1),
            allyLevel = prefs.getInt(KEY_ALLY_LEVEL, 1)
        )
    }

    // ==================== FIREBASE AUTH ====================
    
    fun isLoggedIn(): Boolean {
        return auth.currentUser != null
    }
    
    fun getCurrentUserEmail(): String {
        return auth.currentUser?.email ?: ""
    }
    
    fun getCurrentUserUid(): String {
        return auth.currentUser?.uid ?: ""
    }
    
    fun signInWithEmailPassword(email: String, password: String, callback: AuthCallback) {
        auth.signInWithEmailAndPassword(email, password)
            .addOnCompleteListener(this) { task ->
                if (task.isSuccessful) {
                    val user = auth.currentUser
                    callback.onSuccess(user?.email ?: "", user?.uid ?: "")
                } else {
                    callback.onError(task.exception?.message ?: "Authentication failed")
                }
            }
    }
    
    fun createAccount(email: String, password: String, callback: AuthCallback) {
        auth.createUserWithEmailAndPassword(email, password)
            .addOnCompleteListener(this) { task ->
                if (task.isSuccessful) {
                    val user = auth.currentUser
                    // Create initial user data in Firestore
                    createUserDataInCloud(user?.uid ?: "")
                    callback.onSuccess(user?.email ?: "", user?.uid ?: "")
                } else {
                    callback.onError(task.exception?.message ?: "Registration failed")
                }
            }
    }
    
    fun signOut() {
        auth.signOut()
    }

    // ==================== CLOUD SAVE (FIRESTORE) ====================
    
    private fun createUserDataInCloud(uid: String) {
        val userData = hashMapOf(
            "menuCoins" to 0,
            "cards" to 0,
            "archerLevel" to 1,
            "sheriffLevel" to 1,
            "allyLevel" to 1,
            "createdAt" to System.currentTimeMillis()
        )
        
        db.collection("users").document(uid)
            .set(userData)
            .addOnSuccessListener { 
                android.util.Log.d("Firebase", "User data created successfully")
            }
            .addOnFailureListener { e ->
                android.util.Log.e("Firebase", "Error creating user data", e)
            }
    }
    
    fun saveProgressToCloud(coins: Int, cards: Int, archerLvl: Int, sheriffLvl: Int, allyLvl: Int, callback: CloudCallback) {
        val user = auth.currentUser
        if (user == null) {
            callback.onError("User not logged in")
            return
        }
        
        val userData = hashMapOf(
            "menuCoins" to coins,
            "cards" to cards,
            "archerLevel" to archerLvl,
            "sheriffLevel" to sheriffLvl,
            "allyLevel" to allyLvl,
            "lastSync" to System.currentTimeMillis()
        )
        
        db.collection("users").document(user.uid)
            .set(userData)
            .addOnSuccessListener {
                callback.onSuccess()
            }
            .addOnFailureListener { e ->
                callback.onError(e.message ?: "Failed to save progress")
            }
    }
    
    fun loadProgressFromCloud(callback: ProgressCallback) {
        val user = auth.currentUser
        if (user == null) {
            callback.onError("User not logged in")
            return
        }
        
        db.collection("users").document(user.uid)
            .get()
            .addOnSuccessListener { document ->
                if (document != null && document.exists()) {
                    val progress = GameProgress(
                        menuCoins = document.getLong("menuCoins")?.toInt() ?: 0,
                        cards = document.getLong("cards")?.toInt() ?: 0,
                        archerLevel = document.getLong("archerLevel")?.toInt() ?: 1,
                        sheriffLevel = document.getLong("sheriffLevel")?.toInt() ?: 1,
                        allyLevel = document.getLong("allyLevel")?.toInt() ?: 1
                    )
                    callback.onSuccess(progress)
                } else {
                    callback.onError("No saved data found")
                }
            }
            .addOnFailureListener { e ->
                callback.onError(e.message ?: "Failed to load progress")
            }
    }

    // ==================== DATA CLASSES ====================
    
    data class GameProgress(
        val menuCoins: Int,
        val cards: Int,
        val archerLevel: Int,
        val sheriffLevel: Int,
        val allyLevel: Int
    )
    
    interface AuthCallback {
        fun onSuccess(email: String, uid: String)
        fun onError(error: String)
    }
    
    interface CloudCallback {
        fun onSuccess()
        fun onError(error: String)
    }
    
    interface ProgressCallback {
        fun onSuccess(progress: GameProgress)
        fun onError(error: String)
    }
}
