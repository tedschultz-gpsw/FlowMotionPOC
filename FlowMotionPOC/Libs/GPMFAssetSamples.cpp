/*! @file GPMFAssetSamples.cpp
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

#include <stdint.h>
#include "GPMFTimeline.h"
#include "GPMFAssetSamples.h"
#include "GPMFAssetSample.h"
#include "GPMF_mp4reader.h"
#include "GPMFAssetSource.h"
#include "GPMF_parser.h"
#include "GPMF_mp4reader.h"
#include "GPMFTypes.h"


GPMFAssetSamples::GPMFAssetSamples(GPMFAssetSource& gpmfasset)
{
    m_Asset = &gpmfasset;
}

GPMFAssetSamples::~GPMFAssetSamples()
{
}

uint32_t GPMFAssetSamples::GetNextSampleSegment(void *assetTrackSample, void *trackSample, double inTime, double outTime)
{
    GPMF_ERR err = GPMF_OK;
    uint32_t bufferSize = 0;
    
    GPMFAssetTrackSegment *gpmfassetTrackSample = (GPMFAssetTrackSegment *)assetTrackSample;
    
    //Skip over these
    if(gpmfassetTrackSample->m_Sample.sampleKey == FOURCC("DVID") || gpmfassetTrackSample->m_Sample.sampleKey == FOURCC("DVNM") ||
       gpmfassetTrackSample->m_Sample.sampleKey == FOURCC("STRM") ||  gpmfassetTrackSample->m_Sample.sampleKey == FOURCC("DEVC"))
    {
        return GPMF_OK;
    }
    
    if(gpmfassetTrackSample->m_Sample.sampleSCAL != 1)
    {
        GPMF_SampleType type =  GPMF_TYPE_FLOAT;
        if(gpmfassetTrackSample->m_Sample.sampleKey == STR2FOURCC("GPS5")) {
            type = GPMF_TYPE_DOUBLE;
        }
        bufferSize = GPMF_ScaledDataSize((GPMF_stream *)trackSample, type);
    } else {
        bufferSize = GPMF_FormattedDataSize((GPMF_stream *)trackSample);
    }
    
    unsigned char *buffer = (unsigned char *)malloc(bufferSize);
    
    if(!buffer)
    {
        return GPMF_ERROR_MEMORY;
    }
    
    memset(buffer, 0, bufferSize);
    
    if (gpmfassetTrackSample->m_Sample.sampleCount > 0)
    {
        if(gpmfassetTrackSample->m_Sample.sampleType == GPMF_TYPE_COMPLEX ){
            err = GPMF_FormattedData((GPMF_stream *)trackSample, buffer, bufferSize, 0, gpmfassetTrackSample->m_Sample.sampleRepeat);
        } else {
            if(gpmfassetTrackSample->m_Sample.sampleSCAL != 1) {
                GPMF_SampleType gpmfType = GPMF_TYPE_FLOAT;
                if(gpmfassetTrackSample->m_Sample.sampleKey == STR2FOURCC("GPS5")) {
                    gpmfType = GPMF_TYPE_DOUBLE;
                }
                err = GPMF_ScaledData((GPMF_stream *)trackSample, buffer, bufferSize, 0, gpmfassetTrackSample->m_Sample.sampleRepeat, gpmfType);
                gpmfassetTrackSample->m_Sample.sampleType = gpmfType;
                gpmfassetTrackSample->m_Sample.sampleSCAL = 1;
                gpmfassetTrackSample->m_Sample.sampleTypeSize = GPMF_SizeofType(gpmfType);
                gpmfassetTrackSample->m_Sample.sampleStructSize = gpmfassetTrackSample->m_Sample.sampleTypeSize * gpmfassetTrackSample->m_Sample.sampleElementsInStruct;
            } else {
                err = GPMF_FormattedData((GPMF_stream *)trackSample, buffer, bufferSize, 0, gpmfassetTrackSample->m_Sample.sampleCount);
            }
        }
        
        unsigned char *sampleBuffer = buffer;
        double sampleDurationInSecs =  ((gpmfassetTrackSample->m_AssetTrack->m_TrackSegmentOutTime) - (gpmfassetTrackSample->m_AssetTrack->m_TrackSegmentInTime));
        double sampleTime = gpmfassetTrackSample->m_AssetTrack->m_TrackSegmentInTime;
        double sampleCount = gpmfassetTrackSample->m_Sample.sampleRepeat;
       
        // check for infinity
        if(sampleDurationInSecs > .001)
        {
            gpmfassetTrackSample->m_Sample.sampleRate = sampleCount/sampleDurationInSecs;
        } else {
            gpmfassetTrackSample->m_Sample.sampleRate = 1;
        }
        
        double secspersample = (sampleDurationInSecs/sampleCount);
        
        if(gpmfassetTrackSample->m_Sample.sampleType == GPMF_TYPE_COMPLEX || gpmfassetTrackSample->m_Sample.sampleType == GPMF_STRING_UTF8 ||
           gpmfassetTrackSample->m_Sample.sampleType == GPMF_STRING || sampleTime == GPMF_GLOBAL_TIME)
        {
            AddAssetTimeSample(gpmfassetTrackSample, sampleTime, gpmfassetTrackSample->m_Sample.sampleRate, sampleBuffer, bufferSize, gpmfassetTrackSample->m_Sample.sampleRepeat);
            
        } else {
            uint32_t sampleBufferSize =  gpmfassetTrackSample->m_Sample.sampleStructSize;
            for (uint32_t i = 0; i < sampleCount; i++)
            {
                AddAssetTimeSample(gpmfassetTrackSample, sampleTime, gpmfassetTrackSample->m_Sample.sampleRate, sampleBuffer, sampleBufferSize, 1);
                
                sampleTime += secspersample;
                sampleBuffer += sampleBufferSize;
            }
        }
    }
    
    free(buffer);
    return GPMF_OK;
}

GPMFAssetSample *GPMFAssetSamples::AddAssetTimeSample(void *assetTrackSample, double sampleTime, double sampleRate, void *sampleBuffer, uint32_t sampleSize, uint32_t sampleRepeat)
{
    GPMFAssetTrackSegment *gpmfassetTrackSample = (GPMFAssetTrackSegment *)assetTrackSample;
    GPMFAssetSample *gpmfAssetSample = new GPMFAssetSample;
    if(gpmfAssetSample)
    {
        gpmfAssetSample->m_Sample.sampleKey = gpmfassetTrackSample->m_Sample.sampleKey;
        gpmfAssetSample->m_Sample.sampleType = gpmfassetTrackSample->m_Sample.sampleType;
        gpmfAssetSample->m_Sample.sampleStructSize = gpmfassetTrackSample->m_Sample.sampleStructSize;
        gpmfAssetSample->m_Sample.sampleRepeat = sampleRepeat;
        gpmfAssetSample->m_Sample.sampleCount = gpmfassetTrackSample->m_Sample.sampleElementsInStruct;
        gpmfAssetSample->m_Sample.sampleElementsInStruct = gpmfassetTrackSample->m_Sample.sampleElementsInStruct;
        gpmfAssetSample->m_Sample.sampleTSMP = gpmfassetTrackSample->m_Sample.sampleTSMP;
        gpmfAssetSample->m_Sample.sampleSTMP = gpmfassetTrackSample->m_Sample.sampleSTMP;
        gpmfAssetSample->m_Sample.sampleNest = gpmfassetTrackSample->m_Sample.sampleNest;
        gpmfAssetSample->m_Sample.deviceID = gpmfassetTrackSample->m_Sample.deviceID;
        gpmfAssetSample->m_Sample.sampleSCAL = gpmfassetTrackSample->m_Sample.sampleSCAL;
        strcpy (gpmfAssetSample->m_Sample.deviceName, gpmfassetTrackSample->m_Sample.deviceName);
        gpmfAssetSample->m_Sample.sampleTypeSize = gpmfassetTrackSample->m_Sample.sampleTypeSize;
        gpmfAssetSample->m_Sample.sampleTime = sampleTime;
        gpmfAssetSample->m_Sample.sampleRate = sampleRate;
        if(sampleSize > 0)
        {
            GPMF_LIB_ERROR err = gpmfAssetSample->AllocateBuffer(sampleSize);
            if(err != GPMF_LIB_OK)
            {
                gpmfAssetSample->m_Sample.sampleBuffer = NULL;
            }
        } else {
            gpmfAssetSample->m_Sample.sampleBuffer = NULL;
        }
        if(gpmfAssetSample->m_Sample.sampleBuffer)
        {
            memset(gpmfAssetSample->m_Sample.sampleBuffer, 0, sampleSize);
            gpmfAssetSample->m_Sample.sampleBufferSize = sampleSize;
            memcpy(gpmfAssetSample->m_Sample.sampleBuffer, sampleBuffer, sampleSize);
            strcpy (gpmfAssetSample->m_Sample.complexType, gpmfassetTrackSample->m_Sample.complexType);
            gpmfassetTrackSample->m_AssetTrack->m_AssetTrackTimeline->AddAssetTimeSample(gpmfAssetSample);
        } else {
            delete gpmfAssetSample;
            gpmfAssetSample = NULL;
        }
    }
    
    return gpmfAssetSample;
}

