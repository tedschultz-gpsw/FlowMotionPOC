/*! @file GPMFAssetSourceMP4Reader.h
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

class GPMFAssetSourceMP4Reader : GPMFAssetSourceVideo
{
public:
    GPMFAssetSourceMP4Reader();
    virtual ~GPMFAssetSourceMP4Reader();
    
    GPMF_LIB_ERROR Open(char* filename);
    GPMF_LIB_ERROR OpenUDTA(char* filename);
    void Close();
    float GetAssetDuration();
    GPMF_LIB_ERROR GetAssetDuration(GPMFRational *rationalTime);
    double GetSampleRate(uint32_t fourcc);
    double GetSampleRate(uint32_t fourcc, double *firstsampletime, double *lastsampletime);
    uint32_t *GetPayload(uint32_t *lastpayload, uint32_t index);
    uint32_t GetPayloadCount();
    uint32_t GetPayloadSize(uint32_t index);
    uint32_t GetPayloadTime(uint32_t index, double *in, double *out);
    GPMFRational GetVideoFrameRateAndCount(GPMF_LIB_ERROR& error, uint32_t *count);
    void ModifyGlobalSettings(GPMF_LIB_ERROR& error, void *ms, uint32_t origfourCC, uint32_t newfourCC, char newType, uint32_t newStructSize, uint32_t newRepeat, void* newData);

private:
    size_t m_MP4Object;
    size_t m_MP4PayloadResource;
    bool m_IsUDTA;
};

#endif /* GPMF_ASSET_SOURCE_MP4_READER_H */
