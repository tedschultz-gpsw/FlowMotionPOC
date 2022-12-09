/*! @file GPMFAssetSourceMP4Reader.cpp
 *
 *  @brief libGPMF
 *
 *  @version 1.0.0
 *
 *  (C) Copyright 2019 GoPro Inc (http://gopro.com/).
 *
 *  Licensed under either:
 *  - Apache License, Version 2.0, http://www.apache.org/licenses/LICENSE-2.0
 *  - MIT license, http://opensource.org/licenses/MIT
 *  at your option.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "GPMFAssetSource.h"
#include "GPMF_parser.h"
#include "GPMF_mp4reader.h"
#include "GPMF_utils.h"
#include "GPMFAssetSourceMP4Reader.h"

GPMFAssetSourceMP4Reader::GPMFAssetSourceMP4Reader()
{
    m_IsUDTA = false;
    m_MP4Object = NULL;
    m_MP4PayloadResource = NULL;
}

GPMFAssetSourceMP4Reader::~GPMFAssetSourceMP4Reader()
{
    Close();
}

GPMF_LIB_ERROR GPMFAssetSourceMP4Reader::Open(char* filename)
{
    GPMF_LIB_ERROR err = GPMF_LIB_OK;
    Close();
    m_IsUDTA = false;
    m_MP4Object = OpenMP4Source(filename, MOV_GPMF_TRAK_TYPE, MOV_GPMF_TRAK_SUBTYPE, 0);
    if(!m_MP4Object)
    {
        err = GPMF_LIB_ERROR_FILE_OPEN_FAILED;
    }
    return err;
}

GPMF_LIB_ERROR GPMFAssetSourceMP4Reader::OpenUDTA(char* filename)
{
    GPMF_LIB_ERROR err = GPMF_LIB_OK;
    m_IsUDTA = true;
    Close();
    m_MP4Object = OpenMP4SourceUDTA(filename, 0);
    if(!m_MP4Object)
    {
        err = GPMF_LIB_ERROR_FILE_OPEN_FAILED;
    }
    return err;
}

void GPMFAssetSourceMP4Reader::Close()
{
    if (m_MP4PayloadResource)
    {
        ::FreePayloadResource(m_MP4PayloadResource);
    }
    m_MP4PayloadResource = NULL;

    if(m_MP4Object)
    {
        ::CloseSource(m_MP4Object);
    }
    m_MP4Object = NULL;
}

float GPMFAssetSourceMP4Reader::GetAssetDuration()
{
    return GetDuration(m_MP4Object);
}

GPMFRational GPMFAssetSourceMP4Reader::GetVideoFrameRateAndCount(GPMF_LIB_ERROR& error, uint32_t *count)
{
    GPMFRational rational = {0,0};
    *count = 0;
    error = GPMF_LIB_OK;
    uint32_t numer = 0;
    uint32_t demon  = 0;
    uint32_t frameCount = 0;
    
    frameCount = ::GetVideoFrameRateAndCount(m_MP4Object, &numer, &demon);
    *count = frameCount;
    rational.denominator = (int32_t)demon;
    rational.numerator = (int32_t) numer;
    
    return rational;
}

GPMF_LIB_ERROR GPMFAssetSourceMP4Reader::GetAssetDuration(GPMFRational *rationalTime)
{
    float duration = GetDuration(m_MP4Object);
    //Double2Rational(duration, 100, rationalTime);
    *rationalTime = (double)duration;
    return GPMF_LIB_OK;
}

double GPMFAssetSourceMP4Reader::GetSampleRate(uint32_t fourcc)
{
    double firstsampletime = 0;
    double lastsampletime = 0;
    return GetSampleRate(fourcc, &firstsampletime, &lastsampletime);
}

double GPMFAssetSourceMP4Reader::GetSampleRate(uint32_t fourcc, double *firstsampletime, double *lastsampletime)

{
    mp4callbacks cbobject;
    cbobject.mp4handle = m_MP4Object;
    cbobject.cbGetNumberPayloads = ::GetNumberPayloads;
    cbobject.cbGetPayload = ::GetPayload;
    cbobject.cbGetPayloadSize = ::GetPayloadSize;
    cbobject.cbGetPayloadResource = ::GetPayloadResource;
    cbobject.cbGetPayloadTime = ::GetPayloadTime;
    cbobject.cbFreePayloadResource = ::FreePayloadResource;
    cbobject.cbGetEditListOffsetRationalTime = ::GetEditListOffsetRationalTime;

    return ::GetGPMFSampleRate(cbobject, fourcc, 0, GPMF_SAMPLE_RATE_FAST,
                             firstsampletime, lastsampletime);
}

uint32_t *GPMFAssetSourceMP4Reader::GetPayload(uint32_t *lastpayload, uint32_t index)
{
    uint32_t payload_size = GetPayloadSize(index);
    m_MP4PayloadResource = ::GetPayloadResource(m_MP4PayloadResource, payload_size);

    return ::GetPayload(m_MP4Object, m_MP4PayloadResource, index);
}

uint32_t GPMFAssetSourceMP4Reader::GetPayloadCount()
{
    return ::GetNumberPayloads(m_MP4Object);
}

uint32_t GPMFAssetSourceMP4Reader::GetPayloadSize(uint32_t index)
{
    uint32_t payloadsize =  ::GetPayloadSize(m_MP4Object, index);
    return payloadsize;
}

uint32_t GPMFAssetSourceMP4Reader::GetPayloadTime(uint32_t index, double *in, double *out)
{
    if(m_IsUDTA == true)
    {
        *in = GPMF_GLOBAL_TIME;
        *out = 0;
        return 0;
    } else {
        return ::GetPayloadTime(m_MP4Object, index, in, out);
    }
}

void GPMFAssetSourceMP4Reader::ModifyGlobalSettings(GPMF_LIB_ERROR& error, void *ms, uint32_t origfourCC, uint32_t newfourCC, char newType, uint32_t newStructSize, uint32_t newRepeat, void* newData)
{
    error = GPMF_LIB_FAIL;
    uint32_t count = GetPayloadCount();
    
    if(count == 1)
    {
        GPMF_stream fs;
        ::GPMF_CopyState((GPMF_stream*)ms, &fs);
        ::GPMF_ResetState(&fs);
        
        GPMF_ERR err = ::GPMF_Modify(&fs, origfourCC, newfourCC, (GPMF_SampleType) newType, newStructSize, newRepeat, newData);
        if(GPMF_OK == err)
        {
            uint32_t *payload = fs.buffer;
            uint32_t payloadsize = GetPayloadSize(0);
            uint32_t writesize = ::WritePayload(m_MP4Object, payload, payloadsize, 0);
            if (writesize == payloadsize)
            {
                error = GPMF_LIB_OK;
            } else {
                error = GPMF_LIB_FAIL;
            }
        }
    }
}
