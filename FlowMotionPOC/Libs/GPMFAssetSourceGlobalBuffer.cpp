/*! @file GPMFAssetSourceGlobalBuffer.cpp
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

#include "GPMFAssetSourceGlobalBuffer.h"

GPMFAssetSourceGlobalBuffer::GPMFAssetSourceGlobalBuffer()
{
    m_buffer = NULL;
    m_buffersize = 0;
}

GPMFAssetSourceGlobalBuffer::~GPMFAssetSourceGlobalBuffer()
{
}

uint32_t GPMFAssetSourceGlobalBuffer::SetGPMFGlobalBuffer(const char *buffer, uint32_t buffersize)
{
    m_buffer = buffer;
    m_buffersize = buffersize;
   
    return GPMF_LIB_OK;
}

GPMF_LIB_ERROR GPMFAssetSourceGlobalBuffer::GetAssetDuration(GPMFRational *rationalTime)
{
    rationalTime->denominator = 0;
    rationalTime->numerator = 0;
    return GPMF_LIB_OK;
}

float GPMFAssetSourceGlobalBuffer::GetAssetDuration()
{
    return 0;
}

double GPMFAssetSourceGlobalBuffer::GetSampleRate(uint32_t fourcc)
{
    return 0;
}

double GPMFAssetSourceGlobalBuffer::GetSampleRate(uint32_t fourcc, double *firstsampletime, double *lastsampletime)
{
    *firstsampletime = 0L;
    *lastsampletime = 0L;
    return GPMF_LIB_OK;
}

uint32_t *GPMFAssetSourceGlobalBuffer::GetPayload(uint32_t *lastpayload, uint32_t index)
{
    return (uint32_t *)m_buffer;
}

uint32_t GPMFAssetSourceGlobalBuffer::GetPayloadCount()
{
    return 1;
}

uint32_t GPMFAssetSourceGlobalBuffer::GetPayloadSize(uint32_t index)
{
    return m_buffersize;
}

uint32_t GPMFAssetSourceGlobalBuffer::GetPayloadTime(uint32_t index, double *in, double *out)
{
    *in = 0;
    *out = 0;
    return GPMF_LIB_OK;
}
