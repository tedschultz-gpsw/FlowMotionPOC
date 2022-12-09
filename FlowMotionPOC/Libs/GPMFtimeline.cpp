/*! @file GPMFtimeline.cpp
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
 *! \mainpage libGPMF
 *
 * \section intro_sec Introduction
 *
 * GPMFtimeline is the public class to libGPMF.
 */

#include "GPMFTimeline.h"
#include "GPMFTimeRange.h"

/**
 * \class GPMFTimeline
 *
 * \brief This class provides routines to Read and Write GPMF Metadata
 *
 * This class provides reading and writing of GPMF metadata to a timeline and
 * global Metadata.  It support MP4, LRV and JPEG file formats.
 *
 */

/**
 * @brief GPMFTimeline class contructor
 */
GPMFTimeline::GPMFTimeline()
{
    m_AssetSource = new GPMFAssetSource();
    m_sample = NULL;
}

/**
 * @brief GPMFTimeline class destructor
 */
GPMFTimeline::~GPMFTimeline()
{
    if(m_AssetSource){
        m_AssetSource->Close();
        delete(m_AssetSource);
        m_AssetSource = NULL;
    }
    
    if(m_sample)
        delete m_sample;
}

/**
 * @brief OpenURI
 * @param uri - the uri to open
 * @param error - GPMF_LIB_OK if no error
 */
void GPMFTimeline::OpenURI(const std::string &uri, GPMF_LIB_ERROR& error)
{
    error = (GPMF_LIB_ERROR)m_AssetSource->Open((char *)uri.c_str());
}

/**
 * @brief SetGPMFGlobalBuffer
 * @param buffer = GPMF input buffer
 * @param buffersize = GPMF input buffersize
 * @param error - GPMF_LIB_OK if no error
 */
void GPMFTimeline::SetGPMFGlobalBuffer(const char *buffer, uint32_t buffersize, GPMF_LIB_ERROR& error)
{
    m_AssetSource->SetGPMFGlobalBuffer(buffer, buffersize, error);
}

/**
 * @brief GetTrackDuration
 * @param error - GPMF_LIB_OK if no error
 * @return GPMFRational - duration in rational format
 */
GPMFRational GPMFTimeline::GetTrackDuration(GPMF_LIB_ERROR& error)
{
    GPMFRational gpmfRational;
    error = (GPMF_LIB_ERROR)m_AssetSource->GetTrackDuration(&gpmfRational);
    return gpmfRational;
}

/**
 * @brief GetVideoFrameRateAndCount
 * @param error - GPMF_LIB_OK if no error
 * @param count - Frame count
 * @return GPMFRational - frame rate
 */
GPMFRational GPMFTimeline::GetVideoFrameRateAndCount(GPMF_LIB_ERROR& error, uint32_t *count)
{
    return m_AssetSource->GetVideoFrameRateAndCount(error, count);
}
    
/**
 * @brief SetDeviceIds
 * @param deviceIds - array of deviceIds
 * @param error - GPMF_LIB_OK if no error
 */
void GPMFTimeline::SetDeviceIds(std::vector<std::string> deviceIds, GPMF_LIB_ERROR& error)
{
    error = m_AssetSource->SetDeviceIds(deviceIds);
}

/**
 * @brief SetDeviceNames
 * @param deviceNames - array of deviceNames
 * @param error - GPMF_LIB_OK if no error
 */
void GPMFTimeline::SetDeviceNames(std::vector<std::string> deviceNames, GPMF_LIB_ERROR& error)
{
    error = m_AssetSource->SetDeviceNames(deviceNames);
}

/**
 * @brief GetNextTimeRange
 * @param error - GPMF_LIB_OK if no error
 * @return GPMFTimeRange class ptr, caller need call delete to free it
 */
GPMFTimeRange *GPMFTimeline::GetNextTimeRange(GPMF_LIB_ERROR &error)
{
    return m_AssetSource->GetNextTimeRange(error);
}

/**
 * @brief Parse
 * @param error - GPMF_LIB_OK if no error
 * @param parseType - (optional) defaults to GPMF_FILE_TYPE_ALL
 * @param fourcc - (optional) array of fourcc, defaults to all
 * @param timeRange - (optional) timeRange, defaults to full duration
 */
void GPMFTimeline::Parse(GPMF_LIB_ERROR& error, GPMF_PARSE_TYPE parseType, const std::vector<std::string> fourcc, GPMFTimeRange *timeRange)
{
    m_AssetSource->Parse(error, parseType, fourcc, timeRange);
}

