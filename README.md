# Triangle_cpp

Vulkan で三角形を描画する C++ 実装。Rust 実装は同リポジトリ群の `rust/` 側にある。

## 前提環境

| 対象 | バージョン | 備考 |
|------|------|------|
| Visual Studio Community | 18 | PlatformToolset **v145** / C++20 (`stdcpp20`) |
| Vulkan SDK | 1.4.357.0 | `C:\VulkanSDK\1.4.357.0` |
| GLFW | 3.4.0 | NuGet パッケージ（`packages.config` で管理） |

構成は `Debug`・`Release` × `x64` の 2 通り。32 bit（`Win32`）は用意していない。
Vulkan SDK の 32 bit ライブラリが導入されておらず、対象環境も 64 bit だけのため。

## 環境構築手順

### 1. Vulkan SDK を入れる

```bash
winget install -e --id KhronosGroup.VulkanSDK --source winget
```

インストーラーが以下を自動で設定する（確認済み）。

- 環境変数 `VULKAN_SDK` = `C:\VulkanSDK\1.4.357.0`（`VK_SDK_PATH` も同じ値）
- システムの `Path` に `C:\VulkanSDK\1.4.357.0\Bin` を追加（`glslc` / `glslangValidator` が使えるようになる）

> **注意**: 環境変数はプロセス起動時に読み込まれる。インストール前から開いていたターミナルや
> Visual Studio では `VULKAN_SDK` が空のままなので、**開き直す**こと。

入ったかどうかは次で確認する。

```bash
echo $VULKAN_SDK && glslc --version
```

### 2. GLFW を NuGet で入れる

Visual Studio で `cpp.slnx` を開き、[NuGet パッケージの管理] から `glfw` 3.4.0 をインストールする。
結果として次が作られる・書き換わる。

- `packages.config` … 依存の記録
- `packages/glfw.3.4.0/` … パッケージ本体
- `cpp.vcxproj` … `glfw.targets` の `<Import>` と、復元漏れを検出する `EnsureNuGetPackageBuildImports` ターゲット

`glfw.targets` が include パス・リンクするライブラリ・DLL のコピーまで面倒を見るため、
**プロジェクト側で GLFW のパス設定を書く必要はない。**

`packages/` は `.gitignore` で追跡対象外にしてある。クローン直後は中身が無いので、
Visual Studio でビルドすれば `packages.config` から自動で復元される。復元されていない場合は
`EnsureNuGetPackageBuildImports` ターゲットが「NuGet パッケージが見つからない」という
エラーで止めてくれる。

> **v145 と GLFW の関係**: このパッケージが同梱するライブラリは v120 / v140 / v141 / v142 / v143 まで。
> v145 用は無いが、`glfw.targets` が「PlatformToolsetVersion が 143 以上 150 未満なら v143 を使う」と
> 判定するため、v145 でもそのまま通る。既定は動的リンクで、ビルド後に `glfw3.dll` が出力先へ自動コピーされる。

### 3. Vulkan をプロジェクトから参照する

GLFW と違い Vulkan SDK は NuGet ではないので、`cpp.vcxproj` に参照設定を書いてある。
全構成に一括で効くよう、条件なしの `ItemDefinitionGroup` を 1 つ置いている。

```xml
<ItemDefinitionGroup>
  <ClCompile>
    <AdditionalIncludeDirectories>$(ProjectDir)src;$(VULKAN_SDK)\Include;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
  </ClCompile>
  <Link>
    <AdditionalDependencies>$(VULKAN_SDK)\Lib\vulkan-1.lib;%(AdditionalDependencies)</AdditionalDependencies>
  </Link>
</ItemDefinitionGroup>
```

## ビルドと実行

Visual Studio で `cpp.slnx` を開いてビルドするか、`cpp/` で次を実行する。

```bash
"C:/Program Files/Microsoft Visual Studio/18/Community/MSBuild/Current/Bin/MSBuild.exe" cpp.vcxproj -p:Configuration=Debug -p:Platform=x64
```

出力は `x64/Debug/` に `cpp.exe`・`cpp.pdb`・`glfw3.dll`（NuGet の targets が自動コピー）。
中間ファイルは `x64/Debug/obj/`。

> `OutDir` / `IntDir` は `cpp.vcxproj` で明示している。既定のままだと `IntDir` に `$(ProjectName)` が
> 含まれ、中間ファイルが `cpp/cpp/x64/Debug/` という紛らわしい入れ子に出るため。

```bash
./x64/Debug/cpp.exe
```

環境が正しければ、コンソールに次の 2 行が出てウィンドウが開く。

```
Vulkan インスタンスを生成しました.
バリデーション: 有効
```

