/*! @file GPMFAssetSourceMP4Writer.cpp
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "GPMFAssetSource.h"
#include "GPMF_mp4writer.h"
#include "GPMF_writer.h"
#include "GPMFAssetSourceMP4Writer.h"

#define SCRATCH_BUFFER_SIZE 1024

GPMFAssetSourceMP4Writer::GPMFAssetSourceMP4Writer()
{
}

GPMFAssetSourceMP4Writer::~GPMFAssetSourceMP4Writer()
{
}

size_t GPMFAssetSourceMP4Writer::Open(char *filename, uint32_t file_time_base, uint32_t payload_duration)
{
    //size_t handleB = 0;
    m_MP4Object =  OpenMP4Export(filename, file_time_base, payload_duration);
    m_Handle = GPMFWriteServiceInit();
    m_HandleB = GPMFWriteStreamOpen(m_Handle, GPMF_CHANNEL_TIMED, GPMF_DEVICE_ID_CAMERA, (char *)"libGPMF", NULL, 0);
    m_ScratchBuffer = (uint32_t *)malloc(SCRATCH_BUFFER_SIZE);
    GPMFWriteSetScratchBuffer(m_Handle, m_ScratchBuffer, SCRATCH_BUFFER_SIZE);
    return m_MP4Object;
}

void GPMFAssetSourceMP4Writer::Close(size_t handle)
{
    CloseExport(m_MP4Object);
}

uint32_t GPMFAssetSourceMP4Writer::Export(size_t handle, uint32_t *payload, uint32_t payload_size)
{
    return ExportPayload(m_MP4Object, payload, payload_size);
}

