//---------------------------------------
/// @file   GlfwContext.cpp
///
/// @brief  GLFW ライブラリの初期化状態を管理するラッパー.
///
/// @date   2026/08/16
///
/// @author disaster
//---------------------------------------

#include "GlfwContext.h"

#include <cstdint>
#include <stdexcept>
#include <utility>

#include <GLFW/glfw3.h>

/// <summary>
/// GLFW を初期化し、Vulkan が使える環境かどうかもあわせて確かめる.
/// </summary>
/// <exception cref="std::runtime_error">初期化に失敗した場合、または Vulkan を利用できない場合.</exception>
GlfwContext::GlfwContext()
{
    if (glfwInit() != GLFW_TRUE)
    {
        throw std::runtime_error("GLFW の初期化に失敗しました.");
    }

    m_IsInitialized = true;

    if (glfwVulkanSupported() != GLFW_TRUE)
    {
        // NOTE:
        // コンストラクターが例外を投げた場合、そのオブジェクトのデストラクターは呼ばれない.
        // 初期化済みの GLFW はここで自分で片付ける必要がある.
        Destroy();

        throw std::runtime_error("この環境では Vulkan が利用できません.");
    }
}

/// <summary>
/// 初期化済みの GLFW を終了する.
/// </summary>
GlfwContext::~GlfwContext()
{
    Destroy();
}

/// <summary>
/// 所有権を移してムーブ構築する.
/// </summary>
/// <param name="other">移動元. 呼び出し後は空になる.</param>
GlfwContext::GlfwContext(GlfwContext&& other) noexcept : m_IsInitialized{std::exchange(other.m_IsInitialized, false)}
{
}

/// <summary>
/// 自分の持つ初期化状態を終了してから、所有権を移す.
/// </summary>
/// <param name="other">移動元. 呼び出し後は空になる.</param>
/// <returns>自分自身.</returns>
GlfwContext& GlfwContext::operator=(GlfwContext&& other) noexcept
{
    if (this != &other)
    {
        Destroy();

        m_IsInitialized = std::exchange(other.m_IsInitialized, false);
    }

    return *this;
}

/// <summary>
/// Vulkan インスタンスの生成に必要な拡張の一覧を取得する.
/// ウィンドウへ描画するには、環境ごとに異なるサーフェス用の拡張が要る. その一覧を GLFW が教えてくれる.
/// </summary>
/// <returns>拡張名の一覧.</returns>
/// <exception cref="std::runtime_error">取得に失敗した場合.</exception>
std::vector<const char*> GlfwContext::GetRequiredInstanceExtensions() const
{
    uint32_t extensionCount{};
    const char** const extensionNames = glfwGetRequiredInstanceExtensions(&extensionCount);

    if (extensionNames == nullptr)
    {
        throw std::runtime_error("GLFW が要求する Vulkan 拡張を取得できませんでした.");
    }

    return std::vector<const char*>(extensionNames, extensionNames + extensionCount);
}

/// <summary>
/// 溜まっているウィンドウイベントを処理する.
/// </summary>
void GlfwContext::PollEvents() const
{
    glfwPollEvents();
}

/// <summary>
/// 初期化済みなら GLFW を終了する.
/// 終了済み・未初期化のいずれでも安全に呼べる.
/// </summary>
void GlfwContext::Destroy() noexcept
{
    if (m_IsInitialized)
    {
        glfwTerminate();
        m_IsInitialized = false;
    }
}
