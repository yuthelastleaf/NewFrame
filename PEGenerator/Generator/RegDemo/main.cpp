#include <windows.h>
#include <string>
#include <map>
#include <variant>
#include <functional>
#include <type_traits>

namespace RegistryUtils {

    HKEY GetRootKey(const std::string& rootKeyStr) {
        static const std::map<std::string, HKEY> keyMap = {
            {"HKEY_CLASSES_ROOT", HKEY_CLASSES_ROOT},
            {"HKEY_CURRENT_USER", HKEY_CURRENT_USER},
            {"HKEY_LOCAL_MACHINE", HKEY_LOCAL_MACHINE},
            {"HKEY_USERS", HKEY_USERS},
            {"HKEY_CURRENT_CONFIG", HKEY_CURRENT_CONFIG}
        };

        auto it = keyMap.find(rootKeyStr);
        if (it != keyMap.end()) {
            return it->second;
        }
        return nullptr; // 或者抛出异常
    }

    HKEY GetRootKey(const std::wstring& rootKeyStr) {
        static const std::map<std::wstring, HKEY> keyMap = {
            {L"HKEY_CLASSES_ROOT", HKEY_CLASSES_ROOT},
            {L"HKEY_CURRENT_USER", HKEY_CURRENT_USER},
            {L"HKEY_LOCAL_MACHINE", HKEY_LOCAL_MACHINE},
            {L"HKEY_USERS", HKEY_USERS},
            {L"HKEY_CURRENT_CONFIG", HKEY_CURRENT_CONFIG}
        };

        auto it = keyMap.find(rootKeyStr);
        if (it != keyMap.end()) {
            return it->second;
        }
        return nullptr; // 或者抛出异常
    }
    
    using RegValue = std::variant<std::string, DWORD, std::wstring>;

    // 创建或打开键，并返回其句柄>
    inline LONG CreateOrOpenKey(HKEY hRootKey, const std::wstring& subKey, HKEY* hResultKey) {
        DWORD dwDisposition; // 用于接收键是新创建还是打开的信息
        LONG lResult = RegCreateKeyExW(hRootKey, subKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, hResultKey, &dwDisposition);
        return lResult;
    }

    inline LONG CreateOrOpenKey(HKEY hRootKey, const std::string& subKey, HKEY* hResultKey) {
        DWORD dwDisposition; // 用于接收键是新创建还是打开的信息
        LONG lResult = RegCreateKeyExA(hRootKey, subKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, hResultKey, &dwDisposition);
        return lResult;
    }

    // 递归创建路径中的所有键
    LONG EnsurePathExists(HKEY hRootKey, const std::wstring& fullPath) {
        size_t pos = 0;
        LONG lResult = ERROR_SUCCESS;

        while (pos != std::wstring::npos) {
            pos = fullPath.find(L'\\', pos + 1);
            std::wstring currentPath = fullPath.substr(0, pos);

            HKEY hNextKey;
            lResult = CreateOrOpenKey(hRootKey, currentPath, &hNextKey);
            if (hNextKey != hRootKey) {
                RegCloseKey(hNextKey);
            }
            if (lResult != ERROR_SUCCESS) {
                break;
            }
        }

        return lResult;
    }

    LONG EnsurePathExists(HKEY hRootKey, const std::string& fullPath) {
        size_t pos = 0;
        HKEY hCurrentKey = hRootKey;
        LONG lResult = ERROR_SUCCESS;

        while (pos != std::string::npos) {
            pos = fullPath.find(L'\\', pos + 1);
            std::string currentPath = fullPath.substr(0, pos);

            HKEY hNextKey;
            lResult = CreateOrOpenKey(hCurrentKey, currentPath, &hNextKey);
            if (lResult != ERROR_SUCCESS) {
                if (hCurrentKey != hRootKey) {
                    RegCloseKey(hCurrentKey);
                }
                return lResult;
            }

            // 如果不是根键，关闭当前键
            if (hCurrentKey != hRootKey) {
                RegCloseKey(hCurrentKey);
            }
            hCurrentKey = hNextKey;
        }

        // 最后一步，确保调用方可以关闭打开的键
        if (hCurrentKey != hRootKey) {
            RegCloseKey(hCurrentKey);
        }

        return ERROR_SUCCESS;
    }

