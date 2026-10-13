from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout

class CppUIConan(ConanFile):
    name = "cppui"
    version = "0.1.0"
    license = "MIT"
    author = "Willy"
    description = "Modern C++20 cross-platform UI framework"
    topics = ("gui", "ui", "skia", "cpp20")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_skia": [True, False]
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_skia": False
    }

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["CPPUI_BUILD_TESTS"] = False
        tc.variables["CPPUI_BUILD_EXAMPLES"] = False
        tc.variables["CPPUI_ENABLE_SKIA"] = self.options.with_skia
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["cppui_core"]
        if self.settings.os == "Windows":
            self.cpp_info.system_libs = ["dwrite", "gdi32", "user32", "ole32", "imm32", "shell32", "comdlg32"]
        elif self.settings.os == "Linux":
            self.cpp_info.system_libs = ["pthread", "dl", "X11", "wayland-client"]
