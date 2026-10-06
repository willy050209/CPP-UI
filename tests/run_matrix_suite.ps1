$clangPath = "C:\Program Files\LLVM\bin\clang++.exe"
$gccPath   = "C:\msys64\ucrt64\bin\g++.exe"
$vsDevCmd  = "C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\Tools\VsDevCmd.bat"
$outDir    = "build\matrix_test"

if (-not (Test-Path $outDir)) {
    New-Item -ItemType Directory -Path $outDir -Force | Out-Null
}

$results = @()

function Record-Result($compiler, $std, $target, $compileStatus, $runStatus, $benchmark) {
    $script:results += [PSCustomObject]@{
        Compiler = $compiler
        Standard = $std
        Target   = $target
        Compile  = $compileStatus
        Run      = $runStatus
        Benchmark= $benchmark
    }
}

# =============================================================================
# 1. Clang++ 測試矩陣 (C++14, C++17, C++20, C++23)
# =============================================================================
$clangStandards = @("c++14", "c++17", "c++20", "c++23")
foreach ($s in $clangStandards) {
    # 1. Smoke Test
    $smokeExe = "$outDir\smoke_clang_$s.exe"
    & $clangPath "-std=$s" -I include -I include/compat tests\smoke_test.cpp -o $smokeExe
    if ($LASTEXITCODE -eq 0) {
        & $smokeExe | Out-Null
        $runOk = if ($LASTEXITCODE -eq 0) { "PASS" } else { "FAIL" }
        Record-Result "Clang++" $s "SmokeTest" "PASS" $runOk "N/A"
    } else {
        Record-Result "Clang++" $s "SmokeTest" "FAIL" "N/A" "N/A"
    }

    # 2. DisplayList Test
    $dlExe = "$outDir\dl_clang_$s.exe"
    & $clangPath "-std=$s" -O2 -I include -I include/compat tests\display_list_test.cpp src\rendering\DisplayListRenderer.cpp src\rendering\backend\SoftwareRenderContext.cpp -lgdi32 -luser32 -o $dlExe
    if ($LASTEXITCODE -eq 0) {
        $output = & $dlExe
        $runOk = if ($LASTEXITCODE -eq 0) { "PASS" } else { "FAIL" }
        $match = [regex]::Match($output, "50,000 Ops recorded in ([0-9.]+) ms")
        $timeStr = if ($match.Success) { "$($match.Groups[1].Value) ms" } else { "PASS" }
        Record-Result "Clang++" $s "DisplayList" "PASS" $runOk $timeStr
    } else {
        Record-Result "Clang++" $s "DisplayList" "FAIL" "N/A" "N/A"
    }

    # 3. PAL & Backend Test
    $palExe = "$outDir\pal_clang_$s.exe"
    & $clangPath "-std=$s" -O2 -I include -I include/compat tests\pal_test.cpp src\rendering\backend\SoftwareRenderContext.cpp -lgdi32 -luser32 -o $palExe
    if ($LASTEXITCODE -eq 0) {
        & $palExe | Out-Null
        $runOk = if ($LASTEXITCODE -eq 0) { "PASS" } else { "FAIL" }
        Record-Result "Clang++" $s "PAL_Backend" "PASS" $runOk "N/A"
    } else {
        Record-Result "Clang++" $s "PAL_Backend" "FAIL" "N/A" "N/A"
    }
}

# =============================================================================
# 2. G++ (MinGW UCRT64) 測試矩陣 (C++14, C++17, C++20, C++23)
# =============================================================================
$gccStandards = @("c++14", "c++17", "c++20", "c++23")
foreach ($s in $gccStandards) {
    # 1. Smoke Test
    $smokeExe = "$outDir\smoke_gcc_$s.exe"
    & $gccPath "-std=$s" -static -I include -I include/compat tests\smoke_test.cpp -o $smokeExe
    if ($LASTEXITCODE -eq 0) {
        & $smokeExe | Out-Null
        $runOk = if ($LASTEXITCODE -eq 0) { "PASS" } else { "FAIL" }
        Record-Result "G++" $s "SmokeTest" "PASS" $runOk "N/A"
    } else {
        Record-Result "G++" $s "SmokeTest" "FAIL" "N/A" "N/A"
    }

    # 2. DisplayList Test
    $dlExe = "$outDir\dl_gcc_$s.exe"
    & $gccPath "-std=$s" -O2 -static -I include -I include/compat tests\display_list_test.cpp src\rendering\DisplayListRenderer.cpp src\rendering\backend\SoftwareRenderContext.cpp -lgdi32 -luser32 -o $dlExe
    if ($LASTEXITCODE -eq 0) {
        $output = & $dlExe
        $runOk = if ($LASTEXITCODE -eq 0) { "PASS" } else { "FAIL" }
        $match = [regex]::Match($output, "50,000 Ops recorded in ([0-9.]+) ms")
        $timeStr = if ($match.Success) { "$($match.Groups[1].Value) ms" } else { "PASS" }
        Record-Result "G++" $s "DisplayList" "PASS" $runOk $timeStr
    } else {
        Record-Result "G++" $s "DisplayList" "FAIL" "N/A" "N/A"
    }

    # 3. PAL & Backend Test
    $palExe = "$outDir\pal_gcc_$s.exe"
    & $gccPath "-std=$s" -O2 -static -I include -I include/compat tests\pal_test.cpp src\rendering\backend\SoftwareRenderContext.cpp -lgdi32 -luser32 -o $palExe
    if ($LASTEXITCODE -eq 0) {
        & $palExe | Out-Null
        $runOk = if ($LASTEXITCODE -eq 0) { "PASS" } else { "FAIL" }
        Record-Result "G++" $s "PAL_Backend" "PASS" $runOk "N/A"
    } else {
        Record-Result "G++" $s "PAL_Backend" "FAIL" "N/A" "N/A"
    }
}

