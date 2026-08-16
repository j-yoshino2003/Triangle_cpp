//---------------------------------------
/// @file   VulkanInstance.h
///
/// @brief  VkInstance の寿命を管理するラッパー.
///
/// @date   2026/08/16
///
/// @author disaster
//---------------------------------------

#pragma once

#include <vector>

#include <vulkan/vulkan.h>

/// <summary>
/// VkInstance の生成・破棄を受け持つ.
/// 生ハンドルの後始末をこのクラスへ閉じ込めることで、利用側を Rule of 0 に保つ.
/// </summary>
class VulkanInstance
{
public:
    /// Vulkan インスタンスを生成する.
    explicit VulkanInstance(const std::vector<const char*>& requiredExtensions);

    /// 保持しているインスタンスを破棄する.
    ~VulkanInstance();

    /// コピー構築. 同じハンドルを二重に破棄しないよう禁止する.
    VulkanInstance(const VulkanInstance&) = delete;

    /// コピー代入. 同じハンドルを二重に破棄しないよう禁止する.
    VulkanInstance& operator=(const VulkanInstance&) = delete;

    /// ムーブ構築. 所有権を移す.
    VulkanInstance(VulkanInstance&& other) noexcept;

    /// ムーブ代入. 所有権を移す.
    VulkanInstance& operator=(VulkanInstance&& other) noexcept;

    /// 生の VkInstance を取得する.
    VkInstance Get() const;

    /// バリデーションレイヤーを有効にして生成できたかどうかを返す.
    bool IsValidationEnabled() const;

private:
    /// 保持しているハンドルを破棄する.
    void Destroy() noexcept;

private:
    /// アプリケーション名.
    static constexpr const char* APPLICATION_NAME = "Triangle (C++)";

    /// Vulkan インスタンス.
    VkInstance m_Instance{};

    /// バリデーションレイヤーを有効にできたか.
    bool m_IsValidationEnabled{};
};
