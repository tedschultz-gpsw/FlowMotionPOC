/*! @file GPMFAssetTrackTimeline.cpp
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

#define PRINTF_4CC(k)            ((k) >> 0) & 0xff, ((k) >> 8) & 0xff, ((k) >> 16) & 0xff, ((k) >> 24) & 0xff
#define STR2FOURCC(s)            ((s[0]<<0)|(s[1]<<8)|(s[2]<<16)|(s[3]<<24))
#define PRINTF_4CCSwap(k)        ((k) >> 24) & 0xff, ((k) >> 16) & 0xff, ((k) >> 8) & 0xff, ((k) >> 0) & 0xff
#if VERBOSE_OUTPUT
#define LIMITOUTPUT        arraysize = structsize;
#else
#define LIMITOUTPUT        if (arraysize > 1 && repeat > 3) repeat = 3, dots = 1; else if (repeat > 6) repeat = 6, dots = 1;
#endif
#include "GPMFAssetTrackTimeline.h"
#include "GPMFAssetSource.h"
#include "GPMF_parser.h"
#include "GPMFTypes.h"
#include <vector>
#include <set>
#include <iostream>
#include <cctype>
#include <sstream>

#define VER_JSON   "1.0.0"
#ifdef __cplusplus
extern "C" {
#endif
    extern void PrintGPMF(GPMF_stream *ms);
#ifdef __cplusplus
}
#endif

GPMFAssetTrackTimeline::GPMFAssetTrackTimeline(GPMFAssetSource& gpmfasset)
{
    m_FP = NULL;
    m_Asset = &gpmfasset;
    m_MinimalJSON = false;
    m_CachedFourcc = 0;
}

GPMFAssetTrackTimeline::~GPMFAssetTrackTimeline()
{
    for (auto const &pair: m_TrackTimeline) {
        GPMFAssetSample *sample = pair.second;
        if(sample)
        {
            delete sample;
        }
    }
    
    for (auto timerange: m_Timeranges) {
        delete timerange;
    }
    
    m_Timeranges.clear();
}

void GPMFAssetTrackTimeline::AddAssetTimeSample(GPMFAssetSample *assetSample)
{
    m_TrackTimeline.push_back( std::pair<double, GPMFAssetSample*>(assetSample->m_Sample.sampleTime, assetSample) );
}

void GPMFAssetTrackTimeline::AddSegmentTime(double inTime, double outTime)
{
    double duration = outTime - inTime;
    m_SegmentTimes.push_back( std::pair<double, double>(inTime, duration) );
}

GPMF_LIB_ERROR GPMFAssetTrackTimeline::GetSampleCount(uint32_t fourcc, uint32_t *sampleCount)
{
    // this can be optimized
    std::vector<std::pair<double, GPMFAssetSample*> > afourccSubset = GetFourCCSet(fourcc, m_TrackTimeline);
    *sampleCount = (uint32_t)afourccSubset.size();
    afourccSubset.clear();
    return GPMF_LIB_OK;
}

GPMF_LIB_ERROR GPMFAssetTrackTimeline::GetSample(uint32_t fourcc, uint32_t index, GPMFSampleptr samplePtr)
{
    if(m_CachedFourcc != fourcc)
    {
        m_FourccSubset = GetFourCCSet(fourcc, m_TrackTimeline);
    }
    
    m_CachedFourcc = fourcc;
    
    size_t count = m_FourccSubset.size();
    
    if(index >= count)
        return GPMF_LIB_ERROR_MEMORY;
    
    std::pair<double, GPMFAssetSample*> sample = m_FourccSubset[index];
    
    GPMFAssetSample *assetSample = sample.second;
    if(assetSample)
    {
        *samplePtr = assetSample->m_Sample;
        if (samplePtr->sampleBuffer == NULL)
        {
            return GPMF_LIB_ERROR_MEMORY;
        }
        return GPMF_LIB_OK;
    } else {
        return GPMF_LIB_ERROR_MEMORY;
    }
}

uint32_t GPMFAssetTrackTimeline::GetSampleTimeRangeCount()
{
    return (uint32_t)m_SegmentTimes.size();
}

GPMFTimeRange *GPMFAssetTrackTimeline::GetSampleTimeRange(uint32_t index)
{
    if(index < m_SegmentTimes.size())
    {
        GPMFTimeRange *timeRange = new GPMFTimeRange;
        timeRange->m_startTime = m_SegmentTimes[index].first;
        timeRange->m_duration = m_SegmentTimes[index].second;
        m_Timeranges.push_back(timeRange); // keep for cleanup
        return timeRange;
    } else {
        return nullptr;
    }
}

GPMF_LIB_ERROR GPMFAssetTrackTimeline::ExportTimeline(FILE *file, GPMF_EXPORT_FILE_TYPE fileType)
{
    if(fileType == GPMF_FILE_TYPE_RAW)
    {
        PrintGPMFInternal(m_Asset->m_MS);
    } else {
        m_FP = file;
        //int count = m_TrackTimeline.size();
        
        std::stable_sort(m_TrackTimeline.begin(), m_TrackTimeline.end(), [](const std::pair<double, GPMFAssetSample*> & a, const std::pair<double, GPMFAssetSample*> & b){return a.first < b.first;});
        printJsonTimeline(m_TrackTimeline);
    }
    return GPMF_LIB_OK;
}

GPMF_LIB_ERROR GPMFAssetTrackTimeline::ExportTimeline(FILE *file, uint32_t fourcc, GPMF_EXPORT_FILE_TYPE fileType)
{
    m_FP = file;
    std::vector<std::pair<double, GPMFAssetSample*> > afourccSubset = GetFourCCSet(fourcc, m_TrackTimeline);
    printJsonTimeline(afourccSubset);
    afourccSubset.clear();
    return GPMF_LIB_OK;
}

GPMF_LIB_ERROR GPMFAssetTrackTimeline::ExportTimeline(FILE *file, uint32_t fourcc, double inTime, double outTime, GPMF_EXPORT_FILE_TYPE fileType)
{
    m_FP = file;
    std::vector<std::pair<double, GPMFAssetSample*> > afourccSubset = GetFourCCSet(fourcc, inTime, outTime, m_TrackTimeline);
    printJsonTimeline(afourccSubset);
    afourccSubset.clear();
    return GPMF_LIB_OK;
}

GPMF_LIB_ERROR GPMFAssetTrackTimeline::ExportTimeline(FILE *file, double inTime, double outTime, GPMF_EXPORT_FILE_TYPE fileType)
{
    m_FP = file;
    std::vector<std::pair<double, GPMFAssetSample*> > timedSubset = GetTimedSet(inTime, outTime, m_TrackTimeline);
    printJsonTimeline(timedSubset);
    return GPMF_LIB_OK;
}

std::vector<std::pair<double, GPMFAssetSample*> > GPMFAssetTrackTimeline::GetTimedSet(uint32_t in, uint32_t out, std::vector<std::pair<double, GPMFAssetSample*> > set)
{
    std::vector<std::pair<double, GPMFAssetSample*> > subset;
    for (auto const &pair: set) {
        
        GPMFAssetSample *sample = pair.second;
        if(sample->m_Sample.sampleTime >= in && sample->m_Sample.sampleTime <= out)
        {
            if(m_Asset->m_DeviceIdList.size() > 0)
            {
                std::vector<uint32_t>::iterator it;
                for(it = m_Asset->m_DeviceIdList.begin(); it != m_Asset->m_DeviceIdList.end(); it++)
                {
                    uint32_t fourc = *it;
                    
                    if(fourc == sample->m_Sample.deviceID)
                    {
                        subset.push_back(pair);
                        break;
                    }
                }
            } else if(m_Asset->m_DeviceNameList.size() > 0)
            {
                std::vector<std::string>::iterator it;
                for(it = m_Asset->m_DeviceNameList.begin(); it != m_Asset->m_DeviceNameList.end(); it++)
                {
                    if (it->compare(sample->m_Sample.deviceName) == 0)
                    {
                        subset.push_back(pair);
                        break;
                    }
                }
            } else {
                subset.push_back(pair);
            }
        }
    }
    std::stable_sort(subset.begin(), subset.end(), [](const std::pair<double, GPMFAssetSample*> & a, const std::pair<double, GPMFAssetSample*> & b){return a.first < b.first;});
    
    return subset;
}

std::vector<std::pair<double, GPMFAssetSample*> > GPMFAssetTrackTimeline::GetFourCCSet(uint32_t fourcc, std::vector<std::pair<double, GPMFAssetSample*> > set)
{
    std::vector<std::pair<double, GPMFAssetSample*> > subset;
    for (auto const &pair: set) {
        
        GPMFAssetSample *sample = pair.second;
        if(fourcc == sample->m_Sample.sampleKey)
        {
            if(m_Asset->m_DeviceIdList.size() > 0)
            {
                std::vector<uint32_t>::iterator it;
                for(it = m_Asset->m_DeviceIdList.begin(); it != m_Asset->m_DeviceIdList.end(); it++)
                {
                    uint32_t fourc = *it;
                    
                    if(fourc == sample->m_Sample.deviceID)
                    {
                        subset.push_back(pair);
                        break;
                    }
                }
            } else if(m_Asset->m_DeviceNameList.size() > 0)
            {
                std::vector<std::string>::iterator it;
                for(it = m_Asset->m_DeviceNameList.begin(); it != m_Asset->m_DeviceNameList.end(); it++)
                {
                    if (it->compare(sample->m_Sample.deviceName) == 0)
                    {
                        subset.push_back(pair);
                        break;
                    }
                }
            } else {
                subset.push_back(pair);
            }
        }
    }
    std::stable_sort(subset.begin(), subset.end(), [](const std::pair<double, GPMFAssetSample*> & a, const std::pair<double, GPMFAssetSample*> & b){return a.first < b.first;});
    
    return subset;
}

std::vector<std::pair<double, GPMFAssetSample*> > GPMFAssetTrackTimeline::GetFourCCSet(uint32_t fourcc, uint32_t in, uint32_t out, std::vector<std::pair<double, GPMFAssetSample*> > set)
{
    std::vector<std::pair<double, GPMFAssetSample*> > subset;
    for (auto const &pair: set) {
        
        GPMFAssetSample *sample = pair.second;
        if(fourcc == sample->m_Sample.sampleKey && sample->m_Sample.sampleTime >= in && sample->m_Sample.sampleTime <= out)
        {
            if(m_Asset->m_DeviceIdList.size() > 0)
            {
                std::vector<uint32_t>::iterator it;
                for(it = m_Asset->m_DeviceIdList.begin(); it != m_Asset->m_DeviceIdList.end(); it++)
                {
                    uint32_t fourc = *it;
                    
                    if(fourc == sample->m_Sample.deviceID)
                    {
                        subset.push_back(pair);
                        break;
                    }
                }
            } else if(m_Asset->m_DeviceNameList.size() > 0)
            {
                std::vector<std::string>::iterator it;
                for(it = m_Asset->m_DeviceNameList.begin(); it != m_Asset->m_DeviceNameList.end(); it++)
                {
                    if (it->compare(sample->m_Sample.deviceName) == 0)
                    {
                        subset.push_back(pair);
                        break;
                    }
                }
            } else {
                subset.push_back(pair);
            }
        }
    }
    std::stable_sort(subset.begin(), subset.end(), [](const std::pair<double, GPMFAssetSample*> & a, const std::pair<double, GPMFAssetSample*> & b){return a.first < b.first;});
    
    return subset;
}

void GPMFAssetTrackTimeline::printJsonTimeline(std::vector<std::pair<double, GPMFAssetSample*> > timeLine)
{
    fprintf(m_FP, "{");
    fprintf(m_FP, "\n  \"VERSION\": \"LIBGPMF2JSON %s\",\n", VER_JSON);
    uint32_t prev_milliseconds = 10000;
    uint32_t prev_deviceName[32] = {0};
    
    bool first_time = true;
    char buff[32];
    
    for (auto const &pair: timeLine) {
        GPMFAssetSample *sample = pair.second;
        bool foundDeviceID = false;
        // Check for deviceID
        if(m_Asset->m_DeviceIdList.size() > 0)
        {
            std::vector<uint32_t>::iterator it;
            for(it = m_Asset->m_DeviceIdList.begin(); it != m_Asset->m_DeviceIdList.end(); it++)
            {
                uint32_t fourc = *it;
                
                if(fourc == sample->m_Sample.deviceID)
                {
                    foundDeviceID = true;
                    break;
                }
            }
            
        } else if(m_Asset->m_DeviceNameList.size() > 0)
        {
            std::vector<std::string>::iterator it;
            for(it = m_Asset->m_DeviceNameList.begin(); it != m_Asset->m_DeviceNameList.end(); it++)
            {
                if (it->compare(sample->m_Sample.deviceName) == 0)
                {
                    foundDeviceID = true;
                    break;
                }
            }
            
        } else {
            foundDeviceID = true; // all device id's
        }
        
        if(foundDeviceID == false)
        {
            continue;
        }
        
        sprintf(buff, "%c%c%c%c", PRINTF_4CC(sample->m_Sample.sampleKey));
        
        uint32_t milliseconds = pair.first * 1000;
        uint32_t  hr = 0, min = 0, sec = 0, day = 0;
        
        if(prev_milliseconds != milliseconds)
        {
            if(pair.first > GPMF_GLOBAL_TIME)
            {
                while (milliseconds >= 1000) {
                    milliseconds = (milliseconds - 1000);
                    sec = sec + 1;
                    if (sec >= 60) min = min + 1;
                    if (sec == 60) sec = 0;
                    if (min >= 60) hr = hr + 1;
                    if (min == 60) min = 0;
                    if (hr >= 24) {
                        hr = (hr - 24);
                        day = day + 1;
                    }
                }
                if(!first_time)
                {
                    fprintf(m_FP, "\n\t},");
                } else {
                    fprintf(m_FP, "\n\t\"%s\" : {\n", sample->m_Sample.deviceName);
                    strcpy((char *)prev_deviceName, (const char *)sample->m_Sample.deviceName);
                    
                }
                
                fprintf(m_FP, "\n\t\"%02d:%02d:%02d.%03d\" : {\n",
                        hr,
                        min,
                        sec,
                        milliseconds);
                
            } else {
                fprintf(m_FP, "\n\t\"%s\" : {\n", sample->m_Sample.deviceName);
                strcpy((char *)prev_deviceName, (const char *)sample->m_Sample.deviceName);
                
                //                if(sample->m_Sample.deviceID == 1)
                //                {
                //                    fprintf(m_FP, "\t\t\"%s\" : \"%i\",\n", "deviceID", sample->m_Sample.deviceID);
                //                } else {
                //                    fprintf(m_FP, "\t\t\"%s\" : \"%c%c%c%c\",\n", "sampleDeviceID", PRINTF_4CC(sample->m_Sample.deviceID));
                //                }
                //                fprintf(m_FP, "\t\t\t\"%s\" : \"%s\",\n", "sampleDeviceName", sample->m_Sample.deviceName);
            }
            
        } else {
            if(strcmp((const char *)sample->m_Sample.deviceName, (const char *)prev_deviceName) != 0)
            {
                fprintf(m_FP, ",\n");
                fprintf(m_FP, "\t},\n\t\"%s\" : {\n", sample->m_Sample.deviceName);
            } else {
                fprintf(m_FP, ",\n");
            }
            strcpy((char *)prev_deviceName, (const char *)sample->m_Sample.deviceName);
            
        }
        fprintf(m_FP, "\t\t\"%s\" : {\n", buff);
        if(pair.first > GPMF_GLOBAL_TIME)
        {
            if(sample->m_Sample.deviceID == 1)
            {
                fprintf(m_FP, "\t\t\t\"%s\" : \"%i\",\n", "deviceID", sample->m_Sample.deviceID);
            } else {
                uint8_t a = (sample->m_Sample.deviceID&0xff);  // check for external metadata flag
                
                if(a != 1)
                {
                    fprintf(m_FP, "\t\t\t\"%s\" : \"%c%c%c%c\",\n", "sampleDeviceID", PRINTF_4CC(BYTESWAP32(sample->m_Sample.deviceID)));
                } else {
                    fprintf(m_FP, "\t\t\t\"%s\" : \"%i\",\n", "deviceID", sample->m_Sample.deviceID);
                }
                
            }
            fprintf(m_FP, "\t\t\t\"%s\" : \"%s\",\n", "sampleDeviceName", sample->m_Sample.deviceName);
            
            if(m_MinimalJSON != true)
            {
                fprintf(m_FP, "\t\t\t\"%s\" : %i,\n", "sampleSize", sample->m_Sample.sampleTypeSize);
                fprintf(m_FP, "\t\t\t\"%s\" : %i,\n", "elementsInStruct", sample->m_Sample.sampleElementsInStruct);
                fprintf(m_FP, "\t\t\t\"%s\" : %i,\n", "structSize", sample->m_Sample.sampleStructSize);
                fprintf(m_FP, "\t\t\t\"%s\" : %i,\n", "sampleRepeat", sample->m_Sample.sampleRepeat);
            }
        }
        if(sample->m_Sample.sampleSTMP != 0)
        {
            fprintf(m_FP, "\t\t\t\"%s\" : %llu,\n", "sampleSTMP", sample->m_Sample.sampleSTMP);
        }
        
        if(sample->m_Sample.sampleRate > 0 && m_MinimalJSON != true)
        {
            fprintf(m_FP, "\t\t\t\"%s\" : %f,\n", "sampleRate", sample->m_Sample.sampleRate);
        }
        
        fprintf(m_FP, "\t\t\t\"%s\" : \"%s\",\n", "sampleSizeType", Type2String((int32_t)sample->m_Sample.sampleType));
        
        if(sample->m_Sample.sampleType == GPMF_COMPLEX) {
            char *buffer = (char *)sample->m_Sample.sampleBuffer;
            fprintf(m_FP, "\t\t\t\"%s\" : ", "sampleBuffer");
            if(sample->m_Sample.sampleCount > 1) fprintf(m_FP, "[");
            char typearray[64];
            uint32_t elements = sizeof(typearray);
            uint8_t *bdata = (uint8_t *)buffer;
            uint32_t i;
            //if(sample->m_Sample.sampleBufferSize > 0)
            //{
            if (GPMF_LIB_OK == GPMF_ExpandComplexTYPE(sample->m_Sample.complexType, sample->m_Sample.sampleType, typearray, &elements)){
                
                for(uint32_t j=0; j < sample->m_Sample.sampleRepeat; j++)
                {
                    for (i = 0; i < elements; i++) {
                        uint32_t elementsize = GPMF_SizeofType((GPMF_SampleType)typearray[i]);
                        PrintfFormattedData((uint32_t)typearray[i], elementsize, 1, bdata);
                        bdata += elementsize;
                        if (i+1 < elements) fprintf(m_FP, ", ");
                        
                    }
                    if (j+1 < sample->m_Sample.sampleRepeat) fprintf(m_FP, ", ");
                }
            }
            //            } else {
            //                fprintf(m_FP, "\"\",");
            //            }
            
            if(sample->m_Sample.sampleCount > 1) fprintf(m_FP, "]");
            fprintf(m_FP, ",\n");
        } else {
            uint8_t *buffer  = (uint8_t *)sample->m_Sample.sampleBuffer;
            fprintf(m_FP, "\t\t\t\"%s\" : ", "sampleBuffer");
            
            if(sample->m_Sample.sampleBufferSize > 0)
            {
                if((sample->m_Sample.sampleElementsInStruct > 1 || sample->m_Sample.sampleRepeat > 1)){
                    if(sample->m_Sample.sampleStructSize > 1 || (sample->m_Sample.sampleType != GPMF_STRING_UTF8 && sample->m_Sample.sampleType != GPMF_STRING)) {
                        fprintf(m_FP, "[");
                    }
                }
            } else {
                fprintf(m_FP, "\"\"");
            }
            
            PrintfFormattedData(sample->m_Sample.sampleType, sample->m_Sample.sampleStructSize, sample->m_Sample.sampleRepeat,  buffer);
            if((sample->m_Sample.sampleElementsInStruct > 1 || sample->m_Sample.sampleRepeat > 1))
            {
                if(sample->m_Sample.sampleStructSize > 1 || (sample->m_Sample.sampleType != GPMF_STRING_UTF8 && sample->m_Sample.sampleType != GPMF_STRING)) {
                    fprintf(m_FP, "]");
                }
            }
            fprintf(m_FP, ",\n");
        }
        fprintf(m_FP, "\t\t\t\"%s\" : %i\n", "sampleBufferSize", sample->m_Sample.sampleBufferSize);
        fprintf(m_FP, "\t\t}");
        prev_milliseconds = milliseconds;
        first_time = false;
    }
    fprintf(m_FP, "\n\t}\n}\n\n");
}

const char *GPMFAssetTrackTimeline::Type2String(int32_t type)
{
    switch ((int)type)
    {
        case GPMF_SIGNED_BYTE:             return "SIGNED_BYTE"; break;
        case GPMF_UNSIGNED_BYTE:           return "UNSIGNED_BYTE"; break;
        case GPMF_STRING_UTF8:             return "STRING_UTF8"; break;
        case GPMF_STRING:                  return "STRING"; break;
        case GPMF_SIGNED_SHORT:            return "SIGNED_SHORT"; break;
        case GPMF_UNSIGNED_LONG_HEX:       return "UNSIGNED_LONG_HEX"; break;
        case GPMF_SIGNED_LONG:             return "SIGNED_LONG"; break;
        case GPMF_UNSIGNED_LONG:           return "UNSIGNED_LONG"; break;
        case GPMF_UNSIGNED_SHORT:          return "UNSIGNED_SHORT"; break;
        case GPMF_FLOAT:                   return "FLOAT"; break;
        case GPMF_FOURCC:                  return "FOURCC"; break;
        case GPMF_HIDDEN:                  return "HIDDEN"; break;
        case GPMF_Q15_16_FIXED_POINT:      return "Q15_16_FIXED_POINT"; break;
        case GPMF_Q31_32_FIXED_POINT:      return "Q31_32_FIXED_POINT"; break;
        case GPMF_DOUBLE:                  return "DOUBLE"; break;
        case GPMF_SIGNED_64BIT_INT:        return "SIGNED_64BIT_INT"; break;
        case GPMF_UNSIGNED_64BIT_INT:      return "UNSIGNED_64BIT_INT"; break;
        case GPMF_GUID:                    return "GUID"; break;
        case GPMF_UTC_DATE_TIME:           return "UTC_DATE_TIME"; break;
        case GPMF_COMPLEX_SIZE_1:          return "COMPLEX_SIZE_1"; break;
        case GPMF_COMPLEX_SIZE_2:          return "COMPLEX_SIZE_2"; break;
        case GPMF_COMPLEX_SIZE_3:          return "COMPLEX_SIZE_3"; break;
        case GPMF_COMPLEX_SIZE_4:          return "COMPLEX_SIZE_4"; break;
        case GPMF_COMPLEX_SIZE_8:          return "COMPLEX_SIZE_8"; break;
        case GPMF_XML:                     return "XML"; break;
        case GPMF_COMPLEX:                 return "COMPLEX"; break;
        case GPMF_NEST:                    return "NEST"; break;
        default:                                    return "";
    }
}

void GPMFAssetTrackTimeline::PrintfFormattedData(uint32_t type, uint32_t structsize, uint32_t repeat, void *data)
{
    switch (type)
    {
        case GPMF_STRING_ASCII:
        {
            unsigned char t[256] = {0};
            uint32_t size = structsize * repeat;
            uint32_t arraysize = structsize;
            
            if (size > 255)
            {
                size = 255;
            }
            memcpy(t, (char *)data, size);
            t[size] = 0;
            {
                uint32_t i, j, pos = 0;
                if(structsize == 1) fprintf(m_FP, "\"");
                
                for (i = 0; i < repeat; i++) {
                    if(structsize > 1) fprintf(m_FP, "\"");
                    
                    for (j = 0; j < arraysize; j++) {
                        
                        if (t[pos] != '\0' ){//&& t[pos] != ' ')
                            if (t[pos] == '/')
                                fprintf(m_FP, "%c", '-');
                            else if (t[pos] == 178)
                                fprintf(m_FP, "%c", ' ');
                            else if (t[pos] == 181)
                                fprintf(m_FP, "µ");
                            else
                                fprintf(m_FP, "%c", t[pos]);
                        }
                        
                        pos++;
                    }
                    if(structsize > 1) fprintf(m_FP, "\"");
                    
                    if ((structsize > 1) && (i+1 < repeat)) fprintf(m_FP, ", ");
                    
                }
                if(structsize == 1) fprintf(m_FP, "\"");
                
            }
        }
            break;
        case GPMF_SIGNED_BYTE:
        {
            int8_t *b = (int8_t *)data;
            uint32_t arraysize = structsize;
            while (repeat--) {
                arraysize = structsize;
                while (arraysize--){
                    fprintf(m_FP, "%d", (int8_t)*b);
                    b++;
                }
                if (repeat != 0) fprintf(m_FP, ", ");
            }
        }
            break;
        case GPMF_UNSIGNED_BYTE:
        {
            uint8_t *b = (uint8_t *)data;
            uint32_t arraysize = structsize;
            while (repeat--) {
                arraysize = structsize;
                while (arraysize--) {
                    fprintf(m_FP, "%d", *b);
                    b++;
                }
                if (repeat != 0) fprintf(m_FP, ", ");
            }
        }
            break;
        case GPMF_DOUBLE:
        {
            double *d;
            uint32_t arraysize = structsize / sizeof(uint64_t);
            d = (double *)data;
            while (repeat--) {
                arraysize = structsize / sizeof(uint64_t);
                while (arraysize--) {
                    fprintf(m_FP, "%2.4f", *d++);
                    if (arraysize != 0) fprintf(m_FP, ", ");
                }
                if (repeat != 0) fprintf(m_FP, ", ");
            }
        }
            break;
        case GPMF_FLOAT:
        {
            float *f;
            uint32_t arraysize = structsize / sizeof(uint32_t);
            f = (float *)data;
            while (repeat--)
            {
                arraysize = structsize / sizeof(uint32_t);
                while (arraysize--)
                {
                    fprintf(m_FP, "%2.4f", *f++);
                    if (arraysize != 0) fprintf(m_FP, ", ");
                }
                if (repeat != 0) fprintf(m_FP, ", ");
            }
        }
            break;
        case GPMF_FOURCC:
        {
            uint32_t *L = (uint32_t *)data;
            uint32_t arraysize = structsize / sizeof(uint32_t);
            while (repeat--)
            {
                arraysize = structsize / sizeof(uint32_t);
                fprintf(m_FP, "\"");
                
                while (arraysize--)
                {
                    fprintf(m_FP, "%c%c%c%c", PRINTF_4CC(*L));
                    L++;
                }
                fprintf(m_FP, "\"");
            }
        }
            break;
        case GPMF_GUID: // display GUID in this formatting ABCDEF01-02030405-06070809-10111213
        {
            uint8_t *B = (uint8_t *)data;
            uint32_t arraysize = structsize;
            while (repeat--) {
                arraysize = structsize;
                while (arraysize--) {
                    fprintf(m_FP, "%02X", *B);
                    if (arraysize != 0) fprintf(m_FP, ", ");
                    B++;
                }
                if (repeat != 0) fprintf(m_FP, ", ");
            }
        }
            break;
        case GPMF_SIGNED_SHORT:
        {
            int16_t *s = (int16_t *)data;
            uint32_t arraysize = structsize / sizeof(int16_t);
            while (repeat--){
                arraysize = structsize / sizeof(int16_t);
                while (arraysize--){
                    fprintf(m_FP, "%i", *s);
                    if (arraysize != 0) fprintf(m_FP, ", ");
                    s++;
                }
                if (repeat != 0) fprintf(m_FP, ", ");
            }
        }
            break;
        case GPMF_UNSIGNED_SHORT:
        {
            uint16_t *S = (uint16_t *)data;
            uint32_t arraysize = structsize / sizeof(uint16_t);
            while (repeat--) {
                arraysize = structsize / sizeof(uint16_t);
                while (arraysize--) {
                    fprintf(m_FP, "%i", (*S));
                    if (arraysize != 0) fprintf(m_FP, ", ");
                    S++;
                }
                if (repeat != 0) fprintf(m_FP, ", ");
            }
        }
            break;
        case GPMF_SIGNED_LONG:
        {
            int32_t *l = (int32_t *)data;
            uint32_t arraysize = structsize / sizeof(uint32_t);
            while (repeat--) {
                arraysize = structsize / sizeof(uint32_t);
                
                while (arraysize--)
                {
                    fprintf(m_FP, "%d", (int32_t)(*l));
                    if (arraysize != 0) fprintf(m_FP, ", ");
                    l++;
                }
                if (repeat != 0) fprintf(m_FP, ", ");
            }
        }
            break;
        case GPMF_UNSIGNED_LONG:
        {
            uint32_t *L = (uint32_t *)data;
            uint32_t arraysize = structsize / sizeof(uint32_t);
            while (repeat--) {
                arraysize = structsize / sizeof(uint32_t);
                while (arraysize--) {
                    fprintf(m_FP, "%u", (*L));
                    if (arraysize != 0) fprintf(m_FP, ", ");
                    L++;
                }
                if (repeat != 0) fprintf(m_FP, ", ");
            }
        }
            break;
        case GPMF_Q15_16_FIXED_POINT:
        {
            int32_t *q = (int32_t *)data;
            uint32_t arraysize = structsize / sizeof(int32_t);
            while (repeat--) {
                arraysize = structsize / sizeof(int32_t);
                while (arraysize--) {
                    double dq = (*q);
                    dq /= (double)65536.0;
                    fprintf(m_FP, "%.3f", dq);
                    if (arraysize != 0) fprintf(m_FP, ", ");
                    q++;
                }
                if (repeat != 0) fprintf(m_FP, ", ");
            }
        }
            break;
        case GPMF_Q31_32_FIXED_POINT:
        {
            int64_t *Q = (int64_t *)data;
            uint32_t arraysize = structsize / sizeof(int64_t);
            while (repeat--) {
                arraysize = structsize / sizeof(int64_t);
                while (arraysize--) {
                    uint64_t Q64 = (uint64_t)(*Q);
                    double dq = (double)(Q64 >> (uint64_t)32);
                    dq += (double)(Q64 & (uint64_t)0xffffffff) / (double)0x100000000;
                    fprintf(m_FP, "%.3f", dq);
                    if (arraysize != 0) fprintf(m_FP, ", ");
                    Q++;
                }
                if (repeat != 0) fprintf(m_FP, ", ");
            }
        }
            break;
        case GPMF_UTC_DATE_TIME:
        {
            char *U = (char *)data;
            uint32_t arraysize = structsize / 16;
            while (repeat--) {
                arraysize = structsize / 16;
                char t[17];
                t[16] = 0;
                while (arraysize--) {
#ifdef _WINDOWS
                    strncpy_s(t, 17, U, 16);
#else
                    strncpy(t, U, 16);
#endif
                    fprintf(m_FP, "\"%s\"", t);
                    U += 16;
                }
                if (repeat != 0) fprintf(m_FP, ", ");
            }
        }
            break;
        case GPMF_SIGNED_64BIT_INT:
        {
            int64_t *J = (int64_t *)data;
            uint32_t arraysize = structsize / sizeof(int64_t);
            while (repeat--) {
                arraysize = structsize / sizeof(int64_t);
                while (arraysize--) {
                    fprintf(m_FP, "%lld", (long long int)(*J));
                    if (arraysize != 0) fprintf(m_FP, ", ");
                    J++;
                }
                if (repeat != 0) fprintf(m_FP, ", ");
            }
        }
            break;
        case GPMF_UNSIGNED_64BIT_INT:
        {
            uint64_t *J = (uint64_t *)data;
            uint32_t arraysize = structsize / sizeof(uint64_t);
            while (repeat--) {
                arraysize = structsize / sizeof(uint64_t);
                while (arraysize--) {
                    fprintf(m_FP, "%llu", (long long unsigned int)(*J));
                    if (arraysize != 0) fprintf(m_FP, ", ");
                    J++;
                }
                if (repeat != 0) fprintf(m_FP, ", ");
            }
        }
            break;
        default:
            break;
    }
}




void GPMFAssetTrackTimeline::PrintGPMFInternal(void *msv)
{
#ifdef ENABLE_PRINT_GPMF
    uint32_t payloads = m_Asset->GetPayloadCount();
    uint32_t *lastpayload = 0;
    uint32_t *payload;
    GPMF_stream metadata_stream, *ms = &metadata_stream;
    
    for(uint32_t i = 0; i < payloads; i++)
    {
        payload = m_Asset->GetPayload(lastpayload, i);
        uint32_t payloadsize =  m_Asset->GetPayloadSize(i);
        GPMF_Init(ms, payload, (int)payloadsize);
        do
        {
            PrintGPMF(ms);  // printf current GPMF KLV
        } while (GPMF_OK == GPMF_Next(ms, GPMF_RECURSE_LEVELS));
    }
#endif
}

GPMF_LIB_ERROR GPMFAssetTrackTimeline::ResetTimeline(void)
{
    for (auto const &pair: m_TrackTimeline) {
        GPMFAssetSample *sample = pair.second;
        if(sample)
        {
            delete sample;
        }
    }
    m_TrackTimeline.clear();
    m_CachedFourcc = 0;
    return GPMF_LIB_OK;
}



