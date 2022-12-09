/*! @file GPMFAssetSource.h
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

#ifndef GPMF_ASSET_SOURCE_H
#define GPMF_ASSET_SOURCE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <vector>
#include <string>

#include "GPMFTypes.h"
#include "GPMFAssetSample.h"
#include "GPMFAssetTrack.h"
#include "GPMFAssetSourceImage.h"
#include "GPMFAssetSourceVideo.h"
#include "GPMFAssetSourceGlobalBuffer.h"
#include "GPMFTimeRange.h"

#define GPMF_GLOBAL_TIME -999

class GPMFAssetTrack;

class GPMFAssetSource
{
public:
    GPMFAssetSource();
    virtual ~GPMFAssetSource();
    
    GPMF_LIB_ERROR Open(char* path);
    void SetGPMFGlobalBuffer(const char *buffer, uint32_t buffersize, GPMF_LIB_ERROR& error);
    void Close();
    GPMF_LIB_ERROR GetTrackDuration(double *startTime, double *endTime);
    GPMF_LIB_ERROR GetTrackDuration(GPMFRational *rationalTime);
    GPMFRational GetVideoFrameRateAndCount(GPMF_LIB_ERROR& error, uint32_t *count);
    GPMF_LIB_ERROR SetDeviceID(uint32_t id);
    void Parse(GPMF_LIB_ERROR& error, GPMF_PARSE_TYPE type = GPMF_FILE_TYPE_ALL, const std::vector<std::string> fourcc = {}, GPMFTimeRange *timeRange = nullptr);
    float GetAssetDuration();
    double GetSampleRate(uint32_t fourcc);
    double GetSampleRate(uint32_t fourcc, double *firstsampletime, double *lastsampletime);
    uint32_t *GetPayload(uint32_t *lastpayload, uint32_t index);
    uint32_t GetPayloadCount();
    uint32_t GetPayloadSize(uint32_t index);
    uint32_t GetPayloadTime(uint32_t index, double *in, double *out);
    GPMF_LIB_ERROR GetSampleCount(const char *fourcc, uint32_t *sampleCount);
    GPMF_LIB_ERROR GetSample(const char *fourcc, uint32_t index, GPMFSampleptr samplePtr);
    GPMF_LIB_ERROR GetNextSample(const char *fourcc, GPMFSampleptr samplePtr);
    GPMFTimeRange *GetNextTimeRange(GPMF_LIB_ERROR &error);
    GPMF_LIB_ERROR SetTrackType(const std::vector<std::string> fourcc);
    GPMF_LIB_ERROR SetParseType(GPMF_PARSE_TYPE type);
    GPMF_LIB_ERROR SetDeviceIds(std::vector<std::string> deviceIds);
    GPMF_LIB_ERROR SetDeviceNames(std::vector<std::string> deviceNames);
    GPMF_LIB_ERROR ExportTimeline(const char *file, GPMF_EXPORT_FILE_TYPE fileType = GPMF_FILE_TYPE_CONSOLE);
    GPMF_LIB_ERROR ExportTimeline(const char* file, const char *fourcc, GPMF_EXPORT_FILE_TYPE fileType = GPMF_FILE_TYPE_CONSOLE);
    GPMF_LIB_ERROR ExportTimeline(const char* file, const char *fourccArray[], uint32_t arrayCount, GPMF_EXPORT_FILE_TYPE fileType = GPMF_FILE_TYPE_CONSOLE);
    GPMF_LIB_ERROR ExportTimeline(const char* file, const char *fourcc, double inTime, double outTime, GPMF_EXPORT_FILE_TYPE fileType = GPMF_FILE_TYPE_CONSOLE);
    GPMF_LIB_ERROR ExportTimeline(const char* file, double inTime, double outTime, GPMF_EXPORT_FILE_TYPE fileType = GPMF_FILE_TYPE_CONSOLE);
    double CalculateSampleRate(uint32_t fourcc, double *firstsampletime, double *lastsampletime);
    void ModifyGlobalSettings(GPMF_LIB_ERROR& error, uint32_t origfourCC, uint32_t newfourCC, char newType, uint32_t newStructSize, uint32_t newRepeat, void* newData);


    GPMFAssetTrack *m_AssetTrack;
    void *m_MS;
    uint32_t m_Payloads;
    uint32_t *m_Payload;
    std::vector<uint32_t> m_DeviceIdList;
    std::vector<std::string> m_DeviceNameList;

    GPMF_FILE_TYPE m_FileType;

private:
    void Init();
    GPMF_LIB_ERROR OpenGPMF(std::string fileName);
    GPMF_LIB_ERROR OpenUDTA(std::string fileName);
    GPMF_FILE_TYPE GetFiletype();
    GPMF_LIB_ERROR Read(GPMFTimeRange *timeRange);
    GPMF_LIB_ERROR ReadTracks(uint32_t fourcc, double inTime, double outTime);
    GPMF_LIB_ERROR ReadAllTracks(void);
    void GetTimeSegments(void);
    void ReadUDTA(uint32_t fourcc);
    
    GPMFAssetSourceImage *m_AssetSourceImage;
    GPMFAssetSourceVideo *m_AssetSourceVideo;
    GPMFAssetSourceGlobalBuffer *m_AssetSourceBuffer;

    std::string m_Filepath;
    std::string m_TrackType;
    uint32_t m_DeviceID;
    std::vector<std::pair<std::string, uint32_t> > m_FourccList;
    uint32_t m_SampleIndex;
    uint32_t m_TimeRageIndex;
    GPMF_PARSE_TYPE m_ParseType;
};

#endif /* GPMF_ASSET_SOURCE_H */
