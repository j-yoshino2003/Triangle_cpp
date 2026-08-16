//---------------------------------------
/// @file   main.cpp
///
/// @brief  Vulkan で三角形を描画するアプリケーションのエントリーポイント.
///
/// @date   2026/08/16
///
/// @author disaster
//---------------------------------------

#include <cstdlib>
#include <exception>
#include <iostream>

// NOTE:
// windows.h は min / max マクロなどで衝突を起こしやすいため、取り込む範囲を絞ってから読み込む.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "Application.h"

/// <summary>
/// アプリケーションのエントリーポイント.
/// </summary>
/// <returns>EXIT_SUCCESS = 正常終了、EXIT_FAILURE = 途中で失敗した.</returns>
int main()
{
    // NOTE:
    // コンパイラーに /utf-8 を指定しているため、文字列リテラルは UTF-8 で埋め込まれる.
    // コンソール側の既定は 932 (Shift_JIS) なので、出力コードページも UTF-8 に合わせる.
    // これを忘れると日本語が文字化けする.
    SetConsoleOutputCP(CP_UTF8);

    try
    {
        Application application{};

        application.Run();
    }
    catch (const std::exception& error)
    {
        std::cerr << "エラー: " << error.what() << std::endl;

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
