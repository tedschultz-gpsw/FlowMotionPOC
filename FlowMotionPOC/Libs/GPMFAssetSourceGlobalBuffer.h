/*! @file GPMFAssetSourceGlobalBuffer.h
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

#ifndef GPMF_ASSET_SOURCE_BUFFER_H
#define GPMF_ASSET_SOURCE_BUFFER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "GPMFTypes.h"

class GPMFAssetSourceGlobalBuffer
{
public:
    GPMFAssetSourceGlobalBuffer();
    virtual ~GPMFAssetSourceGlobalBuffer();
    
    uint32_t SetGPMFGlobalBuffer(const char *buffer, uint32_t buffersize);
    float GetAssetDuration();
    GPMF_LIB_ERROR GetAssetDuration(GPMFRational *rationalTime);
    double GetSampleRate(uint32_t fourcc);
    double GetSampleRate(uint32_t fourcc, double *firstsampletime, double *lastsampletime);
    uint32_t *GetPayload(uint32_t *lastpayload, uint32_t index);
    uint32_t GetPayloadCount();
    uint32_t GetPayloadSize(uint32_t index);
    uint32_t GetPayloadTime(uint32_t index, double *in, double *out);
    
private:
    const char *m_buffer;
    uint32_t m_buffersize;
};

#endif /* GPMF_ASSET_SOURCE_IMAGE_H */
