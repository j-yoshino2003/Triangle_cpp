//---------------------------------------
/// @file   GlfwContext.h
///
/// @brief  GLFW ライブラリの初期化状態を管理するラッパー.
///
/// @date   2026/08/16
///
/// @author disaster
//---------------------------------------

#pragma once

#include <vector>

/// <summary>
/// GLFW の初期化・終了を受け持つ.
/// glfwInit と glfwTerminate は対で呼ぶ必要があるため、その対応をこのクラスへ閉じ込める.
/// </summary>
class GlfwContext
{
public:
    /// GLFW を初期化する.
    GlfwContext();

    /// 初期化済みの GLFW を終了する.
    ~GlfwContext();

    /// コピー構築. 同じ初期化状態を二重に終了しないよう禁止する.
    GlfwContext(const GlfwContext&) = delete;

    /// コピー代入. 同じ初期化状態を二重に終了しないよう禁止する.
    GlfwContext& operator=(const GlfwContext&) = delete;

    /// ムーブ構築. 所有権を移す.
    GlfwContext(GlfwContext&& other) noexcept;

    /// ムーブ代入. 所有権を移す.
    GlfwContext& operator=(GlfwContext&& other) noexcept;

    /// Vulkan インスタンスの生成に必要な拡張の一覧を取得する.
    std::vector<const char*> GetRequiredInstanceExtensions() const;

    /// 溜まっているウィンドウイベントを処理する.
    void PollEvents() const;

private:
    /// 初期化済みなら GLFW を終了する.
    void Destroy() noexcept;

private:
    /// GLFW を初期化できたか.
    bool m_IsInitialized{};
};
