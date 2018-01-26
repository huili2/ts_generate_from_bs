/*!
 * \copy
 *     Copyright (c)  2009-2013, Cisco Systems
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
 *
 *  welsDecoderExt.cpp
 *
 *  Abstract
 *      Cisco OpenH264 decoder extension utilization
 *
 *  History
 *      3/12/2009 Created
 *
 *
 ************************************************************************/
//#include <assert.h>
#include "PlusExt.h"
#include "Trace.h"
#include "codec_def.h"
#include "typedefs.h"
#include "memory_align.h"
#include "utils.h"
#include "version.h"

//#include "macros.h"
#include "decoder.h"
#include "decoder_core.h"

#include "measure_time.h"
extern "C" {
#include "decoder_core.h"
#include "manage_dec_ref.h"
}
#include "error_code.h"
#include "crt_util_safe_x.h" // Safe CRT routines like util for cross platforms
#include <time.h>
#if defined(_WIN32) /*&& defined(_DEBUG)*/

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#else
#include <sys/time.h>
#endif

namespace WelsDec {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/***************************************************************************
*   Description:
*       class CWelsDecoder constructor function, do initialization  and
*       alloc memory required
*
*   Input parameters: none
*
*   return: none
***************************************************************************/
CWelsDecoder::CWelsDecoder (void)
  : m_pDecContext (NULL),
    m_pWelsTrace (NULL) {
#ifdef OUTPUT_BIT_STREAM
  char chFileName[1024] = { 0 };  //for .264
  int iBufUsed = 0;
  int iBufLeft = 1023;
  int iCurUsed;

  char chFileNameSize[1024] = { 0 }; //for .len
  int iBufUsedSize = 0;
  int iBufLeftSize = 1023;
  int iCurUsedSize;
#endif//OUTPUT_BIT_STREAM


  m_pWelsTrace = new welsCodecTrace();
  if (m_pWelsTrace != NULL) {
    m_pWelsTrace->SetCodecInstance (this);
    m_pWelsTrace->SetTraceLevel (WELS_LOG_ERROR);

    WelsLog (&m_pWelsTrace->m_sLogCtx, WELS_LOG_INFO, "CWelsDecoder::CWelsDecoder() entry");
  }

#ifdef OUTPUT_BIT_STREAM
  SWelsTime sCurTime;

  WelsGetTimeOfDay (&sCurTime);

  iCurUsed     = WelsSnprintf (chFileName,  iBufLeft,  "bs_0x%p_", (void*)this);
  iCurUsedSize = WelsSnprintf (chFileNameSize, iBufLeftSize, "size_0x%p_", (void*)this);

  iBufUsed += iCurUsed;
  iBufLeft -= iCurUsed;
  if (iBufLeft > 0) {
    iCurUsed = WelsStrftime (&chFileName[iBufUsed], iBufLeft, "%y%m%d%H%M%S", &sCurTime);
    iBufUsed += iCurUsed;
    iBufLeft -= iCurUsed;
  }

  iBufUsedSize += iCurUsedSize;
  iBufLeftSize -= iCurUsedSize;
  if (iBufLeftSize > 0) {
    iCurUsedSize = WelsStrftime (&chFileNameSize[iBufUsedSize], iBufLeftSize, "%y%m%d%H%M%S", &sCurTime);
    iBufUsedSize += iCurUsedSize;
    iBufLeftSize -= iCurUsedSize;
  }

  if (iBufLeft > 0) {
    iCurUsed = WelsSnprintf (&chFileName[iBufUsed], iBufLeft, ".%03.3u.264", WelsGetMillisecond (&sCurTime));
    iBufUsed += iCurUsed;
    iBufLeft -= iCurUsed;
  }

  if (iBufLeftSize > 0) {
    iCurUsedSize = WelsSnprintf (&chFileNameSize[iBufUsedSize], iBufLeftSize, ".%03.3u.len",
                                 WelsGetMillisecond (&sCurTime));
    iBufUsedSize += iCurUsedSize;
    iBufLeftSize -= iCurUsedSize;
  }


  m_pFBS = WelsFopen (chFileName, "wb");
  m_pFBSSize = WelsFopen (chFileNameSize, "wb");
#endif//OUTPUT_BIT_STREAM

}

/***************************************************************************
*   Description:
*       class CWelsDecoder destructor function, destroy allocced memory
*
*   Input parameters: none
*
*   return: none
***************************************************************************/
CWelsDecoder::~CWelsDecoder() {
  if (m_pWelsTrace != NULL) {
    WelsLog (&m_pWelsTrace->m_sLogCtx, WELS_LOG_INFO, "CWelsDecoder::~CWelsDecoder()");
  }

  UninitDecoder();

#ifdef OUTPUT_BIT_STREAM
  if (m_pFBS) {
    WelsFclose (m_pFBS);
    m_pFBS = NULL;
  }
  if (m_pFBSSize) {
    WelsFclose (m_pFBSSize);
    m_pFBSSize = NULL;
  }
#endif//OUTPUT_BIT_STREAM

  if (m_pWelsTrace != NULL) {
    delete m_pWelsTrace;
    m_pWelsTrace = NULL;
  }
}

long CWelsDecoder::Initialize (const SDecodingParam* pParam) {
  int iRet = ERR_NONE;
  if (m_pWelsTrace == NULL) {
    return cmMallocMemeError;
  }

  if (pParam == NULL) {
    WelsLog (&m_pWelsTrace->m_sLogCtx, WELS_LOG_ERROR, "CWelsDecoder::Initialize(), invalid input argument.");
    return cmInitParaError;
  }

  // H.264 decoder initialization,including memory allocation,then open it ready to decode
  iRet = InitDecoder (pParam);
  if (iRet)
    return iRet;

  return cmResultSuccess;
}

long CWelsDecoder::Uninitialize() {
  UninitDecoder();

  return ERR_NONE;
}

void CWelsDecoder::UninitDecoder (void) {
  if (NULL == m_pDecContext)
    return;

  WelsLog (&m_pWelsTrace->m_sLogCtx, WELS_LOG_INFO, "CWelsDecoder::UninitDecoder(), openh264 codec version = %s.",
           VERSION_NUMBER);

  WelsEndDecoder (m_pDecContext);

  if (m_pDecContext->pMemAlign != NULL) {
    WelsLog (&m_pWelsTrace->m_sLogCtx, WELS_LOG_INFO,
             "CWelsDecoder::UninitDecoder(), verify memory usage (%d bytes) after free..",
             m_pDecContext->pMemAlign->WelsGetMemoryUsage());
    delete m_pDecContext->pMemAlign;
    m_pDecContext->pMemAlign = NULL;
  }

  if (NULL != m_pDecContext) {
    WelsFree (m_pDecContext, "m_pDecContext");

    m_pDecContext = NULL;
  }

}

// the return value of this function is not suitable, it need report failure info to upper layer.
int32_t CWelsDecoder::InitDecoder (const SDecodingParam* pParam) {

  WelsLog (&m_pWelsTrace->m_sLogCtx, WELS_LOG_INFO,
           "CWelsDecoder::init_decoder(), openh264 codec version = %s", VERSION_NUMBER);

  //reset decoder context
  if (m_pDecContext) //free
    UninitDecoder();
  m_pDecContext = (PWelsDecoderContext)WelsMallocz (sizeof (SWelsDecoderContext), "m_pDecContext");
  if (NULL == m_pDecContext)
    return cmMallocMemeError;
  int32_t iCacheLineSize = 16;   // on chip cache line size in byte
  m_pDecContext->pMemAlign = new CMemoryAlign (iCacheLineSize);
  WELS_VERIFY_RETURN_PROC_IF (cmMallocMemeError, (NULL == m_pDecContext->pMemAlign), UninitDecoder())

  //fill in default value into context
  WelsDecoderDefaults (m_pDecContext, &m_pWelsTrace->m_sLogCtx);

  //check param and update decoder context
  m_pDecContext->pParam = (SDecodingParam*) m_pDecContext->pMemAlign->WelsMallocz (sizeof (SDecodingParam),
                          "SDecodingParam");
  WELS_VERIFY_RETURN_PROC_IF (cmMallocMemeError, (NULL == m_pDecContext->pParam), UninitDecoder());
  int32_t iRet = DecoderConfigParam (m_pDecContext, pParam);
  WELS_VERIFY_RETURN_IFNEQ (iRet, cmResultSuccess);

  //init decoder
  WELS_VERIFY_RETURN_PROC_IF (cmMallocMemeError, WelsInitDecoder (m_pDecContext, &m_pWelsTrace->m_sLogCtx),
                              UninitDecoder())

  return cmResultSuccess;
}

int32_t CWelsDecoder::ResetDecoder() {
  // TBC: need to be modified when context and trace point are null
  if (m_pDecContext != NULL && m_pWelsTrace != NULL) {
    WelsLog (&m_pWelsTrace->m_sLogCtx, WELS_LOG_INFO, "ResetDecoder(), context error code is %d",
             m_pDecContext->iErrorCode);
    SDecodingParam sPrevParam;
    memcpy (&sPrevParam, m_pDecContext->pParam, sizeof (SDecodingParam));

    WELS_VERIFY_RETURN_PROC_IF (cmInitParaError, InitDecoder (&sPrevParam), UninitDecoder());
  } else if (m_pWelsTrace != NULL) {
    WelsLog (&m_pWelsTrace->m_sLogCtx, WELS_LOG_ERROR, "ResetDecoder() failed as decoder context null");
  }
  return ERR_INFO_UNINIT;
}

/*
 * Set Option
 */
long CWelsDecoder::SetOption (DECODER_OPTION eOptID, void* pOption) {
  int iVal = 0;

  if (m_pDecContext == NULL && eOptID != DECODER_OPTION_TRACE_LEVEL &&
      eOptID != DECODER_OPTION_TRACE_CALLBACK && eOptID != DECODER_OPTION_TRACE_CALLBACK_CONTEXT)
    return dsInitialOptExpected;
  if (eOptID == DECODER_OPTION_END_OF_STREAM) { // Indicate bit-stream of the final frame to be decoded
    if (pOption == NULL)
      return cmInitParaError;

    iVal = * ((int*)pOption); // boolean value for whether enabled End Of Stream flag

    m_pDecContext->bEndOfStreamFlag = iVal ? true : false;

    return cmResultSuccess;
  } else if (eOptID == DECODER_OPTION_TRACE_LEVEL) {
    if (m_pWelsTrace) {
      uint32_t level = * ((uint32_t*)pOption);
      m_pWelsTrace->SetTraceLevel (level);
    }
    return cmResultSuccess;
  } else if (eOptID == DECODER_OPTION_TRACE_CALLBACK) {
    if (m_pWelsTrace) {
      WelsTraceCallback callback = * ((WelsTraceCallback*)pOption);
      m_pWelsTrace->SetTraceCallback (callback);
      WelsLog (&m_pWelsTrace->m_sLogCtx, WELS_LOG_INFO,
               "CWelsDecoder::SetOption():DECODER_OPTION_TRACE_CALLBACK callback = %p.",
               callback);
    }
    return cmResultSuccess;
  } else if (eOptID == DECODER_OPTION_TRACE_CALLBACK_CONTEXT) {
    if (m_pWelsTrace) {
      void* ctx = * ((void**)pOption);
      m_pWelsTrace->SetTraceCallbackContext (ctx);
    }
    return cmResultSuccess;
  } else if (eOptID == DECODER_OPTION_GET_STATISTICS) {
    WelsLog (&m_pWelsTrace->m_sLogCtx, WELS_LOG_WARNING,
             "CWelsDecoder::SetOption():DECODER_OPTION_GET_STATISTICS: this option is get-only!");
    return cmInitParaError;
  } else if (eOptID == DECODER_OPTION_STATISTICS_LOG_INTERVAL) {
    if (pOption) {
      m_pDecContext->sDecoderStatistics.iStatisticsLogInterval = (* ((unsigned int*)pOption));
      return cmResultSuccess;
    }
  } else if (eOptID == DECODER_OPTION_GET_SAR_INFO) {
    WelsLog (&m_pWelsTrace->m_sLogCtx, WELS_LOG_WARNING,
             "CWelsDecoder::SetOption():DECODER_OPTION_GET_SAR_INFO: this option is get-only!");
    return cmInitParaError;
  }
  return cmInitParaError;
}

/*
 *  Get Option
 */
long CWelsDecoder::GetOption (DECODER_OPTION eOptID, void* pOption) {
  int iVal = 0;

  if (m_pDecContext == NULL)
    return cmInitExpected;

  if (pOption == NULL)
    return cmInitParaError;

  if (DECODER_OPTION_END_OF_STREAM == eOptID) {
    iVal = m_pDecContext->bEndOfStreamFlag;
    * ((int*)pOption) = iVal;
    return cmResultSuccess;
  }
#ifdef LONG_TERM_REF
  else if (DECODER_OPTION_IDR_PIC_ID == eOptID) {
    iVal = m_pDecContext->uiCurIdrPicId;
    * ((int*)pOption) = iVal;
    return cmResultSuccess;
  } else if (DECODER_OPTION_FRAME_NUM == eOptID) {
    iVal = m_pDecContext->iFrameNum;
    * ((int*)pOption) = iVal;
    return cmResultSuccess;
  } else if (DECODER_OPTION_LTR_MARKING_FLAG == eOptID) {
    iVal = m_pDecContext->bCurAuContainLtrMarkSeFlag;
    * ((int*)pOption) = iVal;
    return cmResultSuccess;
  } else if (DECODER_OPTION_LTR_MARKED_FRAME_NUM == eOptID) {
    iVal = m_pDecContext->iFrameNumOfAuMarkedLtr;
    * ((int*)pOption) = iVal;
    return cmResultSuccess;
  }
#endif
  else if (DECODER_OPTION_VCL_NAL == eOptID) { //feedback whether or not have VCL NAL in current AU
    iVal = m_pDecContext->iFeedbackVclNalInAu;
    * ((int*)pOption) = iVal;
    return cmResultSuccess;
  } else if (DECODER_OPTION_TEMPORAL_ID == eOptID) { //if have VCL NAL in current AU, then feedback the temporal ID
    iVal = m_pDecContext->iFeedbackTidInAu;
    * ((int*)pOption) = iVal;
    return cmResultSuccess;
  } else if (DECODER_OPTION_GET_STATISTICS == eOptID) { // get decoder statistics info for real time debugging
    SDecoderStatistics* pDecoderStatistics = (static_cast<SDecoderStatistics*> (pOption));

    memcpy (pDecoderStatistics, &m_pDecContext->sDecoderStatistics, sizeof (SDecoderStatistics));

    if (m_pDecContext->sDecoderStatistics.uiDecodedFrameCount != 0) { //not original status
      pDecoderStatistics->fAverageFrameSpeedInMs = (float) (m_pDecContext->dDecTime) /
          (m_pDecContext->sDecoderStatistics.uiDecodedFrameCount);
      pDecoderStatistics->fActualAverageFrameSpeedInMs = (float) (m_pDecContext->dDecTime) /
          (m_pDecContext->sDecoderStatistics.uiDecodedFrameCount + m_pDecContext->sDecoderStatistics.uiFreezingIDRNum +
           m_pDecContext->sDecoderStatistics.uiFreezingNonIDRNum);
    }
    return cmResultSuccess;
  } else if (eOptID == DECODER_OPTION_STATISTICS_LOG_INTERVAL) {
    if (pOption) {
      iVal = m_pDecContext->sDecoderStatistics.iStatisticsLogInterval;
      * ((unsigned int*)pOption) = iVal;
      return cmResultSuccess;
    }
  } else if (DECODER_OPTION_GET_SAR_INFO == eOptID) { //get decoder SAR info in VUI
    PVuiSarInfo pVuiSarInfo = (static_cast<PVuiSarInfo> (pOption));
    memset (pVuiSarInfo, 0, sizeof (SVuiSarInfo));
    if (!m_pDecContext->pSps) {
      return cmInitExpected;
    } else {
      pVuiSarInfo->uiSarWidth = m_pDecContext->pSps->sVui.uiSarWidth;
      pVuiSarInfo->uiSarHeight = m_pDecContext->pSps->sVui.uiSarHeight;
      pVuiSarInfo->bOverscanAppropriateFlag = m_pDecContext->pSps->sVui.bOverscanAppropriateFlag;
      return cmResultSuccess;
    }
  } else if (DECODER_OPTION_PROFILE == eOptID) {
    if (!m_pDecContext->pSps) {
      return cmInitExpected;
    }
    iVal = (int) m_pDecContext->pSps->uiProfileIdc;
    * ((int*)pOption) = iVal;
    return cmResultSuccess;
  } else if (DECODER_OPTION_LEVEL == eOptID) {
    if (!m_pDecContext->pSps) {
      return cmInitExpected;
    }
    iVal = (int) m_pDecContext->pSps->uiLevelIdc;
    * ((int*)pOption) = iVal;
    return cmResultSuccess;
  }

  return cmInitParaError;
}

void CWelsDecoder::OutputStatisticsLog (SDecoderStatistics& sDecoderStatistics) {
  if ((sDecoderStatistics.uiDecodedFrameCount > 0) && (sDecoderStatistics.iStatisticsLogInterval > 0)
      && ((sDecoderStatistics.uiDecodedFrameCount % sDecoderStatistics.iStatisticsLogInterval) == 0)) {
    WelsLog (&m_pWelsTrace->m_sLogCtx, WELS_LOG_INFO,
             "DecoderStatistics: uiWidth=%d, uiHeight=%d, fAverageFrameSpeedInMs=%.1f, fActualAverageFrameSpeedInMs=%.1f, \
              uiDecodedFrameCount=%d, uiResolutionChangeTimes=%d, uiIDRCorrectNum=%d, \
              uiAvgEcRatio=%d, uiAvgEcPropRatio=%d, uiEcIDRNum=%d, uiEcFrameNum=%d, \
              uiIDRLostNum=%d, uiFreezingIDRNum=%d, uiFreezingNonIDRNum=%d, iAvgLumaQp=%d, \
              iSpsReportErrorNum=%d, iSubSpsReportErrorNum=%d, iPpsReportErrorNum=%d, iSpsNoExistNalNum=%d, iSubSpsNoExistNalNum=%d, iPpsNoExistNalNum=%d, \
              uiProfile=%d, uiLevel=%d, \
              iCurrentActiveSpsId=%d, iCurrentActivePpsId=%d,",
             sDecoderStatistics.uiWidth,
             sDecoderStatistics.uiHeight,
             sDecoderStatistics.fAverageFrameSpeedInMs,
             sDecoderStatistics.fActualAverageFrameSpeedInMs,

             sDecoderStatistics.uiDecodedFrameCount,
             sDecoderStatistics.uiResolutionChangeTimes,
             sDecoderStatistics.uiIDRCorrectNum,

             sDecoderStatistics.uiAvgEcRatio,
             sDecoderStatistics.uiAvgEcPropRatio,
             sDecoderStatistics.uiEcIDRNum,
             sDecoderStatistics.uiEcFrameNum,

             sDecoderStatistics.uiIDRLostNum,
             sDecoderStatistics.uiFreezingIDRNum,
             sDecoderStatistics.uiFreezingNonIDRNum,
             sDecoderStatistics.iAvgLumaQp,

             sDecoderStatistics.iSpsReportErrorNum,
             sDecoderStatistics.iSubSpsReportErrorNum,
             sDecoderStatistics.iPpsReportErrorNum,
             sDecoderStatistics.iSpsNoExistNalNum,
             sDecoderStatistics.iSubSpsNoExistNalNum,
             sDecoderStatistics.iPpsNoExistNalNum,

             sDecoderStatistics.uiProfile,
             sDecoderStatistics.uiLevel,

             sDecoderStatistics.iCurrentActiveSpsId,
             sDecoderStatistics.iCurrentActivePpsId);
  }
}

DECODING_STATE CWelsDecoder::DecodeParser (const unsigned char* kpSrc,
    const int kiSrcLen,
    SParserBsInfo* pDstInfo) {
  if (m_pDecContext == NULL || m_pDecContext->pParam == NULL) {
    if (m_pWelsTrace != NULL) {
      WelsLog (&m_pWelsTrace->m_sLogCtx, WELS_LOG_ERROR, "Call DecodeParser without Initialize.\n");
    }
    return dsInitialOptExpected;
  }

  if (!m_pDecContext->pParam->bParseOnly) {
    WelsLog (&m_pWelsTrace->m_sLogCtx, WELS_LOG_ERROR, "bParseOnly should be true for this API calling! \n");
    m_pDecContext->iErrorCode |= dsInvalidArgument;
    return dsInvalidArgument;
  }
  int64_t iEnd, iStart = WelsTime();
  if (CheckBsBuffer (m_pDecContext, kiSrcLen)) {
    if (ResetDecoder())
      return dsOutOfMemory;

    return dsErrorFree;
  }
  if (kiSrcLen > 0 && kpSrc != NULL) {
#ifdef OUTPUT_BITSTREAM
    if (m_pFBS) {
      WelsFwrite (kpSrc, sizeof (unsigned char), kiSrcLen, m_pFBS);
      WelsFflush (m_pFBS);
    }
#endif//OUTPUT_BIT_STREAM
    m_pDecContext->bEndOfStreamFlag = false;
  } else {
    //For application MODE, the error detection should be added for safe.
    //But for CONSOLE MODE, when decoding LAST AU, kiSrcLen==0 && kpSrc==NULL.
    m_pDecContext->bEndOfStreamFlag = true;
    m_pDecContext->bInstantDecFlag = true;
  }

  m_pDecContext->iErrorCode = dsErrorFree; //initialize at the starting of AU decoding.
  if (!m_pDecContext->bFramePending) { //frame complete
    m_pDecContext->pParserBsInfo->iNalNum = 0;
  }
  pDstInfo->iNalNum = 0;
  pDstInfo->iSpsWidthInPixel = pDstInfo->iSpsHeightInPixel = 0;
  if (pDstInfo) {
    m_pDecContext->uiTimeStamp = pDstInfo->uiInBsTimeStamp;
    pDstInfo->uiOutBsTimeStamp = 0;
  } else {
    m_pDecContext->uiTimeStamp = 0;
  }
  WelsDecodeBs (m_pDecContext, kpSrc, kiSrcLen, NULL, NULL, pDstInfo);
  if (m_pDecContext->iErrorCode & dsOutOfMemory) {
    if (ResetDecoder())
      return dsOutOfMemory;
    return dsErrorFree;
  }

  if (!m_pDecContext->bFramePending && m_pDecContext->pParserBsInfo->iNalNum) {
    memcpy (pDstInfo, m_pDecContext->pParserBsInfo, sizeof (SParserBsInfo));

    if (m_pDecContext->iErrorCode == ERR_NONE) { //update statistics: decoding frame count
      m_pDecContext->sDecoderStatistics.uiDecodedFrameCount++;
      if (m_pDecContext->sDecoderStatistics.uiDecodedFrameCount == 0) { //exceed max value of uint32_t
        ResetDecStatNums (&m_pDecContext->sDecoderStatistics);
        m_pDecContext->sDecoderStatistics.uiDecodedFrameCount++;
      }
    }
  }

  m_pDecContext->bInstantDecFlag = false; //reset no-delay flag

  if (m_pDecContext->iErrorCode && m_pDecContext->bPrintFrameErrorTraceFlag) {
    WelsLog (&m_pWelsTrace->m_sLogCtx, WELS_LOG_INFO, "decode failed, failure type:%d \n", m_pDecContext->iErrorCode);
    m_pDecContext->bPrintFrameErrorTraceFlag = false;
  }
  iEnd = WelsTime();
  m_pDecContext->dDecTime += (iEnd - iStart) / 1e3;

  return (DECODING_STATE) m_pDecContext->iErrorCode;
}

} // namespace WelsDec


