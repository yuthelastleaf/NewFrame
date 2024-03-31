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
        return nullptr; // �����׳��쳣
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
        return nullptr; // �����׳��쳣
    }
    
    using RegValue = std::variant<std::string, DWORD, std::wstring>;

    // ������򿪼�������������>
    inline LONG CreateOrOpenKey(HKEY hRootKey, const std::wstring& subKey, HKEY* hResultKey) {
        DWORD dwDisposition; // ���ڽ��ռ����´������Ǵ򿪵���Ϣ
        LONG lResult = RegCreateKeyExW(hRootKey, subKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, hResultKey, &dwDisposition);
        return lResult;
    }

    inline LONG CreateOrOpenKey(HKEY hRootKey, const std::string& subKey, HKEY* hResultKey) {
        DWORD dwDisposition; // ���ڽ��ռ����´������Ǵ򿪵���Ϣ
        LONG lResult = RegCreateKeyExA(hRootKey, subKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, hResultKey, &dwDisposition);
        return lResult;
    }

    // �ݹ鴴��·���е����м�
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

            // ������Ǹ������رյ�ǰ��
            if (hCurrentKey != hRootKey) {
                RegCloseKey(hCurrentKey);
            }
            hCurrentKey = hNextKey;
        }

        // ���һ����ȷ�����÷����Թرմ򿪵ļ�
        if (hCurrentKey != hRootKey) {
            RegCloseKey(hCurrentKey);
        }

        return ERROR_SUCCESS;
    }

    LONG SetRegValue(const std::wstring& fullPathWithValueName, const RegValue& valueData) {
        // Ѱ�Ҹ�����ֵ���Ƶ�λ��
        size_t rootEndPos = fullPathWithValueName.find(L"\\");
        size_t valueNameStartPos = fullPathWithValueName.rfind(L"\\");
        if (rootEndPos == std::wstring::npos || valueNameStartPos == std::wstring::npos || rootEndPos == valueNameStartPos) {
            return ERROR_INVALID_PARAMETER; // �����׳��쳣
        }

        std::wstring rootKeyStr = fullPathWithValueName.substr(0, rootEndPos);
        std::wstring subKey = fullPathWithValueName.substr(rootEndPos + 1, valueNameStartPos - rootEndPos - 1);
        std::wstring valueName = fullPathWithValueName.substr(valueNameStartPos + 1);

        HKEY hRootKey = GetRootKey(rootKeyStr);
        if (!hRootKey) {
            return ERROR_INVALID_PARAMETER; // �����׳��쳣
        }

        // ȷ��·������
        LONG lResult = EnsurePathExists(hRootKey, subKey);
        if (lResult != ERROR_SUCCESS) {
            return lResult;
        }

        // �����յļ�
        HKEY hKey;
        lResult = RegOpenKeyExW(hRootKey, subKey.c_str(), 0, KEY_SET_VALUE, &hKey);
        if (lResult != ERROR_SUCCESS) {
            return lResult;
        }

        // ����ֵ
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
        // Ѱ�Ҹ�����ֵ���Ƶ�λ��
        size_t rootEndPos = fullPathWithValueName.find("\\");
        size_t valueNameStartPos = fullPathWithValueName.rfind("\\");
        if (rootEndPos == std::string::npos || valueNameStartPos == std::string::npos || rootEndPos == valueNameStartPos) {
            return ERROR_INVALID_PARAMETER; // �����׳��쳣
        }

        std::string rootKeyStr = fullPathWithValueName.substr(0, rootEndPos);
        std::string subKey = fullPathWithValueName.substr(rootEndPos + 1, valueNameStartPos - rootEndPos - 1);
        std::string valueName = fullPathWithValueName.substr(valueNameStartPos + 1);

        HKEY hRootKey = GetRootKey(rootKeyStr);
        if (!hRootKey) {
            return ERROR_INVALID_PARAMETER; // �����׳��쳣
        }

        // ȷ��·������
        LONG lResult = EnsurePathExists(hRootKey, subKey);
        if (lResult != ERROR_SUCCESS) {
            return lResult;
        }

        // �����յļ�
        HKEY hKey;
        lResult = RegOpenKeyExA(hRootKey, subKey.c_str(), 0, KEY_SET_VALUE, &hKey);
        if (lResult != ERROR_SUCCESS) {
            return lResult;
        }

        // ����ֵ
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

