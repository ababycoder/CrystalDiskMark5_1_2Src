/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The MIT License
//
//                                             Copyright (c) 2007-2015 hiyohiyo
/*---------------------------------------------------------------------------*/

#pragma once

#define WM_USER_UPDATE_SCORE	(WM_USER+1)
#define WM_USER_UPDATE_MESSAGE	(WM_USER+2)
#define WM_USER_EXIT_BENCHMARK	(WM_USER+3)

#define TIMER_ID 5963

// Common Message
#define ALL_0X00_0FILL _T("<0Fill>")

enum TEST_DATA_TYPE
{
	TEST_DATA_RANDOM = 0,
	TEST_DATA_ALL0X00,
	TEST_DATA_ALL0XFF,
};

enum DISK_SPD_CMD
{
	TEST_CREATE_FILE,
	TEST_DELETE_FILE,
	TEST_SEQUENTIAL_READ_MULTI_QT,
	TEST_SEQUENTIAL_WRITE_MULTI_QT,
	TEST_RANDOM_READ_4K_MULTI_QT,
	TEST_RANDOM_WRITE_4K_MULTI_QT,
	TEST_SEQUENTIAL_READ,
	TEST_SEQUENTIAL_WRITE,
	TEST_RANDOM_READ_4K,
	TEST_RANDOM_WRITE_4K,
};

UINT ExecDiskBenchAll(void* dlg);
UINT ExecDiskBenchSequentialMultiQT(void* dlg);
UINT ExecDiskBenchRandom4KBMultiQT(void* dlg);
UINT ExecDiskBenchSequential(void* dlg);
UINT ExecDiskBenchRandom4KB(void* dlg);
