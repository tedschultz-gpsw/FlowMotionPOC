/*! @file GPMFTimeline.h
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

#include "GPMFTypes.h"
#include "GPMFAssetSource.h"
#include "GPMFTimeRange.h"

#ifndef GPMF_TIMELINE_H
#define GPMF_TIMELINE_H

/**
 * \class GPMFTimeline
 *
 * \brief This class provides routines to Read and Write GPMF Metadata
 *
 * This class provides reading and writing of GPMF metadata to a timeline and
 * global Metadata.  It support MP4, LRV and JPEG file formats.
 *
 */
class GPMFTimeline
{
public:
    /**
     * @brief GPMFTimeline class contructor
     */
    GPMFTimeline();
    
    /**
     * @brief GPMFTimeline class destructor
     */
    virtual ~GPMFTimeline();
    
    /**
     * @brief OpenURI
     * @param uri - the uri to open
     * @param error - GPMF_LIB_OK if no error
     */
    void OpenURI(const std::string &uri, GPMF_LIB_ERROR& error);
    
    /**
     * @brief SetGPMFGlobalBuffer
     * @param buffer = GPMF input buffer
     * @param buffersize = GPMF input buffersize
     * @param error - GPMF_LIB_OK if no error
     */
    void SetGPMFGlobalBuffer(const char *buffer, uint32_t buffersize, GPMF_LIB_ERROR& error);
    
    /**
     * @brief GetTrackDuration
     * @param error - GPMF_LIB_OK if no error
     * @return GPMFRational - duration in rational format
     */
    GPMFRational GetTrackDuration(GPMF_LIB_ERROR& error);
    
    /**
     * @brief GetVideoFrameRateAndCount
     * @param error - GPMF_LIB_OK if no error
     * @param count - Frame count
     * @return GPMFRational - frame rate
     */
    GPMFRational GetVideoFrameRateAndCount(GPMF_LIB_ERROR& error, uint32_t *count);
    
    /**
     * @brief SetDeviceIds
     * @param deviceIds - array of deviceIds
     * @param error - GPMF_LIB_OK if no error
     */
    void SetDeviceIds(std::vector<std::string> deviceIds, GPMF_LIB_ERROR& error);
    
    /**
     * @brief SetDeviceNames
     * @param deviceNames - array of SetDeviceNames
     * @param error - GPMF_LIB_OK if no error
     */
    void SetDeviceNames(std::vector<std::string> deviceNames, GPMF_LIB_ERROR& error);
    
    /**
     * @brief GetNextTimeRange
     * @param error - GPMF_LIB_OK if no error
     * @return GPMFTimeRange class ptr, caller does not delete GPMFTimeRange ptr
     */
    GPMFTimeRange *GetNextTimeRange(GPMF_LIB_ERROR &error);
    
    /**
     * @brief Parse
     * @param error - GPMF_LIB_OK if no error
     * @param parseType - (optional) defaults to GPMF_FILE_TYPE_ALL
     * @param fourcc - (optional) array of fourcc, defaults to all
     * @param timeRange - (optional) timeRange, defaults to full duration
     */
    void Parse(GPMF_LIB_ERROR& error, GPMF_PARSE_TYPE parseType = GPMF_FILE_TYPE_ALL, const std::vector<std::string> fourcc = {}, GPMFTimeRange *timeRange = nullptr);
    
    /**
     * @brief GetNextSample
     * @param fourcc - fourcc of sample
     * @param error - GPMF_LIB_OK if no error
     * @return GPMFSampleptr, caller does not delete GPMFSampleptr
     */
    GPMFSampleptr GetNextSample(const std::string fourcc, GPMF_LIB_ERROR& error);
    
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
    void ModifyGlobalSettings(GPMF_LIB_ERROR& error, uint32_t origfourCC, uint32_t newfourCC, char newType, uint32_t newStructSize, uint32_t newRepeat, void* newData);

    /**
     * @brief AddSampleToTimeline
     * @param sample - sample as GPMFSample
     * @param timeRange - GPMFTimeRange of sample
     * @param error - GPMF_LIB_OK if no error
     */
    void AddSampleToTimeline(const std::unique_ptr<GPMFSampleptr> sample, GPMFTimeRange timeRange, GPMF_LIB_ERROR& error);
    
    /**
     * @brief ExportTimeline
     * @param error - GPMF_LIB_OK if no error
     * @param fileType - (optional) GPMF_EXPORT_FILE_TYPE to export to, GPMF_FILE_TYPE_CONSOLE is default
     * @param uri - (optional) file to export to
     * @param fourcc - (optional) array of fourcc, defaults to all
     * @param timeRange - (optional) timeRange, defaults to full duration
     */
    void ExportTimeline(GPMF_LIB_ERROR& error, GPMF_EXPORT_FILE_TYPE fileType = GPMF_FILE_TYPE_CONSOLE, std::string *uri = nullptr, std::vector<std::string> *fourcc = nullptr, GPMFTimeRange *timeRange = nullptr);
    
    /**
     * @brief GetSampleCount
     * @param [in]  fourcc - fourcc of interest
     * @param [out] sampleCount - Sample count of actual 4cc instances (note, does not expand sample repeat counts)
     * @return GPMF_LIB_ERROR - GPMF_LIB_OK if no error
     */
    GPMF_LIB_ERROR GetSampleCount(const std::string fourcc, uint32_t *sampleCount);

    /**
     * @brief GetSample
     * @param [in]  fourcc - fourcc of interest
     * @param [in] index - index of sample count
     * @param [out] error - GPMF_LIB_OK if no error
     * @return GPMFSampleptr, caller does not delete GPMFSampleptr
     */
    GPMFSampleptr GetSample(const std::string fourcc, uint32_t index, GPMF_LIB_ERROR& error);

    
private:
    /**
     * @brief private member for asset source class
     */
    GPMFAssetSource *m_AssetSource;
    GPMFSampleptr m_sample;
};

#endif /* GPMF_TIMELINE_H */
