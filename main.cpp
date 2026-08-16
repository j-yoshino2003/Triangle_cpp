//---------------------------------------
/// @file   main.cpp
///
/// @brief  Vulkan で三角形を描画するアプリケーションのエントリーポイント.
///
/// @date   2026/08/16
///
/// @author disaster
//---------------------------------------

#include <cstdint>
#include <cstdlib>
#include <iostream>

// NOTE:
// windows.h は min / max マクロなどで衝突を起こしやすいため、取り込む範囲を絞ってから読み込む.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// NOTE:
// GLFW_INCLUDE_VULKAN を定義すると glfw3.h が vulkan.h を取り込み、
// glfwCreateWindowSurface などの Vulkan 連携 API が使えるようになる.
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace
{

/// ウィンドウの幅.
constexpr int WINDOW_WIDTH = 800;

/// ウィンドウの高さ.
constexpr int WINDOW_HEIGHT = 600;

/// ウィンドウのタイトル.
constexpr const char* WINDOW_TITLE = "Triangle (C++)";

/// <summary>
/// Vulkan のインスタンス拡張の数を表示して、SDK とローダーが動くことを確認する.
/// </summary>
/// <returns>取得に成功したら true.</returns>
bool ReportInstanceExtensions()
{
    uint32_t extensionCount{};

    if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr) != VK_SUCCESS)
    {
        std::cerr << "Vulkan のインスタンス拡張を取得できませんでした." << std::endl;

        return false;
    }

    std::cout << "Vulkan インスタンス拡張の数: " << extensionCount << std::endl;

    return true;
}

} // namespace

// TODO:
// ここは環境確認用の暫定コード.
// 三角形の描画に着手する際、GLFW と Vulkan のハンドルは破棄処理を持つ
// ラッパークラスへ隔離し、呼び出し側を Rule of 0 に保つ.
int main()
{
    // NOTE:
    // コンパイラーに /utf-8 を指定しているため、文字列リテラルは UTF-8 で埋め込まれる.
    // コンソール側の既定は 932 (Shift_JIS) なので、出力コードページも UTF-8 に合わせる.
    // これを忘れると日本語が文字化けする.
    SetConsoleOutputCP(CP_UTF8);

    if (glfwInit() != GLFW_TRUE)
    {
        std::cerr << "GLFW の初期化に失敗しました." << std::endl;

        return EXIT_FAILURE;
    }

    if (glfwVulkanSupported() != GLFW_TRUE)
    {
        std::cerr << "この環境では Vulkan が利用できません." << std::endl;
        glfwTerminate();

        return EXIT_FAILURE;
    }

    if (!ReportInstanceExtensions())
    {
        glfwTerminate();

        return EXIT_FAILURE;
    }

    // GLFW は既定で OpenGL のコンテキストを作るため、Vulkan で使うときは明示的に無効化する.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);

    if (window == nullptr)
    {
        std::cerr << "ウィンドウの生成に失敗しました." << std::endl;
        glfwTerminate();

        return EXIT_FAILURE;
    }

    while (glfwWindowShouldClose(window) == GLFW_FALSE)
    {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
