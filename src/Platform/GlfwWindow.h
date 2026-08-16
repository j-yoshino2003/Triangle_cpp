//---------------------------------------
/// @file   GlfwWindow.h
///
/// @brief  GLFW のウィンドウの寿命を管理するラッパー.
///
/// @date   2026/08/16
///
/// @author disaster
//---------------------------------------

#pragma once

/// GLFW のウィンドウ. 中身は GLFW 側にしかないため、ここでは前方宣言で足りる.
struct GLFWwindow;

/// <summary>
/// GLFWwindow の生成・破棄を受け持つ.
/// 生ハンドルの後始末をこのクラスへ閉じ込めることで、利用側を Rule of 0 に保つ.
/// </summary>
class GlfwWindow
{
public:
    /// ウィンドウを生成する.
    GlfwWindow(const int _Width, const int _Height, const char* const _Title);

    /// 保持しているウィンドウを破棄する.
    ~GlfwWindow();

    /// コピー構築. 同じハンドルを二重に破棄しないよう禁止する.
    GlfwWindow(const GlfwWindow&) = delete;

    /// コピー代入. 同じハンドルを二重に破棄しないよう禁止する.
    GlfwWindow& operator=(const GlfwWindow&) = delete;

    /// ムーブ構築. 所有権を移す.
    GlfwWindow(GlfwWindow&& other) noexcept;

    /// ムーブ代入. 所有権を移す.
    GlfwWindow& operator=(GlfwWindow&& other) noexcept;

    /// 生の GLFWwindow を取得する.
    GLFWwindow* Get() const;

    /// 閉じる要求が出ているかどうかを返す.
    bool ShouldClose() const;

private:
    /// 保持しているハンドルを破棄する.
    void Destroy() noexcept;

private:
    /// GLFW のウィンドウ.
    GLFWwindow* m_Window{};
};
