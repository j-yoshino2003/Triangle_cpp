//---------------------------------------
/// @file   GlfwWindow.cpp
///
/// @brief  GLFW のウィンドウの寿命を管理するラッパー.
///
/// @date   2026/08/16
///
/// @author disaster
//---------------------------------------

#include "Platform/GlfwWindow.h"

#include <stdexcept>
#include <utility>

#include <GLFW/glfw3.h>

/// <summary>
/// ウィンドウを生成する.
/// </summary>
/// <param name="_Width">ウィンドウの幅.</param>
/// <param name="_Height">ウィンドウの高さ.</param>
/// <param name="_Title">タイトルバーに表示する文字列.</param>
/// <exception cref="std::runtime_error">生成に失敗した場合.</exception>
GlfwWindow::GlfwWindow(const int _Width, const int _Height, const char* const _Title)
{
    // GLFW は既定で OpenGL のコンテキストを作るため、Vulkan で使うときは明示的に無効化する.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_Window = glfwCreateWindow(_Width, _Height, _Title, nullptr, nullptr);

    if (m_Window == nullptr)
    {
        throw std::runtime_error("ウィンドウの生成に失敗しました.");
    }
}

/// <summary>
/// 保持しているウィンドウを破棄する.
/// </summary>
GlfwWindow::~GlfwWindow()
{
    Destroy();
}

/// <summary>
/// 所有権を移してムーブ構築する.
/// </summary>
/// <param name="other">移動元. 呼び出し後は空になる.</param>
GlfwWindow::GlfwWindow(GlfwWindow&& other) noexcept : m_Window{std::exchange(other.m_Window, nullptr)}
{
}

/// <summary>
/// 自分の持つウィンドウを破棄してから、所有権を移す.
/// </summary>
/// <param name="other">移動元. 呼び出し後は空になる.</param>
/// <returns>自分自身.</returns>
GlfwWindow& GlfwWindow::operator=(GlfwWindow&& other) noexcept
{
    if (this != &other)
    {
        Destroy();

        m_Window = std::exchange(other.m_Window, nullptr);
    }

    return *this;
}

/// <summary>
/// 生の GLFWwindow を取得する.
/// </summary>
/// <returns>GLFW のウィンドウ.</returns>
GLFWwindow* GlfwWindow::Get() const
{
    return m_Window;
}

/// <summary>
/// 閉じる要求が出ているかどうかを返す.
/// </summary>
/// <returns>true = 閉じる要求が出ている、false = 出ていない.</returns>
bool GlfwWindow::ShouldClose() const
{
    return glfwWindowShouldClose(m_Window) == GLFW_TRUE;
}

/// <summary>
/// 保持しているハンドルを破棄する.
/// 破棄済み・未生成のいずれでも安全に呼べる.
/// </summary>
void GlfwWindow::Destroy() noexcept
{
    if (m_Window != nullptr)
    {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
    }
}
