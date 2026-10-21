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
using System.Text;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

public class WinCap {
    [DllImport("user32.dll", SetLastError = true)]
    public static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);

    [DllImport("user32.dll", SetLastError = true)]
    public static extern bool GetClientRect(IntPtr hWnd, out RECT lpRect);

    [DllImport("user32.dll")]
    public static extern bool SetProcessDpiAwarenessContext(IntPtr dpiContext);

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

                    // 嚴格排除主控台、IME 與 UAC 視窗
                    if (className.Contains("Console") || className.Contains("IME") || className.Contains("UAC")) {
                        return true;
                    }
                    if (className.Contains("CPPUI") || (w > 150 && h > 150)) {
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
        try {
            SetProcessDpiAwarenessContext(new IntPtr(-4));
        } catch {}

        RECT r;
        if (!GetClientRect(hWnd, out r)) return false;
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

$handle = [WinCap]::FindWindowByProcessId([uint32]$proc.Id)
$retry = 0
while ($handle -eq [IntPtr]::Zero -and $retry -lt 20) {
    Start-Sleep -Milliseconds 250
    $handle = [WinCap]::FindWindowByProcessId([uint32]$proc.Id)
    $retry++
}

if ($handle -ne [IntPtr]::Zero) {
    [WinCap]::SetForegroundWindow($handle)
    Start-Sleep -Milliseconds 1200
    $success = [WinCap]::Capture($handle, $OutputPath)
    if ($success) {
        $fileSize = (Get-Item $OutputPath).Length
        Write-Host "Window captured successfully: $OutputPath ($fileSize bytes)"
    } else {
        Write-Error "BitBlt failed to capture window."
    }
} else {
    Write-Error "Could not retrieve Window Handle for PID $($proc.Id)."
}

Stop-Process -Id $proc.Id -Force
