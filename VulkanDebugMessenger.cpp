//---------------------------------------
/// @file   VulkanDebugMessenger.cpp
///
/// @brief  バリデーションレイヤーからのメッセージを受け取るデバッグメッセンジャー.
///
/// @date   2026/08/16
///
/// @author disaster
//---------------------------------------

#include "VulkanDebugMessenger.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{

/// 有効にするバリデーションレイヤーの名前.
constexpr const char* VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";

// NOTE:
// Vulkan には関数ポインターとして渡すため、メンバー関数にはできない.
// ヘッダーへ出さずに済むよう、この翻訳単位に閉じた自由関数として置く.
/// <summary>
/// バリデーションレイヤーからのメッセージを受け取って標準エラーへ出力する.
/// </summary>
/// <param name="_Severity">メッセージの深刻度.</param>
/// <param name="_Type">メッセージの種類.</param>
/// <param name="_CallbackData">メッセージ本体.</param>
/// <param name="_UserData">利用者が登録した任意のデータ. 今は使わない.</param>
/// <returns>常に VK_FALSE. VK_TRUE を返すと呼び出し元の Vulkan 関数が中断される.</returns>
VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(const VkDebugUtilsMessageSeverityFlagBitsEXT _Severity,
                                             const VkDebugUtilsMessageTypeFlagsEXT _Type,
                                             const VkDebugUtilsMessengerCallbackDataEXT* const _CallbackData,
                                             void* const _UserData)
{
    // 使わない引数を明示的に無視する.
    static_cast<void>(_Type);
    static_cast<void>(_UserData);

    if (_Severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        std::cerr << "[Vulkan] " << _CallbackData->pMessage << std::endl;
    }

    return VK_FALSE;
}

} // namespace

bool VulkanDebugMessenger::IsRequested()
{
    // NOTE:
    // バリデーションレイヤーは検査のぶん実行が遅くなるため、デバッグビルドでのみ使う.
#ifdef _DEBUG
    return true;
#else
    return false;
#endif
}

bool VulkanDebugMessenger::IsAvailable()
{
    uint32_t layerCount{};

    if (vkEnumerateInstanceLayerProperties(&layerCount, nullptr) != VK_SUCCESS)
    {
        return false;
    }

    std::vector<VkLayerProperties> layers(layerCount);

    if (vkEnumerateInstanceLayerProperties(&layerCount, layers.data()) != VK_SUCCESS)
    {
        return false;
    }

    return std::any_of(layers.begin(), layers.end(), [](const VkLayerProperties& layer)
                       { return std::strcmp(layer.layerName, VALIDATION_LAYER_NAME) == 0; });
}

const char* VulkanDebugMessenger::GetLayerName()
{
    return VALIDATION_LAYER_NAME;
}

VkDebugUtilsMessengerCreateInfoEXT VulkanDebugMessenger::MakeCreateInfo()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;

    return createInfo;
}

VulkanDebugMessenger::VulkanDebugMessenger(const VkInstance _Instance) : m_Instance{_Instance}
{
    // NOTE:
    // vkCreateDebugUtilsMessengerEXT は拡張の関数で、ローダーが直接公開していない.
    // 実体はインスタンス経由で取得する必要がある.
    const auto createFunction = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT"));

    if (createFunction == nullptr)
    {
        throw std::runtime_error("vkCreateDebugUtilsMessengerEXT を取得できませんでした.");
    }

    const VkDebugUtilsMessengerCreateInfoEXT createInfo{MakeCreateInfo()};

    if (createFunction(m_Instance, &createInfo, nullptr, &m_Messenger) != VK_SUCCESS)
    {
        throw std::runtime_error("デバッグメッセンジャーの生成に失敗しました.");
    }
}

VulkanDebugMessenger::~VulkanDebugMessenger()
{
    Destroy();
}

VulkanDebugMessenger::VulkanDebugMessenger(VulkanDebugMessenger&& other) noexcept
    : m_Instance{std::exchange(other.m_Instance, VK_NULL_HANDLE)},
      m_Messenger{std::exchange(other.m_Messenger, VK_NULL_HANDLE)}
{
}

VulkanDebugMessenger& VulkanDebugMessenger::operator=(VulkanDebugMessenger&& other) noexcept
{
    if (this != &other)
    {
        Destroy();

        m_Instance = std::exchange(other.m_Instance, VK_NULL_HANDLE);
        m_Messenger = std::exchange(other.m_Messenger, VK_NULL_HANDLE);
    }

    return *this;
}

void VulkanDebugMessenger::Destroy() noexcept
{
    if (m_Messenger == VK_NULL_HANDLE)
    {
        return;
    }

    const auto destroyFunction = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT"));

    if (destroyFunction != nullptr)
    {
        destroyFunction(m_Instance, m_Messenger, nullptr);
    }

    m_Messenger = VK_NULL_HANDLE;
}
