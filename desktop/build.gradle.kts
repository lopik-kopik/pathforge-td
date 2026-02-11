plugins {
    alias(libs.plugins.jetbrains.kotlin.jvm)
    application
}

application {
    mainClass.set("com.pathforge.desktop.DesktopLauncherKt")
}

dependencies {
    implementation(project(":game"))
}
