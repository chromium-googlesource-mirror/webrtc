/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/*
 * Implementation of main NetEQ API.
 */

#include "webrtc_neteq.h"
#include "webrtc_neteq_internal.h"

#include <assert.h>
#include <string.h>

#include "typedefs.h"
#include "signal_processing_library.h"

#include "neteq_error_codes.h"
#include "mcu_dsp_common.h"
#include "rtcp.h"

#define RETURN_ON_ERROR( macroExpr, macroInstPtr )  { \
    if ((macroExpr) != 0) { \
    if ((macroExpr) == -1) { \
    (macroInstPtr)->ErrorCode = - (NETEQ_OTHER_ERROR); \
    } else { \
    (macroInstPtr)->ErrorCode = -((WebRtc_Word16) (macroExpr)); \
    } \
    return(-1); \
    } }

int WebRtcNetEQ_strncpy(WebRtc_Word8 *strDest, int numberOfElements,
                        const WebRtc_Word8 *strSource, int count)
{
    /* check vector lengths */
    if (count > numberOfElements)
    {
        strDest[0] = '\0';
        return (-1);
    }
    else
    {
        strncpy(strDest, strSource, count);
        return (0);
    }
}

/**********************************************************
 * NETEQ Functions
 */

/*****************************************
 * Info functions
 */

int WebRtcNetEQ_GetVersion(WebRtc_Word8 *version)
{
    char versionString[] = "3.3.0\0    ";
    char endChar[] = " ";
    int i = 0;
    while ((versionString[i] != endChar[0]) && (i <= 20))
    {
        version[i] = versionString[i]; /* To avoid using strcpy */
        i++;
    }
    return (0);
}

int WebRtcNetEQ_GetErrorCode(void *inst)
{
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
    if (NetEqMainInst == NULL) return (-1);
    return (NetEqMainInst->ErrorCode);
}

int WebRtcNetEQ_GetErrorName(int errorCode, WebRtc_Word8 *errorName, int maxStrLen)
{
    if ((errorName == NULL) || (maxStrLen <= 0))
    {
        return (-1);
    }

    if (errorCode < 0)
    {
        errorCode = -errorCode; // absolute value
    }

    switch (errorCode)
    {
        case 1: // could be -1
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "OTHER_ERROR", maxStrLen);
            break;
        }
        case 1001:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "FAULTY_INSTRUCTION", maxStrLen);
            break;
        }
        case 1002:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "FAULTY_NETWORK_TYPE", maxStrLen);
            break;
        }
        case 1003:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "FAULTY_DELAYVALUE", maxStrLen);
            break;
        }
        case 1004:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "FAULTY_PLAYOUTMODE", maxStrLen);
            break;
        }
        case 1005:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "CORRUPT_INSTANCE", maxStrLen);
            break;
        }
        case 1006:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "ILLEGAL_MASTER_SLAVE_SWITCH", maxStrLen);
            break;
        }
        case 1007:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "MASTER_SLAVE_ERROR", maxStrLen);
            break;
        }
        case 2001:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "UNKNOWN_BUFSTAT_DECISION", maxStrLen);
            break;
        }
        case 2002:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "RECOUT_ERROR_DECODING", maxStrLen);
            break;
        }
        case 2003:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "RECOUT_ERROR_SAMPLEUNDERRUN", maxStrLen);
            break;
        }
        case 2004:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "RECOUT_ERROR_DECODED_TOO_MUCH",
                maxStrLen);
            break;
        }
        case 3001:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "RECIN_CNG_ERROR", maxStrLen);
            break;
        }
        case 3002:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "RECIN_UNKNOWNPAYLOAD", maxStrLen);
            break;
        }
        case 3003:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "RECIN_BUFFERINSERT_ERROR", maxStrLen);
            break;
        }
        case 4001:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "PBUFFER_INIT_ERROR", maxStrLen);
            break;
        }
        case 4002:
        case 4003:
        case 4004:
        case 4005:
        case 4006:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "PBUFFER_INSERT_ERROR1", maxStrLen);
            break;
        }
        case 4007:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "UNKNOWN_G723_HEADER", maxStrLen);
            break;
        }
        case 4008:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "PBUFFER_NONEXISTING_PACKET", maxStrLen);
            break;
        }
        case 4009:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "PBUFFER_NOT_INITIALIZED", maxStrLen);
            break;
        }
        case 4010:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "AMBIGUOUS_ILBC_FRAME_SIZE", maxStrLen);
            break;
        }
        case 5001:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "CODEC_DB_FULL", maxStrLen);
            break;
        }
        case 5002:
        case 5003:
        case 5004:
        case 5005:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "CODEC_DB_NOT_EXIST", maxStrLen);
            break;
        }
        case 5006:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "CODEC_DB_UNKNOWN_CODEC", maxStrLen);
            break;
        }
        case 5007:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "CODEC_DB_PAYLOAD_TAKEN", maxStrLen);
            break;
        }
        case 5008:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "CODEC_DB_UNSUPPORTED_CODEC", maxStrLen);
            break;
        }
        case 5009:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "CODEC_DB_UNSUPPORTED_FS", maxStrLen);
            break;
        }
        case 6001:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "DTMF_DEC_PARAMETER_ERROR", maxStrLen);
            break;
        }
        case 6002:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "DTMF_INSERT_ERROR", maxStrLen);
            break;
        }
        case 6003:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "DTMF_GEN_UNKNOWN_SAMP_FREQ", maxStrLen);
            break;
        }
        case 6004:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "DTMF_NOT_SUPPORTED", maxStrLen);
            break;
        }
        case 7001:
        case 7002:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "RED_SPLIT_ERROR", maxStrLen);
            break;
        }
        case 7003:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "RTP_TOO_SHORT_PACKET", maxStrLen);
            break;
        }
        case 7004:
        {
            WebRtcNetEQ_strncpy(errorName, maxStrLen, "RTP_CORRUPT_PACKET", maxStrLen);
            break;
        }
        default:
        {
            /* check for decoder error ranges */
            if (errorCode >= 6010 && errorCode <= 6810)
            {
                /* iSAC error code */
                WebRtcNetEQ_strncpy(errorName, maxStrLen, "iSAC ERROR", maxStrLen);
                break;
            }

            WebRtcNetEQ_strncpy(errorName, maxStrLen, "UNKNOWN_ERROR", maxStrLen);
            return (-1);
        }
    }

    return (0);
}

/* Assign functions (create not allowed in order to avoid malloc in lib) */
int WebRtcNetEQ_AssignSize(int *sizeinbytes)
{
    *sizeinbytes = (sizeof(MainInst_t) * 2) / sizeof(WebRtc_Word16);
    return (0);
}

