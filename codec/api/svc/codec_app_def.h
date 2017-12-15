/*!
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



#ifndef WELS_VIDEO_CODEC_APPLICATION_DEFINITION_H__
#define WELS_VIDEO_CODEC_APPLICATION_DEFINITION_H__
/**
  * @file  codec_app_def.h
  * @brief Data and /or structures introduced in Cisco OpenH264 application
*/

#include "codec_def.h"
/* Constants */
#define MAX_TEMPORAL_LAYER_NUM          4
#define MAX_SPATIAL_LAYER_NUM           4
#define MAX_QUALITY_LAYER_NUM           4

#define MAX_LAYER_NUM_OF_FRAME          128
#define MAX_NAL_UNITS_IN_LAYER          128     ///< predetermined here, adjust it later if need

#define MAX_RTP_PAYLOAD_LEN             1000
#define AVERAGE_RTP_PAYLOAD_LEN         800


#define SAVED_NALUNIT_NUM_TMP           ( (MAX_SPATIAL_LAYER_NUM*MAX_QUALITY_LAYER_NUM) + 1 + MAX_SPATIAL_LAYER_NUM )  ///< SPS/PPS + SEI/SSEI + PADDING_NAL
#define MAX_SLICES_NUM_TMP              ( ( MAX_NAL_UNITS_IN_LAYER - SAVED_NALUNIT_NUM_TMP ) / 3 )


#define AUTO_REF_PIC_COUNT  -1          ///< encoder selects the number of reference frame automatically
#define UNSPECIFIED_BIT_RATE 0          ///< to do: add detail comment

/**
 * @brief Struct of OpenH264 version
 */
///
/// E.g. SDK version is 1.2.0.0, major version number is 1, minor version number is 2, and revision number is 0.
typedef struct  _tagVersion {
  unsigned int uMajor;                  ///< The major version number
  unsigned int uMinor;                  ///< The minor version number
  unsigned int uRevision;               ///< The revision number
  unsigned int uReserved;               ///< The reserved number, it should be 0.
} OpenH264Version;

/**
* @brief Decoding status
*/
typedef enum {
  /**
  * Errors derived from bitstream parsing
  */
  dsErrorFree           = 0x00,   ///< bit stream error-free
  dsFramePending        = 0x01,   ///< need more throughput to generate a frame output,
  dsRefLost             = 0x02,   ///< layer lost at reference frame with temporal id 0
  dsBitstreamError      = 0x04,   ///< error bitstreams(maybe broken internal frame) the decoder cared
  dsDepLayerLost        = 0x08,   ///< dependented layer is ever lost
  dsNoParamSets         = 0x10,   ///< no parameter set NALs involved
  dsDataErrorConcealed  = 0x20,   ///< current data error concealed specified

  /**
  * Errors derived from logic level
  */
  dsInvalidArgument     = 0x1000, ///< invalid argument specified
  dsInitialOptExpected  = 0x2000, ///< initializing operation is expected
  dsOutOfMemory         = 0x4000, ///< out of memory due to new request
  /**
  * ANY OTHERS?
  */
  dsDstBufNeedExpan     = 0x8000  ///< actual picture size exceeds size of dst pBuffer feed in decoder, so need expand its size

} DECODING_STATE;

/**
* @brief Option types introduced in decoder application
*/
typedef enum {
  DECODER_OPTION_END_OF_STREAM = 1,     ///< end of stream flag
  DECODER_OPTION_VCL_NAL,               ///< feedback whether or not have VCL NAL in current AU for application layer
  DECODER_OPTION_TEMPORAL_ID,           ///< feedback temporal id for application layer
  DECODER_OPTION_FRAME_NUM,             ///< feedback current decoded frame number
  DECODER_OPTION_IDR_PIC_ID,            ///< feedback current frame belong to which IDR period
  DECODER_OPTION_LTR_MARKING_FLAG,      ///< feedback wether current frame mark a LTR
  DECODER_OPTION_LTR_MARKED_FRAME_NUM,  ///< feedback frame num marked by current Frame
  DECODER_OPTION_ERROR_CON_IDC,         ///< indicate decoder error concealment method
  DECODER_OPTION_TRACE_LEVEL,
  DECODER_OPTION_TRACE_CALLBACK,        ///< a void (*)(void* context, int level, const char* message) function which receives log messages
  DECODER_OPTION_TRACE_CALLBACK_CONTEXT,///< context info of trace callbac

  DECODER_OPTION_GET_STATISTICS,        ///< feedback decoder statistics
  DECODER_OPTION_GET_SAR_INFO,          ///< feedback decoder Sample Aspect Ratio info in Vui
  DECODER_OPTION_PROFILE,               ///< get current AU profile info, only is used in GetOption
  DECODER_OPTION_LEVEL,                 ///< get current AU level info,only is used in GetOption
  DECODER_OPTION_STATISTICS_LOG_INTERVAL,///< set log output interval

} DECODER_OPTION;

