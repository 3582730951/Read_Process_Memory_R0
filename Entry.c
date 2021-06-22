#include <MyFirst.h>




NTSTATUS DriverEntry(PDRIVER_OBJECT pdriver, PUNICODE_STRING reg_path)
{
	NTSTATUS status = STATUS_SUCCESS;
	/*设置分配函数*/
	pdriver->DriverUnload = DrvUload;
	pdriver->MajorFunction[IRP_MJ_CREATE] = DrvCreateClose;
	pdriver->MajorFunction[IRP_MJ_CLOSE] = DrvCreateClose;
	pdriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DrvContol;
	
	/*初始化字符串*/
	RtlInitUnicodeString(&DRIVER_NAME, L"\\Device\\Love24+1 7+1 22+2");
	RtlInitUnicodeString(&SYM_NAME, L"\\??\\Love24+1 7+1 22+2");

	/*创建设备*/
	status=IoCreateDevice(pdriver, 0, &DRIVER_NAME, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &Pdevice);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("Create Device Faild.\n");
		return status;
	}

	/*创建符号链接*/

	status = IoCreateSymbolicLink(&SYM_NAME, &DRIVER_NAME);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("Create Symbolic Link Faild.\n");
		IoDeleteDevice(Pdevice);
		return status;
	}

	/*设置为直接读写方式*/
	pdriver->Flags |= DO_DIRECT_IO;

	/*设置为不初始化设备即无设备句柄*/
	pdriver->Flags &= ~DO_DEVICE_INITIALIZING;

	/*申请一片内存 这里申请4096 请R3调用不要每次I0操作不要超过4096内存不然容易蓝*/
	/*申请的是分页内存*/
	g_read_address = ExAllocatePool(PagedPool, 4096);
	g_write_address = ExAllocatePool(PagedPool, 4096);
	RtlZeroMemory(g_read_address, 4096);
	RtlZeroMemory(g_write_address, 4096);
   /* 这个坑是 WriteMmemory 的关于中断级的   
	这里有坑，MDL是APC中断级 AttchProcess 是pass_level
	如果不取消对象引用来降低中断级会直接让 KeUnstackDetachProcess(&stack); 这条CPU收到
	但是因为中断级高于该内核函数所在的中断级，直接结果就是CPU直接关闭 哎，真多坑
	*/
	return STATUS_SUCCESS;
}

NTSTATUS DrvUload(PDRIVER_OBJECT pdriver)
{
	NTSTATUS status = STATUS_SUCCESS;

	if (pdriver->DeviceObject)
	{
		status=IoDeleteSymbolicLink(&SYM_NAME);
		if (!NT_SUCCESS(status))
		{
			DbgPrint("Delete SymbolicLink Faild.\n");
			return status;
		}
		IoDeleteDevice(Pdevice);
	}

	DbgPrint("DrvUload Success.\n");
	return status;
}

