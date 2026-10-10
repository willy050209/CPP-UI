/**
 * @file DialogCommon.cpp
 * @brief 實作檔案對話框篩選器解析、樣式匹配、大小格式化與全域後端管理者
 */

#include "ui/dialogs/DialogCommon.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <sstream>

namespace cppui {
namespace dialogs {

namespace {

// 全域預設對話框後端模式
DialogBackendMode s_defaultBackendMode = DialogBackendMode::Auto;

// 輔助函式：去除字串前後空白
static std::string TrimString(compat::string_view str) {
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<uint8_t>(str[start]))) {
        ++start;
    }
    size_t end = str.size();
    while (end > start && std::isspace(static_cast<uint8_t>(str[end - 1]))) {
        --end;
    }
    return std::string(str.substr(start, end - start));
}

// 輔助函式：字元不分大小寫比對
static bool CharEqualsIgnoreCase(char a, char b) noexcept {
    return std::tolower(static_cast<uint8_t>(a)) == std::tolower(static_cast<uint8_t>(b));
}

} // namespace

std::vector<FileFilterItem> DialogUtilities::ParseFilterString(compat::string_view filter) {
    std::vector<FileFilterItem> result;
    if (filter.empty()) {
        return result;
    }

    // 以 '|' 分割
    std::vector<std::string> tokens;
    size_t start = 0;
    for (size_t i = 0; i < filter.size(); ++i) {
        if (filter[i] == '|') {
            tokens.push_back(std::string(filter.substr(start, i - start)));
            start = i + 1;
        }
    }
    tokens.push_back(std::string(filter.substr(start)));

    // 成對處理：tokens[i] 為描述，tokens[i + 1] 為樣式
    for (size_t i = 0; i + 1 < tokens.size(); i += 2) {
        FileFilterItem item;
        item.description = TrimString(tokens[i]);

        std::string patternToken = tokens[i + 1];
        size_t pStart = 0;
        for (size_t p = 0; p < patternToken.size(); ++p) {
            if (patternToken[p] == ';') {
                std::string singlePattern = TrimString(compat::string_view(patternToken.data() + pStart, p - pStart));
                if (!singlePattern.empty()) {
                    item.patterns.push_back(singlePattern);
                }
                pStart = p + 1;
            }
        }
        std::string lastPattern = TrimString(compat::string_view(patternToken.data() + pStart, patternToken.size() - pStart));
        if (!lastPattern.empty()) {
            item.patterns.push_back(lastPattern);
        }

        if (!item.description.empty() || !item.patterns.empty()) {
            result.push_back(std::move(item));
        }
    }

    return result;
}

bool DialogUtilities::MatchesPattern(compat::string_view fileName, compat::string_view pattern) {
    if (pattern.empty()) {
        return fileName.empty();
    }
    if (pattern == "*.*" || pattern == "*") {
        return true;
    }

    // 通用萬用字元比對演算法 (* 與 ?)
    size_t fIdx = 0;
    size_t pIdx = 0;
    size_t starIdx = std::string::npos;
    size_t matchIdx = 0;

    while (fIdx < fileName.size()) {
        if (pIdx < pattern.size() && (pattern[pIdx] == '?' || CharEqualsIgnoreCase(pattern[pIdx], fileName[fIdx]))) {
            ++fIdx;
            ++pIdx;
        } else if (pIdx < pattern.size() && pattern[pIdx] == '*') {
            starIdx = pIdx;
            matchIdx = fIdx;
            ++pIdx;
        } else if (starIdx != std::string::npos) {
            pIdx = starIdx + 1;
            ++matchIdx;
            fIdx = matchIdx;
        } else {
            return false;
        }
    }

    while (pIdx < pattern.size() && pattern[pIdx] == '*') {
        ++pIdx;
    }

    return pIdx == pattern.size();
}

bool DialogUtilities::MatchesFilter(compat::string_view fileName, const FileFilterItem& item) {
    if (item.patterns.empty()) {
        return true;
    }
    for (const auto& pat : item.patterns) {
        if (MatchesPattern(fileName, pat)) {
            return true;
        }
    }
    return false;
}

std::string DialogUtilities::EnsureExtension(compat::string_view filePath, compat::string_view defaultExt) {
    std::string path(filePath);
    if (path.empty() || defaultExt.empty()) {
        return path;
    }

    // 尋找最後一個路徑分隔符號
    size_t lastSlash = path.find_last_of("/\\");
    size_t lastDot = path.find_last_of('.');

    // 若最後一個點在分隔符號之前，表示檔名本身沒有副檔名
    if (lastDot == std::string::npos || (lastSlash != std::string::npos && lastDot < lastSlash)) {
        if (defaultExt.front() != '.') {
            path.push_back('.');
        }
        path.append(defaultExt.data(), defaultExt.size());
    }

    return path;
}

std::string DialogUtilities::FormatFileSize(uint64_t bytes) {
    char buffer[64];
    constexpr uint64_t kb = 1024ULL;
    constexpr uint64_t mb = kb * 1024ULL;
    constexpr uint64_t gb = mb * 1024ULL;

    if (bytes >= gb) {
        double val = static_cast<double>(bytes) / static_cast<double>(gb);
        std::snprintf(buffer, sizeof(buffer), "%.2f GB", val);
    } else if (bytes >= mb) {
        double val = static_cast<double>(bytes) / static_cast<double>(mb);
        std::snprintf(buffer, sizeof(buffer), "%.2f MB", val);
    } else if (bytes >= kb) {
        double val = static_cast<double>(bytes) / static_cast<double>(kb);
        std::snprintf(buffer, sizeof(buffer), "%.1f KB", val);
    } else {
        std::snprintf(buffer, sizeof(buffer), "%llu B", static_cast<unsigned long long>(bytes));
    }
    return std::string(buffer);
}

void DialogManager::SetDefaultBackendMode(DialogBackendMode mode) noexcept {
    s_defaultBackendMode = mode;
}

DialogBackendMode DialogManager::GetDefaultBackendMode() noexcept {
    return s_defaultBackendMode;
}

} // namespace dialogs
} // namespace cppui