    LONG SetRegValue(const std::wstring& fullPathWithValueName, const RegValue& valueData) {
        // 寻找根键和值名称的位置
        size_t rootEndPos = fullPathWithValueName.find(L"\\");
        size_t valueNameStartPos = fullPathWithValueName.rfind(L"\\");
        if (rootEndPos == std::wstring::npos || valueNameStartPos == std::wstring::npos || rootEndPos == valueNameStartPos) {
            return ERROR_INVALID_PARAMETER; // 或者抛出异常
        }

        std::wstring rootKeyStr = fullPathWithValueName.substr(0, rootEndPos);
        std::wstring subKey = fullPathWithValueName.substr(rootEndPos + 1, valueNameStartPos - rootEndPos - 1);
        std::wstring valueName = fullPathWithValueName.substr(valueNameStartPos + 1);

        HKEY hRootKey = GetRootKey(rootKeyStr);
        if (!hRootKey) {
            return ERROR_INVALID_PARAMETER; // 或者抛出异常
        }

        // 确保路径存在
        LONG lResult = EnsurePathExists(hRootKey, subKey);
        if (lResult != ERROR_SUCCESS) {
            return lResult;
        }

        // 打开最终的键
        HKEY hKey;
        lResult = RegOpenKeyExW(hRootKey, subKey.c_str(), 0, KEY_SET_VALUE, &hKey);
        if (lResult != ERROR_SUCCESS) {
            return lResult;
        }

        // 设置值
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>) {
                lResult = RegSetValueExA(hKey, std::string(valueName.begin(), valueName.end()).c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(arg.c_str()), arg.size() + 1);
            }
            else if constexpr (std::is_same_v<T, std::wstring>) {
                lResult = RegSetValueExW(hKey, valueName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(arg.c_str()), (arg.size() + 1) * sizeof(wchar_t));
            }
            else if constexpr (std::is_same_v<T, DWORD>) {
                lResult = RegSetValueExW(hKey, valueName.c_str(), 0, REG_DWORD, reinterpret_cast<const BYTE*>(&arg), sizeof(arg));
            }
            }, valueData);

        RegCloseKey(hKey);
        return lResult;
    }

    LONG SetRegValue(const std::string& fullPathWithValueName, const RegValue& valueData) {
        // 寻找根键和值名称的位置
        size_t rootEndPos = fullPathWithValueName.find("\\");
        size_t valueNameStartPos = fullPathWithValueName.rfind("\\");
        if (rootEndPos == std::string::npos || valueNameStartPos == std::string::npos || rootEndPos == valueNameStartPos) {
            return ERROR_INVALID_PARAMETER; // 或者抛出异常
        }

        std::string rootKeyStr = fullPathWithValueName.substr(0, rootEndPos);
        std::string subKey = fullPathWithValueName.substr(rootEndPos + 1, valueNameStartPos - rootEndPos - 1);
        std::string valueName = fullPathWithValueName.substr(valueNameStartPos + 1);

        HKEY hRootKey = GetRootKey(rootKeyStr);
        if (!hRootKey) {
            return ERROR_INVALID_PARAMETER; // 或者抛出异常
        }

        // 确保路径存在
        LONG lResult = EnsurePathExists(hRootKey, subKey);
        if (lResult != ERROR_SUCCESS) {
            return lResult;
        }

        // 打开最终的键
        HKEY hKey;
        lResult = RegOpenKeyExA(hRootKey, subKey.c_str(), 0, KEY_SET_VALUE, &hKey);
        if (lResult != ERROR_SUCCESS) {
            return lResult;
        }

        // 设置值
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>) {
                lResult = RegSetValueExA(hKey, std::string(valueName.begin(), valueName.end()).c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(arg.c_str()), arg.size() + 1);
            }
            else if constexpr (std::is_same_v<T, std::wstring>) {
                lResult = RegSetValueExA(hKey, valueName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(arg.c_str()), (arg.size() + 1) * sizeof(wchar_t));
            }
            else if constexpr (std::is_same_v<T, DWORD>) {
                lResult = RegSetValueExA(hKey, valueName.c_str(), 0, REG_DWORD, reinterpret_cast<const BYTE*>(&arg), sizeof(arg));
            }
            }, valueData);

        RegCloseKey(hKey);
        return lResult;
    }

}

int main() {

    RegistryUtils::SetRegValue(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\testdata", L"tttt");
    RegistryUtils::SetRegValue(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\testdata", "ttttstring");
    RegistryUtils::SetRegValue(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\testdata\\mynewdata\\heihei", "ttttstring");

    return 0;
}

