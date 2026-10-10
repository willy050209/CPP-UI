param(
    [string]$CompilerFilter = "",
    [string]$StandardFilter = "",
    [int]$MaxConcurrency = [Math]::Min(14, [Environment]::ProcessorCount)
)

$swTotal = [System.Diagnostics.Stopwatch]::StartNew()

# =============================================================================
# 0. 環境與編譯器偵測 (MSVC 環境單次快取注入)
# =============================================================================
$rootDir = (Resolve-Path "$PSScriptRoot\..").Path
$outDir  = "$rootDir\build\matrix_test"
$objBase = "$outDir\obj"
$binBase = "$outDir\bin"
$logBase = "$outDir\logs"

foreach ($dir in @($outDir, $objBase, $binBase, $logBase)) {
    if (-not (Test-Path $dir)) {
        New-Item -ItemType Directory -Path $dir -Force | Out-Null
    }
}

# 1. 動態查找 Clang++
$clangPath = if (Get-Command clang++.exe -ErrorAction SilentlyContinue) { (Get-Command clang++.exe).Source }
             elseif (Test-Path "C:\Program Files\LLVM\bin\clang++.exe") { "C:\Program Files\LLVM\bin\clang++.exe" }
             else { "clang++.exe" }
$hasClang = $false
try {
    & $clangPath --version *>$null
    if ($LASTEXITCODE -eq 0) { $hasClang = $true }
} catch { $hasClang = $false }

# 2. 動態查找 G++
$gccPath = if (Get-Command g++.exe -ErrorAction SilentlyContinue) { (Get-Command g++.exe).Source }
           elseif (Test-Path "C:\msys64\ucrt64\bin\g++.exe") { "C:\msys64\ucrt64\bin\g++.exe" }
           else { "g++.exe" }
$hasGcc = $false
try {
    & $gccPath --version *>$null
    if ($LASTEXITCODE -eq 0) { $hasGcc = $true }
} catch { $hasGcc = $false }

# 3. 動態查找 VsDevCmd.bat 並單次捕捉 MSVC 環境變數
$vsDevCmdCandidates = @(
    "C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\Tools\VsDevCmd.bat",
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat",
    "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat",
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat",
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsDevCmd.bat",
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"
)
$vsDevCmd = $vsDevCmdCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $vsDevCmd) {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $installPath = & $vswhere -latest -property installationPath
        if ($installPath -and (Test-Path "$installPath\Common7\Tools\VsDevCmd.bat")) {
            $vsDevCmd = "$installPath\Common7\Tools\VsDevCmd.bat"
        }
    }
}

