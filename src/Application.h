//---------------------------------------
/// @file   Application.h
///
/// @brief  三角形を描画するアプリケーション本体.
///
/// @date   2026/08/16
///
/// @author disaster
//---------------------------------------

#pragma once

#include <optional>

#include "Platform/GlfwContext.h"
#include "Platform/GlfwWindow.h"
#include "Vulkan/VulkanDebugMessenger.h"
#include "Vulkan/VulkanInstance.h"

/// <summary>
/// 描画に必要な資源をまとめて持ち、ウィンドウが閉じられるまで動かし続ける.
/// 資源はすべて専用ラッパーが持つため、このクラス自身は Rule of 0 で済む.
/// </summary>
class Application
{
public:
    /// 描画に必要な資源を生成する.
    Application();

    /// ウィンドウが閉じられるまでイベントを処理し続ける.
    void Run();

private:
    /// ウィンドウの幅.
    static constexpr int WINDOW_WIDTH = 800;

    /// ウィンドウの高さ.
    static constexpr int WINDOW_HEIGHT = 600;

    /// ウィンドウのタイトル.
    static constexpr const char* WINDOW_TITLE = "Triangle (C++)";

    // NOTE:
    // メンバーは宣言と逆の順で破棄される. 依存される側を前に、依存する側を後ろに並べておくと、
    // 破棄の順序が自動的に正しくなる. 例えばメッセンジャーはインスタンスに属するため後ろに置く.
    // この順序が崩れると、破棄済みのハンドルを参照することになる.

    /// GLFW の初期化状態.
    GlfwContext m_Glfw{};

    /// 描画先のウィンドウ.
    GlfwWindow m_Window;

    /// Vulkan インスタンス.
    VulkanInstance m_Instance;

    /// バリデーションからのメッセージを受け取るメッセンジャー. 検査が無効なときは空になる.
    std::optional<VulkanDebugMessenger> m_DebugMessenger{};
};
