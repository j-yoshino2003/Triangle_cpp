//---------------------------------------
/// @file   VulkanInstance.h
///
/// @brief  VkInstance とデバッグメッセンジャーの寿命を管理するラッパー.
///
/// @date   2026/08/16
///
/// @author disaster
//---------------------------------------

#pragma once

#include <vector>

#include <vulkan/vulkan.h>

/// <summary>
/// VkInstance と VkDebugUtilsMessengerEXT の生成・破棄を受け持つ.
/// 生ハンドルの後始末をこのクラスへ閉じ込めることで、利用側を Rule of 0 に保つ.
/// </summary>
class VulkanInstance
{
public:
    /// <summary>
    /// Vulkan インスタンスを生成する.
    /// デバッグビルドではバリデーションレイヤーとデバッグメッセンジャーも有効にする.
    /// </summary>
    /// <param name="requiredExtensions">ウィンドウ側が要求する拡張の一覧.</param>
    /// <exception cref="std::runtime_error">生成に失敗した場合.</exception>
    explicit VulkanInstance(const std::vector<const char*>& requiredExtensions);

    ~VulkanInstance();

    // コピーすると同じハンドルを二重に破棄してしまうため禁止する.
    VulkanInstance(const VulkanInstance&) = delete;
    VulkanInstance& operator=(const VulkanInstance&) = delete;

    VulkanInstance(VulkanInstance&& other) noexcept;
    VulkanInstance& operator=(VulkanInstance&& other) noexcept;

    /// <summary>
    /// 生の VkInstance を取得する.
    /// </summary>
    /// <returns>Vulkan インスタンス.</returns>
    VkInstance Get() const;

private:
    /// 保持しているハンドルを破棄する.
    void Destroy() noexcept;

    VkInstance m_Instance{};
    VkDebugUtilsMessengerEXT m_DebugMessenger{};
};
