plugins {
    alias(libs.plugins.jetbrains.kotlin.jvm)
}

dependencies {
    implementation(project(":core"))
    implementation(libs.gdx.core)
    implementation(libs.gdx.freetype)
}