/**
* @brief Feedback that whether or not have VCL NAL in current AU
*/
typedef enum {
  FEEDBACK_NON_VCL_NAL = 0,
  FEEDBACK_VCL_NAL,
  FEEDBACK_UNKNOWN_NAL
} FEEDBACK_VCL_NAL_IN_AU;

/**
* @brief Type of layer being encoded
*/
typedef enum {
  NON_VIDEO_CODING_LAYER = 0,
  VIDEO_CODING_LAYER = 1
} LAYER_TYPE;
/**
* @brief Enumerate the type of video bitstream which is provided to decoder
*/
typedef enum {
  VIDEO_BITSTREAM_AVC               = 0,
  VIDEO_BITSTREAM_SVC               = 1,
  VIDEO_BITSTREAM_DEFAULT           = VIDEO_BITSTREAM_SVC
} VIDEO_BITSTREAM_TYPE;

/**
* @brief Enumerate the type of profile id
*/
typedef enum {
  PRO_UNKNOWN   = 0,
  PRO_BASELINE  = 66,
  PRO_MAIN      = 77,
  PRO_EXTENDED  = 88,
  PRO_HIGH      = 100,
  PRO_HIGH10    = 110,
  PRO_HIGH422   = 122,
  PRO_HIGH444   = 144,
  PRO_CAVLC444  = 244,

  PRO_SCALABLE_BASELINE = 83,
  PRO_SCALABLE_HIGH     = 86
} EProfileIdc;

/**
* @brief Enumerate the type of level id
*/
typedef enum {
  LEVEL_UNKNOWN = 0,
  LEVEL_1_0 = 10,
  LEVEL_1_B = 9,
  LEVEL_1_1 = 11,
  LEVEL_1_2 = 12,
  LEVEL_1_3 = 13,
  LEVEL_2_0 = 20,
  LEVEL_2_1 = 21,
  LEVEL_2_2 = 22,
  LEVEL_3_0 = 30,
  LEVEL_3_1 = 31,
  LEVEL_3_2 = 32,
  LEVEL_4_0 = 40,
  LEVEL_4_1 = 41,
  LEVEL_4_2 = 42,
  LEVEL_5_0 = 50,
  LEVEL_5_1 = 51,
  LEVEL_5_2 = 52
} ELevelIdc;

/**
* @brief Enumerate the type of wels log
*/
enum {
  WELS_LOG_QUIET       = 0x00,          ///< quiet mode
  WELS_LOG_ERROR       = 1 << 0,        ///< error log iLevel
  WELS_LOG_WARNING     = 1 << 1,        ///< Warning log iLevel
  WELS_LOG_INFO        = 1 << 2,        ///< information log iLevel
  WELS_LOG_DEBUG       = 1 << 3,        ///< debug log, critical algo log
  WELS_LOG_DETAIL      = 1 << 4,        ///< per packet/frame log
  WELS_LOG_RESV        = 1 << 5,        ///< resversed log iLevel
  WELS_LOG_LEVEL_COUNT = 6,
  WELS_LOG_DEFAULT     = WELS_LOG_WARNING   ///< default log iLevel in Wels codec
};

/**
* @brief Enumerate the type of sample aspect ratio
*/
typedef enum {
  ASP_UNSPECIFIED = 0,
  ASP_1x1 = 1,
  ASP_12x11 = 2,
  ASP_10x11 = 3,
  ASP_16x11 = 4,
  ASP_40x33 = 5,
  ASP_24x11 = 6,
  ASP_20x11 = 7,
  ASP_32x11 = 8,
  ASP_80x33 = 9,
  ASP_18x11 = 10,
  ASP_15x11 = 11,
  ASP_64x33 = 12,
  ASP_160x99 = 13,
  
  ASP_EXT_SAR = 255
} ESampleAspectRatio;


/**
* @brief Define a new struct to show the property of video bitstream.
*/
typedef struct {
  unsigned int          size;          ///< size of the struct
  VIDEO_BITSTREAM_TYPE  eVideoBsType;  ///< video stream type (AVC/SVC)
} SVideoProperty;