int WebRtcNetEQ_Assign(void **inst, void *NETEQ_inst_Addr)
{
    int ok = 0;
    MainInst_t *NetEqMainInst = (MainInst_t*) NETEQ_inst_Addr;
    *inst = NETEQ_inst_Addr;
    if (*inst == NULL) return (-1);
    /* Clear memory */
    WebRtcSpl_MemSetW16((WebRtc_Word16*) NetEqMainInst, 0,
        (sizeof(MainInst_t) / sizeof(WebRtc_Word16)));
    ok = WebRtcNetEQ_McuReset(&NetEqMainInst->MCUinst);
    if (ok != 0)
    {
        NetEqMainInst->ErrorCode = -ok;
        return (-1);
    }
    return (0);
}

int WebRtcNetEQ_GetRecommendedBufferSize(void *inst, const enum WebRtcNetEQDecoder *codec,
                                         int noOfCodecs, enum WebRtcNetEQNetworkType nwType,
                                         int *MaxNoOfPackets, int *sizeinbytes)
{
    int ok = 0;
    int multiplier;
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
    if (NetEqMainInst == NULL) return (-1);
    *MaxNoOfPackets = 0;
    *sizeinbytes = 0;
    ok = WebRtcNetEQ_GetDefaultCodecSettings(codec, noOfCodecs, sizeinbytes, MaxNoOfPackets);
    if (ok != 0)
    {
        NetEqMainInst->ErrorCode = -ok;
        return (-1);
    }
    if (nwType == kUDPNormal)
    {
        multiplier = 1;
    }
    else if (nwType == kUDPVideoSync)
    {
        multiplier = 4;
    }
    else if (nwType == kTCPNormal)
    {
        multiplier = 4;
    }
    else if (nwType == kTCPLargeJitter)
    {
        multiplier = 8;
    }
    else if (nwType == kTCPXLargeJitter)
    {
        multiplier = 20;
    }
    else
    {
        NetEqMainInst->ErrorCode = -FAULTY_NETWORK_TYPE;
        return (-1);
    }
    *MaxNoOfPackets = (*MaxNoOfPackets) * multiplier;
    *sizeinbytes = (*sizeinbytes) * multiplier;
    if (ok != 0)
    {
        NetEqMainInst->ErrorCode = -ok;
        return (-1);
    }
    return (ok);
}

int WebRtcNetEQ_AssignBuffer(void *inst, int MaxNoOfPackets, void *NETEQ_Buffer_Addr,
                             int sizeinbytes)
{
    int ok;
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
    if (NetEqMainInst == NULL) return (-1);
    ok = WebRtcNetEQ_PacketBufferInit(&NetEqMainInst->MCUinst.PacketBuffer_inst,
        MaxNoOfPackets, (WebRtc_Word16*) NETEQ_Buffer_Addr, (sizeinbytes >> 1));
    if (ok != 0)
    {
        NetEqMainInst->ErrorCode = -ok;
        return (-1);
    }
    return (ok);
}

/************************************************
 * Init functions
 */

/****************************************************************************
 * WebRtcNetEQ_Init(...)
 *
 * Initialize NetEQ.
 *
 * Input:
 *      - inst          : NetEQ instance
 *		- fs            : Initial sample rate in Hz (may change with payload)
 *
 * Output:
 *		- inst	        : Initialized NetEQ instance
 *
 * Return value			:  0 - Ok
 *						  -1 - Error
 */

int WebRtcNetEQ_Init(void *inst, WebRtc_UWord16 fs)
{
    int ok = 0;

    /* Typecast inst to internal instance format */
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;

    if (NetEqMainInst == NULL)
    {
        return (-1);
    }

#ifdef NETEQ_VAD
    /* Start out with no PostDecode VAD instance */
    NetEqMainInst->DSPinst.VADInst.VADState = NULL;
    /* Also set all VAD function pointers to NULL */
    NetEqMainInst->DSPinst.VADInst.initFunction = NULL;
    NetEqMainInst->DSPinst.VADInst.setmodeFunction = NULL;
    NetEqMainInst->DSPinst.VADInst.VADFunction = NULL;
#endif /* NETEQ_VAD */

    ok = WebRtcNetEQ_DSPinit(NetEqMainInst); /* Init addresses between MCU and DSP */
    RETURN_ON_ERROR(ok, NetEqMainInst);

    ok = WebRtcNetEQ_DSPInit(&NetEqMainInst->DSPinst, fs); /* Init dsp side */
    RETURN_ON_ERROR(ok, NetEqMainInst);
    /* set BGN mode to default, since it is not cleared by DSP init function */
    NetEqMainInst->DSPinst.BGNInst.bgnMode = BGN_ON;

    /* init statistics functions and counters */
    ok = WebRtcNetEQ_ClearInCallStats(&NetEqMainInst->DSPinst);
    RETURN_ON_ERROR(ok, NetEqMainInst);
    ok = WebRtcNetEQ_ClearPostCallStats(&NetEqMainInst->DSPinst);
    RETURN_ON_ERROR(ok, NetEqMainInst);
    ok = WebRtcNetEQ_ResetMcuJitterStat(&NetEqMainInst->MCUinst);
    RETURN_ON_ERROR(ok, NetEqMainInst);

    /* flush packet buffer */
    ok = WebRtcNetEQ_PacketBufferFlush(&NetEqMainInst->MCUinst.PacketBuffer_inst);
    RETURN_ON_ERROR(ok, NetEqMainInst);

    /* set some variables to initial values */
    NetEqMainInst->MCUinst.current_Codec = -1;
    NetEqMainInst->MCUinst.current_Payload = -1;
    NetEqMainInst->MCUinst.first_packet = 1;
    NetEqMainInst->MCUinst.one_desc = 0;
    NetEqMainInst->MCUinst.BufferStat_inst.Automode_inst.extraDelayMs = 0;
    NetEqMainInst->MCUinst.NoOfExpandCalls = 0;
    NetEqMainInst->MCUinst.fs = fs;

#ifdef NETEQ_ATEVENT_DECODE
    /* init DTMF decoder */
    ok = WebRtcNetEQ_DtmfDecoderInit(&(NetEqMainInst->MCUinst.DTMF_inst),fs,560);
    RETURN_ON_ERROR(ok, NetEqMainInst);
#endif

    /* init RTCP statistics */
    WebRtcNetEQ_RTCPInit(&(NetEqMainInst->MCUinst.RTCP_inst), 0);

    /* set BufferStat struct to zero */
    WebRtcSpl_MemSetW16((WebRtc_Word16*) &(NetEqMainInst->MCUinst.BufferStat_inst), 0,
        sizeof(BufstatsInst_t) / sizeof(WebRtc_Word16));

    /* reset automode */
    WebRtcNetEQ_ResetAutomode(&(NetEqMainInst->MCUinst.BufferStat_inst.Automode_inst),
        NetEqMainInst->MCUinst.PacketBuffer_inst.maxInsertPositions);

    NetEqMainInst->ErrorCode = 0;

#ifdef NETEQ_STEREO
    /* set master/slave info to undecided */
    NetEqMainInst->masterSlave = 0;
#endif

    return (ok);
}

