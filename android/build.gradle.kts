plugins {
    alias(libs.plugins.android.application)
}

val natives by configurations.creating

android {
    namespace = "com.pathforge.android"
    compileSdk = 36

    defaultConfig {
        applicationId = "com.pathforge.android"
        minSdk = 31
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    sourceSets {
                getByName("main").jniLibs.srcDir("$buildDir/generated/jniLibs")
    }
}

dependencies {
    implementation(project(":core"))
    implementation(project(":game"))
    implementation(libs.gdx.core)
    implementation(libs.gdx.backend.android)
    implementation(libs.gdx.freetype)
    natives("com.badlogicgames.gdx:gdx-platform:${libs.versions.gdx.get()}:natives-arm64-v8a")
    natives("com.badlogicgames.gdx:gdx-platform:${libs.versions.gdx.get()}:natives-armeabi-v7a")
    natives("com.badlogicgames.gdx:gdx-platform:${libs.versions.gdx.get()}:natives-x86")
    natives("com.badlogicgames.gdx:gdx-platform:${libs.versions.gdx.get()}:natives-x86_64")
    natives("com.badlogicgames.gdx:gdx-freetype-platform:${libs.versions.gdx.get()}:natives-arm64-v8a")
    natives("com.badlogicgames.gdx:gdx-freetype-platform:${libs.versions.gdx.get()}:natives-armeabi-v7a")
    natives("com.badlogicgames.gdx:gdx-freetype-platform:${libs.versions.gdx.get()}:natives-x86")
    natives("com.badlogicgames.gdx:gdx-freetype-platform:${libs.versions.gdx.get()}:natives-x86_64")
    implementation(libs.androidx.core.ktx)
}

val copyAndroidNatives by tasks.registering {
    val outDir = layout.buildDirectory.dir("generated/jniLibs")
    outputs.dir(outDir)

    doLast {
        val output = outDir.get().asFile
        delete(output)

        natives.files.forEach { jar ->
            val abi = when {
                jar.name.contains("arm64-v8a") -> "arm64-v8a"
                jar.name.contains("armeabi-v7a") -> "armeabi-v7a"
                jar.name.contains("x86_64") -> "x86_64"
                jar.name.contains("x86") -> "x86"
                else -> null
            } ?: return@forEach

            copy {
                from(zipTree(jar))
                include("*.so")
                into(File(output, abi))
            }
        }
    }
}

tasks.matching { it.name.startsWith("merge") && it.name.endsWith("JniLibFolders") }.configureEach {
    dependsOn(copyAndroidNatives)
}