using namespace WelsDec;
/*
*       WelsGetDecoderCapability
*       @return: DecCapability information
*/
int WelsGetDecoderCapability (SDecoderCapability* pDecCapability) {
  memset (pDecCapability, 0, sizeof (SDecoderCapability));
  pDecCapability->iProfileIdc = 66; //Baseline
  pDecCapability->iProfileIop = 0xE0; //11100000b
  pDecCapability->iLevelIdc = 32; //level_idc = 3.2
  pDecCapability->iMaxMbps = 216000; //from level_idc = 3.2
  pDecCapability->iMaxFs = 5120; //from level_idc = 3.2
  pDecCapability->iMaxCpb = 20000; //from level_idc = 3.2
  pDecCapability->iMaxDpb = 20480; //from level_idc = 3.2
  pDecCapability->iMaxBr = 20000; //from level_idc = 3.2
  pDecCapability->bRedPicCap = 0; //not support redundant pic

  return ERR_NONE;
}
/* WINAPI is indeed in prefix due to sync to application layer callings!! */

/*
*   WelsCreateDecoder
*   @return:    success in return 0, otherwise failed.
*/
long WelsCreateDecoder (ISVCDecoder** ppDecoder) {

  if (NULL == ppDecoder) {
    return ERR_INVALID_PARAMETERS;
  }

  *ppDecoder = new CWelsDecoder();

  if (NULL == *ppDecoder) {
    return ERR_MALLOC_FAILED;
  }

  return ERR_NONE;
}

