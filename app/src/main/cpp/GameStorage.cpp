#include "GameStorage.h"
#include "AndroidOut.h"

#include <jni.h>
#include <android/native_activity.h>

// Global reference to JVM (set from main.cpp)
extern JavaVM* g_jvm;
extern jobject g_activity;

void GameStorage::saveMenuCoins(int coins) {
    if (!g_jvm || !g_activity) {
        aout << "GameStorage: JVM or Activity not available, cannot save coins" << std::endl;
        return;
    }
    
    JNIEnv* env;
    jint attachResult = g_jvm->AttachCurrentThread(&env, nullptr);
    if (attachResult != JNI_OK) {
        aout << "GameStorage: Failed to attach thread" << std::endl;
        return;
    }
    
    // Get activity class
    jclass activityClass = env->GetObjectClass(g_activity);
    if (!activityClass) {
        aout << "GameStorage: Failed to get activity class" << std::endl;
        g_jvm->DetachCurrentThread();
        return;
    }
    
    // Call saveMenuCoins method
    jmethodID saveMethod = env->GetMethodID(activityClass, "saveMenuCoins", "(I)V");
    if (saveMethod) {
        env->CallVoidMethod(g_activity, saveMethod, coins);
        aout << "GameStorage: Saved " << coins << " coins" << std::endl;
    } else {
        aout << "GameStorage: saveMenuCoins method not found" << std::endl;
    }
    
    env->DeleteLocalRef(activityClass);
    g_jvm->DetachCurrentThread();
}

int GameStorage::loadMenuCoins() {
    if (!g_jvm || !g_activity) {
        aout << "GameStorage: JVM or Activity not available, cannot load coins" << std::endl;
        return 0;
    }
    
    JNIEnv* env;
    jint attachResult = g_jvm->AttachCurrentThread(&env, nullptr);
    if (attachResult != JNI_OK) {
        aout << "GameStorage: Failed to attach thread" << std::endl;
        return 0;
    }
    
    // Get activity class
    jclass activityClass = env->GetObjectClass(g_activity);
    if (!activityClass) {
        aout << "GameStorage: Failed to get activity class" << std::endl;
        g_jvm->DetachCurrentThread();
        return 0;
    }
    
    // Call loadMenuCoins method
    jmethodID loadMethod = env->GetMethodID(activityClass, "loadMenuCoins", "()I");
    int coins = 0;
    if (loadMethod) {
        coins = env->CallIntMethod(g_activity, loadMethod);
        aout << "GameStorage: Loaded " << coins << " coins" << std::endl;
    } else {
        aout << "GameStorage: loadMenuCoins method not found" << std::endl;
    }
    
    env->DeleteLocalRef(activityClass);
    g_jvm->DetachCurrentThread();
    
    return coins;
}

bool GameStorage::isAvailable() {
    return g_jvm != nullptr && g_activity != nullptr;
}

// ==================== ACCOUNT SYSTEM JNI ====================

void GameStorage::saveProgress(int coins, int cards, int archerLvl, int sheriffLvl, int allyLvl) {
    if (!g_jvm || !g_activity) return;
    
    JNIEnv* env;
    if (g_jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) return;
    
    jclass activityClass = env->GetObjectClass(g_activity);
    if (!activityClass) {
        g_jvm->DetachCurrentThread();
        return;
    }
    
    // Try cloud save first if logged in
    jmethodID isLoggedIn = env->GetMethodID(activityClass, "isLoggedIn", "()Z");
    if (isLoggedIn && env->CallBooleanMethod(g_activity, isLoggedIn)) {
        // User is logged in, save to cloud
        jmethodID saveCloud = env->GetMethodID(activityClass, "saveProgressToCloud", "(IIIIILcom/example/myapplication/MainActivity$CloudCallback;)V");
        if (saveCloud) {
            // For simplicity, we fire-and-forget cloud save
            env->CallVoidMethod(g_activity, saveCloud, coins, cards, archerLvl, sheriffLvl, allyLvl, nullptr);
        }
    }
    
    // Always save locally
    jmethodID saveLocal = env->GetMethodID(activityClass, "saveLocalProgress", "(IIIII)V");
    if (saveLocal) {
        env->CallVoidMethod(g_activity, saveLocal, coins, cards, archerLvl, sheriffLvl, allyLvl);
    }
    
    env->DeleteLocalRef(activityClass);
    g_jvm->DetachCurrentThread();
}