NTSTATUS DrvCreateClose(PDEVICE_OBJECT pdevice, PIRP irp)
{

	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS DrvContol(PDEVICE_OBJECT pdevice, PIRP irp)
{
	/*获取IRP栈位置*/
	PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation(irp);

	/*获取对应控制码*/
	ULONG Code = Stack->Parameters.DeviceIoControl.IoControlCode;

	/*数据结构*/
	struct MyProcessData Data;

	/*返回状态*/
	NTSTATUS Status = STATUS_SUCCESS;

	switch (Code)
	{

	case Read_Info:
	{
		/*复制数据*/
		memcpy(&Data, irp->AssociatedIrp.SystemBuffer, sizeof(Data));
		/*获取缓冲区*/
		PUCHAR Buffer = MmGetSystemAddressForMdl(irp->MdlAddress, NormalPagePriority);
		/*读内存*/
		if (Buffer && Data.SourceAddress)
		{
			Data.pid = (HANDLE)(DWORD)Data.pid;
			BOOL flag = MyReadProcessMemory(Data.pid, Data.SourceAddress, Data.ResultAddress, Data.Size);

			if (flag = TRUE)
			{
				KeFlushIoBuffers(irp->MdlAddress, true, FALSE);
			}
			break;
		}



		
	}
	case  Write_Info:
	{
		/*复制数据*/
		memcpy(&Data, irp->AssociatedIrp.SystemBuffer, sizeof(Data));
		/*获取缓冲区*/
		PUCHAR Buffer = MmGetSystemAddressForMdl(irp->MdlAddress, NormalPagePriority);
		/*读内存*/
		if (Buffer && Data.SourceAddress)
		{
			Data.pid = (HANDLE)(DWORD)Data.pid;
			BOOL flag = MyWriteProcessMemory(Data.pid, Data.SourceAddress, Data.ResultAddress, Data.Size);

			if (flag = TRUE)
			{
				KeFlushIoBuffers(irp->MdlAddress, true, FALSE);
			}
			break;
		}
	}
	}
	
	








	//基本操作
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

BOOL MyReadProcessMemory(HANDLE pid, PVOID address, PVOID bufferaddress, SIZE_T BufferSize)
{
	PEPROCESS process = NULL;
	NTSTATUS status = STATUS_SUCCESS;
	status = PsLookupProcessByProcessId(pid, &process);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("Error 0x1.\n");
		return FALSE;
	}
	PEPROCESS peprocess = IoGetCurrentProcess();
	if (!process)
	{
		DbgPrint("Error  0x2");
		return FALSE;
	}
	KAPC_STATE stack = { 0 };
	KeStackAttachProcess(process, &stack);

	__try
	{
		ProbeForRead(address, (SIZE_T)BufferSize, (ULONG)1);
		RtlZeroMemory(&g_read_address, BufferSize);
		memcpy(&g_read_address, address, (SIZE_T)BufferSize);
	}
	__except (1)
	{
		DbgPrint("Read Memory Faild.\n");
		KeUnstackDetachProcess(&stack);
		return FALSE;
	}
	KeUnstackDetachProcess(&stack);

	__try
	{
		memcpy(&bufferaddress, g_read_address, BufferSize);
	}
	__except (1)
	{
		DbgPrint("Copy Memory Faild.\n");
		return FALSE;
	}
	DbgPrint("Process Id: %d Address: %x BufferSize %x", pid, address, BufferSize);
	return TRUE;
}

BOOL MyWriteProcessMemory(HANDLE pid, PVOID address, PVOID bufferaddress, SIZE_T BufferSize)
{

	PEPROCESS process = NULL;
	NTSTATUS status = STATUS_SUCCESS;
	RtlZeroMemory(&g_write_address, BufferSize);
	process = IoGetCurrentProcess();
	status = PsLookupProcessByProcessId(pid, &process);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("Error 0x1\n");
		return FALSE;
	}

	KAPC_STATE stack = { 0 };
	KeStackAttachProcess(process, &stack);
	PMDL mdl = NULL;

	mdl = MmCreateMdl(NULL, address, 4);
	if (mdl == NULL)
	{
		DbgPrint("Error 0x1\n");
		return FALSE;
	}




	MmBuildMdlForNonPagedPool(mdl);
	/*DbgBreakPoint();*/
	//int* ChangeData = NULL;

	__try
	{
		g_write_address = MmMapLockedPages(mdl, KernelMode);
	}
	__except (1)
	{

		DbgPrint("Memory mapping failed.\n");

		IoFreeMdl(mdl);
		ObDereferenceObject(process);
		KeUnstackDetachProcess(&stack);
		return FALSE;
	}

	RtlCopyMemory(g_write_address, &bufferaddress, BufferSize);
	DbgPrint("进程ID:%d 地址:%x 写入数据:%d", pid, address, *(PVOID*)g_write_address);
	IoFreeMdl(mdl);
	MmUnmapLockedPages(g_write_address, mdl);
	ObDereferenceObject(process);
	KeUnstackDetachProcess(&stack);
	return TRUE;

	return TRUE;
}