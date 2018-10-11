/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The MIT License
//
//                                             Copyright (c) 2007-2015 hiyohiyo
/*---------------------------------------------------------------------------*/

#include "stdafx.h"
#include <afxmt.h>

#include "DiskMark.h"
#include "DiskMarkDlg.h"
#include "DiskBench.h"
#include "GetFileVersion.h"

#include <winioctl.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

#pragma warning(disable : 4996)

static CString TestFilePath;
static CString TestFileDir;
static CString DiskSpdExe;

static HANDLE hFile;
static int DiskTestCount;
static UINT64 DiskTestSize;
static void ShowErrorMessage(CString message);
static void Interval(UINT time, LPVOID dlg);

static BOOL Init(LPVOID dlg);
static void DiskSpd(LPVOID dlg, DISK_SPD_CMD cmd);

static UINT Exit(LPVOID dlg);
static void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
static volatile BOOL WaitFlag;

#define DISK_SPD_EXE_32 L"CdmResource\\diskspd\\diskspd32.exe"
#define DISK_SPD_EXE_64 L"CdmResource\\diskspd\\diskspd64.exe"

int ExecAndWait(TCHAR *pszCmd, BOOL bNoWindow)
{
	DebugPrint(L"ExecAndWait() - START");

	DWORD Code;
	BOOL bSuccess;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	if (bNoWindow) {
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
	}
	DebugPrint(L"CreateProcess() - START");

	bSuccess = CreateProcess(
		NULL,	// lpApplicationName
		pszCmd,	// lpCommandLine
		NULL,	// lpProcessAttributes
		NULL,	// lpThreadAttributes
		FALSE,	// bInheritHandle
		0,		// dwCreationFlag
		NULL,	// lpEnvironment
		NULL,	// lpCurrentDirectory
		&si,	// lpStartupInfo
		&pi		// lpProcessInformation
		);
	DebugPrint(L"CreateProcess() - END");
	if (bSuccess != TRUE) return 0;

	DebugPrint(L"WaitForInputIdle()");
	WaitForInputIdle(pi.hProcess, INFINITE);
	DebugPrint(L"WaitForSingleObject()");
	WaitForSingleObject(pi.hProcess, INFINITE);
	DebugPrint(L"GetExitCodeProcess()");
	GetExitCodeProcess(pi.hProcess, &Code);

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	DebugPrint(L"ExecAndWait() - END");

	return Code;
}


void ShowErrorMessage(CString message)
{
	DWORD lastErrorCode = GetLastError();
	CString errorMessage;
	LPVOID lpMessageBuffer;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, lastErrorCode, 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMessageBuffer, 0, NULL);
	errorMessage.Format(_T("0x%08X:%s"), lastErrorCode, lpMessageBuffer);

	AfxMessageBox(message + _T("\r\n") + errorMessage);
	LocalFree( lpMessageBuffer );
}

VOID Interval(LPVOID dlg)
{
	int intervalTime = ((CDiskMarkDlg*) dlg)->m_IntervalTime;
	CString title;

	for (int i = 0; i < intervalTime; i++)
	{
		title.Format(L"Interval Time %d/%d sec", i, intervalTime);
		::PostMessage(((CDiskMarkDlg*) dlg)->GetSafeHwnd(), WM_USER_UPDATE_MESSAGE, (WPARAM) &title, 0);
		Sleep(1000);
		if (!((CDiskMarkDlg*) dlg)->m_DiskBenchStatus)
		{
			return;
		}
	}
}

UINT ExecDiskBenchAll(LPVOID dlg)
{
	if(Init(dlg))
	{
		DiskSpd(dlg, TEST_SEQUENTIAL_READ_MULTI_QT);
		Interval(dlg);
		DiskSpd(dlg, TEST_RANDOM_READ_4K_MULTI_QT);
		Interval(dlg);
		DiskSpd(dlg, TEST_SEQUENTIAL_READ);
		Interval(dlg);
		DiskSpd(dlg, TEST_RANDOM_READ_4K);
		Interval(dlg);
		DiskSpd(dlg, TEST_SEQUENTIAL_WRITE_MULTI_QT);
		Interval(dlg);
		DiskSpd(dlg, TEST_RANDOM_WRITE_4K_MULTI_QT);
		Interval(dlg);
		DiskSpd(dlg, TEST_SEQUENTIAL_WRITE);
		Interval(dlg);
		DiskSpd(dlg, TEST_RANDOM_WRITE_4K);
	}

	return Exit(dlg);
}

