/*! @file GPMFAssetTrack.cpp
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

#include <stdint.h>
#include "GPMFAssetTrack.h"
#include "GPMF_parser.h"
#include "GPMF_mp4reader.h"

GPMFAssetTrack::GPMFAssetTrack(GPMFAssetSource& gpmfasset)
{
    m_Asset = &gpmfasset;
    m_Asset->m_MS = NULL;
    m_Asset->m_Payload = NULL;
    m_AssetTrackTimeline = new GPMFAssetTrackTimeline(gpmfasset);
}

GPMFAssetTrack::~GPMFAssetTrack()
{
    if(m_Asset->m_MS)
    {
        free(m_Asset->m_MS);
        m_Asset->m_MS = NULL;
    }
    
    if(m_AssetTrackTimeline)
    {
        delete m_AssetTrackTimeline;
    }
}

uint32_t GPMFAssetTrack::GetSampleSegments(void)
{
    m_Asset->m_Payloads = m_Asset->GetPayloadCount();
    return m_Asset->m_Payloads;
}

uint32_t GPMFAssetTrack::GetSampleSegmentTime(uint32_t index, double *inTime, double *outTime)
{
    if( !m_Asset->m_MS) {
        m_Asset->m_MS = (GPMF_stream *)malloc(sizeof(GPMF_stream));
        memset(m_Asset->m_MS, 0, sizeof(GPMF_stream));
    }
    
    if(m_Asset->m_MS)
    {
        GPMF_ResetState((GPMF_stream*)m_Asset->m_MS);

        uint32_t ret = GPMF_OK;
        uint32_t payloadsize = m_Asset->GetPayloadSize(index);
        
        m_Asset->m_Payload = m_Asset->GetPayload(m_Asset->m_Payload, index);
        if (m_Asset->m_Payload == NULL)
            return 0;
        
        m_Asset->GetPayloadTime(index, &m_TrackSegmentInTime, &m_TrackSegmentOutTime);
        *inTime = m_TrackSegmentInTime;
        *outTime = m_TrackSegmentOutTime;
        
        ret = GPMF_Init((GPMF_stream *)m_Asset->m_MS, m_Asset->m_Payload, (uint32_t)payloadsize);
        if (ret != GPMF_OK)
            return 0;
        
    }
    
    return 1;
}

GPMF_LIB_ERROR GPMFAssetTrack::GetSampleCount(uint32_t fourcc, uint32_t *sampleCount)
{
    return m_AssetTrackTimeline->GetSampleCount(fourcc, sampleCount);
}

GPMF_LIB_ERROR GPMFAssetTrack::GetSample(uint32_t fourcc, uint32_t index, GPMFSampleptr samplePtr)
{
    return m_AssetTrackTimeline->GetSample(fourcc, index, samplePtr);
}

uint32_t GPMFAssetTrack::GetSampleTimeRangeCount()
{
    return m_AssetTrackTimeline->GetSampleTimeRangeCount();
}

GPMFTimeRange *GPMFAssetTrack::GetSampleTimeRange(uint32_t index)
{
    return m_AssetTrackTimeline->GetSampleTimeRange(index);
}

GPMF_LIB_ERROR GPMFAssetTrack::ExportTimeline(FILE *file, GPMF_EXPORT_FILE_TYPE fileType)
{
    return m_AssetTrackTimeline->ExportTimeline(file, fileType);
}

GPMF_LIB_ERROR GPMFAssetTrack::ExportTimeline(FILE *file, uint32_t fourcc, GPMF_EXPORT_FILE_TYPE fileType)
{
    return m_AssetTrackTimeline->ExportTimeline(file, fourcc, fileType);
}

GPMF_LIB_ERROR GPMFAssetTrack::ExportTimeline(FILE *file, uint32_t fourcc, double inTime, double outTime, GPMF_EXPORT_FILE_TYPE fileType)
{
    return m_AssetTrackTimeline->ExportTimeline(file, fourcc, inTime, outTime, fileType);
}

GPMF_LIB_ERROR GPMFAssetTrack::ExportTimeline(FILE *file, double inTime, double outTime, GPMF_EXPORT_FILE_TYPE fileType)
{
    return m_AssetTrackTimeline->ExportTimeline(file, inTime, outTime, fileType);
}

GPMF_LIB_ERROR GPMFAssetTrack::ResetTimeline(void)
{
    return m_AssetTrackTimeline->ResetTimeline();
}