int WebRtcNetEQ_FlushBuffers(void *inst)
{
    int ok = 0;

    /* Typecast inst to internal instance format */
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;

    if (NetEqMainInst == NULL)
    {
        return (-1);
    }

    /* Flush packet buffer */
    ok = WebRtcNetEQ_PacketBufferFlush(&NetEqMainInst->MCUinst.PacketBuffer_inst);
    RETURN_ON_ERROR(ok, NetEqMainInst);

    /* Set MCU to wait for new codec */
    NetEqMainInst->MCUinst.first_packet = 1;

    /* Flush speech buffer */
    ok = WebRtcNetEQ_FlushSpeechBuffer(&NetEqMainInst->DSPinst);
    RETURN_ON_ERROR(ok, NetEqMainInst);

    return 0;
}

int WebRtcNetEQ_SetAVTPlayout(void *inst, int PlayoutAVTon)
{
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
    if (NetEqMainInst == NULL) return (-1);
#ifdef NETEQ_ATEVENT_DECODE
    NetEqMainInst->MCUinst.AVT_PlayoutOn = PlayoutAVTon;
    return(0);
#else
    if (PlayoutAVTon != 0)
    {
        NetEqMainInst->ErrorCode = -DTMF_NOT_SUPPORTED;
        return (-1);
    }
    else
    {
        return (0);
    }
#endif
}

int WebRtcNetEQ_SetExtraDelay(void *inst, int DelayInMs)
{
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
    if (NetEqMainInst == NULL) return (-1);
    if ((DelayInMs < 0) || (DelayInMs > 1000))
    {
        NetEqMainInst->ErrorCode = -FAULTY_DELAYVALUE;
        return (-1);
    }
    NetEqMainInst->MCUinst.BufferStat_inst.Automode_inst.extraDelayMs = DelayInMs;
    return (0);
}

int WebRtcNetEQ_SetPlayoutMode(void *inst, enum WebRtcNetEQPlayoutMode playoutMode)
{
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
    if (NetEqMainInst == NULL) return (-1);
    if ((playoutMode != kPlayoutOn) && (playoutMode != kPlayoutOff) && (playoutMode
        != kPlayoutFax) && (playoutMode != kPlayoutStreaming))
    {
        NetEqMainInst->ErrorCode = -FAULTY_PLAYOUTMODE;
        return (-1);
    }
    else
    {
        NetEqMainInst->MCUinst.NetEqPlayoutMode = playoutMode;
        return (0);
    }
}

int WebRtcNetEQ_SetBGNMode(void *inst, enum WebRtcNetEQBGNMode bgnMode)
{

    MainInst_t *NetEqMainInst = (MainInst_t*) inst;

    /* Instance sanity */
    if (NetEqMainInst == NULL) return (-1);

    /* Check for corrupt/cleared instance */
    if (NetEqMainInst->MCUinst.main_inst != NetEqMainInst)
    {
        /* Instance is corrupt */
        NetEqMainInst->ErrorCode = CORRUPT_INSTANCE;
        return (-1);
    }

    NetEqMainInst->DSPinst.BGNInst.bgnMode = (enum BGNMode) bgnMode;

    return (0);
}

int WebRtcNetEQ_GetBGNMode(const void *inst, enum WebRtcNetEQBGNMode *bgnMode)
{

    const MainInst_t *NetEqMainInst = (const MainInst_t*) inst;

    /* Instance sanity */
    if (NetEqMainInst == NULL) return (-1);

    *bgnMode = (enum WebRtcNetEQBGNMode) NetEqMainInst->DSPinst.BGNInst.bgnMode;

    return (0);
}

/************************************************
 * CodecDB functions
 */

int WebRtcNetEQ_CodecDbReset(void *inst)
{
    int ok = 0;
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
    if (NetEqMainInst == NULL) return (-1);
    ok = WebRtcNetEQ_DbReset(&NetEqMainInst->MCUinst.codec_DB_inst);
    if (ok != 0)
    {
        NetEqMainInst->ErrorCode = -ok;
        return (-1);
    }

    /* set function pointers to NULL to prevent RecOut from using the codec */
    NetEqMainInst->DSPinst.codec_ptr_inst.funcDecode = NULL;
    NetEqMainInst->DSPinst.codec_ptr_inst.funcDecodeRCU = NULL;
    NetEqMainInst->DSPinst.codec_ptr_inst.funcAddLatePkt = NULL;
    NetEqMainInst->DSPinst.codec_ptr_inst.funcDecode = NULL;
    NetEqMainInst->DSPinst.codec_ptr_inst.funcDecodeInit = NULL;
    NetEqMainInst->DSPinst.codec_ptr_inst.funcDecodePLC = NULL;
    NetEqMainInst->DSPinst.codec_ptr_inst.funcGetMDinfo = NULL;
    NetEqMainInst->DSPinst.codec_ptr_inst.funcUpdBWEst = NULL;
    NetEqMainInst->DSPinst.codec_ptr_inst.funcGetErrorCode = NULL;

    return (0);
}

int WebRtcNetEQ_CodecDbGetSizeInfo(void *inst, WebRtc_Word16 *UsedEntries,
                                   WebRtc_Word16 *MaxEntries)
{
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
    if (NetEqMainInst == NULL) return (-1);
    *MaxEntries = NUM_CODECS;
    *UsedEntries = NetEqMainInst->MCUinst.codec_DB_inst.nrOfCodecs;
    return (0);
}

int WebRtcNetEQ_CodecDbGetCodecInfo(void *inst, WebRtc_Word16 Entry,
                                    enum WebRtcNetEQDecoder *codec)
{
    int i;
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
    if (NetEqMainInst == NULL) return (-1);
    *codec = (enum WebRtcNetEQDecoder) 0;
    if ((Entry >= 0) && (Entry < NetEqMainInst->MCUinst.codec_DB_inst.nrOfCodecs))
    {
        for (i = 0; i < NUM_TOTAL_CODECS; i++)
        {
            if (NetEqMainInst->MCUinst.codec_DB_inst.position[i] == Entry)
            {
                *codec = (enum WebRtcNetEQDecoder) i;
            }
        }
    }
    else
    {
        NetEqMainInst->ErrorCode = -(CODEC_DB_NOT_EXIST1);
        return (-1);
    }
    return (0);
}

int WebRtcNetEQ_CodecDbAdd(void *inst, WebRtcNetEQ_CodecDef *codecInst)
{
    int ok = 0;
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
    if (NetEqMainInst == NULL) return (-1);
    ok = WebRtcNetEQ_DbAdd(&NetEqMainInst->MCUinst.codec_DB_inst, codecInst->codec,
        codecInst->payloadType, codecInst->funcDecode, codecInst->funcDecodeRCU,
        codecInst->funcDecodePLC, codecInst->funcDecodeInit, codecInst->funcAddLatePkt,
        codecInst->funcGetMDinfo, codecInst->funcGetPitch, codecInst->funcUpdBWEst,
        codecInst->funcGetErrorCode, codecInst->codec_state, codecInst->codec_fs);
    if (ok != 0)
    {
        NetEqMainInst->ErrorCode = -ok;
        return (-1);
    }
    return (ok);
}