Debug ビルドではバリデーションレイヤーが有効になり、Vulkan の使い方に誤りがあると
標準エラーへ `[Vulkan] ...` の形で報告される。

## ファイル構成

ソースはすべて `src/` 以下に置き、**役割の層ごとにフォルダーを分ける**。

```
src/
├── main.cpp              エントリーポイント。文字コード設定と例外の受け止めだけ
├── Application.h/.cpp    資源をまとめて持ち、ウィンドウが閉じられるまで回し続ける
├── Platform/             OS・ウィンドウの層（GLFW）
│   ├── GlfwContext.h/.cpp   GLFW の初期化状態（glfwInit / glfwTerminate）
│   └── GlfwWindow.h/.cpp    ウィンドウ（glfwCreateWindow / glfwDestroyWindow）
└── Vulkan/               描画 API の層
    ├── VulkanInstance.h/.cpp        VkInstance
    └── VulkanDebugMessenger.h/.cpp  VkDebugUtilsMessengerEXT とバリデーションの判断
```

1 つのラッパークラスが 1 つの資源だけを持ち、破棄はデストラクターに任せる（Rule of 5）。
利用側の `Application` は資源をメンバーとして並べるだけで済む（Rule of 0）。

`Application` のメンバーは**宣言順に生成され、逆順に破棄される**。依存される側を前に、
依存する側を後ろに並べておけば、破棄の順序は自動的に正しくなる。

### インクルードは src からのパスで書く

`cpp.vcxproj` の `AdditionalIncludeDirectories` に `$(ProjectDir)src` を入れてあるため、
どのファイルからでも同じ書き方になる。**同じフォルダーのヘッダーでも相対では書かない。**

```cpp
// ✅ src からのパス. どこから include しても同じ表記になる.
#include "Platform/GlfwContext.h"
#include "Vulkan/VulkanInstance.h"

// ❌ 相対パス. ファイルを移すたびに書き換えが要る.
#include "GlfwContext.h"
#include "../Vulkan/VulkanInstance.h"
```

## 開発時の注意

### ソースを追加したら 2 か所に登録する

ファイルを置くだけではビルド対象にならない。**パスは `src\` から書く。**

- `cpp.vcxproj` … `<ItemGroup>` に `<ClCompile Include="src\Vulkan\Foo.cpp" />` /
  `<ClInclude Include="src\Vulkan\Foo.h" />`
- `cpp.vcxproj.filters` … 同じファイルを、フォルダーに対応するフィルター
  （`ソース ファイル\Vulkan` など）へ

Visual Studio 上で追加すれば両方とも自動で更新される。新しいフォルダーを作った場合は、
`.filters` に `<Filter Include="ソース ファイル\<名前>">` を GUID 付きで足す。

### 日本語をコンソールに出す

`std::cout` は日本語を出せる。文字化けするかどうかは、**文字列リテラルが実行ファイルに埋め込まれる
ときの文字コード**と、**コンソールの出力コードページ**が一致しているかで決まる。この 2 つを
UTF-8 に揃えてある。

| 設定箇所 | 内容 |
|------|------|
| `cpp.vcxproj` | `AdditionalOptions` に `/utf-8`。ソースと実行時の文字コードを UTF-8 にする |
| `main.cpp` | 起動直後に `SetConsoleOutputCP(CP_UTF8)`。コンソール側も UTF-8 にする |

`/utf-8` を付けないと、MSVC は文字列リテラルをシステムの ANSI コードページ（日本語環境では CP932）へ
変換して埋め込む。コンソールの既定も CP932 なので cmd.exe では正しく表示されるが、Git Bash や
VS Code のように UTF-8 で表示する端末では文字化けする。両方を UTF-8 に固定すればどこでも正しく出る。

`windows.h` は `WIN32_LEAN_AND_MEAN` と `NOMINMAX` を定義してから読み込み、`min` / `max` マクロが
Vulkan・GLFW のコードと衝突しないようにしている。

### Zed で編集する

補完・定義ジャンプが使えるよう、clangd 用の設定を `.clangd` に置いてある。

- clangd 本体は **Zed が自動でインストールする**（`PATH` に `clangd` があればそちらを優先）
- `.clangd` にインクルードパスと `-std=c++20` を書いてある。これは**エディタの解析専用**で、
  ビルドには一切影響しない
- **`.vcxproj` にインクルードパスやマクロを足したら、`.clangd` にも同じものを足すこと**

整形は `.clang-format`（インデント 4 スペース / 120 桁 / Allman）。`.zed/settings.json` で保存時整形を
有効にしてあるので、保存すれば自動で適用される。手動で掛けるなら次を実行する。

```bash
"C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin/clang-format.exe" -i src/main.cpp
```