/*
*   WelsDestroyDecoder
*/
void WelsDestroyDecoder (ISVCDecoder* pDecoder) {
  if (NULL != pDecoder) {
    delete (CWelsDecoder*)pDecoder;
  }
}
int GenerateTsFromBs(const char* kpH264FileName, const char* kpOuputFileName, double dFps) {
  FILE* pH264File = NULL;
  FILE* pTsFile = NULL;

  SDecodingParam sDecParam = { 0 };
  ISVCDecoder* pDecoder;
  if (WelsCreateDecoder(&pDecoder) || (NULL == pDecoder)) {
    printf("Create Decoder failed.\n");
    return 1;
  }

  sDecParam.bParseOnly = true;
  sDecParam.uiTargetDqLayer = -1;

  if (pDecoder->Initialize(&sDecParam)) {
    printf("Decoder initialization failed.\n");
    return 1;
  }
  unsigned long long uiTimeStamp = 0;
  int iRet = 0;
  int32_t iSliceSize;
  int32_t iSliceIndex = 0;
  uint8_t* pBuf = NULL;
  uint8_t uiStartCode[4] = { 0, 0, 0, 1 };

  uint8_t* pData[3] = { NULL };
  SParserBsInfo sParserBsInfo;

  int32_t iBufPos = 0;
  int32_t iFileSize;
  int32_t i = 0;
  int32_t iFrameCount = 0;
  int32_t iEndOfStreamFlag = 0;
  //~end for

  if (pDecoder == NULL) return 1;
  if (kpH264FileName) {
    pH264File = fopen(kpH264FileName, "rb");
    if (pH264File == NULL) {
      fprintf(stderr, "Can not open h264 source file, check its legal path related please..\n");
      return 1;
    }
    fprintf(stderr, "H264 source file name: %s..\n", kpH264FileName);
  }
  else {
    fprintf(stderr, "Can not find any h264 bitstream file to read..\n");
    fprintf(stderr, "----------------decoder return------------------------\n");
    return 1;
  }

  if (kpOuputFileName) {
    pTsFile = fopen(kpOuputFileName, "wb");
    if (pTsFile == NULL) {
      fprintf(stderr, "Can not open timestamp file to output result of decoding..\n");
      return 1;
    }
    else
      fprintf(stderr, "output timestamp file name: %s..\n", kpOuputFileName);
	fprintf(pTsFile, "timestamp,NalUnitType,SliceSize\n");
  }
  else {
    fprintf(stderr, "Can not find any output file to write..\n");
    // any options
  }

  printf("------------------------------------------------------\n");

  fseek(pH264File, 0L, SEEK_END);
  iFileSize = (int32_t)ftell(pH264File);
  if (iFileSize <= 0) {
    fprintf(stderr, "Current Bit Stream File is too small, read error!!!!\n");
    goto label_exit;
  }
  fseek(pH264File, 0L, SEEK_SET);

  pBuf = new uint8_t[iFileSize + 4];
  if (pBuf == NULL) {
    fprintf(stderr, "new buffer failed!\n");
    goto label_exit;
  }

  if (fread(pBuf, 1, iFileSize, pH264File) != (uint32_t)iFileSize) {
    fprintf(stderr, "Unable to read whole file\n");
    goto label_exit;
  }

  memcpy(pBuf + iFileSize, &uiStartCode[0], 4); //confirmed_safe_unsafe_usage

  while (true) {

    if (iBufPos >= iFileSize) {
      iEndOfStreamFlag = true;
      if (iEndOfStreamFlag)
        pDecoder->SetOption(DECODER_OPTION_END_OF_STREAM, (void*)&iEndOfStreamFlag);
      break;
    }
    // Read length from file if needed
    if (1) {
      for (i = 0; i < iFileSize; i++) {
        if ((pBuf[iBufPos + i] == 0 && pBuf[iBufPos + i + 1] == 0 && pBuf[iBufPos + i + 2] == 0 && pBuf[iBufPos + i + 3] == 1
          && i > 0) || (pBuf[iBufPos + i] == 0 && pBuf[iBufPos + i + 1] == 0 && pBuf[iBufPos + i + 2] == 1 && i > 0)) {
          break;
        }
      }
      iSliceSize = i;
    }
    if (iSliceSize < 4) { //too small size, no effective data, ignore
      iBufPos += iSliceSize;
      continue;
    }

    pData[0] = NULL;
    pData[1] = NULL;
    pData[2] = NULL;
    uiTimeStamp++;
    memset(&sParserBsInfo, 0, sizeof(SParserBsInfo));
    ENalUnitType eType = (ENalUnitType) ((*(pBuf + iBufPos+3)) & 0x1f);
    iRet = pDecoder->DecodeParser(pBuf + iBufPos, iSliceSize, &sParserBsInfo);
    if (iRet > 1) {
      printf("\nDecodeFrame2() error! bitstream not correct! Stop!\n");
      goto label_exit;
    }


    memset(&sParserBsInfo, 0, sizeof(SParserBsInfo));
    iRet = pDecoder->DecodeParser(NULL, 0, &sParserBsInfo);
    if (iRet > 1) {
      printf("\nDecodeFrame2() error! bitstream not correct! Stop!\n");
      goto label_exit;
    }
    else {
      fprintf(pTsFile, "%.4f,%d,%d\n", iFrameCount/dFps, eType, iSliceSize);
      if (iRet == 0 && sParserBsInfo.iNalNum > 0) {
        ++iFrameCount;
      }
    }


    iBufPos += iSliceSize;
    ++iSliceIndex;
  }

  fprintf(stderr, "-------------------------------------------------------\n");
  fprintf(stderr, "Frames:\t\t%d\n", iFrameCount);
  fprintf(stderr, "-------------------------------------------------------\n");

  // coverity scan uninitial
label_exit:
  if (pBuf) {
    delete[] pBuf;
    pBuf = NULL;
  }
  if (pH264File) {
    fclose(pH264File);
    pH264File = NULL;
  }
  if (pTsFile) {
    fclose(pTsFile);
    pTsFile = NULL;
  }
  pDecoder->Uninitialize();

  WelsDestroyDecoder(pDecoder);

  return 0;
}
