//---------------------------------------
/// @file   VulkanDebugMessenger.cpp
///
/// @brief  バリデーションレイヤーからのメッセージを受け取るデバッグメッセンジャー.
///
/// @date   2026/08/16
///
/// @author disaster
//---------------------------------------

#include "Vulkan/VulkanDebugMessenger.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <vulkan/vk_enum_string_helper.h>

namespace
{
    /// <summary>
    /// バリデーションレイヤーからのメッセージを受け取って標準エラーへ出力する.
    ///
    /// NOTE:
    /// Vulkan には関数ポインターとして渡すため、メンバー関数にはできない.
    /// ヘッダーへ出さずに済むよう、この翻訳単位に閉じた自由関数として置く.
    /// </summary>
    /// <param name="_Severity">メッセージの深刻度.</param>
    /// <param name="_Type">メッセージの種類.</param>
    /// <param name="_CallbackData">メッセージ本体.</param>
    /// <param name="_UserData">利用者が登録した任意のデータ. 今は使わない.</param>
    /// <returns>VK_TRUE = 呼び出し元の Vulkan 関数を中断する、VK_FALSE = 続行する.</returns>
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

/// <summary>
/// バリデーションによる検査を行いたい状況かどうかを返す.
/// </summary>
/// <returns>true = 検査を行う（デバッグビルド）、false = 行わない（リリースビルド）.</returns>
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

/// <summary>
/// バリデーションレイヤーを使えるかどうかを、使えない場合の理由つきで返す.
/// 列挙 API の失敗はここでしか結果コードを持てないため、詳細もこの場で出力する.
/// </summary>
/// <returns>利用可否と、利用できない場合の理由.</returns>
ValidationLayerAvailability VulkanDebugMessenger::GetAvailability()
{
    uint32_t layerCount{};
    const VkResult countResult = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    if (countResult != VK_SUCCESS)
    {
        std::cerr << "[Vulkan] レイヤー数の取得に失敗しました: " << string_VkResult(countResult) << std::endl;

        return ValidationLayerAvailability::QUERY_FAILED;
    }

    std::vector<VkLayerProperties> layers(layerCount);
    const VkResult listResult = vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    if (listResult != VK_SUCCESS)
    {
        std::cerr << "[Vulkan] レイヤー一覧の取得に失敗しました: " << string_VkResult(listResult) << std::endl;

        return ValidationLayerAvailability::QUERY_FAILED;
    }

    const bool isFound = std::any_of(layers.begin(), layers.end(), [](const VkLayerProperties& layer)
                                     { return std::strcmp(layer.layerName, VALIDATION_LAYER_NAME) == 0; });

    return isFound ? ValidationLayerAvailability::AVAILABLE : ValidationLayerAvailability::NOT_FOUND;
}

/// <summary>
/// 有効にするバリデーションレイヤーの名前を返す.
/// </summary>
/// <returns>レイヤー名.</returns>
const char* VulkanDebugMessenger::GetLayerName()
{
    return VALIDATION_LAYER_NAME;
}

/// <summary>
/// メッセンジャーの生成情報を組み立てる.
/// VkInstanceCreateInfo の pNext へ繋ぐ用途にも使う.
/// </summary>
/// <returns>生成情報.</returns>
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

/// <summary>
/// メッセンジャーを生成する.
/// </summary>
/// <param name="_Instance">生成元のインスタンス. 所有はしない.</param>
/// <exception cref="std::runtime_error">生成に失敗した場合.</exception>
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

    // NOTE:
    // 結果コードを握りつぶすと失敗の理由が分からなくなるため、メッセージへ残す.
    const VkResult result = createFunction(m_Instance, &createInfo, nullptr, &m_Messenger);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(std::string{"デバッグメッセンジャーの生成に失敗しました: "} + string_VkResult(result));
    }
}

/// <summary>
/// 保持しているメッセンジャーを破棄する.
/// </summary>
VulkanDebugMessenger::~VulkanDebugMessenger()
{
    Destroy();
}

/// <summary>
/// 所有権を移してムーブ構築する.
/// </summary>
/// <param name="other">移動元. 呼び出し後は空になる.</param>
VulkanDebugMessenger::VulkanDebugMessenger(VulkanDebugMessenger&& other) noexcept
    : m_Instance{std::exchange(other.m_Instance, VkInstance{})},
      m_Messenger{std::exchange(other.m_Messenger, VK_NULL_HANDLE)}
{
}

/// <summary>
/// 自分の持つメッセンジャーを破棄してから、所有権を移す.
/// </summary>
/// <param name="other">移動元. 呼び出し後は空になる.</param>
/// <returns>自分自身.</returns>
VulkanDebugMessenger& VulkanDebugMessenger::operator=(VulkanDebugMessenger&& other) noexcept
{
    if (this != &other)
    {
        Destroy();

        m_Instance = std::exchange(other.m_Instance, VkInstance{});
        m_Messenger = std::exchange(other.m_Messenger, VK_NULL_HANDLE);
    }

    return *this;
}

/// <summary>
/// 保持しているハンドルを破棄する.
/// 破棄済み・未生成のいずれでも安全に呼べる.
/// </summary>
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
