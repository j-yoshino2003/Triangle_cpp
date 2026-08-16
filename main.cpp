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
#include <optional>
#include <stdexcept>
#include <vector>

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

#include "VulkanDebugMessenger.h"
#include "VulkanInstance.h"

namespace
{

/// ウィンドウの幅.
constexpr int WINDOW_WIDTH = 800;

/// ウィンドウの高さ.
constexpr int WINDOW_HEIGHT = 600;

/// ウィンドウのタイトル.
constexpr const char* WINDOW_TITLE = "Triangle (C++)";

/// <summary>
/// GLFW が Vulkan の利用に必要とする拡張の一覧を取得する.
/// </summary>
/// <returns>拡張名の一覧.</returns>
/// <exception cref="std::runtime_error">取得に失敗した場合.</exception>
std::vector<const char*> GetRequiredExtensions()
{
    uint32_t extensionCount{};
    const char** const extensionNames = glfwGetRequiredInstanceExtensions(&extensionCount);

    if (extensionNames == nullptr)
    {
        throw std::runtime_error("GLFW が要求する Vulkan 拡張を取得できませんでした.");
    }

    return std::vector<const char*>(extensionNames, extensionNames + extensionCount);
}

// NOTE:
// windows.h が CreateWindow をマクロとして定義しているため、その名前は使えない.
/// <summary>
/// Vulkan で描画するためのウィンドウを生成する.
/// </summary>
/// <returns>生成したウィンドウ.</returns>
/// <exception cref="std::runtime_error">生成に失敗した場合.</exception>
GLFWwindow* CreateAppWindow()
{
    // GLFW は既定で OpenGL のコンテキストを作るため、Vulkan で使うときは明示的に無効化する.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* const window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);

    if (window == nullptr)
    {
        throw std::runtime_error("ウィンドウの生成に失敗しました.");
    }

    return window;
}

/// <summary>
/// ウィンドウを表示し、閉じられるまでイベントを処理し続ける.
/// </summary>
/// <exception cref="std::runtime_error">初期化に失敗した場合.</exception>
void Run()
{
    if (glfwInit() != GLFW_TRUE)
    {
        throw std::runtime_error("GLFW の初期化に失敗しました.");
    }

    if (glfwVulkanSupported() != GLFW_TRUE)
    {
        glfwTerminate();

        throw std::runtime_error("この環境では Vulkan が利用できません.");
    }

    GLFWwindow* window{};

    try
    {
        window = CreateAppWindow();

        const VulkanInstance instance{GetRequiredExtensions()};

        // NOTE:
        // メッセンジャーはインスタンスに属するため、インスタンスより後に生成する.
        // ローカル変数は生成と逆の順で破棄されるので、破棄はメッセンジャーが先になる.
        // この順序が崩れると、破棄済みのインスタンスを参照することになる.
        std::optional<VulkanDebugMessenger> debugMessenger{};

        if (instance.IsValidationEnabled())
        {
            debugMessenger.emplace(instance.Get());
        }

        std::cout << "Vulkan インスタンスを生成しました." << std::endl;
        std::cout << "バリデーション: " << (instance.IsValidationEnabled() ? "有効" : "無効") << std::endl;

        while (glfwWindowShouldClose(window) == GLFW_FALSE)
        {
            glfwPollEvents();
        }
    }
    catch (...)
    {
        // NOTE:
        // GLFW は C の API で RAII が効かないため、例外で抜けるときも確実に後始末する.
        if (window != nullptr)
        {
            glfwDestroyWindow(window);
        }

        glfwTerminate();

        throw;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}

} // namespace

/// <summary>
/// アプリケーションのエントリーポイント.
/// </summary>
/// <returns>正常終了なら EXIT_SUCCESS、失敗なら EXIT_FAILURE.</returns>
int main()
{
    // NOTE:
    // コンパイラーに /utf-8 を指定しているため、文字列リテラルは UTF-8 で埋め込まれる.
    // コンソール側の既定は 932 (Shift_JIS) なので、出力コードページも UTF-8 に合わせる.
    // これを忘れると日本語が文字化けする.
    SetConsoleOutputCP(CP_UTF8);

    try
    {
        Run();
    }
    catch (const std::exception& error)
    {
        std::cerr << "エラー: " << error.what() << std::endl;

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