UINT ExecDiskBenchSequentialMultiQT(LPVOID dlg)
{
	if(Init(dlg))
	{
		DiskSpd(dlg, TEST_SEQUENTIAL_READ_MULTI_QT);
		Interval(dlg);
		DiskSpd(dlg, TEST_SEQUENTIAL_WRITE_MULTI_QT);
	}
	return Exit(dlg);
}

UINT ExecDiskBenchRandom4KBMultiQT(LPVOID dlg)
{
	if(Init(dlg))
	{
		DiskSpd(dlg, TEST_RANDOM_READ_4K_MULTI_QT);
		Interval(dlg);
		DiskSpd(dlg, TEST_RANDOM_WRITE_4K_MULTI_QT);
	}
	return Exit(dlg);
}
UINT ExecDiskBenchSequential(LPVOID dlg)
{
	if(Init(dlg))
	{
		DiskSpd(dlg, TEST_SEQUENTIAL_READ);
		Interval(dlg);
		DiskSpd(dlg, TEST_SEQUENTIAL_WRITE);
	}
	return Exit(dlg);
}

UINT ExecDiskBenchRandom4KB(LPVOID dlg)
{
	if(Init(dlg))
	{
		DiskSpd(dlg, TEST_RANDOM_READ_4K);
		Interval(dlg);
		DiskSpd(dlg, TEST_RANDOM_WRITE_4K);
	}
	return Exit(dlg);
}

