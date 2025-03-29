/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#include "amigos_surface_audioalgo.h"

AmigosSurfaceAudioAlgo::AmigosSurfaceAudioAlgo(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false)
{
}

AmigosSurfaceAudioAlgo::~AmigosSurfaceAudioAlgo() {}

void AmigosSurfaceAudioAlgo::_LoadDb()
{
    this->stAlgoInfo.uintSampleRate = this->pDb->GetMod<unsigned int>("SAMPLE_RATE");
    this->stAlgoInfo.uintPeriodSize = this->pDb->GetMod<unsigned int>("PERIOD_SIZE");
    this->stAlgoInfo.strSoundMode   = this->pDb->GetMod<std::string>("SND_MOD");
    this->stAlgoInfo.bSyncMode      = this->pDb->GetMod<int>("SYNC");

    this->stAlgoInfo.stApcInfo.onoff = this->pDb->GetMod<unsigned int>("APC");

    this->stAlgoInfo.stAnrInfo.onoff = this->pDb->GetMod<unsigned int>("ANR");
    if (this->stAlgoInfo.stAnrInfo.onoff)
    {
        this->stAlgoInfo.stAnrInfo.uintUserMode    = this->pDb->GetMod<unsigned int>("ANR_PARAM", "USER_MOD");
        this->stAlgoInfo.stAnrInfo.uintSmoothLevel = this->pDb->GetMod<unsigned int>("ANR_PARAM", "SMOOTH_LEVEL");
        this->stAlgoInfo.stAnrInfo.uintSpeed       = this->pDb->GetMod<unsigned int>("ANR_PARAM", "SPEED");
    }

    this->stAlgoInfo.stEqInfo.onoff = this->pDb->GetMod<unsigned int>("EQHPF");
    if (this->stAlgoInfo.stEqInfo.onoff)
    {
        this->stAlgoInfo.stEqInfo.uintUserMode   = this->pDb->GetMod<unsigned int>("EQHPF_PARAM", "EQ_USER_MOD");
        this->stAlgoInfo.stHpfInfo.uintUserMode  = this->pDb->GetMod<unsigned int>("EQHPF_PARAM", "HPF_USER_MOD");
        this->stAlgoInfo.stHpfInfo.strCutoffFreq = this->pDb->GetMod<unsigned int>("EQHPF_PARAM", "CUTOFF_FREQ");
    }

    this->stAlgoInfo.stAgcInfo.onoff = this->pDb->GetMod<unsigned int>("AGC");
    if (this->stAlgoInfo.stAgcInfo.onoff)
    {
        this->stAlgoInfo.stAgcInfo.uintUserMode         = this->pDb->GetMod<unsigned int>("AGC_PARAM", "USER_MOD");
        this->stAlgoInfo.stAgcInfo.uintDropGainMax      = this->pDb->GetMod<unsigned int>("AGC_PARAM", "DROP_GAIN_MAX");
        this->stAlgoInfo.stAgcInfo.uintAttackTime       = this->pDb->GetMod<unsigned int>("AGC_PARAM", "ATTACK_TIME");
        this->stAlgoInfo.stAgcInfo.uintReleaseTime      = this->pDb->GetMod<unsigned int>("AGC_PARAM", "RELEASE_TIME");
        this->stAlgoInfo.stAgcInfo.intDropGainThreshold = this->pDb->GetMod<int>("AGC_PARAM", "DROP_GAIN_THRESHOLD");
    }

    this->stAlgoInfo.stSedInfo.onoff = this->pDb->GetMod<unsigned int>("SED");
    if (this->stAlgoInfo.stSedInfo.onoff)
    {
        this->stAlgoInfo.stSedInfo.strModelPath      = this->pDb->GetMod<std::string>("SED_PARAM", "MODEL_PATH");
        this->stAlgoInfo.stSedInfo.intDetectMode     = this->pDb->GetMod<int>("SED_PARAM", "DETECT_MODE");
        this->stAlgoInfo.stSedInfo.intSmoothLength   = this->pDb->GetMod<int>("SED_PARAM", "SMOOTH_LENGTH");
        this->stAlgoInfo.stSedInfo.fVadTheshold      = this->pDb->GetMod<float>("SED_PARAM", "VAD_THESHOLD");
        this->stAlgoInfo.stSedInfo.fEventTheshold[0] = this->pDb->GetMod<float>("SED_PARAM", "EVENT_THESHOLD_0");
        this->stAlgoInfo.stSedInfo.fEventTheshold[1] = this->pDb->GetMod<float>("SED_PARAM", "EVENT_THESHOLD_1");
    }

    this->stAlgoInfo.stSrcInfo.onoff = this->pDb->GetMod<unsigned int>("SRC");
    if (this->stAlgoInfo.stSrcInfo.onoff)
    {
        this->stAlgoInfo.stSrcInfo.uintorder         = this->pDb->GetMod<unsigned int>("SRC_PARAM", "ORDER");
        this->stAlgoInfo.stSrcInfo.uintOutSampleRate = this->pDb->GetMod<unsigned int>("SRC_PARAM", "OUT_SAMPLE_RATE");
    }
}

void AmigosSurfaceAudioAlgo::_UnloadDb()
{
    stAlgoInfo.Clear();
}

void AmigosSurfaceAudioAlgo::GetInfo(AlgoInfo &info) const
{
    info = this->stAlgoInfo;
}

void AmigosSurfaceAudioAlgo::SetInfo(const AlgoInfo &info)
{
    this->stAlgoInfo = info;
}
