//---------------------------------------
/// @file   VulkanInstance.cpp
///
/// @brief  VkInstance とデバッグメッセンジャーの寿命を管理するラッパー.
///
/// @date   2026/08/16
///
/// @author disaster
//---------------------------------------

#include "VulkanInstance.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace
{

/// 有効にするバリデーションレイヤーの名前.
constexpr const char* VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";

/// アプリケーション名.
constexpr const char* APPLICATION_NAME = "Triangle (C++)";

// NOTE:
// バリデーションレイヤーは検査のぶん実行が遅くなるため、デバッグビルドでのみ有効にする.
#ifdef _DEBUG
constexpr bool IS_VALIDATION_ENABLED = true;
#else
constexpr bool IS_VALIDATION_ENABLED = false;
#endif

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

/// <summary>
/// バリデーションレイヤーがこの環境で使えるかどうかを調べる.
/// </summary>
/// <returns>使える場合は true.</returns>
bool IsValidationLayerAvailable()
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

/// <summary>
/// デバッグメッセンジャーの生成情報を組み立てる.
/// </summary>
/// <returns>生成情報.</returns>
VkDebugUtilsMessengerCreateInfoEXT MakeDebugMessengerCreateInfo()
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

// NOTE:
// vkCreateDebugUtilsMessengerEXT は拡張の関数で、Vulkan のヘッダーには宣言しかない.
// 実体はインスタンスから取得する必要がある.
VkResult CreateDebugMessenger(const VkInstance _Instance, const VkDebugUtilsMessengerCreateInfoEXT& createInfo,
                              VkDebugUtilsMessengerEXT& debugMessenger)
{
    const auto createFunction = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(_Instance, "vkCreateDebugUtilsMessengerEXT"));

    if (createFunction == nullptr)
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    return createFunction(_Instance, &createInfo, nullptr, &debugMessenger);
}

/// <summary>
/// デバッグメッセンジャーを破棄する.
/// </summary>
/// <param name="_Instance">生成に使ったインスタンス.</param>
/// <param name="_DebugMessenger">破棄するメッセンジャー.</param>
void DestroyDebugMessenger(const VkInstance _Instance, const VkDebugUtilsMessengerEXT _DebugMessenger)
{
    const auto destroyFunction = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(_Instance, "vkDestroyDebugUtilsMessengerEXT"));

    if (destroyFunction != nullptr)
    {
        destroyFunction(_Instance, _DebugMessenger, nullptr);
    }
}

} // namespace

VulkanInstance::VulkanInstance(const std::vector<const char*>& requiredExtensions)
{
    const bool isValidationEnabled = IS_VALIDATION_ENABLED && IsValidationLayerAvailable();

    if (IS_VALIDATION_ENABLED && !isValidationEnabled)
    {
        std::cerr << "[Vulkan] バリデーションレイヤーが見つかりません. 検査なしで続行します." << std::endl;
    }

    // 要求された拡張に、デバッグメッセンジャー用の拡張を足す.
    std::vector<const char*> extensions{requiredExtensions};

    if (isValidationEnabled)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    VkApplicationInfo applicationInfo{};

    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = APPLICATION_NAME;
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "No Engine";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &applicationInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // NOTE:
    // pNext にデバッグメッセンジャーの情報を繋ぐと、インスタンスの生成中と破棄中の
    // 誤りも検査対象になる. メッセンジャー本体はインスタンスが無いと作れないため.
    const VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{MakeDebugMessengerCreateInfo()};

    if (isValidationEnabled)
    {
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = &VALIDATION_LAYER_NAME;
        createInfo.pNext = &debugCreateInfo;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Vulkan インスタンスの生成に失敗しました.");
    }

    if (isValidationEnabled && CreateDebugMessenger(m_Instance, debugCreateInfo, m_DebugMessenger) != VK_SUCCESS)
    {
        Destroy();

        throw std::runtime_error("デバッグメッセンジャーの生成に失敗しました.");
    }
}

VulkanInstance::~VulkanInstance()
{
    Destroy();
}

VulkanInstance::VulkanInstance(VulkanInstance&& other) noexcept
    : m_Instance{std::exchange(other.m_Instance, VK_NULL_HANDLE)},
      m_DebugMessenger{std::exchange(other.m_DebugMessenger, VK_NULL_HANDLE)}
{
}

VulkanInstance& VulkanInstance::operator=(VulkanInstance&& other) noexcept
{
    if (this != &other)
    {
        Destroy();

        m_Instance = std::exchange(other.m_Instance, VK_NULL_HANDLE);
        m_DebugMessenger = std::exchange(other.m_DebugMessenger, VK_NULL_HANDLE);
    }

    return *this;
}

VkInstance VulkanInstance::Get() const
{
    return m_Instance;
}

void VulkanInstance::Destroy() noexcept
{
    // NOTE:
    // メッセンジャーはインスタンスに属するため、インスタンスより先に破棄する.
    if (m_DebugMessenger != VK_NULL_HANDLE)
    {
        DestroyDebugMessenger(m_Instance, m_DebugMessenger);
        m_DebugMessenger = VK_NULL_HANDLE;
    }

    if (m_Instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
    }
}
