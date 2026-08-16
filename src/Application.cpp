//---------------------------------------
/// @file   Application.cpp
///
/// @brief  三角形を描画するアプリケーション本体.
///
/// @date   2026/08/16
///
/// @author disaster
//---------------------------------------

#include "Application.h"

#include <iostream>

/// <summary>
/// 描画に必要な資源を生成する.
/// メンバーは宣言順に生成されるため、GLFW の初期化 → ウィンドウ → インスタンス の順になる.
/// </summary>
/// <exception cref="std::runtime_error">いずれかの生成に失敗した場合.</exception>
Application::Application()
    : m_Window{WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE}, m_Instance{m_Glfw.GetRequiredInstanceExtensions()}
{
    const bool isValidationEnabled = m_Instance.IsValidationEnabled();

    if (isValidationEnabled)
    {
        m_DebugMessenger.emplace(m_Instance.Get());
    }

    std::cout << "Vulkan インスタンスを生成しました." << std::endl;
    std::cout << "バリデーション: " << (isValidationEnabled ? "有効" : "無効") << std::endl;
}

/// <summary>
/// ウィンドウが閉じられるまでイベントを処理し続ける.
/// </summary>
void Application::Run()
{
    while (!m_Window.ShouldClose())
    {
        m_Glfw.PollEvents();
    }
}
