/*
 * File: main.cpp
 * Purpose: Entry point for the platform switching sample demonstrating AppBuilder::UseSkia().
 */

#include "ui/app/Application.hpp"
#include "ui/app/AppBuilder.hpp"
#include "compat/Compat.hpp"
#include "SkiaDemoForm.hpp"

int main() {
    using namespace cppui::app;
    using namespace platform_skia_example;

    compat::println("==================================================");
    compat::println(" CPPUI Example: Platform Switching (Forced Skia)  ");
    compat::println("==================================================");

    // 建立應用程式建構器
    auto builder = Application::CreateBuilder();

    // 顯式宣告：強制使用 Google Skia 向量加速渲染後端
    builder.UseSkia();

    // 註冊表單至 IoC 容器 (亦支援 host.Run<SkiaDemoForm>() 原地自動建構)
    builder.Services().AddTransient<SkiaDemoForm>();

    // 構建 Host 宿主並啟動視窗循環
    auto host = builder.Build();
    return host.Run<SkiaDemoForm>();
}
