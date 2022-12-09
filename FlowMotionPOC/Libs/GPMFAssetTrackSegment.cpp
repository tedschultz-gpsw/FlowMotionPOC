/*! @file GPMFAssetTrackSegment.cpp
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
#include "GPMFAssetTrackSegment.h"
#include "GPMF_parser.h"
#include "GPMF_mp4reader.h"
#include "GPMF_utils.h"

GPMFAssetTrackSegment::GPMFAssetTrackSegment(GPMFAssetSource& gpmfasset, uint32_t trackType)
{
    m_Asset = &gpmfasset;

    m_AssetTrack = NULL;
    m_SampleSegment = NULL;
    m_FirstSampleTime = 0.0;
    m_LastSampleTime = 0.0;

    m_Sample.sampleKey = trackType;
    m_Sample.sampleCount = 0;
    m_Sample.sampleTypeSize = 0;
    m_Sample.sampleType = 0;
    m_Sample.sampleTSMP = 0;
    m_SampleSegment = NULL;
}

GPMFAssetTrackSegment::~GPMFAssetTrackSegment()
{
    if(m_SampleSegment)
        delete m_SampleSegment;
}

double GPMFAssetTrackSegment::GetSampleRate(uint32_t fourcc, uint32_t tracks, double *firstSampletime, double *lastSampletime)
{
    GPMF_stream metadata_stream, *ms = &metadata_stream;
    double metadatalength = *lastSampletime;
    uint32_t teststart = 0;
    uint32_t testend = tracks;
    uint32_t indexcount = tracks;
    double rate = 0.0;
    uint32_t *payload = NULL;
    uint32_t payloadsize = 0;
    uint32_t ret = 0;
    uint32_t flags = GPMF_SAMPLE_RATE_FAST;
    
    if (tracks < 1)
        return 0.0;
    
    payload = m_Asset->GetPayload(NULL, teststart); // second payload
    payloadsize = m_Asset->GetPayloadSize(teststart);
    ret = GPMF_Init(ms, payload, (int)payloadsize);
    
    if (ret != GPMF_OK)
        goto cleanup;
    
    {
        uint64_t minimumtimestamp = 0;
        uint64_t starttimestamp = 0;
        uint64_t endtimestamp = 0;
        uint32_t startsamples = 0;
        uint32_t endsamples = 0;
        double intercept = 0.0;
        
        while (teststart < tracks && ret == GPMF_OK && GPMF_OK != GPMF_FindNext(ms, fourcc, GPMF_RECURSE_LEVELS))
        {
            teststart++;
            payload = m_Asset->GetPayload(payload, teststart); // second last payload
            payloadsize = m_Asset->GetPayloadSize(teststart);
            ret = GPMF_Init(ms, payload, (int)payloadsize);
        }
        
        if (ret == GPMF_OK && payload)
        {
            uint32_t samples = GPMF_PayloadSampleCount(ms);
            GPMF_stream find_stream;
            GPMF_CopyState(ms, &find_stream);
            if (GPMF_OK == GPMF_FindPrev(&find_stream, GPMF_KEY_TOTAL_SAMPLES, GPMF_CURRENT_LEVEL))
                startsamples = BYTESWAP32(*(uint32_t *)GPMF_RawData(&find_stream)) - samples;
            
            GPMF_CopyState(ms, &find_stream);
            if (GPMF_OK == GPMF_FindPrev(&find_stream, GPMF_KEY_TIME_STAMP, GPMF_CURRENT_LEVEL))
                starttimestamp = BYTESWAP64(*(uint64_t *)GPMF_RawData(&find_stream));
            
            if (starttimestamp) // is this earliest in the payload, examine the other streams in this early payload.
            {
                GPMF_stream any_stream;
                GPMF_Init(&any_stream, payload, (int)payloadsize);
                
                minimumtimestamp = starttimestamp;
                while (GPMF_OK == GPMF_FindNext(&any_stream, GPMF_KEY_TIME_STAMP, GPMF_RECURSE_LEVELS))
                {
                    uint64_t timestamp = BYTESWAP64(*(uint64_t *)GPMF_RawData(&any_stream));
                    if (timestamp < minimumtimestamp)
                        minimumtimestamp = timestamp;
                }
            }
            
            testend = tracks;
            do
            {
                testend--;// last payload with the fourcc needed
                payload = m_Asset->GetPayload(payload, testend);
                payloadsize = m_Asset->GetPayloadSize(testend);
                ret = GPMF_Init(ms, payload, (int)payloadsize);
            } while (testend > 0 && GPMF_OK != GPMF_FindNext(ms, fourcc, GPMF_RECURSE_LEVELS));
            
            GPMF_CopyState(ms, &find_stream);
            if (GPMF_OK == GPMF_FindPrev(&find_stream, GPMF_KEY_TOTAL_SAMPLES, GPMF_CURRENT_LEVEL))
                endsamples = BYTESWAP32(*(uint32_t *)GPMF_RawData(&find_stream));
            else // If there is no TSMP we have to count the samples.
            {
                uint32_t i;
                for (i = teststart; i <= testend; i++)
                {
                    payload = m_Asset->GetPayload(payload, i); // second last payload
                    payloadsize = m_Asset->GetPayloadSize(i);
                    if (GPMF_OK == GPMF_Init(ms, payload, (int)payloadsize))
                        if (GPMF_OK == GPMF_FindNext(ms, fourcc, GPMF_RECURSE_LEVELS))
                            endsamples += GPMF_PayloadSampleCount(ms);
                }
            }
            
            if (starttimestamp != 0)
            {
                uint32_t last_samples = GPMF_PayloadSampleCount(ms);
                uint32_t totaltimestamped_samples = endsamples - last_samples - startsamples;
                double time_stamp_scale = 1000000000.0; // scan for nanoseconds, microseconds to seconds, all base 10.
                
                GPMF_CopyState(ms, &find_stream);
                if (GPMF_OK == GPMF_FindPrev(&find_stream, GPMF_KEY_TIME_STAMP, GPMF_CURRENT_LEVEL))
                    endtimestamp = BYTESWAP64(*(uint64_t *)GPMF_RawData(&find_stream));
                
                if (endtimestamp)
                {
                    double approxrate = 1;
                    if (endsamples > startsamples)
                        approxrate = (double)(endsamples - startsamples) / (metadatalength * ((double)(testend - teststart + 1)) / (double)indexcount);
                    
                    if (approxrate == 0.0)
                        approxrate = (double)(samples) / (metadatalength * ((double)(testend - teststart + 1)) / (double)indexcount);
                    
                    
                    while (time_stamp_scale >= 1)
                    {
                        rate = (double)(totaltimestamped_samples) / ((double)(endtimestamp - starttimestamp) / time_stamp_scale);
                        if (rate*0.9 < approxrate && approxrate < rate*1.1)
                            break;
                        
                        time_stamp_scale *= 0.1;
                    }
                    if (time_stamp_scale < 1.0) rate = 0.0;
                    intercept = (((double)minimumtimestamp - (double)starttimestamp) / time_stamp_scale) * rate;
                }
            }
            
            if (rate == 0.0) //Timestamps didn't help weren't available
            {
                if (!(flags & GPMF_SAMPLE_RATE_PRECISE))
                {
                    if (endsamples > startsamples)
                        rate = (double)(endsamples - startsamples) / (metadatalength * ((double)(testend - teststart + 1)) / (double)indexcount);
                    
                    if (rate == 0.0)
                        rate = (double)(samples) / (metadatalength * ((double)(testend - teststart + 1)) / (double)indexcount);
                    
                    double in, out;
                    if (GPMF_OK == m_Asset->GetPayloadTime(teststart, &in, &out))
                        intercept = (double)-in * rate;
                } else // for increased precision, for older GPMF streams sometimes missing the total sample count
                {
                    uint32_t payloadpos = 0, payloadcount = 0;
                    double slope, top = 0.0, bot = 0.0, meanX = 0, meanY = 0;
                    uint32_t *repeatarray = (uint32_t *)malloc(tracks * 4 + 4);
                    memset(repeatarray, 0, tracks * 4 + 4);
                    
                    samples = 0;
                    
                    for (payloadpos = teststart; payloadpos <= testend; payloadpos++)
                    {
                        payload = m_Asset->GetPayload(payload, payloadpos); // second last payload
                        payloadsize = m_Asset->GetPayloadSize(payloadpos);
                        ret = GPMF_Init(ms, payload, (int)payloadsize);
                        
                        if (ret != GPMF_OK)
                            goto cleanup;
                        
                        if (GPMF_OK == GPMF_FindNext(ms, fourcc, GPMF_RECURSE_LEVELS))
                        {
                            GPMF_stream find_stream2;
                            GPMF_CopyState(ms, &find_stream2);
                            
                            payloadcount++;
                            
                            if (GPMF_OK == GPMF_FindNext(&find_stream2, fourcc, GPMF_CURRENT_LEVEL)) // Count the instances, not the repeats
                            {
                                if (repeatarray)
                                {
                                    double in, out;
                                    
                                    do
                                    {
                                        samples++;
                                    } while (GPMF_OK == GPMF_FindNext(ms, fourcc, GPMF_CURRENT_LEVEL));
                                    
                                    repeatarray[payloadpos] = samples;
                                    meanY += (double)samples;
                                    
                                    if (GPMF_OK == m_Asset->GetPayloadTime(payloadpos, &in, &out))
                                        meanX += out;
                                }
                            }
                            else
                            {
                                uint32_t repeat = GPMF_PayloadSampleCount(ms);
                                samples += repeat;
                                
                                if (repeatarray)
                                {
                                    double in, out;
                                    
                                    repeatarray[payloadpos] = samples;
                                    meanY += (double)samples;
                                    
                                    if (GPMF_OK == m_Asset->GetPayloadTime(payloadpos, &in, &out))
                                        meanX += out;
                                }
                            }
                        }
                        else
                        {
                            repeatarray[payloadpos] = 0;
                        }
                    }
                    
                    // Compute the line of best fit for a jitter removed sample rate.
                    // This does assume an unchanging clock, even though the IMU data can thermally impacted causing small clock changes.
                    // TODO: Next enhancement would be a low order polynominal fit the compensate for any thermal clock drift.
                    if (repeatarray)
                    {
                        meanY /= (double)payloadcount;
                        meanX /= (double)payloadcount;
                        
                        for (payloadpos = teststart; payloadpos <= testend; payloadpos++)
                        {
                            double in, out;
                            if (repeatarray[payloadpos] && GPMF_OK == m_Asset->GetPayloadTime(payloadpos, &in, &out))
                            {
                                top += ((double)out - meanX)*((double)repeatarray[payloadpos] - meanY);
                                bot += ((double)out - meanX)*((double)out - meanX);
                            }
                        }
                        
                        slope = top / bot;
                        rate = slope;
                        
                        // This sample code might be useful for compare data latency between channels.
                        intercept = meanY - slope * meanX;
#if 0
                        printf("%c%c%c%c start offset = %f (%.3fms) rate = %f\n", PRINTF_4CC(fourcc), intercept, 1000.0 * intercept / slope, rate);
                        printf("%c%c%c%c first sample at time %.3fms\n", PRINTF_4CC(fourcc), -1000.0 * intercept / slope);
#endif
                    }
                    else
                    {
                        rate = (double)(samples) / (metadatalength * ((double)(testend - teststart + 1)) / (double)indexcount);
                    }
                    
                    free(repeatarray);
                }
            }
            
            if (m_FirstSampleTime && m_LastSampleTime)
            {
                uint32_t endpayload = tracks;
                do
                {
                    endpayload--;// last payload with the fourcc needed
                    payload = m_Asset->GetPayload(payload, endpayload);
                    payloadsize = m_Asset->GetPayloadSize(endpayload);
                    ret = GPMF_Init(ms, payload, (int)payloadsize);
                } while (endpayload > 0 && GPMF_OK != GPMF_FindNext(ms, fourcc, GPMF_RECURSE_LEVELS));
                
                if (endpayload > 0 && ret == GPMF_OK)
                {
                    uint32_t totalsamples = endsamples - startsamples;
                    float timo = 0.0;
                    
                    GPMF_CopyState(ms, &find_stream);
                    if (GPMF_OK == GPMF_FindPrev(&find_stream, GPMF_KEY_TIME_OFFSET, GPMF_CURRENT_LEVEL))
                        GPMF_FormattedData(&find_stream, &timo, 4, 0, 1);
                    
                    double first, last;
                    first = -intercept / rate - timo;
                    last = first + (double)totalsamples / rate;
                    
                    //printf("%c%c%c%c first sample at time %.3fms, last at %.3fms\n", PRINTF_4CC(fourcc), 1000.0*first, 1000.0*last);
                    
                    if (m_FirstSampleTime) *firstSampletime = first;
                    
                    if (m_LastSampleTime) *lastSampletime = last;
                }
            }
        }
    }
    
cleanup:
//        if (payload)
//            FreePayload(payload);
//        payload = NULL;
    
    return rate;
}

GPMF_LIB_ERROR GPMFAssetTrackSegment::GetSamples(GPMFAssetTrack *trackSample)
{
    return GetSamples(trackSample, m_Sample.sampleKey, 0, 0);
}

GPMF_LIB_ERROR GPMFAssetTrackSegment::GetAllSamples(GPMFAssetTrack *trackSample)
{
    return GetSamples(trackSample, 0, 0, 0);
}

GPMF_LIB_ERROR GPMFAssetTrackSegment::GetSamples(GPMFAssetTrack *trackSample, uint32_t trackType, double inTime, double outTime)
{
    memset(&m_Sample, 0, sizeof(GPMFSample));
    GPMF_stream *sample = (GPMF_stream *)m_Asset->m_MS;
    m_AssetTrack = trackSample;
    uint32_t sampleCount = 0;
    GPMF_ERR retVal = GPMF_OK;
    auto recurse = (GPMF_LEVELS) (GPMF_RECURSE_LEVELS|GPMF_TOLERANT);

    if(m_AssetTrack->m_TrackSegmentInTime == GPMF_GLOBAL_TIME)
    {
        m_Sample.sampleTime = GPMF_GLOBAL_TIME;
    }

    while (retVal == GPMF_OK)
    {
        if(trackType == 0)
        {
            retVal = GPMF_Next(sample, recurse);
        } else {
            retVal = GPMF_FindNext(sample, trackType, recurse);
        }
        
        if(retVal != GPMF_OK)
        {
            break;
        }
        
        sampleCount++;
        m_Sample.sampleKey = GPMF_Key(sample);
        
        if(m_Sample.sampleKey != 0)
        {
            
            m_Sample.sampleType = GPMF_Type(sample);
            m_Sample.sampleStructSize = GPMF_StructSize((GPMF_stream *)sample);
            m_Sample.sampleRepeat = GPMF_Repeat((GPMF_stream *)sample);
            m_Sample.sampleCount = GPMF_PayloadSampleCount(sample);
            m_Sample.sampleElementsInStruct = GPMF_ElementsInStruct(sample);
            m_Sample.sampleNest = GPMF_NestLevel((GPMF_stream *)sample);
            uint32_t deviceID = GPMF_DeviceID(sample);
            
            
            if( deviceID == 1 || deviceID == 1001)  // Camera or Karma
            {
                m_Sample.deviceID = deviceID;
            } else {
                m_Sample.deviceID = deviceID;//BYTESWAP32(deviceID);
            }
            
            GPMF_DeviceName(sample, (char *)m_Sample.deviceName, 32);
            
            if(m_Sample.sampleType == GPMF_TYPE_COMPLEX)
            {
                if(m_Sample.sampleRepeat == 0)
                {
                    m_Sample.sampleRepeat = 1;
                }
                GPMF_stream find_stream2;
                GPMF_CopyState((GPMF_stream *)m_Asset->m_MS, &find_stream2);
                if (GPMF_OK == GPMF_FindPrev(&find_stream2, GPMF_KEY_TYPE, GPMF_CURRENT_LEVEL))
                {
                    char *srctype =  (char *)GPMF_RawData(&find_stream2);
                    strcpy(m_Sample.complexType, srctype);
                    uint32_t typelen = GPMF_RawDataSize(&find_stream2);
                    uint32_t struct_size_of_type = GPMF_SizeOfComplexTYPE(srctype, typelen);
                    
                    m_Sample.sampleTypeSize = typelen;
                    
                    if(struct_size_of_type >  0)
                    {
                        m_Sample.sampleTypeSize = typelen * struct_size_of_type;
                    } else {
                        m_Sample.sampleTypeSize = m_Sample.sampleStructSize;
                    }
                }
            } else {
                m_Sample.sampleTypeSize = GPMF_SizeofType((GPMF_SampleType)m_Sample.sampleType);
            }
            
            // TSMP
            {
                GPMF_stream find_stream;
                GPMF_CopyState((GPMF_stream *)m_Asset->m_MS, &find_stream);
                if (GPMF_OK == GPMF_FindPrev(&find_stream, GPMF_KEY_TOTAL_SAMPLES, GPMF_CURRENT_LEVEL))
                {
                    GPMF_FormattedData(sample, &m_Sample.sampleTSMP, 4, 0, 1);
                } else {
                    m_Sample.sampleTSMP = 0;
                }
            }
            
            // STMP
            {
                GPMF_stream find_stream;
                GPMF_CopyState((GPMF_stream *)m_Asset->m_MS, &find_stream);
                if (GPMF_OK == GPMF_FindPrev(&find_stream, GPMF_KEY_TIME_STAMP, GPMF_CURRENT_LEVEL))
                {
                    GPMF_FormattedData(&find_stream, (void *)&m_Sample.sampleSTMP, 8, 0, 1);
                } else {
                    m_Sample.sampleSTMP = 0;
                }
            }
            
            // SCAL
            {
                GPMF_stream find_stream;
                GPMF_CopyState((GPMF_stream *)m_Asset->m_MS, &find_stream);
                if (GPMF_OK == GPMF_FindPrev(&find_stream, GPMF_KEY_SCALE, GPMF_CURRENT_LEVEL))
                {
                    uint32_t sampleStructSize = GPMF_StructSize(&find_stream);
                    GPMF_FormattedData(&find_stream, (void *)&m_Sample.sampleSCAL, sampleStructSize, 0, 1);
                } else {
                    m_Sample.sampleSCAL = 1;
                }
            }
            
            GPMFAssetSamples assetSamples(*m_Asset);
            assetSamples.GetNextSampleSegment((void *)this, sample, inTime, outTime);
        }
    }
    
    GPMF_Free((GPMF_stream *)sample);
    
    if( sampleCount == 0) {
        return GPMF_LIB_ERROR_TYPE_NOT_FOUND;
    } else {
        return GPMF_LIB_OK;
    }
}


