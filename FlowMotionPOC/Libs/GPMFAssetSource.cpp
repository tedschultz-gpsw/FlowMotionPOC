/*! @file GPMFAssetSource.cpp
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

#include "GPMFAssetSource.h"
#include "GPMFAssetSourceVideo.h"
#include "GPMFAssetSourceImage.h"
#include "GPMFAssetTrackSegment.h"
#include "GPMF_common.h"

#include <float.h>
#include <cmath>

#define JPG_SIGNATURE 0xe1ffd8ff
#define MP4_SIGNATURE 0x70797466

GPMFAssetSource::GPMFAssetSource()
{
    Init();
}

GPMFAssetSource::~GPMFAssetSource()
{
    Close();

    if (m_AssetSourceBuffer) {
        delete m_AssetSourceBuffer;
        m_AssetSourceBuffer = NULL;
    }
    
    if(m_AssetTrack) {
        delete m_AssetTrack;
        m_AssetTrack = NULL;
    }
    m_DeviceNameList.clear();
    m_DeviceIdList.clear();
}

void GPMFAssetSource::Init()
{
    m_MS  = NULL;
    m_Payloads  = 0L;
    m_Payload = NULL;
    m_DeviceID = 1;
    m_FourccList.resize(0);
    m_ParseType = GPMF_FILE_TYPE_ALL;
    m_FileType = GPMF_FILE_TYPE_NOT_SUPPORTED;
    m_AssetTrack = NULL;
    m_AssetSourceVideo = NULL;
    m_AssetSourceImage = NULL;
    m_AssetSourceBuffer = NULL;
    m_TimeRageIndex = 0;
    m_SampleIndex = 0;
    m_AssetTrack = new GPMFAssetTrack(*this);
}

GPMF_LIB_ERROR GPMFAssetSource::Open(char* path)
{
    m_Filepath = path;
    m_FileType = GetFiletype();
    GPMF_LIB_ERROR err = OpenGPMF((char *)path);
    if(err == GPMF_LIB_OK){
        GetTimeSegments();
    }
    Close();
    return err;
}

void GPMFAssetSource::SetGPMFGlobalBuffer(const char *buffer, uint32_t buffersize, GPMF_LIB_ERROR& error)
{
    if(m_AssetSourceBuffer) {
        delete m_AssetSourceBuffer;
    }
    
    m_AssetSourceBuffer = new GPMFAssetSourceGlobalBuffer();
    error = (GPMF_LIB_ERROR)m_AssetSourceBuffer->SetGPMFGlobalBuffer(buffer, buffersize);
}

GPMF_LIB_ERROR GPMFAssetSource::SetTrackType(const std::vector<std::string> fourcc)
{
    m_FourccList.resize(0);
    
    for (size_t i=0; i<fourcc.size(); i++) {
        m_FourccList.push_back( std::pair<std::string, uint32_t>(fourcc[i], 0));
    }
    return GPMF_LIB_OK;
}

GPMF_LIB_ERROR GPMFAssetSource::SetParseType(GPMF_PARSE_TYPE type)
{
    m_ParseType = type;
    return GPMF_LIB_OK;
}

GPMF_LIB_ERROR GPMFAssetSource::GetTrackDuration(double *startTime, double *endTime)
{
    *startTime = -DBL_MAX;
    *endTime = GetAssetDuration();
    return GPMF_LIB_OK;
}

GPMF_LIB_ERROR GPMFAssetSource::GetTrackDuration(GPMFRational *rationalTime)
{
    if(m_FileType == GPMF_FILE_TYPE_VIDEO)
    {
        return m_AssetSourceVideo->GetAssetDuration(rationalTime);
    } else if (m_FileType == GPMF_FILE_TYPE_IMAGE) {
        return m_AssetSourceImage->GetAssetDuration(rationalTime);
    }
    return GPMF_LIB_OK;
}

GPMFRational GPMFAssetSource::GetVideoFrameRateAndCount(GPMF_LIB_ERROR& error, uint32_t *count)
{
    if(m_FileType == GPMF_FILE_TYPE_VIDEO)
    {
        return m_AssetSourceVideo->GetVideoFrameRateAndCount(error, count);
    } else {
        error = GPMF_LIB_ERROR_NOT_VALID_FOR_TYPE;
        GPMFRational rational;
        rational.denominator = 0;
        rational.quotient = 0;
        rational.numerator = 0;
        *count = 0;
        return rational;
    }
}

GPMF_LIB_ERROR GPMFAssetSource::SetDeviceIds(std::vector<std::string> deviceIds)
{
    m_DeviceNameList.clear();
    m_DeviceIdList.clear();
    if(deviceIds.size() > 0)
    {
        std::vector<std::string>::iterator it;
        
        for(it = deviceIds.begin(); it != deviceIds.end(); it++) {
            std::string fourc = *it;
            
            if(fourc == "HERO") {
                m_DeviceIdList.push_back(1);
            } else if(fourc == "KRMA") {
                m_DeviceIdList.push_back(1001);
            }  else {
                m_DeviceIdList.push_back(BYTESWAP32(FOURCC(fourc)));
            }
        }
    }
    return GPMF_LIB_OK;
}

GPMF_LIB_ERROR GPMFAssetSource::SetDeviceNames(std::vector<std::string> deviceNames)
{
    m_DeviceNameList.clear();
    m_DeviceIdList.clear();
    if(deviceNames.size() > 0) {
        std::vector<std::string>::iterator it;
        
        for(it = deviceNames.begin(); it != deviceNames.end(); it++) {
            m_DeviceNameList.push_back(*it);
        }
    }
    return GPMF_LIB_OK;
}

GPMF_LIB_ERROR GPMFAssetSource::SetDeviceID(uint32_t id)
{
    m_DeviceID = id;
    return GPMF_LIB_OK;
}

GPMF_LIB_ERROR GPMFAssetSource::OpenGPMF(std::string fileName)
{
    GPMF_LIB_ERROR err = GPMF_LIB_ERROR_MEMORY;
    char *cstr = (char *)fileName.c_str();
    
    if(m_ParseType == GPMF_FILE_TYPE_ALL || m_ParseType == GPMF_FILE_TYPE_GPMF_ONLY)
    {
        if(m_FileType == GPMF_FILE_TYPE_VIDEO)
        {
            m_AssetSourceVideo = new GPMFAssetSourceVideo();
            return m_AssetSourceVideo->Open(cstr);
        } else if (m_FileType == GPMF_FILE_TYPE_IMAGE) {
            m_AssetSourceImage = new GPMFAssetSourceImage();
            return m_AssetSourceImage->Open(cstr);
        }
    }
    return err;
}

GPMF_LIB_ERROR GPMFAssetSource::OpenUDTA(std::string fileName)
{
    GPMF_LIB_ERROR err = GPMF_LIB_OK;
    char *cstr = (char *)fileName.c_str();
    
    if(m_ParseType == GPMF_FILE_TYPE_ALL || m_ParseType == GPMF_FILE_TYPE_UDATA_ONLY)
    {
        if(m_FileType == GPMF_FILE_TYPE_VIDEO)
        {
            m_AssetSourceVideo = new GPMFAssetSourceVideo();
            err = m_AssetSourceVideo->OpenUDTA(cstr);
        } else if (m_FileType == GPMF_FILE_TYPE_IMAGE) {
            m_AssetSourceImage = new GPMFAssetSourceImage();
            err = m_AssetSourceImage->Open(cstr);
        }
    }
    return err;
}

void GPMFAssetSource::Close()
{
    if(m_FileType == GPMF_FILE_TYPE_VIDEO && m_AssetSourceVideo)
    {
        m_AssetSourceVideo->Close();
        delete m_AssetSourceVideo;
        m_AssetSourceVideo = NULL;
    } else if (m_FileType == GPMF_FILE_TYPE_IMAGE && m_AssetSourceImage) {
        m_AssetSourceImage->Close();
        delete m_AssetSourceImage;
        m_AssetSourceImage = NULL;
    }
}

float GPMFAssetSource::GetAssetDuration()
{
    if(m_FileType == GPMF_FILE_TYPE_VIDEO)
    {
        return m_AssetSourceVideo->GetAssetDuration();
    } else if (m_FileType == GPMF_FILE_TYPE_IMAGE) {
        return m_AssetSourceImage->GetAssetDuration();
    } else {
        return m_AssetSourceBuffer->GetAssetDuration();
    }
    return 0;
}

double GPMFAssetSource::GetSampleRate(uint32_t fourcc)
{
    if(m_FileType == GPMF_FILE_TYPE_VIDEO)
    {
        return m_AssetSourceVideo->GetSampleRate(fourcc);
    } else if (m_FileType == GPMF_FILE_TYPE_IMAGE) {
        return m_AssetSourceImage->GetSampleRate(fourcc);
    } else {
        return m_AssetSourceBuffer->GetSampleRate(fourcc);
    }
    return 0;
}

double GPMFAssetSource::GetSampleRate(uint32_t fourcc, double *firstsampletime, double *lastsampletime)
{
    if(m_FileType == GPMF_FILE_TYPE_VIDEO)
    {
        return m_AssetSourceVideo->GetSampleRate(fourcc, firstsampletime, lastsampletime);
    } else if (m_FileType == GPMF_FILE_TYPE_IMAGE) {
        return m_AssetSourceImage->GetSampleRate(fourcc, firstsampletime, lastsampletime);
    } else {
        return m_AssetSourceBuffer->GetSampleRate(fourcc, firstsampletime, lastsampletime);
    }
    return 0;
}

uint32_t *GPMFAssetSource::GetPayload(uint32_t *lastpayload, uint32_t index)
{
    if(m_FileType == GPMF_FILE_TYPE_VIDEO)
    {
        return m_AssetSourceVideo->GetPayload(lastpayload, index);
    } else if (m_FileType == GPMF_FILE_TYPE_IMAGE) {
        return m_AssetSourceImage->GetPayload(lastpayload, index);
    } else {
        return m_AssetSourceBuffer->GetPayload(lastpayload, index);
    }
    return 0;
}

uint32_t GPMFAssetSource::GetPayloadCount()
{
    if(m_FileType == GPMF_FILE_TYPE_VIDEO)
    {
        return m_AssetSourceVideo->GetPayloadCount();
    } else if (m_FileType == GPMF_FILE_TYPE_IMAGE) {
        return m_AssetSourceImage->GetPayloadCount();
    } else {
        return m_AssetSourceBuffer->GetPayloadCount();
    }
    return 0;
}

uint32_t GPMFAssetSource::GetPayloadSize(uint32_t index)
{
    if(m_FileType == GPMF_FILE_TYPE_VIDEO)
    {
        return m_AssetSourceVideo->GetPayloadSize(index);
    } else if (m_FileType == GPMF_FILE_TYPE_IMAGE) {
        return m_AssetSourceImage->GetPayloadSize(index);
    } else {
        return m_AssetSourceBuffer->GetPayloadSize(index);
    }
    return 0;
}

uint32_t GPMFAssetSource::GetPayloadTime(uint32_t index, double *in, double *out)
{
    if(m_FileType == GPMF_FILE_TYPE_VIDEO)
    {
        return m_AssetSourceVideo->GetPayloadTime(index, in, out);
    } else if (m_FileType == GPMF_FILE_TYPE_IMAGE) {
        return m_AssetSourceImage->GetPayloadTime(index, in, out);
    }  else {
        return m_AssetSourceBuffer->GetPayloadTime(index, in, out);
    }
    return 0;
}

void GPMFAssetSource::Parse(GPMF_LIB_ERROR& error, GPMF_PARSE_TYPE type, const std::vector<std::string> fourcc, GPMFTimeRange *timeRange)
{
    SetTrackType(fourcc);
    m_ParseType = type;
    error = GPMF_LIB_OK;
    
    Close();

    m_AssetTrack->ResetTimeline();
    
    if(m_ParseType == GPMF_FILE_TYPE_ALL) {
        if(m_FileType == GPMF_FILE_TYPE_VIDEO) {
            error = OpenUDTA(m_Filepath);
            if(error !=  GPMF_LIB_OK)
                return;
            error = Read(timeRange);
        }
        Close();
        
        // We ignore error for parsing UDTA because we want to parse GPMF also

        error = OpenGPMF(m_Filepath);
        if(error !=  GPMF_LIB_OK)
            return;
        error = Read(timeRange);
    } else if(m_ParseType == GPMF_FILE_TYPE_GPMF_ONLY) {
        error = OpenGPMF(m_Filepath);
        if(error !=  GPMF_LIB_OK)
            return;
        
        error= Read(timeRange);
    } else if(m_ParseType == GPMF_FILE_TYPE_UDATA_ONLY) {
        if(m_FileType == GPMF_FILE_TYPE_VIDEO) {
            error = OpenUDTA(m_Filepath);
            if(error !=  GPMF_LIB_OK)
                return;
            error = Read(timeRange);
        } else {
            error = GPMF_LIB_ERROR_NOT_VALID_FOR_TYPE;
        }
    } else if(m_ParseType == GPMF_FILE_TYPE_BUFFER) {
        m_FileType = GPMF_FILE_TYPE_GLOBAL_BUFFER;
        error = Read(timeRange);
    } else {
        error = GPMF_LIB_ERROR_NOT_VALID_FOR_TYPE;
    }
}

GPMF_LIB_ERROR GPMFAssetSource::Read(GPMFTimeRange *timeRange)
{
    uint16_t size = m_FourccList.size();
    double startTime = 0, endTime = 0;
    GPMF_LIB_ERROR err = GPMF_LIB_FAIL;
    
    if(size > 0) {
        if(timeRange) {
            startTime = timeRange->m_startTime;
            endTime = startTime + timeRange->m_duration;
        } else {
            GetTrackDuration(&startTime, &endTime);
        }
        for(uint32_t i = 0; i < size; i++) {
            err = ReadTracks(FOURCC(m_FourccList[i].first.c_str()), startTime, endTime);
        }
    } else {
        err = ReadAllTracks();
    }
    return err;
}

GPMF_LIB_ERROR GPMFAssetSource::ReadAllTracks(void)
{
    uint32_t tracks = m_AssetTrack->GetSampleSegments();
    GPMF_LIB_ERROR err = GPMF_LIB_FAIL;

    for(uint32_t i = 0; i < tracks; i++) {
        double inTime, outTime;
        m_AssetTrack->GetSampleSegmentTime(i, &inTime, &outTime);
        GPMFAssetTrackSegment trackSample(*this, 0);
        double startTime;
        double endTime;
        GetTrackDuration(&startTime, &endTime);
        err = trackSample.GetSamples(m_AssetTrack, 0, startTime, endTime);
    }
    return err;
}

GPMF_LIB_ERROR GPMFAssetSource::ReadTracks(uint32_t fourcc, double inTime, double outTime)
{
    uint32_t tracks = m_AssetTrack->GetSampleSegments();
    GPMF_LIB_ERROR err = GPMF_LIB_FAIL;

    // round values at 2 decimals to ensure taking all segments
    inTime = std::floor(inTime * 100) / 100;
    outTime = std::ceil(outTime * 100) / 100;

    for(uint32_t i = 0; i < tracks; i++) {
        double inTime2, outTime2;
        m_AssetTrack->GetSampleSegmentTime(i, &inTime2, &outTime2);
        
        if((m_AssetTrack->m_TrackSegmentInTime >= inTime && m_AssetTrack->m_TrackSegmentOutTime <= outTime) || m_AssetTrack->m_TrackSegmentInTime == GPMF_GLOBAL_TIME)
        {
            GPMFAssetTrackSegment trackSample(*this, fourcc);
            err = trackSample.GetSamples(m_AssetTrack, fourcc, inTime, outTime);
        }
        
        if (m_AssetTrack->m_TrackSegmentOutTime > outTime)
        {
            break;
        }
    }
    return err;
}

double GPMFAssetSource::CalculateSampleRate(uint32_t fourcc, double *firstSampletime, double *lastSampletime)
{
    uint32_t segments = m_AssetTrack->GetSampleSegments();
    double inTime, endTime;
    GetTrackDuration(&inTime, &endTime);
    *lastSampletime = endTime;
    double sampleRate = 0;
    GPMFAssetTrackSegment trackSample(*this, fourcc);
    sampleRate = trackSample.GetSampleRate(fourcc, segments, firstSampletime, lastSampletime);
    return sampleRate;
}

void GPMFAssetSource::GetTimeSegments(void)
{
    uint32_t tracks = m_AssetTrack->GetSampleSegments();
    for(uint32_t i = 0; i < tracks; i++) {
        double inTime, outTime;
        m_AssetTrack->GetSampleSegmentTime(i, &inTime, &outTime);
        if(inTime != GPMF_GLOBAL_TIME)
        {
            m_AssetTrack->m_AssetTrackTimeline->AddSegmentTime(inTime, outTime);
        }
    }
}

GPMF_FILE_TYPE GPMFAssetSource::GetFiletype()
{
    uint32_t signature = 0;
    size_t lenRead = 0;
    FILE *fp;
    
    if(m_AssetSourceBuffer != NULL) {
        return GPMF_FILE_TYPE_GLOBAL_BUFFER;
    }
    
    if(m_Filepath.size() == 0) {
        return GPMF_FILE_TYPE_BAD_FILE_NAME;
    }
    
    if (( fp = fopen(m_Filepath.c_str(), "rb")) != NULL) {
        lenRead = fread(&signature, 1, 4, fp);
        
        if(signature == JPG_SIGNATURE) {
            fclose(fp);
            return GPMF_FILE_TYPE_IMAGE;
        } else {
            lenRead = fread(&signature, 1, 4, fp);
            if(signature == MP4_SIGNATURE) {
                fclose(fp);
                return GPMF_FILE_TYPE_VIDEO;
            }
        }
    }
    
    if(fp) {
        fclose(fp);
    }
    return GPMF_FILE_TYPE_NOT_SUPPORTED;
}

GPMF_LIB_ERROR GPMFAssetSource::ExportTimeline(const char *file, GPMF_EXPORT_FILE_TYPE fileType)
{
    GPMF_LIB_ERROR err = GPMF_LIB_ERROR_MEMORY;
    
    if(strstr (file,"stdout")) {
        err = m_AssetTrack->ExportTimeline(stdout, fileType);
    } else {
        FILE *fp;
        if (( fp = fopen(file, "w+")) != NULL) {
            err = m_AssetTrack->ExportTimeline(fp, fileType);
            fclose(fp);
        }
    }
    return err;
}

GPMF_LIB_ERROR GPMFAssetSource::ExportTimeline(const char *file, const char *fourcc, GPMF_EXPORT_FILE_TYPE fileType)
{
    GPMF_LIB_ERROR err = GPMF_LIB_ERROR_MEMORY;
    
    if(strstr (file,"stdout")) {
        err = m_AssetTrack->ExportTimeline(stdout, FOURCC(fourcc), fileType);
    } else {
        FILE *fp;
        if (( fp = fopen(file, "r")) != NULL) {
            err = m_AssetTrack->ExportTimeline(fp, FOURCC(fourcc), fileType);
            fclose(fp);
        }
    }
    return err;
}

GPMF_LIB_ERROR GPMFAssetSource::ExportTimeline(const char *file, const char *fourccArray[], uint32_t arrayCount, GPMF_EXPORT_FILE_TYPE fileType)
{
    GPMF_LIB_ERROR err = GPMF_LIB_ERROR_MEMORY;
    for(uint32_t j = 0; j < arrayCount; j++) {
        if(strstr (file,"stdout")) {
            err = m_AssetTrack->ExportTimeline(stdout, FOURCC(fourccArray[j]), fileType);
        } else {
            FILE *fp;
            if (( fp = fopen(file, "r")) != NULL) {
                err = m_AssetTrack->ExportTimeline(fp, FOURCC(fourccArray[j]), fileType);
                fclose(fp);
            }
        }
    }
    return err;
}

GPMF_LIB_ERROR GPMFAssetSource::ExportTimeline(const char *file, const char *fourcc, double inTime, double outTime, GPMF_EXPORT_FILE_TYPE fileType)
{
    GPMF_LIB_ERROR err = GPMF_LIB_ERROR_MEMORY;
    
    if(strstr (file,"stdout")) {
        err = m_AssetTrack->ExportTimeline(stdout, FOURCC(fourcc), inTime, outTime, fileType);
    } else {
        FILE *fp;
        if (( fp = fopen(file, "r")) != NULL) {
            err = m_AssetTrack->ExportTimeline(fp, FOURCC(fourcc), inTime, outTime, fileType);
            fclose(fp);
        }
    }
    return err;
}

GPMF_LIB_ERROR GPMFAssetSource::ExportTimeline(const char *file, double inTime, double outTime, GPMF_EXPORT_FILE_TYPE fileType)
{
    GPMF_LIB_ERROR err = GPMF_LIB_ERROR_MEMORY;
    
    if(strstr (file,"stdout")) {
        err = m_AssetTrack->ExportTimeline(stdout, inTime, outTime, fileType);
    } else {
        FILE *fp;
        if (( fp = fopen(file, "r")) != NULL) {
            err = m_AssetTrack->ExportTimeline(fp, inTime, outTime, fileType);
            fclose(fp);
        }
    }
    return err;
}

GPMF_LIB_ERROR GPMFAssetSource::GetSampleCount(const char *fourcc, uint32_t *sampleCount)
{
    return m_AssetTrack->GetSampleCount(FOURCC(fourcc), sampleCount);
}

GPMF_LIB_ERROR GPMFAssetSource::GetSample(const char *fourcc, uint32_t index, GPMFSampleptr samplePtr)
{
    return m_AssetTrack->GetSample(FOURCC(fourcc), index, samplePtr);
}

GPMF_LIB_ERROR GPMFAssetSource::GetNextSample(const char *fourcc, GPMFSampleptr samplePtr)
{
    GPMF_LIB_ERROR err = GPMF_LIB_ERROR_FIND;
    
    if(m_FourccList.size() > 0)
    {
        for (std::vector<std::pair<std::string, uint32_t>>::iterator t=m_FourccList.begin(); t!=m_FourccList.end(); ++t)
        {
            if (t->first.compare(fourcc) == 0)
            {
                err = m_AssetTrack->GetSample(FOURCC(fourcc), t->second, samplePtr);
                t->second++;
                break;
            }
        }
    } else {
        err = m_AssetTrack->GetSample(FOURCC(fourcc), m_SampleIndex, samplePtr);
        m_SampleIndex++;
    }
    
    return err;
}

GPMFTimeRange *GPMFAssetSource::GetNextTimeRange(GPMF_LIB_ERROR &error)
{
    error = GPMF_LIB_OK;
    GPMFTimeRange *timeRange = m_AssetTrack->GetSampleTimeRange(m_TimeRageIndex);
    m_TimeRageIndex++;
    if(timeRange == nullptr)
    {
        error = GPMF_LIB_ERROR_TIMERANGE_NOT_FOUND;
    }
    return timeRange;
}

void GPMFAssetSource::ModifyGlobalSettings(GPMF_LIB_ERROR& error, uint32_t origfourCC, uint32_t newfourCC, char newType, uint32_t newStructSize, uint32_t newRepeat, void* newData)
{
    error = GPMF_LIB_OK;

    if(m_FileType == GPMF_FILE_TYPE_VIDEO)
    {
        return m_AssetSourceVideo->ModifyGlobalSettings(error, m_MS, origfourCC, newfourCC, newType, newStructSize, newRepeat, newData);
    } else {
        error = GPMF_LIB_ERROR_NOT_IMPLEMENTED;
    }
}
