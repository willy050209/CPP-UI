<#
.SYNOPSIS
    批量執行所有具有視窗的 CPPUI 範例程式並自動擷取真實視窗 PNG 截圖。

.DESCRIPTION
    遍歷 build/bin/Release/ 目錄下的 19 個範例執行檔，使用 Win32 API 辨識
    CPPUI_PlatformWindowClass 視窗並擷取視窗點陣圖，輸出至 docs/images/ 目錄。
#>

param(
    [string]$BinDir = "build\bin\Release",
    [string]$OutputDir = "docs\images",
    [int]$WaitSeconds = 2
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
}

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

$winApiCode = @'
using System;
using System.Text;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

public class ExampleBatchCap {
    [DllImport("user32.dll", SetLastError = true)]
    public static extern bool SetProcessDpiAwarenessContext(IntPtr dpiContext);

    static ExampleBatchCap() {
        try {
            SetProcessDpiAwarenessContext(new IntPtr(-4)); // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
        } catch {}
    }

    [DllImport("user32.dll", SetLastError = true)]
    public static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);

    [DllImport("user32.dll", SetLastError = true)]
    public static extern bool SetForegroundWindow(IntPtr hWnd);

    [DllImport("user32.dll")]
    public static extern IntPtr GetDC(IntPtr hWnd);

    [DllImport("user32.dll")]
    public static extern int ReleaseDC(IntPtr hWnd, IntPtr hDC);

    [DllImport("gdi32.dll")]
    public static extern bool BitBlt(IntPtr hObject, int nXDest, int nYDest, int nWidth, int nHeight, IntPtr hObjectSource, int nXSrc, int nYSrc, int dwRop);

    [DllImport("user32.dll", SetLastError = true)]
    public static extern bool IsWindowVisible(IntPtr hWnd);

    [DllImport("user32.dll", SetLastError = true)]
    public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint lpdwProcessId);

    [DllImport("user32.dll", CharSet = CharSet.Auto)]
    public static extern int GetClassName(IntPtr hWnd, StringBuilder lpClassName, int nMaxCount);

    public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);

    [DllImport("user32.dll", SetLastError = true)]
    public static extern bool EnumWindows(EnumWindowsProc enumProc, IntPtr lParam);

    [StructLayout(LayoutKind.Sequential)]
    public struct RECT {
        public int Left;
        public int Top;
        public int Right;
        public int Bottom;
    }

    public static IntPtr FindWindowByProcessId(uint pid) {
        IntPtr found = IntPtr.Zero;
        EnumWindows((hWnd, lParam) => {
            uint windowPid;
            GetWindowThreadProcessId(hWnd, out windowPid);
            if (windowPid == pid && IsWindowVisible(hWnd)) {
                StringBuilder cls = new StringBuilder(256);
                GetClassName(hWnd, cls, 256);
                string className = cls.ToString();

                RECT r;
                if (GetWindowRect(hWnd, out r)) {
                    int w = r.Right - r.Left;
                    int h = r.Bottom - r.Top;

                    // 優先匹配 CPPUI 原生視窗類別，或面積大於 150x150 之可見視窗（排除 IME 與 UAC 視窗）
                    if (className.Contains("CPPUI") || (w > 150 && h > 150 && !className.Contains("UAC") && !className.Contains("IME"))) {
                        found = hWnd;
                        return false;
                    }
                }
            }
            return true;
        }, IntPtr.Zero);
        return found;
    }

    public static bool Capture(IntPtr hWnd, string savePath) {
        if (hWnd == IntPtr.Zero) return false;
        RECT r;
        if (!GetWindowRect(hWnd, out r)) return false;
        int w = r.Right - r.Left;
        int h = r.Bottom - r.Top;
        if (w <= 0 || h <= 0) return false;

        IntPtr wndDC = GetDC(hWnd);
        if (wndDC == IntPtr.Zero) return false;

        try {
            using (Bitmap bmp = new Bitmap(w, h)) {
                using (Graphics g = Graphics.FromImage(bmp)) {
                    IntPtr hdc = g.GetHdc();
                    try {
                        BitBlt(hdc, 0, 0, w, h, wndDC, 0, 0, 0x00CC0020); // SRCCOPY
                    } finally {
                        g.ReleaseHdc(hdc);
                    }
                }
                bmp.Save(savePath, ImageFormat.Png);
            }
        } finally {
            ReleaseDC(hWnd, wndDC);
        }
        return true;
    }
}
'@

Add-Type -TypeDefinition $winApiCode -ReferencedAssemblies System.Drawing

