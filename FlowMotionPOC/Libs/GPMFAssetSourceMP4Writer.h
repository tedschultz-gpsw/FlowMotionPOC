/*! @file GPMFAssetSourceMP4Writer.h
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

#ifndef GPMF_ASSET_SOURCE_MP4_READER_H
#define GPMF_ASSET_SOURCE_MP4_READER_H

#include "GPMFAssetSourceVideo.h"

class GPMFAssetSourceMP4Writer : GPMFAssetSourceVideo
{
public:
    GPMFAssetSourceMP4Writer();
    virtual ~GPMFAssetSourceMP4Writer();

    size_t Open(char *filename, uint32_t file_time_base, uint32_t payload_duration);
    uint32_t Export(size_t handle, uint32_t *payload, uint32_t payload_size);
    void Close(size_t handle);
private:
    size_t m_MP4Object;
    size_t m_Handle;
    size_t m_HandleB;
    uint32_t *m_ScratchBuffer;
};

#endif /* GPMF_ASSET_SOURCE_MP4_READER_H */
