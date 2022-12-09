/*! @file GPMFTrimMedia.cpp
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
 * GPMFTrimMedia is the public class to libGPMF.
 */

#include "GPMFTrimMedia.h"
#include "GPMFTimeRange.h"

/**
 * \class GPMFTrimMedia
 *
 * \brief This class provides routines to Read and Write GPMF Metadata
 *
 * This class provides reading and writing of GPMF metadata to a timeline and
 * global Metadata.  It support MP4, LRV and JPEG file formats.
 *
 */

/**
 * @brief GPMFTrimMedia class contructor
 */
GPMFTrimMedia::GPMFTrimMedia()
{
    m_AssetSource = new GPMFAssetSource();
}

/**
 * @brief GPMFTrimMedia class destructor
 */
GPMFTrimMedia::~GPMFTrimMedia()
{
    if(m_AssetSource)
        delete(m_AssetSource);
}

/**
 * @brief OpenURI
 * @param uri - the uri to open
 * @param error - GPMF_LIB_OK if no error
 */
void GPMFTrimMedia::Trim(const std::string &uri, GPMFTimeRange *timeRange, GPMF_EXPORT_FILE_TYPE fileType, const std::string &outputuri, GPMF_LIB_ERROR& error)
{
    error = (GPMF_LIB_ERROR)m_AssetSource->Open((char *)uri.c_str());
}