$hasMsvc = $false
if ($vsDevCmd -and (Test-Path $vsDevCmd)) {
    $lines = & cmd.exe /c "`"$vsDevCmd`" -arch=x64 >nul && set"
    foreach ($l in $lines) {
        if ($l -match "^(PATH|INCLUDE|LIB|LIBPATH)=(.*)$") {
            [Environment]::SetEnvironmentVariable($matches[1], $matches[2], "Process")
        }
    }
    try {
        & cl.exe /nologo /? *>$null
        if ($LASTEXITCODE -eq 0) { $hasMsvc = $true }
    } catch { $hasMsvc = $false }
}

Write-Host "=========================================================================="
Write-Host " CPPUI Multi-Compiler Matrix Runner (Parallel RunspacePool Optimized)"
Write-Host "=========================================================================="
Write-Host "Compilers Detected: Clang++=$hasClang, G++=$hasGcc, MSVC=$hasMsvc"
Write-Host "Parallel Concurrency: $MaxConcurrency threads"
if ($CompilerFilter) { Write-Host "Compiler Filter: $CompilerFilter" }
if ($StandardFilter) { Write-Host "Standard Filter: $StandardFilter" }

# =============================================================================
# 1. 矩陣賽道 (Lanes) 定義
# =============================================================================
$allLanes = @()

if ($hasClang) {
    foreach ($s in @("c++14", "c++17", "c++20", "c++23")) {
        $allLanes += [PSCustomObject]@{
            Compiler     = "Clang++"
            CompilerType = "clang"
            CompilerPath = $clangPath
            Standard     = $s
            LaneId       = "clang_$s"
            ObjExt       = ".o"
        }
    }
}

if ($hasGcc) {
    foreach ($s in @("c++14", "c++17", "c++20", "c++23")) {
        $allLanes += [PSCustomObject]@{
            Compiler     = "G++"
            CompilerType = "gcc"
            CompilerPath = $gccPath
            Standard     = $s
            LaneId       = "gcc_$s"
            ObjExt       = ".o"
        }
    }
}

if ($hasMsvc) {
    foreach ($s in @("c++14", "c++17", "c++20", "c++latest")) {
        $allLanes += [PSCustomObject]@{
            Compiler     = "MSVC"
            CompilerType = "msvc"
            CompilerPath = "cl.exe"
            Standard     = $s
            LaneId       = "msvc_$s"
            ObjExt       = ".obj"
        }
    }
}

if ($CompilerFilter) {
    $allLanes = $allLanes | Where-Object { $_.Compiler -like "*$CompilerFilter*" }
}
if ($StandardFilter) {
    $allLanes = $allLanes | Where-Object { $_.Standard -like "*$StandardFilter*" }
}

if (@($allLanes).Count -eq 0) {
    Write-Warning "No compiler lanes matched current filters."
    exit 0
}

Write-Host "Active Matrix Lanes: $(@($allLanes).Count) lane(s)"

# =============================================================================
# 2. 框架共用原始碼清單 (21 個 TU) 與 測試目標定義
# =============================================================================
$frameworkSources = @(
    "src\core\Visual.cpp",
    "src\core\UIElement.cpp",
    "src\layout\Panel.cpp",
    "src\layout\StackPanel.cpp",
    "src\layout\Border.cpp",
    "src\layout\Canvas.cpp",
    "src\layout\Grid.cpp",
    "src\layout\LayoutManager.cpp",
    "src\controls\TextBlock.cpp",
    "src\controls\Control.cpp",
    "src\controls\ContentControl.cpp",
    "src\controls\Button.cpp",
    "src\controls\CheckBox.cpp",
    "src\controls\TextBox.cpp",
    "src\controls\ScrollViewer.cpp",
    "src\text\FontManager.cpp",
    "src\text\TextShaper.cpp",
    "src\text\BiDiAnalyzer.cpp",
    "src\text\LineBreaker.cpp",
    "src\text\TextLayout.cpp",
    "src\text\TextLayoutEngine.cpp",
    "src\text\DirectWriteTextEngine.cpp",
    "src\rendering\DisplayListRenderer.cpp",
    "src\rendering\backend\SoftwareRenderContext.cpp",
    "src\rendering\backend\SkiaRenderContext.cpp",
    "src\pal\PlatformWindowFactory.cpp",
    "src\pal\generic\PlatformWindowGeneric.cpp",
    "src\pal\win32\PlatformWindowWin32.cpp",
    "src\input\RoutedEvent.cpp",
    "src\input\EventRoute.cpp",
    "src\input\FocusManager.cpp",
    "src\input\InputManager.cpp",
    "src\ioc\ServiceCollection.cpp",
    "src\mvvm\ViewModelBase.cpp",
    "src\mvvm\RelayCommand.cpp",
    "src\app\Form.cpp",
    "src\app\Application.cpp",
    "src\app\AppBuilder.cpp",
    "src\app\Dispatcher.cpp",
    "src\app\Timer.cpp",
    "src\controls\PictureBox.cpp",
    "src\controls\RadioButton.cpp",
    "src\controls\ProgressBar.cpp",
    "src\controls\Slider.cpp",
    "src\controls\ListBox.cpp",
    "src\controls\Separator.cpp",
    "src\controls\Label.cpp",
    "src\graphics\Pen.cpp",
    "src\graphics\Pens.cpp",
    "src\graphics\Brush.cpp",
    "src\graphics\Brushes.cpp",
    "src\graphics\GraphicsPath.cpp",
    "src\graphics\Bitmap.cpp",
    "src\graphics\Graphics.cpp",
    "src\theme\Theme.cpp",
    "src\theme\ThemeManager.cpp",
    "src\dialogs\DialogCommon.cpp",
    "src\dialogs\MessageBox.cpp",
    "src\dialogs\MessageBoxForm.cpp",
    "src\dialogs\FileDialog.cpp",
    "src\dialogs\OpenFileDialog.cpp",
    "src\dialogs\SaveFileDialog.cpp",
    "src\dialogs\FileDialogForm.cpp",
    "src\dialogs\Win32DialogService.cpp"
)

$testTargets = @(
    @{
        Name           = "SmokeTest"
        Source         = "tests\smoke_test.cpp"
        IsSmoke        = $true
        NeedObjs       = @()
        MsvcLibs       = @()
        GnuLibs        = @()
        HasBench       = $false
        BenchRegex     = ""
    },
    @{
        Name           = "DisplayList"
        Source         = "tests\display_list_test.cpp"
        IsSmoke        = $false
        NeedObjs       = @("DisplayListRenderer", "SoftwareRenderContext", "Bitmap")
        MsvcLibs       = @("gdi32.lib", "user32.lib")
        GnuLibs        = @("-lgdi32", "-luser32")
        HasBench       = $true
        BenchRegex     = "50,000 Ops recorded in ([0-9.]+) ms"
    },
    @{
        Name           = "PAL_Backend"
        Source         = "tests\pal_test.cpp"
        IsSmoke        = $false
        NeedObjs       = @("SoftwareRenderContext")
        MsvcLibs       = @("gdi32.lib", "user32.lib")
        GnuLibs        = @("-lgdi32", "-luser32")
        HasBench       = $false
        BenchRegex     = ""
    },
    @{
        Name           = "TextPipeline"
        Source         = "tests\text_pipeline_test.cpp"
        IsSmoke        = $false
        NeedObjs       = @(
            "FontManager", "TextShaper", "BiDiAnalyzer", "LineBreaker",
            "TextLayout", "TextLayoutEngine", "DirectWriteTextEngine",
            "DisplayListRenderer", "SoftwareRenderContext", "Bitmap"
        )
        MsvcLibs       = @("dwrite.lib", "gdi32.lib", "user32.lib", "ole32.lib")
        GnuLibs        = @("-ldwrite", "-lgdi32", "-luser32", "-lole32")
        HasBench       = $false
        BenchRegex     = ""
    },
    @{
        Name           = "LayoutEngine"
        Source         = "tests\layout_engine_test.cpp"
        IsSmoke        = $false
        NeedObjs       = @(
            "Visual", "UIElement", "Panel", "StackPanel", "Border", "Canvas",
            "LayoutManager", "TextBlock", "FontManager", "TextShaper", "BiDiAnalyzer",
            "LineBreaker", "TextLayout", "TextLayoutEngine", "DirectWriteTextEngine",
            "DisplayListRenderer", "SoftwareRenderContext", "RoutedEvent", "EventRoute",
            "FocusManager", "InputManager", "Bitmap", "Theme", "ThemeManager"
        )
        MsvcLibs       = @("dwrite.lib", "gdi32.lib", "user32.lib", "ole32.lib")
        GnuLibs        = @("-ldwrite", "-lgdi32", "-luser32", "-lole32")
        HasBench       = $true
        BenchRegex     = "10,000 Elements Initial Layout: ([0-9.]+) ms"
    },
    @{
        Name           = "InputEvents"
        Source         = "tests\input_events_test.cpp"
        IsSmoke        = $false
        NeedObjs       = @(
            "Visual", "UIElement", "Panel", "StackPanel", "Border", "Canvas",
            "LayoutManager", "TextBlock", "FontManager", "TextShaper", "BiDiAnalyzer",
            "LineBreaker", "TextLayout", "TextLayoutEngine", "DirectWriteTextEngine",
            "DisplayListRenderer", "SoftwareRenderContext", "RoutedEvent", "EventRoute",
            "FocusManager", "InputManager", "Bitmap", "Theme", "ThemeManager"
        )
        MsvcLibs       = @("dwrite.lib", "gdi32.lib", "user32.lib", "ole32.lib")
        GnuLibs        = @("-ldwrite", "-lgdi32", "-luser32", "-lole32")
        HasBench       = $false
        BenchRegex     = ""
    },
    @{
        Name           = "CoordinateTransform"
        Source         = "tests\coordinate_transform_test.cpp"
        IsSmoke        = $true
        NeedObjs       = @()
        MsvcLibs       = @()
        GnuLibs        = @()
        HasBench       = $false
        BenchRegex     = ""
    },
    @{
        Name           = "Controls"
        Source         = "tests\controls_test.cpp"
        IsSmoke        = $false
        NeedObjs       = @(
            "Visual", "UIElement", "Panel", "StackPanel", "Border", "Canvas", "Grid",
            "LayoutManager", "TextBlock", "Control", "ContentControl", "Button", "CheckBox", "TextBox", "ScrollViewer",
            "PictureBox", "RadioButton", "ProgressBar", "Slider", "ListBox", "Separator", "Label",
            "FontManager", "TextShaper", "BiDiAnalyzer", "LineBreaker", "TextLayout", "TextLayoutEngine",
            "DirectWriteTextEngine", "DisplayListRenderer", "SoftwareRenderContext",
            "RoutedEvent", "EventRoute", "FocusManager", "InputManager",
            "Bitmap", "Graphics", "Pen", "Pens", "Brush", "Brushes", "GraphicsPath",
            "Theme", "ThemeManager"
        )
        MsvcLibs       = @("dwrite.lib", "gdi32.lib", "user32.lib", "ole32.lib")
        GnuLibs        = @("-ldwrite", "-lgdi32", "-luser32", "-lole32")
        HasBench       = $false
        BenchRegex     = ""
    },
    @{
        Name           = "FluentDSL"
        Source         = "tests\fluent_dsl_test.cpp"
        IsSmoke        = $false
        NeedObjs       = @(
            "Visual", "UIElement", "Panel", "StackPanel", "Border", "Canvas", "Grid",
            "LayoutManager", "TextBlock", "Control", "ContentControl", "Button", "CheckBox", "TextBox",
            "PictureBox", "RadioButton", "ProgressBar", "Slider", "ListBox", "Separator", "Label",
            "FontManager", "TextShaper", "BiDiAnalyzer", "LineBreaker", "TextLayout", "TextLayoutEngine",
            "DirectWriteTextEngine", "DisplayListRenderer", "SoftwareRenderContext",
            "RoutedEvent", "EventRoute", "FocusManager", "InputManager",
            "Bitmap", "Graphics", "Pen", "Pens", "Brush", "Brushes", "GraphicsPath",
            "Theme", "ThemeManager"
        )
        MsvcLibs       = @("dwrite.lib", "gdi32.lib", "user32.lib", "ole32.lib")
        GnuLibs        = @("-ldwrite", "-lgdi32", "-luser32", "-lole32")
        HasBench       = $false
        BenchRegex     = ""
    },
    @{
        Name           = "CrossPlatformPAL"
        Source         = "tests\pal_crossplatform_test.cpp"
        IsSmoke        = $false
        NeedObjs       = @(
            "PlatformWindowFactory", "PlatformWindowGeneric", "PlatformWindowWin32",
            "SoftwareRenderContext", "FontManager", "TextShaper", "BiDiAnalyzer",
            "LineBreaker", "TextLayout", "TextLayoutEngine", "DirectWriteTextEngine"
        )
        MsvcLibs       = @("dwrite.lib", "gdi32.lib", "user32.lib", "ole32.lib")
        GnuLibs        = @("-ldwrite", "-lgdi32", "-luser32", "-lole32")
        HasBench       = $false
        BenchRegex     = ""
    },
    @{
        Name           = "SkiaBackend"
        Source         = "tests\skia_backend_test.cpp"
        IsSmoke        = $false
        NeedObjs       = @("SkiaRenderContext", "SoftwareRenderContext", "DisplayListRenderer", "Bitmap")
        MsvcLibs       = @("gdi32.lib", "user32.lib")
        GnuLibs        = @("-lgdi32", "-luser32")
        HasBench       = $false
        BenchRegex     = ""
    },
    @{
        Name           = "IoCContainer"
        Source         = "tests\ioc_container_test.cpp"
        IsSmoke        = $false
        NeedObjs       = @("ServiceCollection")
        MsvcLibs       = @()
        GnuLibs        = @()
        HasBench       = $false
        BenchRegex     = ""
    },
    @{
        Name           = "MVVM"
        Source         = "tests\mvvm_test.cpp"
        IsSmoke        = $false
        NeedObjs       = @(
            "ViewModelBase", "RelayCommand", "Application", "Dispatcher", "Form", "AppBuilder",
            "PlatformWindowFactory", "PlatformWindowGeneric", "PlatformWindowWin32", "SkiaRenderContext",
            "ServiceCollection", "Button", "Control", "ContentControl",
            "UIElement", "Visual", "StackPanel", "Panel", "Border", "TextBlock",
            "DisplayListRenderer", "SoftwareRenderContext", "FontManager", "TextShaper",
            "BiDiAnalyzer", "LineBreaker", "TextLayout", "TextLayoutEngine",
            "DirectWriteTextEngine", "RoutedEvent", "EventRoute", "FocusManager", "InputManager", "LayoutManager",
            "Bitmap", "Graphics", "Pen", "Pens", "Brush", "Brushes", "GraphicsPath",
            "Theme", "ThemeManager"
        )
        MsvcLibs       = @("dwrite.lib", "gdi32.lib", "user32.lib", "ole32.lib")
        GnuLibs        = @("-ldwrite", "-lgdi32", "-luser32", "-lole32")
        HasBench       = $false
        BenchRegex     = ""
    },
    @{
        Name           = "AppForm"
        Source         = "tests\app_form_test.cpp"
        IsSmoke        = $false
        NeedObjs       = @(
            "Form", "Application", "AppBuilder", "Dispatcher", "SkiaRenderContext",
            "ServiceCollection", "PlatformWindowFactory",
            "PlatformWindowGeneric", "PlatformWindowWin32", "Button", "Control", "ContentControl",
            "UIElement", "Visual", "StackPanel", "Panel", "Border", "TextBlock",
            "DisplayListRenderer", "SoftwareRenderContext", "FontManager", "TextShaper",
            "BiDiAnalyzer", "LineBreaker", "TextLayout", "TextLayoutEngine",
            "DirectWriteTextEngine", "RoutedEvent", "EventRoute", "FocusManager", "InputManager", "LayoutManager",
            "Bitmap", "Graphics", "Pen", "Pens", "Brush", "Brushes", "GraphicsPath",
            "Theme", "ThemeManager"
        )
        MsvcLibs       = @("dwrite.lib", "gdi32.lib", "user32.lib", "ole32.lib")
        GnuLibs        = @("-ldwrite", "-lgdi32", "-luser32", "-lole32")
        HasBench       = $false
        BenchRegex     = ""
    },
    @{
        Name           = "GraphicsGdi"
        Source         = "tests\graphics_gdi_test.cpp"
        IsSmoke        = $false
        NeedObjs       = @(
            "Bitmap", "Graphics", "Pen", "Pens", "Brush", "Brushes", "GraphicsPath", "PictureBox",
            "Control", "UIElement", "Visual", "SoftwareRenderContext", "DisplayListRenderer",
            "FontManager", "TextShaper", "BiDiAnalyzer", "LineBreaker", "TextLayout", "TextLayoutEngine", "DirectWriteTextEngine",
            "RoutedEvent", "EventRoute", "FocusManager", "InputManager", "Theme", "ThemeManager"
        )
        MsvcLibs       = @("gdi32.lib", "user32.lib", "dwrite.lib", "ole32.lib")
        GnuLibs        = @("-lgdi32", "-luser32", "-ldwrite", "-lole32")
        HasBench       = $false
        BenchRegex     = ""
    },
    @{
        Name           = "ExampleFeatures"
        Source         = "tests\example_features_test.cpp"
        IsSmoke        = $false
        NeedObjs       = @(
            "Form", "Application", "AppBuilder", "Dispatcher", "SkiaRenderContext",
            "ServiceCollection", "PlatformWindowFactory",
            "PlatformWindowGeneric", "PlatformWindowWin32", "Button", "Control", "ContentControl",
            "UIElement", "Visual", "StackPanel", "Panel", "Border", "TextBlock",
            "DisplayListRenderer", "SoftwareRenderContext", "FontManager", "TextShaper",
            "BiDiAnalyzer", "LineBreaker", "TextLayout", "TextLayoutEngine",
            "DirectWriteTextEngine", "RoutedEvent", "EventRoute", "FocusManager", "InputManager", "LayoutManager",
            "Bitmap", "Graphics", "Pen", "Pens", "Brush", "Brushes", "GraphicsPath", "PictureBox",
            "Theme", "ThemeManager"
        )
        MsvcLibs       = @("dwrite.lib", "gdi32.lib", "user32.lib", "ole32.lib")
        GnuLibs        = @("-ldwrite", "-lgdi32", "-luser32", "-lole32")
        HasBench       = $false
        BenchRegex     = ""
    },
    @{
        Name           = "ControlClass"
        Source         = "tests\control_class_test.cpp"
        IsSmoke        = $false
        NeedObjs       = @(
            "Control", "ContentControl", "Button", "TextBox", "TextBlock", "Form", "Application", "Dispatcher",
            "AppBuilder", "ServiceCollection", "SkiaRenderContext",
            "UIElement", "Visual", "StackPanel", "Panel", "Border", "LayoutManager",
            "DisplayListRenderer", "SoftwareRenderContext", "FontManager", "TextShaper",
            "BiDiAnalyzer", "LineBreaker", "TextLayout", "TextLayoutEngine", "DirectWriteTextEngine",
            "PlatformWindowFactory", "PlatformWindowGeneric", "PlatformWindowWin32",
            "Bitmap", "Graphics", "Pen", "Pens", "Brush", "Brushes", "GraphicsPath",
            "RoutedEvent", "EventRoute", "FocusManager", "InputManager",
            "Theme", "ThemeManager"
        )
        MsvcLibs       = @("dwrite.lib", "gdi32.lib", "user32.lib", "ole32.lib")
        GnuLibs        = @("-ldwrite", "-lgdi32", "-luser32", "-lole32")
        HasBench       = $false
        BenchRegex     = ""
    },
    @{
        Name           = "Theme"
        Source         = "tests\theme_test.cpp"
        IsSmoke        = $false
        NeedObjs       = @(
            "Theme", "ThemeManager", "Control", "ContentControl", "Button", "TextBox", "CheckBox", "TextBlock",
            "UIElement", "Visual", "FontManager", "DirectWriteTextEngine",
            "RoutedEvent", "EventRoute", "FocusManager", "InputManager",
            "DisplayListRenderer", "SoftwareRenderContext", "TextLayout", "TextLayoutEngine",
            "TextShaper", "BiDiAnalyzer", "LineBreaker", "Bitmap",
            "Graphics", "Pen", "Pens", "Brush", "Brushes", "GraphicsPath"
        )
        MsvcLibs       = @("dwrite.lib", "gdi32.lib", "user32.lib", "ole32.lib")
        GnuLibs        = @("-ldwrite", "-lgdi32", "-luser32", "-lole32")
        HasBench       = $false
        BenchRegex     = ""
    },
    @{
        Name           = "Dialogs"
        Source         = "tests\dialogs_test.cpp"
        IsSmoke        = $false
        NeedObjs       = @(
            "DialogCommon", "MessageBox", "MessageBoxForm", "FileDialog",
            "OpenFileDialog", "SaveFileDialog", "FileDialogForm", "Win32DialogService",
            "Visual", "UIElement", "Panel", "StackPanel", "Border", "Canvas", "Grid",
            "LayoutManager", "TextBlock", "Control", "ContentControl", "Button", "CheckBox", "TextBox",
            "ScrollViewer", "Form", "Application", "AppBuilder", "Dispatcher",
            "FontManager", "TextShaper", "BiDiAnalyzer", "LineBreaker", "TextLayout", "TextLayoutEngine",
            "DirectWriteTextEngine", "DisplayListRenderer", "SoftwareRenderContext", "SkiaRenderContext",
            "PlatformWindowFactory", "PlatformWindowGeneric", "PlatformWindowWin32",
            "RoutedEvent", "EventRoute", "FocusManager", "InputManager", "ServiceCollection",
            "Bitmap", "Graphics", "Pen", "Pens", "Brush", "Brushes", "GraphicsPath", "PictureBox",
            "Theme", "ThemeManager"
        )
        MsvcLibs       = @("dwrite.lib", "gdi32.lib", "user32.lib", "ole32.lib", "comdlg32.lib", "shell32.lib")
        GnuLibs        = @("-ldwrite", "-lgdi32", "-luser32", "-lole32", "-lcomdlg32", "-lshell32")
        HasBench       = $false
        BenchRegex     = ""
    },
    @{
        Name           = "Timer"
        Source         = "tests\timer_test.cpp"
        IsSmoke        = $false
        NeedObjs       = @(
            "Timer", "Form", "Application", "AppBuilder", "Dispatcher", "SkiaRenderContext",
            "ServiceCollection", "PlatformWindowFactory",
            "PlatformWindowGeneric", "PlatformWindowWin32", "Button", "Control", "ContentControl",
            "UIElement", "Visual", "StackPanel", "Panel", "Border", "TextBlock",
            "DisplayListRenderer", "SoftwareRenderContext", "FontManager", "TextShaper",
            "BiDiAnalyzer", "LineBreaker", "TextLayout", "TextLayoutEngine",
            "DirectWriteTextEngine", "RoutedEvent", "EventRoute", "FocusManager", "InputManager", "LayoutManager",
            "Bitmap", "Graphics", "Pen", "Pens", "Brush", "Brushes", "GraphicsPath",
            "Theme", "ThemeManager"
        )
        MsvcLibs       = @("dwrite.lib", "gdi32.lib", "user32.lib", "ole32.lib")
        GnuLibs        = @("-ldwrite", "-lgdi32", "-luser32", "-lole32")
        HasBench       = $false
        BenchRegex     = ""
    }
)

# 建立每條 Lane 的目錄
foreach ($lane in $allLanes) {
    $laneObjDir = "$objBase\$($lane.LaneId)"
    $laneBinDir = "$binBase\$($lane.LaneId)"
    $laneLogDir = "$logBase\$($lane.LaneId)"
    foreach ($d in @($laneObjDir, $laneBinDir, $laneLogDir)) {
        if (-not (Test-Path $d)) { New-Item -ItemType Directory -Path $d -Force | Out-Null }
    }
}

# =============================================================================
# 3. Phase 1: 平行編譯框架共用物件檔 (Shared Objects Precompilation)
# =============================================================================
Write-Host "`n[Phase 1] Precompiling shared framework objects in parallel across $MaxConcurrency threads..."
$swPhase1 = [System.Diagnostics.Stopwatch]::StartNew()

# 準備 Phase 1 任務清單
# MSVC: 每個 Lane 一個 /MP 任務
# Clang/GCC: 每個 Lane 每個檔案一個任務 (完全飽和多核心)
$phase1Tasks = @()

foreach ($lane in $allLanes) {
    $laneObjDir = "$objBase\$($lane.LaneId)"
    $laneLogDir = "$logBase\$($lane.LaneId)"

    if ($lane.CompilerType -eq "msvc") {
        $phase1Tasks += [PSCustomObject]@{
            Type         = "msvc_batch"
            LaneId       = $lane.LaneId
            CompilerPath = $lane.CompilerPath
            Standard     = $lane.Standard
            ObjDir       = $laneObjDir
            LogFile      = "$laneLogDir\shared_objects.log"
            Sources      = $frameworkSources
        }
    } else {
        foreach ($src in $frameworkSources) {
            $baseName = [System.IO.Path]::GetFileNameWithoutExtension($src)
            $objFile  = "$laneObjDir\$baseName$($lane.ObjExt)"
            $phase1Tasks += [PSCustomObject]@{
                Type         = $lane.CompilerType
                LaneId       = $lane.LaneId
                CompilerPath = $lane.CompilerPath
                Standard     = $lane.Standard
                SrcRel       = $src
                BaseName     = $baseName
                ObjFile      = $objFile
                LogFile      = "$laneLogDir\obj_$baseName.log"
            }
        }
    }
}

$sbPhase1 = {
    param($task, $rootPath)
    if ($task.Type -eq "msvc_batch") {
        $absSources = $task.Sources | ForEach-Object { "$rootPath\$_" }
        $fwdObjDir  = "$($task.ObjDir)/".Replace("\", "/")
        & $task.CompilerPath /nologo /c /MP /O2 /EHsc /utf-8 "/std:$($task.Standard)" `
            /I "$rootPath\include" /I "$rootPath\include\compat" "/Fo$fwdObjDir" @absSources *>$task.LogFile
        return ($LASTEXITCODE -eq 0)
    } elseif ($task.Type -eq "clang") {
        $srcAbs = "$rootPath\$($task.SrcRel)"
        & $task.CompilerPath -c "-std=$($task.Standard)" -O2 `
            -I "$rootPath\include" -I "$rootPath\include\compat" $srcAbs -o $task.ObjFile *>$task.LogFile
        return ($LASTEXITCODE -eq 0)
    } elseif ($task.Type -eq "gcc") {
        $srcAbs = "$rootPath\$($task.SrcRel)"
        & $task.CompilerPath -c "-std=$($task.Standard)" -O2 `
            -I "$rootPath\include" -I "$rootPath\include\compat" $srcAbs -o $task.ObjFile *>$task.LogFile
        return ($LASTEXITCODE -eq 0)
    }
    return $false
}

$pool1 = [runspacefactory]::CreateRunspacePool(1, $MaxConcurrency)
$pool1.Open()

$runningHandles1 = @()
foreach ($task in $phase1Tasks) {
    $ps = [powershell]::Create().AddScript($sbPhase1).AddArgument($task).AddArgument($rootDir)
    $ps.RunspacePool = $pool1
    $runningHandles1 += [PSCustomObject]@{
        Pipe   = $ps
        Handle = $ps.BeginInvoke()
        Task   = $task
    }
}

$p1Failed = 0
$p1Completed = 0
$totalP1 = $runningHandles1.Count

foreach ($h in $runningHandles1) {
    $res = $h.Pipe.EndInvoke($h.Handle)
    if (-not $res) {
        $p1Failed++
        Write-Warning "Failed compiling object for $($h.Task.LaneId): $($h.Task.LogFile)"
    }
    $p1Completed++
    $h.Pipe.Dispose()
}

$pool1.Close()
$swPhase1.Stop()

Write-Host "[Phase 1 Complete] $totalP1 object tasks finished in $([Math]::Round($swPhase1.ElapsedMilliseconds / 1000, 2))s (Failed: $p1Failed)"

if ($p1Failed -gt 0) {
    Write-Error "Phase 1 compilation encountered $p1Failed failures! Aborting matrix test."
    exit 1
}

# =============================================================================
# 4. Phase 2: 平行鏈接與執行測試目標 (Parallel Link & Run 72 Tests)
# =============================================================================
Write-Host "`n[Phase 2] Linking and running test targets in parallel across $MaxConcurrency threads..."
$swPhase2 = [System.Diagnostics.Stopwatch]::StartNew()

$phase2Tasks = @()
foreach ($lane in $allLanes) {
    $laneObjDir = "$objBase\$($lane.LaneId)"
    $laneBinDir = "$binBase\$($lane.LaneId)"
    $laneLogDir = "$logBase\$($lane.LaneId)"

    foreach ($target in $testTargets) {
        $testExe = "$laneBinDir\$($target.Name).exe"
        $phase2Tasks += [PSCustomObject]@{
            LaneId       = $lane.LaneId
            Compiler     = $lane.Compiler
            CompilerType = $lane.CompilerType
            CompilerPath = $lane.CompilerPath
            Standard     = $lane.Standard
            ObjExt       = $lane.ObjExt
            TargetName   = $target.Name
            TargetSrc    = $target.Source
            IsSmoke      = $target.IsSmoke
            NeedObjs     = $target.NeedObjs
            MsvcLibs     = $target.MsvcLibs
            GnuLibs      = $target.GnuLibs
            HasBench     = $target.HasBench
            BenchRegex   = $target.BenchRegex
            LaneObjDir   = $laneObjDir
            TestExe      = $testExe
            CompileLog   = "$laneLogDir\$($target.Name)_compile.log"
            RunLog       = "$laneLogDir\$($target.Name)_run.log"
        }
    }
}

$sbPhase2 = {
    param($task, $rootPath)
    $objExt     = $task.ObjExt
    $laneObjDir = $task.LaneObjDir
    $reqObjFiles = @($task.NeedObjs | ForEach-Object { "$laneObjDir\$_$objExt" })
    $msvcLibs    = @($task.MsvcLibs)
    $gnuLibs     = @($task.GnuLibs)
    $testSrcAbs  = "$rootPath\$($task.TargetSrc)"
    $testExe     = $task.TestExe
    $fwdObjDir   = "$laneObjDir/".Replace("\", "/")

    $compileOk = $false
    if ($task.CompilerType -eq "msvc") {
        if ($task.IsSmoke) {
            & $task.CompilerPath /nologo /EHsc /utf-8 "/std:$($task.Standard)" `
                /I "$rootPath\include" /I "$rootPath\include\compat" `
                $testSrcAbs "/Fe:$testExe" "/Fo$fwdObjDir" *>$task.CompileLog
        } else {
            & $task.CompilerPath /nologo /O2 /EHsc /utf-8 "/std:$($task.Standard)" `
                /I "$rootPath\include" /I "$rootPath\include\compat" `
                $testSrcAbs $reqObjFiles $msvcLibs "/Fe:$testExe" "/Fo$fwdObjDir" *>$task.CompileLog
        }
        $compileOk = ($LASTEXITCODE -eq 0 -and (Test-Path $testExe))
    } elseif ($task.CompilerType -eq "clang") {
        if ($task.IsSmoke) {
            & $task.CompilerPath "-std=$($task.Standard)" `
                -I "$rootPath\include" -I "$rootPath\include\compat" `
                $testSrcAbs -o $testExe *>$task.CompileLog
        } else {
            & $task.CompilerPath "-std=$($task.Standard)" -O2 `
                -I "$rootPath\include" -I "$rootPath\include\compat" `
                $testSrcAbs $reqObjFiles $gnuLibs -o $testExe *>$task.CompileLog
        }
        $compileOk = ($LASTEXITCODE -eq 0 -and (Test-Path $testExe))
    } elseif ($task.CompilerType -eq "gcc") {
        if ($task.IsSmoke) {
            & $task.CompilerPath "-std=$($task.Standard)" -static `
                -I "$rootPath\include" -I "$rootPath\include\compat" `
                $testSrcAbs -o $testExe *>$task.CompileLog
        } else {
            & $task.CompilerPath "-std=$($task.Standard)" -O2 -static `
                -I "$rootPath\include" -I "$rootPath\include\compat" `
                $testSrcAbs $reqObjFiles $gnuLibs -o $testExe *>$task.CompileLog
        }
        $compileOk = ($LASTEXITCODE -eq 0 -and (Test-Path $testExe))
    }

    if (-not $compileOk) {
        return [PSCustomObject]@{
            Compiler  = $task.Compiler
            Standard  = $task.Standard
            Target    = $task.TargetName
            Compile   = "FAIL"
            Run       = "N/A"
            Benchmark = "N/A"
        }
    }

    # 執行測試程式
    $runOut = & $testExe 2>&1
    $runOk = ($LASTEXITCODE -eq 0)
    $runStatus = if ($runOk) { "PASS" } else { "FAIL" }

    $benchVal = "N/A"
    if ($runOk -and $task.HasBench) {
        $outStr = ($runOut -join "`n")
        if ($outStr -match $task.BenchRegex) {
            $benchVal = "$($matches[1]) ms"
        } else {
            $benchVal = "PASS"
        }
    }

    return [PSCustomObject]@{
        Compiler  = $task.Compiler
        Standard  = $task.Standard
        Target    = $task.TargetName
        Compile   = "PASS"
        Run       = $runStatus
        Benchmark = $benchVal
    }
}

$pool2 = [runspacefactory]::CreateRunspacePool(1, $MaxConcurrency)
$pool2.Open()

$runningHandles2 = @()
foreach ($task in $phase2Tasks) {
    $ps = [powershell]::Create().AddScript($sbPhase2).AddArgument($task).AddArgument($rootDir)
    $ps.RunspacePool = $pool2
    $runningHandles2 += [PSCustomObject]@{
        Pipe   = $ps
        Handle = $ps.BeginInvoke()
        Task   = $task
    }
}

$rawResults = @()
foreach ($h in $runningHandles2) {
    $item = $h.Pipe.EndInvoke($h.Handle)
    $rawResults += $item
    $h.Pipe.Dispose()
}

$pool2.Close()
$swPhase2.Stop()
Write-Host "[Phase 2 Complete] $($rawResults.Count) tests compiled and executed in $([Math]::Round($swPhase2.ElapsedMilliseconds / 1000, 2))s"

# =============================================================================
# 5. 格式化輸出報告 (保證排序與原生規格 100% 一致)
# =============================================================================
$compilerOrder = @{ "Clang++" = 1; "G++" = 2; "MSVC" = 3 }
$standardOrder = @{ "c++14" = 1; "c++17" = 2; "c++20" = 3; "c++23" = 4; "c++latest" = 5 }
$targetOrder   = @{
    "SmokeTest"    = 1
    "DisplayList"  = 2
    "PAL_Backend"  = 3
    "TextPipeline" = 4
    "LayoutEngine" = 5
    "InputEvents"  = 6
}

$sortedResults = $rawResults | Sort-Object {
    $cVal = if ($compilerOrder.ContainsKey($_.Compiler)) { $compilerOrder[$_.Compiler] } else { 99 }
    $sVal = if ($standardOrder.ContainsKey($_.Standard)) { $standardOrder[$_.Standard] } else { 99 }
    $tVal = if ($targetOrder.ContainsKey($_.Target))     { $targetOrder[$_.Target] }     else { 99 }
    "$cVal-$sVal-$tVal"
}

$swTotal.Stop()
$totalSec = [Math]::Round($swTotal.ElapsedMilliseconds / 1000, 2)

Write-Host "`n=========================================================================="
Write-Host " CPPUI Multi-Compiler x Multi-Standard Cross-Verification Matrix"
Write-Host "=========================================================================="
$sortedResults | Format-Table -AutoSize

$failures = @($sortedResults | Where-Object { $_.Compile -ne "PASS" -or $_.Run -ne "PASS" })
if ($failures.Count -gt 0) {
    Write-Error "$($failures.Count) test(s) failed in matrix suite!"
    exit 1
} else {
    Write-Host "`nAll $(@($sortedResults).Count) tests PASSED successfully in $totalSec seconds!" -ForegroundColor Green
}
