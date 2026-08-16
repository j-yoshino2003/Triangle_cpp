//---------------------------------------
/// @file   VulkanInstance.cpp
///
/// @brief  VkInstance の寿命を管理するラッパー.
///
/// @date   2026/08/16
///
/// @author disaster
//---------------------------------------

#include "VulkanInstance.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "VulkanDebugMessenger.h"

namespace
{

/// アプリケーション名.
constexpr const char* APPLICATION_NAME = "Triangle (C++)";

} // namespace

VulkanInstance::VulkanInstance(const std::vector<const char*>& requiredExtensions)
{
    const bool isValidationRequested = VulkanDebugMessenger::IsRequested();

    m_IsValidationEnabled = isValidationRequested && VulkanDebugMessenger::IsAvailable();

    if (isValidationRequested && !m_IsValidationEnabled)
    {
        std::cerr << "[Vulkan] バリデーションレイヤーが見つかりません. 検査なしで続行します." << std::endl;
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

    if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Vulkan インスタンスの生成に失敗しました.");
    }
}

VulkanInstance::~VulkanInstance()
{
    Destroy();
}

VulkanInstance::VulkanInstance(VulkanInstance&& other) noexcept
    : m_Instance{std::exchange(other.m_Instance, VK_NULL_HANDLE)},
      m_IsValidationEnabled{std::exchange(other.m_IsValidationEnabled, false)}
{
}

VulkanInstance& VulkanInstance::operator=(VulkanInstance&& other) noexcept
{
    if (this != &other)
    {
        Destroy();

        m_Instance = std::exchange(other.m_Instance, VK_NULL_HANDLE);
        m_IsValidationEnabled = std::exchange(other.m_IsValidationEnabled, false);
    }

    return *this;
}

VkInstance VulkanInstance::Get() const
{
    return m_Instance;
}

bool VulkanInstance::IsValidationEnabled() const
{
    return m_IsValidationEnabled;
}

void VulkanInstance::Destroy() noexcept
{
    if (m_Instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
    }
}
