/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "launcher.hxx"


#ifndef _WINDOWS_
#	define WIN32_LEAN_AND_MEAN
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#	include <windows.h>
#	include <shellapi.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#endif


#include <stdlib.h>
#include <malloc.h>


#ifdef __MINGW32__
extern "C" int APIENTRY WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
#else
extern "C" int APIENTRY _tWinMain( HINSTANCE, HINSTANCE, LPTSTR, int )
#endif
{
	// Retreive startup info

	STARTUPINFO	aStartupInfo;

	ZeroMemory( &aStartupInfo, sizeof(aStartupInfo) );
	aStartupInfo.cb = sizeof( aStartupInfo );
	GetStartupInfo( &aStartupInfo );

	// Retrieve command line

	LPTSTR	lpCommandLine = GetCommandLine();

	LPTSTR	*ppArguments = NULL;
	int		nArguments = 0;

	ppArguments = GetArgv( &nArguments );

    // if ( 1 == nArguments )
	{
		lpCommandLine = (LPTSTR)_alloca( sizeof(_TCHAR) * (_tcslen(lpCommandLine) + _tcslen(APPLICATION_SWITCH) + 2) );

		_tcscpy( lpCommandLine, GetCommandLine() );
		_tcscat( lpCommandLine, _T(" ") );
		_tcscat( lpCommandLine, APPLICATION_SWITCH );
	}


	// Calculate application name

	TCHAR	szApplicationName[MAX_PATH];
	TCHAR	szDrive[MAX_PATH];
	TCHAR	szDir[MAX_PATH];
	TCHAR	szFileName[MAX_PATH];
	TCHAR	szExt[MAX_PATH];

	GetModuleFileName( NULL, szApplicationName, MAX_PATH );
	_tsplitpath( szApplicationName, szDrive, szDir, szFileName, szExt );
	_tmakepath( szApplicationName, szDrive, szDir, OFFICE_IMAGE_NAME, _T(".exe") );

	PROCESS_INFORMATION	aProcessInfo;

	BOOL	fSuccess = CreateProcess(
		szApplicationName,
		lpCommandLine,
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		NULL,
		&aStartupInfo,
		&aProcessInfo );

	if ( fSuccess )
	{
		// Wait for soffice process to be terminated to allow other applications
		// to wait for termination of started process

		WaitForSingleObject( aProcessInfo.hProcess, INFINITE );

		CloseHandle( aProcessInfo.hProcess );
		CloseHandle( aProcessInfo.hThread );

		return 0;
	}

	DWORD	dwError = GetLastError();

	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);

	// Display the string.
	MessageBox( NULL, (LPCTSTR)lpMsgBuf, NULL, MB_OK | MB_ICONERROR );

	// Free the buffer.
	LocalFree( lpMsgBuf );

	return GetLastError();
}

