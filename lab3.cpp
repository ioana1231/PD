#include <windows.h>

#pragma comment(lib, "advapi32.lib")

SERVICE_STATUS g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE g_StopEvent = NULL;
LPCWSTR g_ServiceName = L"HelloWorldService";

void WriteToEventLog(LPCWSTR message)
{
    HANDLE hEventSource = RegisterEventSourceW(NULL, g_ServiceName);
    if (hEventSource != NULL)
    {
        LPCWSTR strings[1] = { message };

        ReportEventW(
            hEventSource,                 // sursa evenimentului
            EVENTLOG_INFORMATION_TYPE,   // tip eveniment
            0,                           // category
            1,                           // event ID
            NULL,                        // user SID
            1,                           // nr. stringuri
            0,                           // nr. bytes raw data
            strings,                     // mesajul
            NULL                         // raw data
        );

        DeregisterEventSource(hEventSource);
    }
}

void UpdateServiceStatus(DWORD currentState, DWORD win32ExitCode, DWORD waitHint)
{
    g_ServiceStatus.dwCurrentState = currentState;
    g_ServiceStatus.dwWin32ExitCode = win32ExitCode;
    g_ServiceStatus.dwWaitHint = waitHint;

    if (currentState == SERVICE_START_PENDING)
        g_ServiceStatus.dwControlsAccepted = 0;
    else
        g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}

void WINAPI ServiceCtrlHandler(DWORD controlCode)
{
    switch (controlCode)
    {
    case SERVICE_CONTROL_STOP:
        if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
            break;

        UpdateServiceStatus(SERVICE_STOP_PENDING, 0, 3000);

        WriteToEventLog(L"End of the World!");

        if (g_StopEvent != NULL)
            SetEvent(g_StopEvent);

        break;

    default:
        break;
    }
}

void WINAPI ServiceMain(DWORD argc, LPWSTR* argv)
{
    g_StatusHandle = RegisterServiceCtrlHandlerW(g_ServiceName, ServiceCtrlHandler);
    if (g_StatusHandle == NULL)
        return;

    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    UpdateServiceStatus(SERVICE_START_PENDING, 0, 3000);

    g_StopEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (g_StopEvent == NULL)
    {
        UpdateServiceStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }

    WriteToEventLog(L"Hello World!");

    UpdateServiceStatus(SERVICE_RUNNING, 0, 0);

    // Serviciul rămâne activ până primește STOP
    WaitForSingleObject(g_StopEvent, INFINITE);

    if (g_StopEvent != NULL)
    {
        CloseHandle(g_StopEvent);
        g_StopEvent = NULL;
    }

    UpdateServiceStatus(SERVICE_STOPPED, 0, 0);
}

int main()
{
    SERVICE_TABLE_ENTRYW ServiceTable[] =
    {
        { (LPWSTR)g_ServiceName, (LPSERVICE_MAIN_FUNCTIONW)ServiceMain },
        { NULL, NULL }
    };

    StartServiceCtrlDispatcherW(ServiceTable);
    return 0;
}