#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>

//#pragma comment(lib,"FltMgr.lib")
#pragma warning(disable:4100)
#pragma warning(disable:4189)

#define MAX_PATH 256

PFLT_FILTER gFilterHandle;
UNICODE_STRING gTargetDir;
UNICODE_STRING gSourceDir;

NTSTATUS SplitUnicodeStringByPosition(PUNICODE_STRING source, USHORT splitPos,
    BOOLEAN includeSeparator, PUNICODE_STRING part1, PUNICODE_STRING part2) {
    USHORT sep_pos = 0;
    if (source == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (includeSeparator) {
        sep_pos = 1;
    }

    // 验证位置
    if (splitPos > source->Length) {
        return STATUS_INVALID_PARAMETER;
    }

    // 根据是否包括分隔符调整分隔位置
    USHORT adjustedSplitPos = splitPos + sep_pos;

    // 计算子字符串的长度
    USHORT part1Length = adjustedSplitPos * sizeof(WCHAR);
    USHORT part2Length = (source->Length / sizeof(WCHAR) - splitPos - 2) * sizeof(WCHAR);

    // 分配内存
    part1 = (PUNICODE_STRING)ExAllocatePoolWithTag(NonPagedPool, sizeof(UNICODE_STRING), 'strg');
    part2 = (PUNICODE_STRING)ExAllocatePoolWithTag(NonPagedPool, sizeof(UNICODE_STRING), 'strg');
    if (part1 == NULL || part2 == NULL) {
        // 内存分配失败
        if (part1) ExFreePool(part1);
        if (part2) ExFreePool(part2);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // 为每个部分分配缓冲区
    part1->Buffer = ExAllocatePoolWithTag(NonPagedPool, part1Length + sizeof(WCHAR), 'strg');
    part2->Buffer = ExAllocatePoolWithTag(NonPagedPool, part2Length + sizeof(WCHAR), 'strg');
    if (part1->Buffer == NULL || part2->Buffer == NULL) {
        // 内存分配失败
        if (part1->Buffer) ExFreePool(part1->Buffer);
        if (*part2->Buffer) ExFreePool(part2->Buffer);
        ExFreePool(part1);
        ExFreePool(part2);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // 初始化长度和最大长度
    part1->Length = part1Length;
    part1->MaximumLength = part1Length + sizeof(WCHAR);
    part2->Length = part2Length;
    part2->MaximumLength = part2Length + sizeof(WCHAR);

    // 复制子字符串
    RtlCopyMemory(part1->Buffer, source->Buffer, part1Length);
    part1->Buffer[part1Length / sizeof(WCHAR)] = L'\0'; // 确保以 null 终止

    RtlCopyMemory(part2->Buffer, source->Buffer + adjustedSplitPos, part2Length);
    part2->Buffer[part2Length / sizeof(WCHAR)] = L'\0'; // 确保以 null 终止

    return STATUS_SUCCESS;
}

// 释放UNICODE_STRING缓冲区
void FreeUnicodeString(PUNICODE_STRING unicodeString) {
    if (unicodeString != NULL) {
        // 释放缓冲区
        if (unicodeString->Buffer != NULL) {
            ExFreePool(unicodeString->Buffer);
        }
        // 释放UNICODE_STRING结构本身
        ExFreePool(unicodeString);
        unicodeString = NULL;
    }
}

// 使用示例
void SplitDevicePath(PUNICODE_STRING source_path, BOOLEAN includeSeparator,
    PUNICODE_STRING devicePath, PUNICODE_STRING filePath) {
    FreeUnicodeString(devicePath);
    FreeUnicodeString(filePath);
    USHORT pos;
    BOOLEAN flag = FALSE;

    if (source_path->Length / sizeof(WCHAR) > 22) {
        for (pos = 22; pos < source_path->Length / sizeof(WCHAR); ++pos) {
            if (source_path->Buffer[pos] == L'\\') {
                flag = TRUE;
                break;
            }
        }
        if (flag) {
            SplitUnicodeStringByPosition(source_path, pos, includeSeparator, &devicePath, &filePath);
        }

    }
    
}

NTSTATUS GetDosDeviceName(
    _In_ PUNICODE_STRING DeviceName,
    _Out_ PUNICODE_STRING DosName
);

NTSTATUS ExtractDevicePath(
    _In_ PUNICODE_STRING FullPath,
    _Out_ PUNICODE_STRING DevicePath
)
{
    ULONG i;
    ULONG devicePathLength = 0;

    // 遍历字符串直到找到第三个反斜杠 (\)
    for (i = 0; i < FullPath->Length / sizeof(WCHAR); i++) {
        if (FullPath->Buffer[i] == L'\\') {
            devicePathLength++;
            if (devicePathLength == 3) {
                break;
            }
        }
    }

    // 如果找到了第三个反斜杠
    if (devicePathLength == 3) {
        DevicePath->Length = (USHORT)(i * sizeof(WCHAR));
        DevicePath->MaximumLength = DevicePath->Length;
        DevicePath->Buffer = FullPath->Buffer;

        return STATUS_SUCCESS;
    }

    // 未找到有效的设备路径
    return STATUS_INVALID_PARAMETER;
}

// Helper function to convert volume path to DOS path
NTSTATUS GetVolumeNameForDosName(PUNICODE_STRING DosName, PUNICODE_STRING VolumeName) {
    NTSTATUS status;
    HANDLE linkHandle;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING linkTarget;
    WCHAR linkTargetBuffer[MAX_PATH];

    // Initialize UNICODE_STRINGs
    RtlInitEmptyUnicodeString(&linkTarget, linkTargetBuffer, sizeof(linkTargetBuffer));

    // Create symbolic link object
    InitializeObjectAttributes(&objectAttributes, DosName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
    status = ZwOpenSymbolicLinkObject(&linkHandle, GENERIC_READ, &objectAttributes);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    // Query the symbolic link
    status = ZwQuerySymbolicLinkObject(linkHandle, &linkTarget, NULL);
    ZwClose(linkHandle);

    if (NT_SUCCESS(status)) {
        RtlCopyUnicodeString(VolumeName, &linkTarget);
    }

    return status;
}

FLT_PREOP_CALLBACK_STATUS
PreCreateOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    UNICODE_STRING dosDeviceName;

    if (Data->Iopb->MajorFunction == IRP_MJ_CREATE) {
        PFLT_FILE_NAME_INFORMATION nameInfo;
        NTSTATUS status;
        UNICODE_STRING devicePath;

        status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP, &nameInfo);
        if (NT_SUCCESS(status)) {
            status = FltParseFileNameInformation(nameInfo);
            if (NT_SUCCESS(status)) {
                ExtractDevicePath(&nameInfo->Name, &devicePath);
                status = GetDosDeviceName(&nameInfo->Name, &dosDeviceName);
                if (NT_SUCCESS(status)) {
                    KdPrint(("Device %wZ is mapped to %wZ\n", &nameInfo->Name, &dosDeviceName));
                }
                else {
                    KdPrint(("Failed to get DOS name for device %wZ: %08x\n", &nameInfo->Name, status));
                }
            }

            // 检查文件是否在C:\test\目录下
            if (RtlPrefixUnicodeString(&gSourceDir, &nameInfo->Name, TRUE)) {
                // 构建新的文件路径
                UNICODE_STRING newPath;
                WCHAR newPathBuffer[MAX_PATH];
                RtlInitEmptyUnicodeString(&newPath, newPathBuffer, sizeof(newPathBuffer));

                RtlAppendUnicodeStringToString(&newPath, &gTargetDir);
                RtlAppendUnicodeToString(&newPath, nameInfo->Name.Buffer + gSourceDir.Length / sizeof(WCHAR));

                // 更新文件路径
                RtlCopyUnicodeString(&Data->Iopb->TargetFileObject->FileName, &newPath);

                KdPrint(("Redirecting file to: %wZ\n", &newPath));
            }

            FltReleaseFileNameInformation(nameInfo);
        }
    }

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

VOID
UnloadDriver(
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
)
{
    UNREFERENCED_PARAMETER(Flags);

    FltUnregisterFilter(gFilterHandle);

    KdPrint(("Minifilter unloaded\n"));
}

const FLT_OPERATION_REGISTRATION Callbacks[] = {

    { IRP_MJ_CREATE,
      0,
      PreCreateOperation,
      NULL},

    { IRP_MJ_OPERATION_END }
};

NTSTATUS GetDosDeviceName(
    _In_ PUNICODE_STRING DeviceName,
    _Out_ PUNICODE_STRING DosName
)
{
    NTSTATUS status;
    HANDLE fileHandle;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;

    // Initialize object attributes
    InitializeObjectAttributes(
        &objectAttributes,
        DeviceName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
    );

    // Open the device
    status = ZwCreateFile(
        &fileHandle,
        FILE_READ_ATTRIBUTES,
        &objectAttributes,
        &ioStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN,
        FILE_NON_DIRECTORY_FILE,
        NULL,
        0
    );

    if (!NT_SUCCESS(status)) {
        KdPrint(("Failed to open device: %08x\n", status));
        return status;
    }

    // Get the file object
    status = ObReferenceObjectByHandle(
        fileHandle,
        FILE_READ_ATTRIBUTES,
        *IoFileObjectType,
        KernelMode,
        (PVOID*)&fileObject,
        NULL
    );

    if (!NT_SUCCESS(status)) {
        ZwClose(fileHandle);
        KdPrint(("Failed to get file object: %08x\n", status));
        return status;
    }

    // Get the device object
    deviceObject = IoGetRelatedDeviceObject(fileObject);

    // Query the DOS device name
    status = IoVolumeDeviceToDosName(deviceObject, DosName);

    // Clean up
    ObDereferenceObject(fileObject);
    ZwClose(fileHandle);

    return status;
}

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    NTSTATUS status = STATUS_SUCCESS;
    FLT_REGISTRATION filterRegistration;
    UNICODE_STRING init_str;
    PUNICODE_STRING volumeDeviceName = NULL;
    PUNICODE_STRING dosDeviceName = NULL;
    UNICODE_STRING dosName;
    WCHAR volumeDeviceNameBuffer[MAX_PATH];

    //RtlInitUnicodeString(&dosDeviceName, L"\\??\\C:");
    //status = GetVolumeNameForDosName(&dosDeviceName, &volumeDeviceName);
    //if (!NT_SUCCESS(status)) {
    //    KdPrint(("Failed to get volume device name: %08x\n", status));
    //    return status;
    //}

    //// 打印 DOS 和设备路径
    //KdPrint(("DOS Name: %wZ\n", &dosDeviceName));
    //KdPrint(("Volume Device Name: %wZ\n", &volumeDeviceName));


    // 设置源目录和目标目录
    // RtlInitUnicodeString(&gSourceDir, L"\\Device\\HarddiskVolume3\\test\\"); // 这个路径应该从volumeDeviceName获取
    // RtlInitUnicodeString(&gTargetDir, L"\\Device\\HarddiskVolume3\\Sandbox\\"); // 这里是你的目标目录

    RtlInitUnicodeString(&init_str, L"\\Device\\HarddiskVolume3\\test\\");

    SplitDevicePath(&init_str, FALSE, dosDeviceName, volumeDeviceName);
    GetDosDeviceName(dosDeviceName, &dosName);

    RtlZeroMemory(&filterRegistration, sizeof(FLT_REGISTRATION));
    filterRegistration.Size = sizeof(FLT_REGISTRATION);
    filterRegistration.Version = FLT_REGISTRATION_VERSION;
    filterRegistration.Flags = 0;
    filterRegistration.ContextRegistration = NULL;
    filterRegistration.OperationRegistration = Callbacks;
    filterRegistration.FilterUnloadCallback = UnloadDriver;

    /*status = FltRegisterFilter(DriverObject, &filterRegistration, &gFilterHandle);
    if (NT_SUCCESS(status)) {
        status = FltStartFiltering(gFilterHandle);
        if (!NT_SUCCESS(status)) {
            FltUnregisterFilter(gFilterHandle);
        }
    }*/

    if (NT_SUCCESS(status)) {
        KdPrint(("Minifilter loaded\n"));
    }
    else {
        KdPrint(("Failed to load Minifilter: %08x\n", status));
    }

    return status;
}