int WebRtcNetEQ_CodecDbRemove(void *inst, enum WebRtcNetEQDecoder codec)
{
    int ok = 0;
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
    if (NetEqMainInst == NULL) return (-1);

    /* check if currently used codec is being removed */
    if (NetEqMainInst->MCUinst.current_Codec == (WebRtc_Word16) codec)
    {
        /* set function pointers to NULL to prevent RecOut from using the codec */
        NetEqMainInst->DSPinst.codec_ptr_inst.funcDecode = NULL;
        NetEqMainInst->DSPinst.codec_ptr_inst.funcDecodeRCU = NULL;
        NetEqMainInst->DSPinst.codec_ptr_inst.funcAddLatePkt = NULL;
        NetEqMainInst->DSPinst.codec_ptr_inst.funcDecode = NULL;
        NetEqMainInst->DSPinst.codec_ptr_inst.funcDecodeInit = NULL;
        NetEqMainInst->DSPinst.codec_ptr_inst.funcDecodePLC = NULL;
        NetEqMainInst->DSPinst.codec_ptr_inst.funcGetMDinfo = NULL;
        NetEqMainInst->DSPinst.codec_ptr_inst.funcUpdBWEst = NULL;
        NetEqMainInst->DSPinst.codec_ptr_inst.funcGetErrorCode = NULL;
    }

    ok = WebRtcNetEQ_DbRemove(&NetEqMainInst->MCUinst.codec_DB_inst, codec);
    if (ok != 0)
    {
        NetEqMainInst->ErrorCode = -ok;
        return (-1);
    }
    return (ok);
}

/*********************************
 * Real-time functions
 */

int WebRtcNetEQ_RecIn(void *inst, WebRtc_Word16 *p_w16datagramstart, WebRtc_Word16 w16_RTPlen,
                      WebRtc_UWord32 uw32_timeRec)
{
    int ok = 0;
    RTPPacket_t RTPpacket;
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
    if (NetEqMainInst == NULL) return (-1);

    /* Check for corrupt/cleared instance */
    if (NetEqMainInst->MCUinst.main_inst != NetEqMainInst)
    {
        /* Instance is corrupt */
        NetEqMainInst->ErrorCode = CORRUPT_INSTANCE;
        return (-1);
    }

    /* Parse RTP header */
    ok = WebRtcNetEQ_RTPPayloadInfo(p_w16datagramstart, w16_RTPlen, &RTPpacket);
    if (ok != 0)
    {
        NetEqMainInst->ErrorCode = -ok;
        return (-1);
    }

    ok = WebRtcNetEQ_RecInInternal(&NetEqMainInst->MCUinst, &RTPpacket, uw32_timeRec);
    if (ok != 0)
    {
        NetEqMainInst->ErrorCode = -ok;
        return (-1);
    }
    return (ok);
}

/****************************************************************************
 * WebRtcNetEQ_RecInRTPStruct(...)
 *
 * Alternative RecIn function, used when the RTP data has already been
 * parsed into an RTP info struct (WebRtcNetEQ_RTPInfo).
 *
 * Input:
 *		- inst	            : NetEQ instance
 *		- rtpInfo		    : Pointer to RTP info
 *		- payloadPtr        : Pointer to the RTP payload (first byte after header)
 *      - payloadLenBytes   : Length (in bytes) of the payload in payloadPtr
 *      - timeRec           : Receive time (in timestamps of the used codec)
 *
 * Return value			    :  0 - Ok
 *                            -1 - Error
 */
int WebRtcNetEQ_RecInRTPStruct(void *inst, WebRtcNetEQ_RTPInfo *rtpInfo,
                               const WebRtc_UWord8 *payloadPtr, WebRtc_Word16 payloadLenBytes,
                               WebRtc_UWord32 uw32_timeRec)
{
    int ok = 0;
    RTPPacket_t RTPpacket;
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
    if (NetEqMainInst == NULL)
    {
        return (-1);
    }

    /* Check for corrupt/cleared instance */
    if (NetEqMainInst->MCUinst.main_inst != NetEqMainInst)
    {
        /* Instance is corrupt */
        NetEqMainInst->ErrorCode = CORRUPT_INSTANCE;
        return (-1);
    }

    /* Load NetEQ's RTP struct from Module RTP struct */
    RTPpacket.payloadType = rtpInfo->payloadType;
    RTPpacket.seqNumber = rtpInfo->sequenceNumber;
    RTPpacket.timeStamp = rtpInfo->timeStamp;
    RTPpacket.ssrc = rtpInfo->SSRC;
    RTPpacket.payload = (const WebRtc_Word16*) payloadPtr;
    RTPpacket.payloadLen = payloadLenBytes;
    RTPpacket.starts_byte1 = 0;

    ok = WebRtcNetEQ_RecInInternal(&NetEqMainInst->MCUinst, &RTPpacket, uw32_timeRec);
    if (ok != 0)
    {
        NetEqMainInst->ErrorCode = -ok;
        return (-1);
    }
    return (ok);
}

int WebRtcNetEQ_RecOut(void *inst, WebRtc_Word16 *pw16_outData, WebRtc_Word16 *pw16_len)
{
    int ok = 0;
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
#ifdef NETEQ_STEREO
    MasterSlaveInfo msInfo;
    msInfo.msMode = NETEQ_MONO;
#endif

    if (NetEqMainInst == NULL) return (-1);

    /* Check for corrupt/cleared instance */
    if (NetEqMainInst->DSPinst.main_inst != NetEqMainInst)
    {
        /* Instance is corrupt */
        NetEqMainInst->ErrorCode = CORRUPT_INSTANCE;
        return (-1);
    }

#ifdef NETEQ_STEREO
    NetEqMainInst->DSPinst.msInfo = &msInfo;
#endif

    ok = WebRtcNetEQ_RecOutInternal(&NetEqMainInst->DSPinst, pw16_outData,
        pw16_len, 0 /* not BGN only */);
    if (ok != 0)
    {
        NetEqMainInst->ErrorCode = -ok;
        return (-1);
    }
    return (ok);
}

/****************************************************************************
 * WebRtcNetEQ_RecOutMasterSlave(...)
 *
 * RecOut function for running several NetEQ instances in master/slave mode.
 * One master can be used to control several slaves.
 *
 * Input:
 *      - inst          : NetEQ instance
 *      - isMaster      : Non-zero indicates that this is the master channel
 *      - msInfo        : (slave only) Information from master
 *
 * Output:
 *		- inst	        : Updated NetEQ instance
 *      - pw16_outData  : Pointer to vector where output should be written
 *      - pw16_len      : Pointer to variable where output length is returned
 *      - msInfo        : (master only) Information to slave(s)
 *
 * Return value			:  0 - Ok
 *						  -1 - Error
 */

