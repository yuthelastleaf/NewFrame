#include <ntddk.h>
#include <ntddmou.h>

#define MOUSE_IDLE_TIMEOUT 5000  // 设置空闲超时时间为5秒

PDEVICE_OBJECT pDeviceObject = NULL;
KTIMER Timer;
KDPC TimerDpc;
LONG LastActivityTimestamp = 0;

VOID UnloadDriver(PDRIVER_OBJECT pDriverObject)
{
    UNREFERENCED_PARAMETER(pDriverObject);
    DbgPrint("Driver Unloaded\n");
}

NTSTATUS MouseClassServiceCallback(
    PDEVICE_OBJECT DeviceObject,
    PMOUSE_INPUT_DATA InputDataStart,
    PMOUSE_INPUT_DATA InputDataEnd,
    PULONG InputDataConsumed
)
{
    // 鼠标事件发生时重置时间戳，表示用户有操作
    LastActivityTimestamp = KeQueryInterruptTime();

    // 正常传递鼠标事件给系统
    return STATUS_SUCCESS;
}

VOID InjectMouseMovement(LONG XOffset, LONG YOffset)
{
    MOUSE_INPUT_DATA mouseData;
    mouseData.UnitId = 0;
    mouseData.Flags = MOUSE_MOVE_ABSOLUTE;  // 绝对移动
    mouseData.LastX = XOffset;
    mouseData.LastY = YOffset;
    mouseData.ButtonFlags = 0;
    mouseData.ButtonData = 0;

    MouseClassServiceCallback(NULL, &mouseData, &mouseData + 1, NULL);
}

VOID TimerRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
)
{
    // 获取当前系统时间
    LONG currentTime = KeQueryInterruptTime();

    // 如果超过了指定的空闲时间，则执行鼠标移动
    if (currentTime - LastActivityTimestamp > MOUSE_IDLE_TIMEOUT * 10000)
    {
        DbgPrint("No mouse activity detected. Injecting mouse movement.\n");

        // 模拟鼠标移动，比如向右移动 100 像素，向下移动 50 像素
        InjectMouseMovement(100, 50);

        // 重置计时器
        LastActivityTimestamp = currentTime;
    }
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
    // 设置驱动卸载例程
    pDriverObject->DriverUnload = UnloadDriver;

    // 初始化定时器和 DPC
    KeInitializeTimer(&Timer);
    KeInitializeDpc(&TimerDpc, TimerRoutine, NULL);

    // 设置定时器，每秒检查一次
    LARGE_INTEGER dueTime;
    dueTime.QuadPart = -10000000;  // 1 秒
    KeSetTimerEx(&Timer, dueTime, 1000, &TimerDpc);  // 每秒触发一次

    // 获取当前时间作为最后的活动时间
    LastActivityTimestamp = KeQueryInterruptTime();

    DbgPrint("Driver Loaded\n");
    return STATUS_SUCCESS;
}
