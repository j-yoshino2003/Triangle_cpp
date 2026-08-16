//---------------------------------------
/// @file   VulkanInstance.cpp
///
/// @brief  VkInstance の寿命を管理するラッパー.
///
/// @date   2026/08/16
///
/// @author disaster
//---------------------------------------

#include "Vulkan/VulkanInstance.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

#include <vulkan/vk_enum_string_helper.h>

#include "Vulkan/VulkanDebugMessenger.h"

/// <summary>
/// Vulkan インスタンスを生成する.
/// バリデーションレイヤーが使える状況なら、あわせて有効にする.
/// </summary>
/// <param name="requiredExtensions">ウィンドウ側が要求する拡張の一覧.</param>
/// <exception cref="std::runtime_error">生成に失敗した場合.</exception>
VulkanInstance::VulkanInstance(const std::vector<const char*>& requiredExtensions)
{
    const bool isValidationRequested = VulkanDebugMessenger::IsRequested();
    const ValidationLayerAvailability availability = VulkanDebugMessenger::GetAvailability();

    m_IsValidationEnabled = isValidationRequested && availability == ValidationLayerAvailability::AVAILABLE;

    // NOTE:
    // どちらの理由でも検査なしで続行するが、利用者に伝える内容は変える.
    // 「見つからない」と「調べられなかった」では次に取るべき対処が違うため.
    if (isValidationRequested && availability == ValidationLayerAvailability::NOT_FOUND)
    {
        std::cerr << "[Vulkan] バリデーションレイヤーが見つかりません. 検査なしで続行します." << std::endl;
    }

    if (isValidationRequested && availability == ValidationLayerAvailability::QUERY_FAILED)
    {
        std::cerr << "[Vulkan] レイヤーの有無を確認できませんでした. 検査なしで続行します." << std::endl;
    }

    // 要求された拡張に、デバッグメッセンジャー用の拡張を足す.
    std::vector<const char*> extensions{requiredExtensions};
    std::vector<const char*> layers{};

    if (m_IsValidationEnabled)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        layers.push_back(VulkanDebugMessenger::GetLayerName());
    }

    VkApplicationInfo applicationInfo{};

    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = APPLICATION_NAME;
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "No Engine";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_4;

    // NOTE:
    // pNext にデバッグメッセンジャーの生成情報を繋ぐと、インスタンスの生成中と破棄中の
    // 誤りも検査対象になる. メッセンジャー本体はインスタンスが無いと作れないため.
    const VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{VulkanDebugMessenger::MakeCreateInfo()};

    VkInstanceCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &applicationInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    createInfo.ppEnabledLayerNames = layers.data();

    if (m_IsValidationEnabled)
    {
        createInfo.pNext = &debugCreateInfo;
    }

    // NOTE:
    // 結果コードをそのまま握りつぶすと、レイヤー不足・拡張名の誤り・ドライバー非互換のどれかが
    // 分からなくなる. 失敗の切り分けに要るのでメッセージへ残す.
    const VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(std::string{"Vulkan インスタンスの生成に失敗しました: "} + string_VkResult(result));
    }
}

/// <summary>
/// 保持しているインスタンスを破棄する.
/// </summary>
VulkanInstance::~VulkanInstance()
{
    Destroy();
}

/// <summary>
/// 所有権を移してムーブ構築する.
/// </summary>
/// <param name="other">移動元. 呼び出し後は空になる.</param>
VulkanInstance::VulkanInstance(VulkanInstance&& other) noexcept
    : m_Instance{std::exchange(other.m_Instance, VkInstance{})},
      m_IsValidationEnabled{std::exchange(other.m_IsValidationEnabled, false)}
{
}

/// <summary>
/// 自分の持つインスタンスを破棄してから、所有権を移す.
/// </summary>
/// <param name="other">移動元. 呼び出し後は空になる.</param>
/// <returns>自分自身.</returns>
VulkanInstance& VulkanInstance::operator=(VulkanInstance&& other) noexcept
{
    if (this != &other)
    {
        Destroy();

        m_Instance = std::exchange(other.m_Instance, VkInstance{});
        m_IsValidationEnabled = std::exchange(other.m_IsValidationEnabled, false);
    }

    return *this;
}

/// <summary>
/// 生の VkInstance を取得する.
/// </summary>
/// <returns>Vulkan インスタンス.</returns>
VkInstance VulkanInstance::Get() const
{
    return m_Instance;
}

/// <summary>
/// バリデーションレイヤーを有効にして生成できたかどうかを返す.
/// </summary>
/// <returns>true = 有効になっている、false = なっていない.</returns>
bool VulkanInstance::IsValidationEnabled() const
{
    return m_IsValidationEnabled;
}

/// <summary>
/// 保持しているハンドルを破棄する.
/// 破棄済み・未生成のいずれでも安全に呼べる.
/// </summary>
void VulkanInstance::Destroy() noexcept
{
    if (m_Instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
    }
}