int WebRtcNetEQ_RecOutMasterSlave(void *inst, WebRtc_Word16 *pw16_outData,
                                  WebRtc_Word16 *pw16_len, void *msInfo,
                                  WebRtc_Word16 isMaster)
{
#ifndef NETEQ_STEREO
    /* Stereo not supported */
    return(-1);
#else
    int ok = 0;
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;

    if (NetEqMainInst == NULL) return (-1);

    /* Check for corrupt/cleared instance */
    if (NetEqMainInst->DSPinst.main_inst != NetEqMainInst)
    {
        /* Instance is corrupt */
        NetEqMainInst->ErrorCode = CORRUPT_INSTANCE;
        return (-1);
    }

    if (msInfo == NULL)
    {
        /* msInfo not provided */
        NetEqMainInst->ErrorCode = NETEQ_OTHER_ERROR;
        return (-1);
    }

    /* translate from external to internal Master/Slave information */
    NetEqMainInst->DSPinst.msInfo = (MasterSlaveInfo *) msInfo;

    /* check that we have not done a master/slave switch without first re-initializing */
    if ((NetEqMainInst->masterSlave == 1 && !isMaster) || /* switch from master to slave */
    (NetEqMainInst->masterSlave == 2 && isMaster)) /* switch from slave to master */
    {
        NetEqMainInst->ErrorCode = ILLEGAL_MASTER_SLAVE_SWITCH;
        return (-1);
    }

    if (!isMaster)
    {
        /* this is the slave */
        NetEqMainInst->masterSlave = 2;
        NetEqMainInst->DSPinst.msInfo->msMode = NETEQ_SLAVE;
    }
    else
    {
        NetEqMainInst->DSPinst.msInfo->msMode = NETEQ_MASTER;
    }

    ok  = WebRtcNetEQ_RecOutInternal(&NetEqMainInst->DSPinst, pw16_outData,
        pw16_len, 0 /* not BGN only */);
    if (ok != 0)
    {
        NetEqMainInst->ErrorCode = -ok;
        return (-1);
    }

    if (isMaster)
    {
        /* this is the master */
        NetEqMainInst->masterSlave = 1;
    }

    return (ok);
#endif
}

int WebRtcNetEQ_GetMasterSlaveInfoSize()
{
#ifdef NETEQ_STEREO
    return (sizeof(MasterSlaveInfo));
#else
    return(-1);
#endif
}

/* Special RecOut that does not do any decoding. */
int WebRtcNetEQ_RecOutNoDecode(void *inst, WebRtc_Word16 *pw16_outData,
                               WebRtc_Word16 *pw16_len)
{
    int ok = 0;
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
#ifdef NETEQ_STEREO
    MasterSlaveInfo msInfo;
#endif

    if (NetEqMainInst == NULL) return (-1);

    /* Check for corrupt/cleared instance */
    if (NetEqMainInst->DSPinst.main_inst != NetEqMainInst)
    {
        /* Instance is corrupt */
        NetEqMainInst->ErrorCode = CORRUPT_INSTANCE;
        return (-1);
    }

#ifdef NETEQ_STEREO
    /* keep same mode as before */
    switch (NetEqMainInst->masterSlave)
    {
        case 1:
        {
            msInfo.msMode = NETEQ_MASTER;
            break;
        }
        case 2:
        {
            msInfo.msMode = NETEQ_SLAVE;
            break;
        }
        default:
        {
            msInfo.msMode = NETEQ_MONO;
            break;
        }
    }

    NetEqMainInst->DSPinst.msInfo = &msInfo;
#endif

    ok = WebRtcNetEQ_RecOutInternal(&NetEqMainInst->DSPinst, pw16_outData,
        pw16_len, 1 /* BGN only */);
    if (ok != 0)
    {
        NetEqMainInst->ErrorCode = -ok;
        return (-1);
    }
    return (ok);
}

int WebRtcNetEQ_GetRTCPStats(void *inst, WebRtcNetEQ_RTCPStat *RTCP_inst)
{
    int ok = 0;
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
    if (NetEqMainInst == NULL) return (-1);
    ok = WebRtcNetEQ_RTCPGetStats(&NetEqMainInst->MCUinst.RTCP_inst,
        &RTCP_inst->fraction_lost, &RTCP_inst->cum_lost, &RTCP_inst->ext_max,
        &RTCP_inst->jitter, 0);
    if (ok != 0)
    {
        NetEqMainInst->ErrorCode = -ok;
        return (-1);
    }
    return (ok);
}

int WebRtcNetEQ_GetRTCPStatsNoReset(void *inst, WebRtcNetEQ_RTCPStat *RTCP_inst)
{
    int ok = 0;
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
    if (NetEqMainInst == NULL) return (-1);
    ok = WebRtcNetEQ_RTCPGetStats(&NetEqMainInst->MCUinst.RTCP_inst,
        &RTCP_inst->fraction_lost, &RTCP_inst->cum_lost, &RTCP_inst->ext_max,
        &RTCP_inst->jitter, 1);
    if (ok != 0)
    {
        NetEqMainInst->ErrorCode = -ok;
        return (-1);
    }
    return (ok);
}

int WebRtcNetEQ_GetSpeechTimeStamp(void *inst, WebRtc_UWord32 *timestamp)
{
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
    if (NetEqMainInst == NULL) return (-1);

    if (NetEqMainInst->MCUinst.TSscalingInitialized)
    {
        *timestamp = WebRtcNetEQ_ScaleTimestampInternalToExternal(&NetEqMainInst->MCUinst,
            NetEqMainInst->DSPinst.videoSyncTimestamp);
    }
    else
    {
        *timestamp = NetEqMainInst->DSPinst.videoSyncTimestamp;
    }

    return (0);
}

/****************************************************************************
 * WebRtcNetEQ_GetSpeechOutputType(...)
 *
 * Get the output type for the audio provided by the latest call to
 * WebRtcNetEQ_RecOut().
 *
 * kOutputNormal = normal audio (possibly processed)
 * kOutputPLC = loss concealment through stretching audio
 * kOutputCNG = comfort noise (codec-internal or RFC3389)
 * kOutputPLCtoCNG = background noise only due to long expand or error
 * kOutputVADPassive = PostDecode VAD signalling passive speaker
 *
 * Input:
 *      - inst          : NetEQ instance
 *
 * Output:
 *		- outputType    : Output type from enum list WebRtcNetEQOutputType
 *
 * Return value			:  0 - Ok
 *						  -1 - Error
 */