# =============================================================================
# 3. MSVC (cl.exe) 測試矩陣 (c++14, c++17, c++20, c++latest)
# =============================================================================
$msvcStandards = @("c++14", "c++17", "c++20", "c++latest")
foreach ($s in $msvcStandards) {
    # 1. Smoke Test
    $smokeExe = "$outDir\smoke_msvc_$s.exe"
    $cmd = "`"$vsDevCmd`" -arch=x64 >nul && cl.exe /nologo /EHsc /utf-8 /std:$s /I include /I include/compat tests\smoke_test.cpp /Fe:$smokeExe >nul"
    cmd.exe /c $cmd
    if ($LASTEXITCODE -eq 0 -and (Test-Path $smokeExe)) {
        & $smokeExe | Out-Null
        $runOk = if ($LASTEXITCODE -eq 0) { "PASS" } else { "FAIL" }
        Record-Result "MSVC" $s "SmokeTest" "PASS" $runOk "N/A"
    } else {
        Record-Result "MSVC" $s "SmokeTest" "FAIL" "N/A" "N/A"
    }

    # 2. DisplayList Test
    $dlExe = "$outDir\dl_msvc_$s.exe"
    $cmd = "`"$vsDevCmd`" -arch=x64 >nul && cl.exe /nologo /O2 /EHsc /utf-8 /std:$s /I include /I include/compat tests\display_list_test.cpp src\rendering\DisplayListRenderer.cpp src\rendering\backend\SoftwareRenderContext.cpp gdi32.lib user32.lib /Fe:$dlExe >nul"
    cmd.exe /c $cmd
    if ($LASTEXITCODE -eq 0 -and (Test-Path $dlExe)) {
        $output = & $dlExe
        $runOk = if ($LASTEXITCODE -eq 0) { "PASS" } else { "FAIL" }
        $match = [regex]::Match($output, "50,000 Ops recorded in ([0-9.]+) ms")
        $timeStr = if ($match.Success) { "$($match.Groups[1].Value) ms" } else { "PASS" }
        Record-Result "MSVC" $s "DisplayList" "PASS" $runOk $timeStr
    } else {
        Record-Result "MSVC" $s "DisplayList" "FAIL" "N/A" "N/A"
    }

    # 3. PAL & Backend Test
    $palExe = "$outDir\pal_msvc_$s.exe"
    $cmd = "`"$vsDevCmd`" -arch=x64 >nul && cl.exe /nologo /O2 /EHsc /utf-8 /std:$s /I include /I include/compat tests\pal_test.cpp src\rendering\backend\SoftwareRenderContext.cpp gdi32.lib user32.lib /Fe:$palExe >nul"
    cmd.exe /c $cmd
    if ($LASTEXITCODE -eq 0 -and (Test-Path $palExe)) {
        & $palExe | Out-Null
        $runOk = if ($LASTEXITCODE -eq 0) { "PASS" } else { "FAIL" }
        Record-Result "MSVC" $s "PAL_Backend" "PASS" $runOk "N/A"
    } else {
        Record-Result "MSVC" $s "PAL_Backend" "FAIL" "N/A" "N/A"
    }
}

# =============================================================================
# 格式化輸出報告
# =============================================================================
Write-Host "`n=========================================================================="
Write-Host " CPPUI Multi-Compiler x Multi-Standard Cross-Verification Matrix"
Write-Host "=========================================================================="
$results | Format-Table -AutoSize