void GameStorage::login(const std::string& email, const std::string& password) {
    if (!g_jvm || !g_activity) return;
    
    JNIEnv* env;
    if (g_jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) return;
    
    jclass activityClass = env->GetObjectClass(g_activity);
    if (!activityClass) {
        g_jvm->DetachCurrentThread();
        return;
    }
    
    jmethodID signIn = env->GetMethodID(activityClass, "signInWithEmailPassword", 
        "(Ljava/lang/String;Ljava/lang/String;Lcom/example/myapplication/MainActivity$AuthCallback;)V");
    
    if (signIn) {
        jstring jEmail = env->NewStringUTF(email.c_str());
        jstring jPassword = env->NewStringUTF(password.c_str());
        env->CallVoidMethod(g_activity, signIn, jEmail, jPassword, nullptr);
        env->DeleteLocalRef(jEmail);
        env->DeleteLocalRef(jPassword);
    }
    
    env->DeleteLocalRef(activityClass);
    g_jvm->DetachCurrentThread();
}

void GameStorage::registerAccount(const std::string& email, const std::string& password) {
    if (!g_jvm || !g_activity) return;
    
    JNIEnv* env;
    if (g_jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) return;
    
    jclass activityClass = env->GetObjectClass(g_activity);
    if (!activityClass) {
        g_jvm->DetachCurrentThread();
        return;
    }
    
    jmethodID createAccount = env->GetMethodID(activityClass, "createAccount",
        "(Ljava/lang/String;Ljava/lang/String;Lcom/example/myapplication/MainActivity$AuthCallback;)V");
    
    if (createAccount) {
        jstring jEmail = env->NewStringUTF(email.c_str());
        jstring jPassword = env->NewStringUTF(password.c_str());
        env->CallVoidMethod(g_activity, createAccount, jEmail, jPassword, nullptr);
        env->DeleteLocalRef(jEmail);
        env->DeleteLocalRef(jPassword);
    }
    
    env->DeleteLocalRef(activityClass);
    g_jvm->DetachCurrentThread();
}

void GameStorage::logout() {
    if (!g_jvm || !g_activity) return;
    
    JNIEnv* env;
    if (g_jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) return;
    
    jclass activityClass = env->GetObjectClass(g_activity);
    if (!activityClass) {
        g_jvm->DetachCurrentThread();
        return;
    }
    
    jmethodID signOut = env->GetMethodID(activityClass, "signOut", "()V");
    if (signOut) {
        env->CallVoidMethod(g_activity, signOut);
    }
    
    env->DeleteLocalRef(activityClass);
    g_jvm->DetachCurrentThread();
}

bool GameStorage::isLoggedIn() {
    if (!g_jvm || !g_activity) return false;
    
    JNIEnv* env;
    if (g_jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) return false;
    
    jclass activityClass = env->GetObjectClass(g_activity);
    if (!activityClass) {
        g_jvm->DetachCurrentThread();
        return false;
    }
    
    jmethodID isLoggedInMethod = env->GetMethodID(activityClass, "isLoggedIn", "()Z");
    bool result = false;
    if (isLoggedInMethod) {
        result = env->CallBooleanMethod(g_activity, isLoggedInMethod);
    }
    
    env->DeleteLocalRef(activityClass);
    g_jvm->DetachCurrentThread();
    
    return result;
}

std::string GameStorage::getCurrentUserEmail() {
    if (!g_jvm || !g_activity) return "";
    
    JNIEnv* env;
    if (g_jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) return "";
    
    jclass activityClass = env->GetObjectClass(g_activity);
    if (!activityClass) {
        g_jvm->DetachCurrentThread();
        return "";
    }
    
    jmethodID getEmail = env->GetMethodID(activityClass, "getCurrentUserEmail", "()Ljava/lang/String;");
    std::string result = "";
    if (getEmail) {
        jstring jEmail = (jstring)env->CallObjectMethod(g_activity, getEmail);
        if (jEmail) {
            const char* email = env->GetStringUTFChars(jEmail, nullptr);
            result = email;
            env->ReleaseStringUTFChars(jEmail, email);
            env->DeleteLocalRef(jEmail);
        }
    }
    
    env->DeleteLocalRef(activityClass);
    g_jvm->DetachCurrentThread();
    
    return result;
}

std::string GameStorage::getCurrentUserUid() {
    if (!g_jvm || !g_activity) return "";
    
    JNIEnv* env;
    if (g_jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) return "";
    
    jclass activityClass = env->GetObjectClass(g_activity);
    if (!activityClass) {
        g_jvm->DetachCurrentThread();
        return "";
    }
    
    jmethodID getUid = env->GetMethodID(activityClass, "getCurrentUserUid", "()Ljava/lang/String;");
    std::string result = "";
    if (getUid) {
        jstring jUid = (jstring)env->CallObjectMethod(g_activity, getUid);
        if (jUid) {
            const char* uid = env->GetStringUTFChars(jUid, nullptr);
            result = uid;
            env->ReleaseStringUTFChars(jUid, uid);
            env->DeleteLocalRef(jUid);
        }
    }
    
    env->DeleteLocalRef(activityClass);
    g_jvm->DetachCurrentThread();
    
    return result;
}