int WebRtcNetEQ_GetSpeechOutputType(void *inst, enum WebRtcNetEQOutputType *outputType)
{
    /* Typecast to internal instance type */
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;

    if (NetEqMainInst == NULL)
    {
        return (-1);
    }

    if ((NetEqMainInst->DSPinst.w16_mode & MODE_BGN_ONLY) != 0)
    {
        /* If last mode was background noise only */
        *outputType = kOutputPLCtoCNG;

    }
    else if ((NetEqMainInst->DSPinst.w16_mode == MODE_CODEC_INTERNAL_CNG)
        || (NetEqMainInst->DSPinst.w16_mode == MODE_RFC3389CNG))
    {
        /* If CN or internal CNG */
        *outputType = kOutputCNG;

#ifdef NETEQ_VAD
    }
    else if ( NetEqMainInst->DSPinst.VADInst.VADDecision == 0 )
    {
        /* post-decode VAD says passive speaker */
        *outputType = kOutputVADPassive;
#endif /* NETEQ_VAD */

    }
    else if ((NetEqMainInst->DSPinst.w16_mode == MODE_EXPAND)
        && (NetEqMainInst->DSPinst.ExpandInst.w16_expandMuteFactor == 0))
    {
        /* Expand mode has faded down to background noise only (very long expand) */
        *outputType = kOutputPLCtoCNG;

    }
    else if (NetEqMainInst->DSPinst.w16_mode == MODE_EXPAND)
    {
        /* PLC mode */
        *outputType = kOutputPLC;

    }
    else
    {
        /* Normal speech output type (can still be manipulated, e.g., accelerated) */
        *outputType = kOutputNormal;
    }

    return (0);
}

/**********************************
 * Functions related to VQmon 
 */

#define WEBRTC_NETEQ_CONCEALMENTFLAG_LOST       0x01
#define WEBRTC_NETEQ_CONCEALMENTFLAG_DISCARDED  0x02
#define WEBRTC_NETEQ_CONCEALMENTFLAG_SUPRESS    0x04
#define WEBRTC_NETEQ_CONCEALMENTFLAG_CNGACTIVE  0x80

int WebRtcNetEQ_VQmonRecOutStatistics(void *inst, WebRtc_UWord16 *validVoiceDurationMs,
                                      WebRtc_UWord16 *concealedVoiceDurationMs,
                                      WebRtc_UWord8 *concealedVoiceFlags)
{
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
    WebRtc_Word16 fs_mult;
    WebRtc_Word16 ms_lost;
    if (NetEqMainInst == NULL) return (-1);
    fs_mult = WebRtcSpl_DivW32W16ResW16(NetEqMainInst->MCUinst.fs, 8000);

    ms_lost = WebRtcSpl_DivW32W16ResW16(
        (WebRtc_Word32) NetEqMainInst->DSPinst.w16_concealedTS, (WebRtc_Word16) (8 * fs_mult));
    if (ms_lost > NetEqMainInst->DSPinst.millisecondsPerCall) ms_lost
        = NetEqMainInst->DSPinst.millisecondsPerCall;

    *validVoiceDurationMs = NetEqMainInst->DSPinst.millisecondsPerCall - ms_lost;
    *concealedVoiceDurationMs = ms_lost;
    if (ms_lost > 0)
    {
        *concealedVoiceFlags = WEBRTC_NETEQ_CONCEALMENTFLAG_LOST;
    }
    else
    {
        *concealedVoiceFlags = 0;
    }
    NetEqMainInst->DSPinst.w16_concealedTS -= ms_lost * (8 * fs_mult);

    return (0);
}

int WebRtcNetEQ_VQmonGetConfiguration(void *inst, WebRtc_UWord16 *absMaxDelayMs,
                                      WebRtc_UWord8 *adaptationRate)
{
    /* Dummy check the inst, just to avoid compiler warnings. */
    if (inst == NULL)
    {
        /* Do nothing. */
    }

    /* Hardcoded variables that are used for VQmon as jitter buffer parameters */
    *absMaxDelayMs = 240;
    *adaptationRate = 1;
    return (0);
}

int WebRtcNetEQ_VQmonGetRxStatistics(void *inst, WebRtc_UWord16 *avgDelayMs,
                                     WebRtc_UWord16 *maxDelayMs)
{
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
    if (NetEqMainInst == NULL) return (-1);
    *avgDelayMs = (WebRtc_UWord16) (NetEqMainInst->MCUinst.BufferStat_inst.avgDelayMsQ8 >> 8);
    *maxDelayMs = (WebRtc_UWord16) NetEqMainInst->MCUinst.BufferStat_inst.maxDelayMs;
    return (0);
}

/*************************************
 * Statistics functions
 */

/* Get the "in-call" statistics from NetEQ.
 * The statistics are reset after the query. */
int WebRtcNetEQ_GetNetworkStatistics(void *inst, WebRtcNetEQ_NetworkStatistics *stats)

