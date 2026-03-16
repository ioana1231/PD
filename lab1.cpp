#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

void AfiseazaValoare(const std::wstring& numeValoare, DWORD tip, const BYTE* date, DWORD dimensiune)
{
    std::wcout << L"Nume valoare: " << (numeValoare.empty() ? L"(Implicit)" : numeValoare) << std::endl;
    std::wcout << L"Tip: ";

    switch (tip)
    {
    case REG_SZ:
        std::wcout << L"REG_SZ" << std::endl;
        std::wcout << L"Date: " << reinterpret_cast<const wchar_t*>(date) << std::endl;
        break;

    case REG_EXPAND_SZ:
        std::wcout << L"REG_EXPAND_SZ" << std::endl;
        std::wcout << L"Date: " << reinterpret_cast<const wchar_t*>(date) << std::endl;
        break;

    case REG_DWORD:
        if (dimensiune >= sizeof(DWORD))
        {
            DWORD valoare = *reinterpret_cast<const DWORD*>(date);
            std::wcout << L"REG_DWORD" << std::endl;
            std::wcout << L"Date: " << valoare << std::endl;
        }
        break;

    case REG_QWORD:
        if (dimensiune >= sizeof(ULONGLONG))
        {
            ULONGLONG valoare = *reinterpret_cast<const ULONGLONG*>(date);
            std::wcout << L"REG_QWORD" << std::endl;
            std::wcout << L"Date: " << valoare << std::endl;
        }
        break;

    case REG_MULTI_SZ:
    {
        std::wcout << L"REG_MULTI_SZ" << std::endl;
        const wchar_t* p = reinterpret_cast<const wchar_t*>(date);
        std::wcout << L"Date:" << std::endl;
        while (*p)
        {
            std::wcout << L"  - " << p << std::endl;
            p += wcslen(p) + 1;
        }
        break;
    }

    case REG_BINARY:
        std::wcout << L"REG_BINARY" << std::endl;
        std::wcout << L"Date (hex): ";
        for (DWORD i = 0; i < dimensiune; i++)
        {
            std::wcout << std::hex << (int)date[i] << L" ";
        }
        std::wcout << std::dec << std::endl;
        break;

    default:
        std::wcout << L"Alt tip (" << tip << L")" << std::endl;
        break;
    }

    std::wcout << L"-----------------------------" << std::endl;
}

int wmain()
{
    HKEY hKey;
    LPCWSTR subcheie = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";

    LONG rezultat = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        subcheie,
        0,
        KEY_READ,
        &hKey
    );

    if (rezultat != ERROR_SUCCESS)
    {
        std::wcerr << L"Eroare la deschiderea subcheii. Cod eroare: " << rezultat << std::endl;
        return 1;
    }

    DWORD nrValori = 0;
    DWORD lungimeMaxNume = 0;
    DWORD dimMaxDate = 0;

    rezultat = RegQueryInfoKeyW(
        hKey,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        &nrValori,
        &lungimeMaxNume,
        &dimMaxDate,
        nullptr,
        nullptr
    );

    if (rezultat != ERROR_SUCCESS)
    {
        std::wcerr << L"Eroare la interogarea informatiilor despre cheie. Cod eroare: " << rezultat << std::endl;
        RegCloseKey(hKey);
        return 1;
    }

    std::wcout << L"Subcheie: " << subcheie << std::endl;
    std::wcout << L"Numar valori: " << nrValori << std::endl;
    std::wcout << L"=============================" << std::endl;

    for (DWORD i = 0; i < nrValori; i++)
    {
        DWORD dimNume = lungimeMaxNume + 1;
        std::vector<wchar_t> numeValoare(dimNume);

        DWORD tip = 0;
        DWORD dimDate = dimMaxDate;
        std::vector<BYTE> date(dimDate);

        rezultat = RegEnumValueW(
            hKey,
            i,
            numeValoare.data(),
            &dimNume,
            nullptr,
            &tip,
            date.data(),
            &dimDate
        );

        if (rezultat == ERROR_SUCCESS)
        {
            std::wstring nume(numeValoare.data(), dimNume);
            AfiseazaValoare(nume, tip, date.data(), dimDate);
        }
        else
        {
            std::wcerr << L"Eroare la enumerarea valorii " << i << L". Cod eroare: " << rezultat << std::endl;
        }
    }

    RegCloseKey(hKey);
    return 0;
}