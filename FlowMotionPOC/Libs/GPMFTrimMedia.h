/*! @file GPMFTrimMedia.h
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
 * \class GPMFTrimMedia
 *
 * \brief This class provides routines to Read and Write GPMF Metadata
 *
 * This class provides reading and writing of GPMF metadata to a timeline and
 * global Metadata.  It support MP4, LRV and JPEG file formats.
 *
 */
class GPMFTrimMedia
{
public:
    /**
     * @brief GPMFTrimMedia class contructor
     */
    GPMFTrimMedia();
    
    /**
     * @brief GPMFTrimMedia class destructor
     */
    virtual ~GPMFTrimMedia();
    
    /**
     * @brief Trim
     * @param uri - the uri to open
     * @param timeRange - time range
     * @param fileType - file type
     * @param outputuri - output file
     * @param error - GPMF_LIB_OK if no error
     */
    void Trim(const std::string &uri, GPMFTimeRange *timeRange, GPMF_EXPORT_FILE_TYPE fileType, const std::string &outputuri, GPMF_LIB_ERROR& error);
    
    
private:
    /**
     * @brief private member for asset source class
     */
    GPMFAssetSource *m_AssetSource;
};

#endif /* GPMF_TIMELINE_H */
