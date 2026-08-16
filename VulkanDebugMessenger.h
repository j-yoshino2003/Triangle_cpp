//---------------------------------------
/// @file   VulkanDebugMessenger.h
///
/// @brief  バリデーションレイヤーからのメッセージを受け取るデバッグメッセンジャー.
///
/// @date   2026/08/16
///
/// @author disaster
//---------------------------------------

#pragma once

#include <vulkan/vulkan.h>

/// <summary>
/// VkDebugUtilsMessengerEXT の生成・破棄を受け持つ.
/// バリデーションレイヤーに関する判断もこのクラスへ集約する.
/// </summary>
class VulkanDebugMessenger
{
public:
    /// <summary>
    /// バリデーションによる検査を行いたい状況かどうかを返す.
    /// </summary>
    /// <returns>デバッグビルドなら true.</returns>
    static bool IsRequested();

    /// <summary>
    /// バリデーションレイヤーがこの環境に存在するかどうかを返す.
    /// </summary>
    /// <returns>存在すれば true.</returns>
    static bool IsAvailable();

    /// <summary>
    /// 有効にするバリデーションレイヤーの名前を返す.
    /// </summary>
    /// <returns>レイヤー名.</returns>
    static const char* GetLayerName();

    /// <summary>
    /// メッセンジャーの生成情報を組み立てる.
    /// VkInstanceCreateInfo の pNext へ繋ぐ用途にも使う.
    /// </summary>
    /// <returns>生成情報.</returns>
    static VkDebugUtilsMessengerCreateInfoEXT MakeCreateInfo();

    /// <summary>
    /// メッセンジャーを生成する.
    /// </summary>
    /// <param name="_Instance">生成元のインスタンス. 所有はしない.</param>
    /// <exception cref="std::runtime_error">生成に失敗した場合.</exception>
    explicit VulkanDebugMessenger(const VkInstance _Instance);

    ~VulkanDebugMessenger();

    // コピーすると同じハンドルを二重に破棄してしまうため禁止する.
    VulkanDebugMessenger(const VulkanDebugMessenger&) = delete;
    VulkanDebugMessenger& operator=(const VulkanDebugMessenger&) = delete;

    VulkanDebugMessenger(VulkanDebugMessenger&& other) noexcept;
    VulkanDebugMessenger& operator=(VulkanDebugMessenger&& other) noexcept;

private:
    /// 保持しているハンドルを破棄する.
    void Destroy() noexcept;

    // NOTE:
    // 破棄には生成元のインスタンスが要るため保持する. 所有はしないので破棄もしない.
    VkInstance m_Instance{};

    VkDebugUtilsMessengerEXT m_Messenger{};
};
