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
    /// バリデーションによる検査を行いたい状況かどうかを返す.
    static bool IsRequested();

    /// バリデーションレイヤーがこの環境に存在するかどうかを返す.
    static bool IsAvailable();

    /// 有効にするバリデーションレイヤーの名前を返す.
    static const char* GetLayerName();

    /// メッセンジャーの生成情報を組み立てる.
    static VkDebugUtilsMessengerCreateInfoEXT MakeCreateInfo();

    /// メッセンジャーを生成する.
    explicit VulkanDebugMessenger(const VkInstance _Instance);

    /// 保持しているメッセンジャーを破棄する.
    ~VulkanDebugMessenger();

    /// コピー構築. 同じハンドルを二重に破棄しないよう禁止する.
    VulkanDebugMessenger(const VulkanDebugMessenger&) = delete;

    /// コピー代入. 同じハンドルを二重に破棄しないよう禁止する.
    VulkanDebugMessenger& operator=(const VulkanDebugMessenger&) = delete;

    /// ムーブ構築. 所有権を移す.
    VulkanDebugMessenger(VulkanDebugMessenger&& other) noexcept;

    /// ムーブ代入. 所有権を移す.
    VulkanDebugMessenger& operator=(VulkanDebugMessenger&& other) noexcept;

private:
    /// 保持しているハンドルを破棄する.
    void Destroy() noexcept;

private:
    /// 有効にするバリデーションレイヤーの名前.
    static constexpr const char* VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";

    /// 生成元のインスタンス. 破棄に必要なだけで、所有はしない.
    VkInstance m_Instance{};

    /// デバッグメッセンジャー.
    VkDebugUtilsMessengerEXT m_Messenger{};
};
