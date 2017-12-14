/*!
 * \copy
 *     Copyright (c)  2004-2013, Cisco Systems
 *     All rights reserved.
 *
 *     Redistribution and use in source and binary forms, with or without
 *     modification, are permitted provided that the following conditions
 *     are met:
 *
 *        * Redistributions of source code must retain the above copyright
 *          notice, this list of conditions and the following disclaimer.
 *
 *        * Redistributions in binary form must reproduce the above copyright
 *          notice, this list of conditions and the following disclaimer in
 *          the documentation and/or other materials provided with the
 *          distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *     "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *     FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *     COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *     BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *     CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *     LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *     ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *     POSSIBILITY OF SUCH DAMAGE.
 *
 * h264dec.cpp:         Wels Decoder Console Implementation file
 */

#if defined (_WIN32)
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <tchar.h>
#else
#include <string.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>

using namespace std;

typedef int (*GENERATE_TS) (const char* , const char*);
GENERATE_TS pFunc;

int main (int iArgC, char* pArgV[]) {
  string strInputFile (""), strOutputFile ("");

  if (iArgC != 3) {
    printf ("usage: TestConsole.exe input.264 out_ts_file\n");
    return 1;
  } else {
    strInputFile = pArgV[1];
    strOutputFile = pArgV[2];

    if (strInputFile.empty()) {
      printf ("No input file specified in configuration file.\n");
      return 1;
    }
  }
#ifdef _DEBUG
  HINSTANCE dllHandle = LoadLibrary("C:\\Users\\wayne\\Documents\\GitHub\\ts_generate_from_264bs\\bin\\Win32\\Debug\\TsGenPlus.dll");
#else
  HINSTANCE dllHandle = LoadLibrary("C:\\Users\\wayne\\Documents\\GitHub\\ts_generate_from_264bs\\bin\\x64\\Release\\TsGenPlus.dll");
#endif
  if (dllHandle) {
    pFunc = (GENERATE_TS) GetProcAddress(dllHandle, "GenerateTsFromBs");
  }

  if (!pFunc) {
    FreeLibrary(dllHandle);
    return 1;
  } else {
    int iRet = pFunc(strInputFile.c_str(), strOutputFile.c_str());
    FreeLibrary(dllHandle);
  }

  return 0;
}