{

    WebRtc_UWord16 tempU16;
    WebRtc_UWord32 tempU32, tempU32_2;
    int numShift;
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;

    /* Instance sanity */
    if (NetEqMainInst == NULL) return (-1);

    /*******************/
    /* Get buffer size */
    /*******************/

    if (NetEqMainInst->MCUinst.fs != 0)
    {
        WebRtc_Word32 temp32;
        /* Query packet buffer for number of samples. */
        temp32 = WebRtcNetEQ_PacketBufferGetSize(
            &NetEqMainInst->MCUinst.PacketBuffer_inst);

        /* Divide by sample rate.
         * Calculate temp32 * 1000 / fs to get result in ms. */
        stats->currentBufferSize = (WebRtc_UWord16)
            WebRtcSpl_DivU32U16(temp32 * 1000, NetEqMainInst->MCUinst.fs);

        /* Add number of samples yet to play in sync buffer. */
        temp32 = (WebRtc_Word32) (NetEqMainInst->DSPinst.endPosition -
            NetEqMainInst->DSPinst.curPosition);
        stats->currentBufferSize += (WebRtc_UWord16)
            WebRtcSpl_DivU32U16(temp32 * 1000, NetEqMainInst->MCUinst.fs);
    }
    else
    {
        /* Sample rate not initialized. */
        stats->currentBufferSize = 0;
    }

    /***************************/
    /* Get optimal buffer size */
    /***************************/

    if (NetEqMainInst->MCUinst.fs != 0 && NetEqMainInst->MCUinst.fs <= WEBRTC_SPL_WORD16_MAX)
    {
        /* preferredBufferSize = Bopt * packSizeSamples / (fs/1000) */
        stats->preferredBufferSize
            = (WebRtc_UWord16) WEBRTC_SPL_MUL_16_16(
                (WebRtc_Word16) ((NetEqMainInst->MCUinst.BufferStat_inst.Automode_inst.optBufLevel) >> 8), /* optimal buffer level in packets shifted to Q0 */
                WebRtcSpl_DivW32W16ResW16(
                    (WebRtc_Word32) NetEqMainInst->MCUinst.BufferStat_inst.Automode_inst.packetSpeechLenSamp, /* samples per packet */
                    WebRtcSpl_DivW32W16ResW16( (WebRtc_Word32) NetEqMainInst->MCUinst.fs, (WebRtc_Word16) 1000 ) /* samples per ms */
                ) );

        /* add extra delay */
        if (NetEqMainInst->MCUinst.BufferStat_inst.Automode_inst.extraDelayMs > 0)
        {
            stats->preferredBufferSize
                += NetEqMainInst->MCUinst.BufferStat_inst.Automode_inst.extraDelayMs;
        }
    }
    else
    {
        /* sample rate not initialized */
        stats->preferredBufferSize = 0;
    }

    /***********************/
    /* Calculate loss rate */
    /***********************/

    /* timestamps elapsed since last report */
    tempU32 = NetEqMainInst->MCUinst.lastReportTS;

    if (NetEqMainInst->MCUinst.lostTS == 0)
    {
        /* no losses */
        stats->currentPacketLossRate = 0;
    }
    else if (NetEqMainInst->MCUinst.lostTS < tempU32)
    {
        /* calculate shifts; we want the result in Q14 */
        numShift = WebRtcSpl_NormU32(NetEqMainInst->MCUinst.lostTS); /* numerator shift for normalize */

        if (numShift < 14)
        {
            /* cannot shift numerator 14 steps; shift denominator too */
            tempU32 = WEBRTC_SPL_RSHIFT_U32(tempU32, 14-numShift); /* right-shift */
        }
        else
        {
            /* shift no more than 14 steps */
            numShift = 14;
        }

        if (tempU32 == 0)
        {
            /* check for zero denominator; result should be zero in this case */
            stats->currentPacketLossRate = 0;
        }
        else
        {
            /* check that denominator fits in signed 16-bit */
            while (tempU32 > WEBRTC_SPL_WORD16_MAX)
            {
                tempU32 >>= 1; /* right-shift 1 step */
                numShift--; /* compensate in numerator */
            }
            tempU16 = (WebRtc_UWord16) tempU32;

            /* do the shift of numerator */
            tempU32
                = WEBRTC_SPL_SHIFT_W32( (WebRtc_UWord32) NetEqMainInst->MCUinst.lostTS, numShift);

            stats->currentPacketLossRate = (WebRtc_UWord16) WebRtcSpl_DivU32U16(tempU32,
                tempU16);
        }
    }
    else
    {
        /* lost count is larger than elapsed time count; probably timestamp wrap-around or something else wrong */
        /* set loss rate = 1 */
        stats->currentPacketLossRate = 1 << 14; /* 1 in Q14 */
    }

    /**************************/
    /* Calculate discard rate */
    /**************************/

    /* timestamps elapsed since last report */
    tempU32 = NetEqMainInst->MCUinst.lastReportTS;

    /* number of discarded samples */
    tempU32_2
        = WEBRTC_SPL_MUL_16_U16( (WebRtc_Word16) NetEqMainInst->MCUinst.PacketBuffer_inst.packSizeSamples,
            NetEqMainInst->MCUinst.PacketBuffer_inst.discardedPackets);

    if (tempU32_2 == 0)
    {
        /* no discarded samples */
        stats->currentDiscardRate = 0;
    }
    else if (tempU32_2 < tempU32)
    {
        /* calculate shifts; we want the result in Q14 */
        numShift = WebRtcSpl_NormU32(tempU32_2); /* numerator shift for normalize */

        if (numShift < 14)
        {
            /* cannot shift numerator 14 steps; shift denominator too */
            tempU32 = WEBRTC_SPL_RSHIFT_U32(tempU32, 14-numShift); /* right-shift */
        }
        else
        {
            /* shift no more than 14 steps */
            numShift = 14;
        }

        if (tempU32 == 0)
        {
            /* check for zero denominator; result should be zero in this case */
            stats->currentDiscardRate = 0;
        }
        else
        {
            /* check that denominator fits in signed 16-bit */
            while (tempU32 > WEBRTC_SPL_WORD16_MAX)
            {
                tempU32 >>= 1; /* right-shift 1 step */
                numShift--; /* compensate in numerator */
            }
            tempU16 = (WebRtc_UWord16) tempU32;

            /* do the shift of numerator */
            tempU32 = WEBRTC_SPL_SHIFT_W32( tempU32_2, numShift);

            stats->currentDiscardRate = (WebRtc_UWord16) WebRtcSpl_DivU32U16(tempU32, tempU16);
        }
    }
    else
    {
        /* lost count is larger than elapsed time count; probably timestamp wrap-around or something else wrong */
        /* set loss rate = 1 */
        stats->currentDiscardRate = 1 << 14; /* 1 in Q14 */
    }

    /*************************************************************/
    /* Calculate Accelerate, Expand and Pre-emptive Expand rates */
    /*************************************************************/

    /* timestamps elapsed since last report */
    tempU32 = NetEqMainInst->MCUinst.lastReportTS;

    if (NetEqMainInst->DSPinst.statInst.accelerateLength == 0)
    {
        /* no accelerate */
        stats->currentAccelerateRate = 0;
    }
    else if (NetEqMainInst->DSPinst.statInst.accelerateLength < tempU32)
    {
        /* calculate shifts; we want the result in Q14 */
        numShift = WebRtcSpl_NormU32(NetEqMainInst->DSPinst.statInst.accelerateLength); /* numerator shift for normalize */

        if (numShift < 14)
        {
            /* cannot shift numerator 14 steps; shift denominator too */
            tempU32 = WEBRTC_SPL_RSHIFT_U32(tempU32, 14-numShift); /* right-shift */
        }
        else
        {
            /* shift no more than 14 steps */
            numShift = 14;
        }

        if (tempU32 == 0)
        {
            /* check for zero denominator; result should be zero in this case */
            stats->currentAccelerateRate = 0;
        }
        else
        {
            /* check that denominator fits in signed 16-bit */
            while (tempU32 > WEBRTC_SPL_WORD16_MAX)
            {
                tempU32 >>= 1; /* right-shift 1 step */
                numShift--; /* compensate in numerator */
            }
            tempU16 = (WebRtc_UWord16) tempU32;

            /* do the shift of numerator */
            tempU32
                = WEBRTC_SPL_SHIFT_W32( NetEqMainInst->DSPinst.statInst.accelerateLength, numShift);

            stats->currentAccelerateRate = (WebRtc_UWord16) WebRtcSpl_DivU32U16(tempU32,
                tempU16);
        }
    }
    else
    {
        /* lost count is larger than elapsed time count; probably timestamp wrap-around or something else wrong */
        /* set loss rate = 1 */
        stats->currentAccelerateRate = 1 << 14; /* 1 in Q14 */
    }

    /* timestamps elapsed since last report */
    tempU32 = NetEqMainInst->MCUinst.lastReportTS;

    if (NetEqMainInst->DSPinst.statInst.expandLength == 0)
    {
        /* no expand */
        stats->currentExpandRate = 0;
    }
    else if (NetEqMainInst->DSPinst.statInst.expandLength < tempU32)
    {
        /* calculate shifts; we want the result in Q14 */
        numShift = WebRtcSpl_NormU32(NetEqMainInst->DSPinst.statInst.expandLength); /* numerator shift for normalize */

        if (numShift < 14)
        {
            /* cannot shift numerator 14 steps; shift denominator too */
            tempU32 = WEBRTC_SPL_RSHIFT_U32(tempU32, 14-numShift); /* right-shift */
        }
        else
        {
            /* shift no more than 14 steps */
            numShift = 14;
        }

        if (tempU32 == 0)
        {
            /* check for zero denominator; result should be zero in this case */
            stats->currentExpandRate = 0;
        }
        else
        {
            /* check that denominator fits in signed 16-bit */
            while (tempU32 > WEBRTC_SPL_WORD16_MAX)
            {
                tempU32 >>= 1; /* right-shift 1 step */
                numShift--; /* compensate in numerator */
            }
            tempU16 = (WebRtc_UWord16) tempU32;

            /* do the shift of numerator */
            tempU32
                = WEBRTC_SPL_SHIFT_W32( NetEqMainInst->DSPinst.statInst.expandLength, numShift);

            stats->currentExpandRate = (WebRtc_UWord16) WebRtcSpl_DivU32U16(tempU32, tempU16);
        }
    }
    else
    {
        /* lost count is larger than elapsed time count; probably timestamp wrap-around or something else wrong */
        /* set loss rate = 1 */
        stats->currentExpandRate = 1 << 14; /* 1 in Q14 */
    }

    /* timestamps elapsed since last report */
    tempU32 = NetEqMainInst->MCUinst.lastReportTS;

    if (NetEqMainInst->DSPinst.statInst.preemptiveLength == 0)
    {
        /* no pre-emptive expand */
        stats->currentPreemptiveRate = 0;
    }
    else if (NetEqMainInst->DSPinst.statInst.preemptiveLength < tempU32)
    {
        /* calculate shifts; we want the result in Q14 */
        numShift = WebRtcSpl_NormU32(NetEqMainInst->DSPinst.statInst.preemptiveLength); /* numerator shift for normalize */

        if (numShift < 14)
        {
            /* cannot shift numerator 14 steps; shift denominator too */
            tempU32 = WEBRTC_SPL_RSHIFT_U32(tempU32, 14-numShift); /* right-shift */
        }
        else
        {
            /* shift no more than 14 steps */
            numShift = 14;
        }

        if (tempU32 == 0)
        {
            /* check for zero denominator; result should be zero in this case */
            stats->currentPreemptiveRate = 0;
        }
        else
        {
            /* check that denominator fits in signed 16-bit */
            while (tempU32 > WEBRTC_SPL_WORD16_MAX)
            {
                tempU32 >>= 1; /* right-shift 1 step */
                numShift--; /* compensate in numerator */
            }
            tempU16 = (WebRtc_UWord16) tempU32;

            /* do the shift of numerator */
            tempU32
                = WEBRTC_SPL_SHIFT_W32( NetEqMainInst->DSPinst.statInst.preemptiveLength, numShift);

            stats->currentPreemptiveRate = (WebRtc_UWord16) WebRtcSpl_DivU32U16(tempU32,
                tempU16);
        }
    }
    else
    {
        /* lost count is larger than elapsed time count; probably timestamp wrap-around or something else wrong */
        /* set loss rate = 1 */
        stats->currentPreemptiveRate = 1 << 14; /* 1 in Q14 */
    }

    /* reset counters */
    WebRtcNetEQ_ResetMcuInCallStats(&(NetEqMainInst->MCUinst));
    WebRtcNetEQ_ClearInCallStats(&(NetEqMainInst->DSPinst));

    return (0);
}

