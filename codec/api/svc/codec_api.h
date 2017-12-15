/*!
 *@page License
 *
 * \copy
 *     Copyright (c)  2013, Cisco Systems
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
 */

#ifndef WELS_VIDEO_CODEC_SVC_API_H__
#define WELS_VIDEO_CODEC_SVC_API_H__

#ifndef __cplusplus
#if defined(_MSC_VER) && (_MSC_VER < 1800)
typedef unsigned char bool;
#else
#include <stdbool.h>
#endif
#endif

#include "codec_app_def.h"
#include "codec_def.h"

#if defined(_WIN32) || defined(__cdecl)
#define EXTAPI __cdecl
#else
#define EXTAPI
#endif

/**
  * @file codec_api.h
*/

/**
  * @page Overview
  *   * This page is for openh264 codec API usage.
  *   * For how to use the decoder,please refer to page UsageExampleForDecoder
  *   * For more detail about ISVDecoder,please refer to page ISVCDecoder
*/

/**
  * @page DecoderUsageExample
  *
  * @brief
  *   * An example for using the decoder for Decoding only or Parsing only
  *
  * Step 1:decoder declaration
  * @code
  *
  *  //decoder declaration
  *  ISVCDecoder *pSvcDecoder;
  *  //input: encoded bitstream start position; should include start code prefix
  *  unsigned char *pBuf =...;
  *  //input: encoded bit stream length; should include the size of start code prefix
  *  int iSize =...;
  *  //output: [0~2] for Y,U,V buffer for Decoding only
  *  unsigned char *pData[3] =...;
  *  //in-out: for Decoding only: declare and initialize the output buffer info, this should never co-exist with Parsing only
  *  SBufferInfo sDstBufInfo;
  *  memset(&sDstBufInfo, 0, sizeof(SBufferInfo));
  *  //in-out: for Parsing only: declare and initialize the output bitstream buffer info for parse only, this should never co-exist with Decoding only
  *  SParserBsInfo sDstParseInfo;
  *  memset(&sDstParseInfo, 0, sizeof(SParserBsInfo));
  *  sDstParseInfo.pDstBuff = new unsigned char[PARSE_SIZE]; //In Parsing only, allocate enough buffer to save transcoded bitstream for a frame
  *
  * @endcode
  *
  * Step 2:decoder creation
  * @code
  *  CreateDecoder(pSvcDecoder);
  * @endcode
  *
  * Step 3:declare required parameter, used to differentiate Decoding only and Parsing only
  * @code
  *  SDecodingParam sDecParam = {0};
  *  sDecParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
  *  //for Parsing only, the assignment is mandatory
  * @endcode
  *
  * Step 4:initialize the parameter and decoder context, allocate memory
  * @code
  *  Initialize(&sDecParam);
  * @endcode
  *
  * Step 5:do actual decoding process in slice level;
  *        this can be done in a loop until data ends
  * @code
  *  //for Parsing only
  *  iRet = DecodeParser(pBuf, iSize, &sDstParseInfo);
  *  //decode failed
  *  If (iRet != 0){
  *      RequestIDR or something like that.
  *  }
  *  //for Decoding only, pData can be used for render.
  *  if (sDstBufInfo.iBufferStatus==1){
  *      output pData[0], pData[1], pData[2];
  *  }
  * //for Parsing only, sDstParseInfo can be used for, e.g., HW decoding
  *  if (sDstBufInfo.iNalNum > 0){
  *      Hardware decoding sDstParseInfo;
  *  }
   * @endcode
  *
  * Step 6:uninitialize the decoder and memory free
  * @code
  *  Uninitialize();
  * @endcode
  *
  * Step 7:destroy the decoder
  * @code
  *  DestroyDecoder();
  * @endcode
  *
*/

#ifdef __cplusplus
/**
* @brief Decoder definition
*/
class ISVCDecoder {
 public:

  /**
  * @brief  Initilaize decoder
  * @param  pParam  parameter for decoder
  * @return 0 - success; otherwise - failed;
  */
  virtual long EXTAPI Initialize (const SDecodingParam* pParam) = 0;

  /// Uninitialize the decoder
  virtual long EXTAPI Uninitialize() = 0;

  /**
  * @brief   This function parse input bitstream only, and rewrite possible SVC syntax to AVC syntax
  * @param   pSrc the h264 stream to be decoded
  * @param   iSrcLen the length of h264 stream
  * @param   pDstInfo bit stream info
  * @return  0 - success; otherwise -failed;
  */
  virtual DECODING_STATE EXTAPI DecodeParser (const unsigned char* pSrc,
      const int iSrcLen,
      SParserBsInfo* pDstInfo) = 0;

  /**
  * @brief   Set option for decoder, detail option type, please refer to enumurate DECODER_OPTION.
  * @param   pOption  option for decoder such as OutDataFormat, Eos Flag, EC method, ...
  * @return  CM_RETURN: 0 - success; otherwise - failed;
  */
  virtual long EXTAPI SetOption (DECODER_OPTION eOptionId, void* pOption) = 0;

  /**
  * @brief   Get option for decoder, detail option type, please refer to enumurate DECODER_OPTION.
  * @param   pOption  option for decoder such as OutDataFormat, Eos Flag, EC method, ...
  * @return  CM_RETURN: 0 - success; otherwise - failed;
  */
  virtual long EXTAPI GetOption (DECODER_OPTION eOptionId, void* pOption) = 0;
  virtual ~ISVCDecoder() {}
};


extern "C"
{
#else
typedef struct ISVCDecoderVtbl ISVCDecoderVtbl;
typedef const ISVCDecoderVtbl* ISVCDecoder;
struct ISVCDecoderVtbl {
long (*Initialize) (ISVCDecoder*, const SDecodingParam* pParam);
long (*Uninitialize) (ISVCDecoder*);

DECODING_STATE (*DecodeParser) (ISVCDecoder*, const unsigned char* pSrc,
                                const int iSrcLen,
                                SParserBsInfo* pDstInfo);

long (*SetOption) (ISVCDecoder*, DECODER_OPTION eOptionId, void* pOption);
long (*GetOption) (ISVCDecoder*, DECODER_OPTION eOptionId, void* pOption);
};
#endif

typedef void (*WelsTraceCallback) (void* ctx, int level, const char* string);

/** @brief   Get the capability of decoder
 *  @param   pDecCapability  decoder capability
 *  @return  0 - success; otherwise - failed;
*/
int WelsGetDecoderCapability (SDecoderCapability* pDecCapability);


/** @brief   Create decoder
 *  @param   ppDecoder decoder
 *  @return  0 - success; otherwise - failed;
*/
long WelsCreateDecoder (ISVCDecoder** ppDecoder);


/** @brief   Destroy decoder
 *  @param   pDecoder  decoder
 *  @return  void
*/
void WelsDestroyDecoder (ISVCDecoder* pDecoder);

/** @brief   Get codec version
 *           Note, old versions of Mingw (GCC < 4.7) are buggy and use an
 *           incorrect/different ABI for calling this function, making it
 *           incompatible with MSVC builds.
 *  @return  The linked codec version
*/
OpenH264Version WelsGetCodecVersion (void);

/** @brief   Get codec version
 *  @param   pVersion  struct to fill in with the version
*/
void WelsGetCodecVersionEx (OpenH264Version *pVersion);

#ifdef __cplusplus
}
#endif

#endif//WELS_VIDEO_CODEC_SVC_API_H__