/**
 * @brief GetNextSample
 * @param fourcc - fourcc of sample
 * @param error - GPMF_LIB_OK if no error
 * @return GPMFSampleptr.
 */
GPMFSampleptr GPMFTimeline::GetNextSample(const std::string fourcc, GPMF_LIB_ERROR& error)
{
    if(m_sample)
        delete m_sample;
    m_sample = new GPMFSample();
    error =  (GPMF_LIB_ERROR) m_AssetSource->GetNextSample(fourcc.c_str(), m_sample);
    
    if(error == GPMF_LIB_OK)
    {
        return m_sample;
    } else {
        delete m_sample;
        m_sample = NULL;
        // Either did not find or walked to end of samples
        if( error == GPMF_LIB_ERROR_MEMORY)
        {
            error = GPMF_LIB_OK;
        }
        return NULL;
    }
}

/**
 * @brief AddSampleToTimeline
 * @param sample - sample as GPMFSample
 * @param timeRange - GPMFTimeRange of sample
 * @param error - GPMF_LIB_OK if no error
 */
void GPMFTimeline::AddSampleToTimeline(const std::unique_ptr<GPMFSampleptr> sample, GPMFTimeRange timeRange, GPMF_LIB_ERROR& error)
{
    error = GPMF_LIB_ERROR_NOT_IMPLEMENTED;
}

/**
 * @brief ExportTimeline
 * @param error - GPMF_LIB_OK if no error
 * @param fileType - (optional) GPMF_EXPORT_FILE_TYPE to export to, GPMF_FILE_TYPE_CONSOLE is default
 * @param uri - (optional) file to export to
 * @param fourcc - (optional) array of fourcc, defaults to all
 * @param timeRange - (optional) timeRange, defaults to full duration
 */
void GPMFTimeline::ExportTimeline(GPMF_LIB_ERROR& error, GPMF_EXPORT_FILE_TYPE fileType, std::string *uri, std::vector<std::string> *fourcc, GPMFTimeRange *timeRange)
{
    if(uri != NULL)
    {
        error = (GPMF_LIB_ERROR)m_AssetSource->ExportTimeline(uri->c_str(), fileType);
    } else {
        error = (GPMF_LIB_ERROR)m_AssetSource->ExportTimeline("stdout", fileType);
    }
}

/**
 * @brief GetSampleCount
 * @param [in]  fourcc - fourcc of interest
 * @param [out] sampleCount - Sample count of actual 4cc instances (note, does not expand sample repeat counts)
 * @return GPMF_LIB_ERROR - GPMF_LIB_OK if no error
 */
GPMF_LIB_ERROR GPMFTimeline::GetSampleCount(const std::string fourcc, uint32_t *sampleCount)
{
    if (m_AssetSource == NULL)
    {
        return GPMF_LIB_FAIL;
    }
    
    return m_AssetSource->GetSampleCount(fourcc.c_str(), sampleCount);
}

/**
 * @brief GetSample
 * @param [in]  fourcc - fourcc of interest
 * @param [in] index - index of sample count
 * @param [out] error - GPMF_LIB_OK if no error
 * @return GPMFSampleptr, caller does not delete GPMFSampleptr
 */
GPMFSampleptr GPMFTimeline::GetSample(const std::string fourcc, uint32_t index, GPMF_LIB_ERROR& error)
{
    if (m_AssetSource == NULL)
    {
        return NULL;
    }
    
    if(m_sample)
        delete m_sample;
    m_sample = new GPMFSample();
    error =  (GPMF_LIB_ERROR) m_AssetSource->GetSample(fourcc.c_str(), index, m_sample);
    
    if(error == GPMF_LIB_OK)
    {
        return m_sample;
    } else {
        delete m_sample;
        m_sample = NULL;
        // Either did not find or walked to end of samples
        if( error == GPMF_LIB_ERROR_MEMORY)
        {
            error = GPMF_LIB_OK;
        }
        return NULL;
    }
}

/**
 * @brief ModifyGlobalSettings
 * @param error - GPMF_LIB_OK if no error
 * @param origfourCC - original fourcc of sample
 * @param newfourCC - new fourcc of sample
 * @param newType - GPMF_SampleType
 * @param newStructSize - new struct size
 * @param newRepeat - new repeat of sample
 * @param newData - new data of sample
 */
void GPMFTimeline::ModifyGlobalSettings(GPMF_LIB_ERROR& error, uint32_t origfourCC, uint32_t newfourCC, char newType, uint32_t newStructSize, uint32_t newRepeat, void* newData)
{
    return m_AssetSource->ModifyGlobalSettings(error, origfourCC, newfourCC, newType, newStructSize, newRepeat, newData);
}