int WebRtcNetEQ_GetRawFrameWaitingTimes(void *inst,
                                        int max_length,
                                        int* waiting_times_ms) {
  int i = 0;
  MainInst_t *main_inst = (MainInst_t*) inst;
  if (main_inst == NULL) return -1;

  while ((i < max_length) && (i < main_inst->MCUinst.len_waiting_times)) {
    waiting_times_ms[i] = main_inst->MCUinst.waiting_times[i] *
        main_inst->DSPinst.millisecondsPerCall;
    ++i;
  }
  assert(i <= kLenWaitingTimes);
  WebRtcNetEQ_ResetWaitingTimeStats(&main_inst->MCUinst);
  return i;
}

/****************************************************************************
 * WebRtcNetEQ_SetVADInstance(...)
 *
 * Provide a pointer to an allocated VAD instance. If function is never 
 * called or it is called with NULL pointer as VAD_inst, the post-decode
 * VAD functionality is disabled. Also provide pointers to init, setmode
 * and VAD functions. These are typically pointers to WebRtcVad_Init,
 * WebRtcVad_set_mode and WebRtcVad_Process, respectively, all found in the
 * interface file webrtc_vad.h.
 *
 * Input:
 *      - NetEQ_inst        : NetEQ instance
 *		- VADinst		    : VAD instance
 *		- initFunction	    : Pointer to VAD init function
 *		- setmodeFunction   : Pointer to VAD setmode function
 *		- VADfunction	    : Pointer to VAD function
 *
 * Output:
 *		- NetEQ_inst	    : Updated NetEQ instance
 *
 * Return value			    :  0 - Ok
 *						      -1 - Error
 */

int WebRtcNetEQ_SetVADInstance(void *NetEQ_inst, void *VAD_inst,
                               WebRtcNetEQ_VADInitFunction initFunction,
                               WebRtcNetEQ_VADSetmodeFunction setmodeFunction,
                               WebRtcNetEQ_VADFunction VADFunction)
{

    /* Typecast to internal instance type */
    MainInst_t *NetEqMainInst = (MainInst_t*) NetEQ_inst;
    if (NetEqMainInst == NULL)
    {
        return (-1);
    }

#ifdef NETEQ_VAD

    /* Store pointer in PostDecode VAD struct */
    NetEqMainInst->DSPinst.VADInst.VADState = VAD_inst;

    /* Store function pointers */
    NetEqMainInst->DSPinst.VADInst.initFunction = initFunction;
    NetEqMainInst->DSPinst.VADInst.setmodeFunction = setmodeFunction;
    NetEqMainInst->DSPinst.VADInst.VADFunction = VADFunction;

    /* Call init function and return the result (ok or fail) */
    return(WebRtcNetEQ_InitVAD(&NetEqMainInst->DSPinst.VADInst, NetEqMainInst->DSPinst.fs));

#else /* NETEQ_VAD not defined */
    return (-1);
#endif /* NETEQ_VAD */

}

/****************************************************************************
 * WebRtcNetEQ_SetVADMode(...)
 *
 * Pass an aggressiveness mode parameter to the post-decode VAD instance.
 * If this function is never called, mode 0 (quality mode) is used as default.
 *
 * Input:
 *      - inst          : NetEQ instance
 *		- mode  		: mode parameter (same range as WebRtc VAD mode)
 *
 * Output:
 *		- inst	        : Updated NetEQ instance
 *
 * Return value			:  0 - Ok
 *						  -1 - Error
 */

int WebRtcNetEQ_SetVADMode(void *inst, WebRtc_Word16 mode)
{

    /* Typecast to internal instance type */
    MainInst_t *NetEqMainInst = (MainInst_t*) inst;
    if (NetEqMainInst == NULL)
    {
        return (-1);
    }

#ifdef NETEQ_VAD

    /* Set mode and return result */
    return(WebRtcNetEQ_SetVADModeInternal(&NetEqMainInst->DSPinst.VADInst, mode));

#else /* NETEQ_VAD not defined */
    return (-1);
#endif /* NETEQ_VAD */

}