BOOL Init(void* dlg)
{
	BOOL FlagArc;
	BOOL result;
	static CString cstr;
	TCHAR drive;

	ULARGE_INTEGER freeBytesAvailableToCaller;
	ULARGE_INTEGER totalNumberOfBytes;
	ULARGE_INTEGER totalNumberOfFreeBytes;

	// Init m_Ini
	TCHAR *ptrEnd;
	TCHAR temp[MAX_PATH];
	::GetModuleFileName(NULL, temp, MAX_PATH);
	if ((ptrEnd = _tcsrchr(temp, '\\')) != NULL)
	{
		*ptrEnd = '\0';
	}

#ifdef _M_X64
	DiskSpdExe.Format(L"%s\\%s", temp, DISK_SPD_EXE_64);
#else
	DiskSpdExe.Format(L"%s\\%s", temp, DISK_SPD_EXE_32);
#endif

	if (! IsFileExist(DiskSpdExe))
	{
		AfxMessageBox(((CDiskMarkDlg*) dlg)->m_MesDiskSpdNotFound);
		((CDiskMarkDlg*) dlg)->m_DiskBenchStatus = FALSE;
		return FALSE;
	}
	DiskTestCount = ((CDiskMarkDlg*) dlg)->m_IndexTestCount + 1;
	DiskTestSize   = (UINT64)_tstoi(((CDiskMarkDlg*)dlg)->m_ValueTestSize);

	CString RootPath;
	if(((CDiskMarkDlg*)dlg)->m_MaxIndexTestDrive != ((CDiskMarkDlg*)dlg)->m_IndexTestDrive)
	{

		drive = ((CDiskMarkDlg*)dlg)->m_ValueTestDrive.GetAt(0);
		cstr.Format(_T("%C:"), drive);
		GetDiskFreeSpaceEx(cstr, &freeBytesAvailableToCaller, &totalNumberOfBytes, &totalNumberOfFreeBytes);
		if (totalNumberOfBytes.QuadPart < ((ULONGLONG)8 * 1024 * 1024 * 1024)) // < 8 GB
		{
			((CDiskMarkDlg*)dlg)->m_TestDriveInfo.Format(_T("%C: %.1f%% (%.1f/%.1f MiB)"), drive,
				(double)(totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart) / (double)totalNumberOfBytes.QuadPart * 100,
				(totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart) / 1024 / 1024.0,
				totalNumberOfBytes.QuadPart / 1024 / 1024.0);
		}
		else
		{
			((CDiskMarkDlg*)dlg)->m_TestDriveInfo.Format(_T("%C: %.1f%% (%.1f/%.1f GiB)"), drive,
				(double)(totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart) / (double)totalNumberOfBytes.QuadPart * 100,
				(totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart) / 1024 / 1024 / 1024.0,
				totalNumberOfBytes.QuadPart / 1024 / 1024 / 1024.0);
		}
		RootPath.Format(_T("%c:\\"), drive);
	}
	else
	{
		RootPath = ((CDiskMarkDlg*)dlg)->m_TestTargetPath;
		RootPath += L"\\";
	}

	TestFileDir.Format(_T("%sCrystalDiskMark%08X"), RootPath, timeGetTime());
	CreateDirectory(TestFileDir, NULL);
	TestFilePath.Format(_T("%s\\CrystalDiskMark%08X.tmp"), TestFileDir, timeGetTime());

	DWORD FileSystemFlags;
	GetVolumeInformation(RootPath, NULL, NULL, NULL, NULL, &FileSystemFlags, NULL, NULL);
	if(FileSystemFlags & FS_VOL_IS_COMPRESSED)
	{
		FlagArc = TRUE;
	}
	else
	{
		FlagArc = FALSE;
	}

// Check Disk Capacity //
	OSVERSIONINFO osVersionInfo;
	osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osVersionInfo);

	ULARGE_INTEGER FreeBytesAvailableToCaller, TotalNumberOfBytes, TotalNumberOfFreeBytes;
	GetDiskFreeSpaceEx(RootPath, &FreeBytesAvailableToCaller, &TotalNumberOfBytes, &TotalNumberOfFreeBytes);
	if(DiskTestSize > TotalNumberOfFreeBytes.QuadPart / 1024 / 1024 )
	{
		AfxMessageBox(((CDiskMarkDlg*)dlg)->m_MesDiskCapacityError);
		((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
		return FALSE;
	}

// Preapare Test File
	hFile = ::CreateFile(TestFilePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL|FILE_FLAG_NO_BUFFERING|FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(((CDiskMarkDlg*)dlg)->m_MesDiskCreateFileError);
		((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
		return FALSE;
	}

// COMPRESSION_FORMAT_NONE
	USHORT lpInBuffer = COMPRESSION_FORMAT_NONE;
	DWORD lpBytesReturned = 0;
	DeviceIoControl(hFile, FSCTL_SET_COMPRESSION, (LPVOID) &lpInBuffer,
				sizeof(USHORT), NULL, 0, (LPDWORD)&lpBytesReturned, NULL);


	// Fill Test Data
	char* buf = NULL;
	int BufSize;
	int Loop;
	int i;
	DWORD writesize;
	BufSize = 1024 * 1024;
	Loop = (int)DiskTestSize;

	buf = (char*) VirtualAlloc(NULL, BufSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (buf == NULL)
	{
		AfxMessageBox(_T("Failed VirtualAlloc()."));
		((CDiskMarkDlg*) dlg)->m_DiskBenchStatus = FALSE;
		return FALSE;
	}

	DebugPrint(L"Fill Buffer - START");
	if (((CDiskMarkDlg*) dlg)->m_TestData == TEST_DATA_ALL0X00)
	{
		for (i = 0; i < BufSize; i++)
		{
			buf[i] = 0;
		}
	}
	else
	{
		// Compatible with DiskSpd
		for (i = 0; i < BufSize; i++)
		{
			buf[i] = (char) (rand() % 256);
		}
	}
	DebugPrint(L"Fill Buffer - END");

	DebugPrint(L"Fill Test File - START");
	for (i = 0; i < Loop; i++)
	{
		if (((CDiskMarkDlg*) dlg)->m_DiskBenchStatus)
		{
			result = WriteFile(hFile, buf, BufSize, &writesize, NULL);
		}
		else
		{
			CloseHandle(hFile);
			VirtualFree(buf, BufSize, MEM_RELEASE);
			((CDiskMarkDlg*) dlg)->m_DiskBenchStatus = FALSE;
			return FALSE;
		}
	}
	DebugPrint(L"Fill Test File - END");
	VirtualFree(buf, BufSize, MEM_RELEASE);
	CloseHandle(hFile);

	return TRUE;
}

void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if(idEvent == TIMER_ID)
	{
		WaitFlag = FALSE;
		KillTimer(hwnd, idEvent);
	}
}

UINT Exit(void* dlg)
{
	DeleteFile(TestFilePath);
	RemoveDirectory(TestFileDir);

	static CString cstr;
	cstr = _T("");

	if(((CDiskMarkDlg*)dlg)->m_TestData == TEST_DATA_ALL0X00)
	{
		cstr = ALL_0X00_0FILL;
	}
	else
	{
		cstr = _T("");
	}

	::PostMessage(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), WM_USER_UPDATE_MESSAGE, NULL, (LPARAM)&cstr);
	::PostMessage(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), WM_USER_EXIT_BENCHMARK, 0, 0);

	((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
	((CDiskMarkDlg*)dlg)->m_WinThread = NULL;

	return 0;
}

void DiskSpd(void* dlg, DISK_SPD_CMD cmd)
{
	static CString cstr;
	double score;
	double *maxScore;
	CString command;
	CString title;
	CString qt;
	CString option;
	CString bufOption;

	int j;

	if (!((CDiskMarkDlg*) dlg)->m_DiskBenchStatus)
	{
		return;
	}


	if (((CDiskMarkDlg*) dlg)->m_TestData == TEST_DATA_ALL0X00)
	{
		bufOption += L" -Z";
	}
	else
	{
		switch (cmd)
		{
		case TEST_SEQUENTIAL_WRITE_MULTI_QT:
			bufOption += L" -Z128K";
			break;
		case TEST_SEQUENTIAL_WRITE:
			bufOption += L" -Z1M";
			break;
		case TEST_RANDOM_WRITE_4K_MULTI_QT:
		case TEST_RANDOM_WRITE_4K:
			bufOption += L" -Z4K";
			break;
		}
	}

	switch (cmd)
	{
	case TEST_CREATE_FILE:
		title = L"Preparing...";
		::PostMessage(((CDiskMarkDlg*) dlg)->GetSafeHwnd(), WM_USER_UPDATE_MESSAGE, (WPARAM) &title, 0);

		option.Format(L"-c%dM", DiskTestSize);
		option += bufOption;
		command.Format(_T("\"%s\" %s \"%s\""), DiskSpdExe, option, TestFilePath);
		ExecAndWait((TCHAR*) (command.GetString()), TRUE);

		return;
		break;
	case TEST_SEQUENTIAL_READ_MULTI_QT:
		title.Format(L"Sequential Read Multi");
		qt.Format(L"[Q=%d/T=%d]", ((CDiskMarkDlg*) dlg)->m_SequentialMultiQueues, ((CDiskMarkDlg*) dlg)->m_SequentialMultiThreads);
		option.Format(L"-b128K -d5 -o%d -t%d -W0 -S -w0", ((CDiskMarkDlg*) dlg)->m_SequentialMultiQueues, ((CDiskMarkDlg*) dlg)->m_SequentialMultiThreads);
		maxScore = &(((CDiskMarkDlg*) dlg)->m_SequentialReadMultiQTScore);
		break;
	case TEST_SEQUENTIAL_WRITE_MULTI_QT:
		title.Format(L"Sequential Write Multi");
		qt.Format(L"[Q=%d/T=%d]", ((CDiskMarkDlg*) dlg)->m_SequentialMultiQueues, ((CDiskMarkDlg*) dlg)->m_SequentialMultiThreads);
		option.Format(L"-b128K -d5 -o%d -t%d -W0 -S -w100", ((CDiskMarkDlg*) dlg)->m_SequentialMultiQueues, ((CDiskMarkDlg*) dlg)->m_SequentialMultiThreads);
		option += bufOption;
		maxScore = &(((CDiskMarkDlg*) dlg)->m_SequentialWriteMultiQTScore);
		break;
	case TEST_RANDOM_READ_4K_MULTI_QT:
		title.Format(L"Random Read 4KiB Multi");
		qt.Format(L"[Q=%d/T=%d]", ((CDiskMarkDlg*) dlg)->m_RandomMultiQueues, ((CDiskMarkDlg*) dlg)->m_RandomMultiThreads);
		option.Format(L"-b4K -d5 -o%d -t%d -W0 -r -S -w0", ((CDiskMarkDlg*) dlg)->m_RandomMultiQueues, ((CDiskMarkDlg*) dlg)->m_RandomMultiThreads);
		maxScore = &(((CDiskMarkDlg*) dlg)->m_RandomRead4KBMultiQTScore);
		break;
	case TEST_RANDOM_WRITE_4K_MULTI_QT:
		title.Format(L"Random Write 4KiB Multi");
		qt.Format(L"[Q=%d/T=%d]", ((CDiskMarkDlg*) dlg)->m_RandomMultiQueues, ((CDiskMarkDlg*) dlg)->m_RandomMultiThreads);
		option.Format(L"-b4K -d5 -o%d -t%d -W0 -r -S -w100", ((CDiskMarkDlg*) dlg)->m_RandomMultiQueues, ((CDiskMarkDlg*) dlg)->m_RandomMultiThreads);
		option += bufOption;
		maxScore = &(((CDiskMarkDlg*) dlg)->m_RandomWrite4KBMultiQTScore);
		break;
	case TEST_SEQUENTIAL_READ:
		title = L"Sequential Read";
		option = L"-b1M -d5 -o1 -t1 -W0 -S -w0";
		maxScore = &(((CDiskMarkDlg*) dlg)->m_SequentialReadScore);
		break;
	case TEST_SEQUENTIAL_WRITE:
		title = L"Sequential Write";
		option = L"-b1M -d5 -o1 -t1 -W0 -S -w100" + bufOption;
		maxScore = &(((CDiskMarkDlg*) dlg)->m_SequentialWriteScore);
		break;
	case TEST_RANDOM_READ_4K:
		title = L"Random Read 4KiB";
		option = L"-b4K -d5 -o1 -t1 -W0 -r -S -w0";
		maxScore = &(((CDiskMarkDlg*) dlg)->m_RandomRead4KBScore);
		break;
	case TEST_RANDOM_WRITE_4K:
		title = L"Random Write 4KiB";
		option = L"-b4K -d5 -o1 -t1 -W0 -r -S -w100" + bufOption;
		maxScore = &(((CDiskMarkDlg*) dlg)->m_RandomWrite4KBScore);
		break;
	}

	score = 0.0;
	*maxScore = 0.0;
	for (j = 0; j <= DiskTestCount; j++)
	{
		if (j == 0)
		{
			cstr.Format(L"Preparing... %s", title);
		}
		else
		{
			cstr.Format(L"%s [%d/%d]", title, j, DiskTestCount);
		}
		::PostMessage(((CDiskMarkDlg*) dlg)->GetSafeHwnd(), WM_USER_UPDATE_MESSAGE, (WPARAM) &cstr, 0);
		
		command.Format(_T("\"%s\" %s \"%s\""), DiskSpdExe, option, TestFilePath);

		score = ExecAndWait((TCHAR*) (command.GetString()), TRUE) / 10 / 1000.0;

		if (j > 0 && score > *maxScore)
		{
			*maxScore = score;
			::PostMessage(((CDiskMarkDlg*) dlg)->GetSafeHwnd(), WM_USER_UPDATE_SCORE, 0, 0);
		}

		if (!((CDiskMarkDlg*) dlg)->m_DiskBenchStatus)
		{
			return;
		}
	}
	::PostMessage(((CDiskMarkDlg*) dlg)->GetSafeHwnd(), WM_USER_UPDATE_SCORE, 0, 0);
}