$tasks = @(
    @{ Exe = "hello_world.exe"; Output = "hello_world_preview.png"; Args = "" },
    @{ Exe = "bmi_calculator.exe"; Output = "bmi_calculator_preview.png"; Args = "" },
    @{ Exe = "graphics_drawing_demo.exe"; Output = "graphics_drawing_preview.png"; Args = "" },
    @{ Exe = "graphics_drawing_demo.exe"; Output = "graphics_grayscale_preview.png"; Args = "--grayscale" },
    @{ Exe = "drawing_cardioid_example.exe"; Output = "drawing_cardioid_preview.png"; Args = "" },
    @{ Exe = "dialog_gallery.exe"; Output = "dialog_gallery_preview.png"; Args = "" },
    @{ Exe = "popup_gallery.exe"; Output = "popup_gallery_preview.png"; Args = "" },
    @{ Exe = "virtualized_list_demo.exe"; Output = "virtualized_list_preview.png"; Args = "" },
    @{ Exe = "devtools_gallery.exe"; Output = "devtools_gallery_preview.png"; Args = "" },
    @{ Exe = "mvvm_example.exe"; Output = "mvvm_example_preview.png"; Args = "" },
    @{ Exe = "ioc_example.exe"; Output = "ioc_example_preview.png"; Args = "" },
    @{ Exe = "async_events_example.exe"; Output = "async_events_preview.png"; Args = "" },
    @{ Exe = "platform_skia_example.exe"; Output = "platform_skia_preview.png"; Args = "" },
    @{ Exe = "m1_window_demo.exe"; Output = "m1_window_demo_preview.png"; Args = "" },
    @{ Exe = "m2_displaylist_demo.exe"; Output = "m2_displaylist_demo_preview.png"; Args = "" },
    @{ Exe = "m3_text_demo.exe"; Output = "m3_text_demo_preview.png"; Args = "" },
    @{ Exe = "m4_layout_demo.exe"; Output = "m4_layout_demo_preview.png"; Args = "" },
    @{ Exe = "m5_input_demo.exe"; Output = "m5_input_demo_preview.png"; Args = "" },
    @{ Exe = "m6_fluent_rad_demo.exe"; Output = "m6_fluent_rad_demo_preview.png"; Args = "" },
    @{ Exe = "m7_mvvm_app_demo.exe"; Output = "m7_mvvm_app_demo_preview.png"; Args = "" }
)

Write-Host "=========================================================="
Write-Host " CPPUI 範例程式真實視窗影像批量擷取工具"
Write-Host " 總計待擷取任務: $($tasks.Count)"
Write-Host "=========================================================="

$successCount = 0
$failCount = 0

foreach ($t in $tasks) {
    $exePath = Join-Path $BinDir $t.Exe
    $outPath = Join-Path $OutputDir $t.Output

    if (-not (Test-Path $exePath)) {
        Write-Warning "執行檔不存在: $exePath，略過。"
        $failCount++
        continue
    }

    Write-Host -NoNewline "[Capturing] $($t.Exe) $(if ($t.Args) { '(' + $t.Args + ')' }) -> $($t.Output) ... "

    try {
        if ($t.Args) {
            $p = Start-Process -FilePath $exePath -ArgumentList $t.Args -PassThru
        } else {
            $p = Start-Process -FilePath $exePath -PassThru
        }

        Start-Sleep -Seconds $WaitSeconds

        $handle = [ExampleBatchCap]::FindWindowByProcessId([uint32]$p.Id)
        $retry = 0
        while ($handle -eq [IntPtr]::Zero -and $retry -lt 20) {
            Start-Sleep -Milliseconds 250
            $handle = [ExampleBatchCap]::FindWindowByProcessId([uint32]$p.Id)
            $retry++
        }

        if ($handle -ne [IntPtr]::Zero) {
            [ExampleBatchCap]::SetForegroundWindow($handle)
            Start-Sleep -Milliseconds 500
            $ok = [ExampleBatchCap]::Capture($handle, $outPath)
            if ($ok -and (Test-Path $outPath)) {
                $size = (Get-Item $outPath).Length
                if ($size -gt 1000) {
                    Write-Host "OK ($size bytes)" -ForegroundColor Green
                    $successCount++
                } else {
                    Write-Host "FAILED (Size too small: $size bytes)" -ForegroundColor Red
                    $failCount++
                }
            } else {
                Write-Host "FAILED (Capture returned false)" -ForegroundColor Red
                $failCount++
            }
        } else {
            Write-Host "FAILED (Window not found)" -ForegroundColor Red
            $failCount++
        }

        Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue
    } catch {
        Write-Host "ERROR ($($_.Exception.Message))" -ForegroundColor Red
        $failCount++
    }
}

Write-Host "=========================================================="
Write-Host " 擷取完成: 成功 $successCount 個, 失敗 $failCount 個"
Write-Host "=========================================================="

if ($failCount -gt 0) {
    exit 1
} else {
    exit 0
}