/**
* @brief SVC Decoding Parameters, reserved here and potential applicable in the future
*/
typedef struct TagSVCDecodingParam {
  char*     pFileNameRestructed;       ///< file name of reconstructed frame used for PSNR calculation based debug

  unsigned int  uiCpuLoad;             ///< CPU load
  unsigned char uiTargetDqLayer;       ///< setting target dq layer id

  bool bParseOnly;                     ///< decoder for parse only, no reconstruction. When it is true, SPS/PPS size should not exceed SPS_PPS_BS_SIZE (128). Otherwise, it will return error info

  SVideoProperty   sVideoProperty;    ///< video stream property
} SDecodingParam, *PDecodingParam;


/**
* @brief The capability of decoder, for SDP negotiation
*/
typedef struct TagDecoderCapability {
  int iProfileIdc;     ///< profile_idc
  int iProfileIop;     ///< profile-iop
  int iLevelIdc;       ///< level_idc
  int iMaxMbps;        ///< max-mbps
  int iMaxFs;          ///< max-fs
  int iMaxCpb;         ///< max-cpb
  int iMaxDpb;         ///< max-dpb
  int iMaxBr;          ///< max-br
  bool bRedPicCap;     ///< redundant-pic-cap
} SDecoderCapability;

/**
* @brief Structure for parse only output
*/
typedef struct TagParserBsInfo {
  int iNalNum;                                 ///< total NAL number in current AU
  unsigned char* pDstBuff;                     ///< outputted dst buffer for parsed bitstream
  int iSpsWidthInPixel;                        ///< required SPS width info
  int iSpsHeightInPixel;                       ///< required SPS height info
  unsigned long long uiInBsTimeStamp;               ///< input BS timestamp
  unsigned long long uiOutBsTimeStamp;             ///< output BS timestamp
} SParserBsInfo, *PParserBsInfo;


/**
* @brief  Structure for decoder statistics
*/
typedef struct TagVideoDecoderStatistics {
  unsigned int uiWidth;                        ///< the width of encode/decode frame
  unsigned int uiHeight;                       ///< the height of encode/decode frame
  float fAverageFrameSpeedInMs;                ///< average_Decoding_Time
  float fActualAverageFrameSpeedInMs;          ///< actual average_Decoding_Time, including freezing pictures
  unsigned int uiDecodedFrameCount;            ///< number of frames
  unsigned int uiResolutionChangeTimes;        ///< uiResolutionChangeTimes
  unsigned int uiIDRCorrectNum;                ///< number of correct IDR received
  //EC on related
  unsigned int
  uiAvgEcRatio;                                ///< when EC is on, the average ratio of total EC areas, can be an indicator of reconstruction quality
  unsigned int
  uiAvgEcPropRatio;                            ///< when EC is on, the rough average ratio of propogate EC areas, can be an indicator of reconstruction quality
  unsigned int uiEcIDRNum;                     ///< number of actual unintegrity IDR or not received but eced
  unsigned int uiEcFrameNum;                   ///<
  unsigned int uiIDRLostNum;                   ///< number of whole lost IDR
  unsigned int uiFreezingIDRNum;               ///< number of freezing IDR with error (partly received), under resolution change
  unsigned int uiFreezingNonIDRNum;            ///< number of freezing non-IDR with error
  int iAvgLumaQp;                              ///< average luma QP. default: -1, no correct frame outputted
  int iSpsReportErrorNum;                      ///< number of Sps Invalid report
  int iSubSpsReportErrorNum;                   ///< number of SubSps Invalid report
  int iPpsReportErrorNum;                      ///< number of Pps Invalid report
  int iSpsNoExistNalNum;                       ///< number of Sps NoExist Nal
  int iSubSpsNoExistNalNum;                    ///< number of SubSps NoExist Nal
  int iPpsNoExistNalNum;                       ///< number of Pps NoExist Nal

  unsigned int uiProfile;                ///< Profile idc in syntax
  unsigned int uiLevel;                  ///< level idc according to Annex A-1

  int iCurrentActiveSpsId;                     ///< current active SPS id
  int iCurrentActivePpsId;                     ///< current active PPS id

  unsigned int iStatisticsLogInterval;                  ///< frame interval of statistics log
} SDecoderStatistics; // in building, coming soon

/**
* @brief Structure for sample aspect ratio (SAR) info in VUI
*/
typedef struct TagVuiSarInfo {
  unsigned int uiSarWidth;                     ///< SAR width
  unsigned int uiSarHeight;                    ///< SAR height
  bool bOverscanAppropriateFlag;               ///< SAR overscan flag
} SVuiSarInfo, *PVuiSarInfo;

#endif//WELS_VIDEO_CODEC_APPLICATION_DEFINITION_H__
