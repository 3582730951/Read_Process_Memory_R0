#ifndef __MYFIRST__H
#define __MYFIRST__H
//��Ӧ��һЩ�ٷ�ͷ�ļ�
#include <ntifs.h>
#include <ntddk.h>
#include <windef.h>

//����R3 R0ͨѶ��д�ṹ��
struct  MyProcessData
{
	HANDLE pid; //��д�Ľ���PID
	PVOID64 SourceAddress; //��Ҫ�ĵ�ַ
	PVOID64 ResultAddress;//������ֵ�ַ
	SIZE_T Size;//��д�Ĵ�С

};

//����IO������ ֱ������ǰ���븴�� 
/*
	����1�� �ļ��豸ֱ����д FILE_DEVICE_UNKNOWN ��֪����ʲô�豸
	����2�� �Զ����IO������ ���˽�����0x1024�Ժ󣬺���0-1024΢������
	����3��	�豸�����ʽ��ֱ����д METHOD_OUT_DIRECT�������
	����4�� ��Ȩ�ޣ�ֱ����д FILE_ANY_ACCESS ��ȡȫ��Ȩ��
*/
#define Read_Info CTL_CODE (FILE_DEVICE_UNKNOWN,0x9991,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)
#define Write_Info CTL_CODE (FILE_DEVICE_UNKNOWN,0x9992,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)
/*���� Relese �汾��ʱ�򽫸ú��������*/
#define LOVEYOU 519+1 1312+2 24+1 7+1 22+2

/*�ñ�����Ϊȫ��*/
/* �������֣��ⲿR3������ */
UNICODE_STRING DRIVER_NAME, SYM_NAME = { 0 };
/* �����豸���յ� */
PDEVICE_OBJECT Pdevice;
/*��������ڴ�*/
PVOID g_read_address = { 0 };
/*����д���ڴ�*/
PVOID g_write_address = { 0 };

/*��������*/
NTSTATUS DrvUload(PDRIVER_OBJECT pdriver);

NTSTATUS DrvCreateClose(PDEVICE_OBJECT pdevice, PIRP irp);

NTSTATUS DrvContol(PDEVICE_OBJECT pdevice, PIRP irp);

BOOL MyReadProcessMemory(HANDLE pid, PVOID address, PVOID bufferaddress, SIZE_T BufferSize);

BOOL MyWriteProcessMemory(HANDLE pid, PVOID address, PVOID bufferaddress, SIZE_T BufferSize);
#endif
