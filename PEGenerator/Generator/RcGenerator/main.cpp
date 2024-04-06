#include "resource.h"

#include <windows.h>
#include <fstream>
#include <iostream>
#include <string>
#include <codecvt>

bool ExtractResource(const HINSTANCE hInstance, const WORD resourceID, const LPCWSTR outputPath) {
    HRSRC hRes = FindResource(hInstance, MAKEINTRESOURCE(resourceID), RT_RCDATA);
    if (hRes == NULL) {
        return false; // 资源未找到
    }

    HGLOBAL hData = LoadResource(hInstance, hRes);
    if (hData == NULL) {
        return false; // 资源加载失败
    }

    LPVOID pData = LockResource(hData);
    if (pData == NULL) {
        return false; // 资源锁定失败
    }

    DWORD dataSize = SizeofResource(hInstance, hRes);

    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile) {
        return false; // 文件打开失败
    }

    outFile.write(reinterpret_cast<const char*>(pData), dataSize);
    outFile.close();

    return true;
}

// 窄字符转换为宽字符
std::wstring Ansi2WChar(const char* pszSrc)
{
    bool flag = false;
    size_t len = 0;
    size_t trans_len = strlen(pszSrc) + 1;
    std::wstring wstr_res;

    wchar_t* pwszDst = new wchar_t[trans_len];
    errno_t err = mbstowcs_s(&len, pwszDst, trans_len, pszSrc, trans_len);

    if (len > 0) {

        wstr_res = pwszDst;
    }
    return wstr_res;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <ResourceID> <OutputPath>" << std::endl;
        return 1;
    }

    WORD resourceID = static_cast<WORD>(std::stoi(argv[1]));
    std::wstring file_path = Ansi2WChar(argv[2]);

    if (ExtractResource(GetModuleHandle(NULL), resourceID, file_path.c_str())) {
        std::wcout << L"Resource extracted successfully to " << file_path.c_str() << std::endl;
    } else {
        std::cerr << "Failed to extract resource." << std::endl;
    }

    return 0;
}


