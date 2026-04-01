#include <windows.h>
#include <setupapi.h>
#include <iostream>
#include <vector>
#include <string>

#pragma comment(lib, "setupapi.lib")

void AfiseazaProprietate(HDEVINFO hDevInfo, SP_DEVINFO_DATA& deviceInfoData, DWORD propertyId, const wchar_t* propertyName)
{
    DWORD dataType = 0;
    DWORD requiredSize = 0;

    SetupDiGetDeviceRegistryPropertyW(
        hDevInfo,
        &deviceInfoData,
        propertyId,
        &dataType,
        nullptr,
        0,
        &requiredSize
    );

    if (requiredSize == 0)
    {
        std::wcout << L"  " << propertyName << L": (indisponibil)" << std::endl;
        return;
    }

    std::vector<BYTE> buffer(requiredSize);

    if (!SetupDiGetDeviceRegistryPropertyW(
        hDevInfo,
        &deviceInfoData,
        propertyId,
        &dataType,
        buffer.data(),
        requiredSize,
        &requiredSize))
    {
        std::wcout << L"  " << propertyName << L": (eroare la citire)" << std::endl;
        return;
    }

    std::wcout << L"  " << propertyName << L": ";

    switch (dataType)
    {
    case REG_SZ:
    case REG_EXPAND_SZ:
        std::wcout << reinterpret_cast<wchar_t*>(buffer.data()) << std::endl;
        break;

    case REG_MULTI_SZ:
    {
        wchar_t* p = reinterpret_cast<wchar_t*>(buffer.data());
        bool first = true;
        while (*p)
        {
            if (!first)
                std::wcout << L" | ";
            std::wcout << p;
            p += wcslen(p) + 1;
            first = false;
        }
        std::wcout << std::endl;
        break;
    }

    case REG_DWORD:
        if (buffer.size() >= sizeof(DWORD))
            std::wcout << *reinterpret_cast<DWORD*>(buffer.data()) << std::endl;
        else
            std::wcout << L"(valoare DWORD invalida)" << std::endl;
        break;

    default:
        std::wcout << L"(tip nereprezentat: " << dataType << L")" << std::endl;
        break;
    }
}

int wmain()
{
    std::wcout << L"Lista dispozitivelor prezente in sistem:\n" << std::endl;

    HDEVINFO hDevInfo = SetupDiGetClassDevsW(
        nullptr,
        nullptr,
        nullptr,
        DIGCF_PRESENT | DIGCF_ALLCLASSES
    );

    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        std::wcerr << L"Eroare: nu s-a putut obtine lista de dispozitive." << std::endl;
        return 1;
    }

    SP_DEVINFO_DATA deviceInfoData;
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    DWORD index = 0;

    while (SetupDiEnumDeviceInfo(hDevInfo, index, &deviceInfoData))
    {
        std::wcout << L"========================================" << std::endl;
        std::wcout << L"Dispozitivul [" << index << L"]" << std::endl;

        AfiseazaProprietate(hDevInfo, deviceInfoData, SPDRP_DEVICEDESC,    L"Descriere");
        AfiseazaProprietate(hDevInfo, deviceInfoData, SPDRP_FRIENDLYNAME,  L"Friendly Name");
        AfiseazaProprietate(hDevInfo, deviceInfoData, SPDRP_MFG,           L"Producator");
        AfiseazaProprietate(hDevInfo, deviceInfoData, SPDRP_CLASS,         L"Clasa");
        AfiseazaProprietate(hDevInfo, deviceInfoData, SPDRP_CLASSGUID,     L"Class GUID");
        AfiseazaProprietate(hDevInfo, deviceInfoData, SPDRP_HARDWAREID,    L"Hardware ID");
        AfiseazaProprietate(hDevInfo, deviceInfoData, SPDRP_COMPATIBLEIDS, L"Compatible IDs");
        AfiseazaProprietate(hDevInfo, deviceInfoData, SPDRP_ENUMERATOR_NAME, L"Enumerator");
        AfiseazaProprietate(hDevInfo, deviceInfoData, SPDRP_LOCATION_INFORMATION, L"Locatie");
        AfiseazaProprietate(hDevInfo, deviceInfoData, SPDRP_SERVICE,       L"Service");
        AfiseazaProprietate(hDevInfo, deviceInfoData, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME, L"PDO Name");

        index++;
    }

    if (GetLastError() != ERROR_NO_MORE_ITEMS)
    {
        std::wcerr << L"Eroare la enumerarea dispozitivelor." << std::endl;
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);

    std::wcout << L"\nNumar total de dispozitive gasite: " << index << std::endl;
    return 0;
}