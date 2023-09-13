#include <initguid.h>
#include <windows.h>
#include <appmodel.h>
#include <shobjidl.h>
#include <fstream>
#include <filesystem>
#include <iostream>

#pragma comment (lib, "Ole32.lib")

auto generatePS1ScriptToMoveThisFileIntoShellStartup() {
    std::ofstream ps1File("putinstartup.ps1");
    ps1File << "Move-Item -Path \"" << __argv[0] << "\" -Destination \"$env:APPDATA\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\MinimiseFix.exe\" -Force" << std::endl;
    ps1File << "Remove-Item -Path $MyInvocation.MyCommand.Source -Force" << std::endl;
    ps1File.close();

    ShellExecute(NULL, L"open", L"powershell.exe", L"-ExecutionPolicy Bypass -File putinstartup.ps1", NULL, SW_HIDE);
}

int main() {
    if (std::filesystem::current_path().generic_u8string().find("Startup") == std::string::npos)
        generatePS1ScriptToMoveThisFileIntoShellStartup();
    WCHAR** packageNames = NULL, *pBuffer = NULL;
    IPackageDebugSettings* pPackageDebugSettings = NULL;
    UINT32 u32Index = 0, u32Count = 0, u32BufferLength = 0;
    HANDLE hHeap = GetProcessHeap();

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    CoCreateInstance(CLSID_PackageDebugSettings, NULL, CLSCTX_INPROC_SERVER, IID_IPackageDebugSettings, (LPVOID*)&pPackageDebugSettings);

    if (GetPackagesByPackageFamily(L"Microsoft.MinecraftUWP_8wekyb3d8bbwe", &u32Count, NULL, &u32BufferLength, NULL) != ERROR_INSUFFICIENT_BUFFER)
        return 1;

    packageNames = (WCHAR**)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(PWSTR) * u32Count);
    pBuffer = (WCHAR*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(WCHAR) * u32BufferLength);
    if (GetPackagesByPackageFamily(L"Microsoft.MinecraftUWP_8wekyb3d8bbwe", &u32Count, packageNames, &u32BufferLength, pBuffer) == ERROR_SUCCESS)
        for (u32Index = 0; u32Index < u32Count; u32Index += 1)
            pPackageDebugSettings->EnableDebugging(packageNames[u32Index], NULL, NULL);

    pPackageDebugSettings->Release();
    HeapFree(hHeap, 0, packageNames);
    HeapFree(hHeap, 0, pBuffer);
    CloseHandle(hHeap);
    CoUninitialize();

    return 0;
}
