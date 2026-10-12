param(
    [Parameter(Mandatory=$true)]
    [string]$ExePath,

    [Parameter(Mandatory=$true)]
    [string]$OutputPath,

    [string]$ArgumentList = "",

    [int]$WaitSeconds = 2
)

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

$winApiCode = @'
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

public class WinCap {
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
                RECT r;
                if (GetWindowRect(hWnd, out r)) {
                    if (r.Right - r.Left > 100 && r.Bottom - r.Top > 100) {
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

$outDir = Split-Path -Parent $OutputPath
if ($outDir -and -not (Test-Path $outDir)) {
    New-Item -ItemType Directory -Path $outDir -Force | Out-Null
}

if ($ArgumentList) {
    $proc = Start-Process -FilePath $ExePath -ArgumentList $ArgumentList -PassThru
} else {
    $proc = Start-Process -FilePath $ExePath -PassThru
}
Start-Sleep -Seconds $WaitSeconds

$handle = $proc.MainWindowHandle
$retry = 0
while ($handle -eq [IntPtr]::Zero -and $retry -lt 15) {
    Start-Sleep -Milliseconds 300
    $proc.Refresh()
    $handle = $proc.MainWindowHandle
    if ($handle -eq [IntPtr]::Zero) {
        $handle = [WinCap]::FindWindowByProcessId([uint32]$proc.Id)
    }
    $retry++
}

if ($handle -ne [IntPtr]::Zero) {
    [WinCap]::SetForegroundWindow($handle)
    Start-Sleep -Milliseconds 500
    $success = [WinCap]::Capture($handle, $OutputPath)
    if ($success) {
        Write-Host "Window captured successfully: $OutputPath"
    } else {
        Write-Error "BitBlt failed to capture window."
    }
} else {
    Write-Error "Could not retrieve Window Handle for PID $($proc.Id)."
}

Stop-Process -Id $proc.Id -Force
