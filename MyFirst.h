#ifndef __MYFIRST__H
#define __MYFIRST__H
//先应用一些官方头文件
#include <ntifs.h>
#include <ntddk.h>
#include <windef.h>

//定义R3 R0通讯读写结构体
struct  MyProcessData
{
	HANDLE pid; //读写的进程PID
	PVOID64 SourceAddress; //想要的地址
	PVOID64 ResultAddress;//结果保持地址
	SIZE_T Size;//读写的大小

};

//定义IO控制码 直接在以前代码复制 
/*
	参数1： 文件设备直接填写 FILE_DEVICE_UNKNOWN 不知道是什么设备
	参数2： 自定义的IO控制码 个人建议在0x1024以后，好像0-1024微软用了
	参数3：	设备输出方式，直接填写 METHOD_OUT_DIRECT这个即可
	参数4： 打开权限，直接填写 FILE_ANY_ACCESS 获取全部权限
*/
#define Read_Info CTL_CODE (FILE_DEVICE_UNKNOWN,0x9991,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)
#define Write_Info CTL_CODE (FILE_DEVICE_UNKNOWN,0x9992,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)
/*发布 Relese 版本的时候将该宏进行屏蔽*/
#define LOVEYOU 519+1 1312+2 24+1 7+1 22+2

/*该变量均为全局*/
/* 驱动名字，外部R3链接名 */
UNICODE_STRING DRIVER_NAME, SYM_NAME = { 0 };
/* 创建设备接收的 */
PDEVICE_OBJECT Pdevice;
/*申请读的内存*/
PVOID g_read_address = { 0 };
/*申请写的内存*/
PVOID g_write_address = { 0 };

/*函数声明*/
NTSTATUS DrvUload(PDRIVER_OBJECT pdriver);

NTSTATUS DrvCreateClose(PDEVICE_OBJECT pdevice, PIRP irp);

NTSTATUS DrvContol(PDEVICE_OBJECT pdevice, PIRP irp);

BOOL MyReadProcessMemory(HANDLE pid, PVOID address, PVOID bufferaddress, SIZE_T BufferSize);

BOOL MyWriteProcessMemory(HANDLE pid, PVOID address, PVOID bufferaddress, SIZE_T BufferSize);
#endif
