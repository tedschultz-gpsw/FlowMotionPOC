/*! @file GPMFAssetTrack.h
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

#ifndef GPMF_ASSET_TRACK_H
#define GPMF_ASSET_TRACK_H

#include "GPMFAssetSource.h"
#include "GPMFAssetTrackTimeline.h"
#include "GPMFTimeRange.h"

class GPMFAssetSource;
class GPMFAssetTrackTimeline;

class GPMFAssetTrack
{
public:
    GPMFAssetTrack(GPMFAssetSource& gpmfasset);
    virtual ~GPMFAssetTrack();
    
    GPMF_LIB_ERROR GetSampleCount(uint32_t fourcc, uint32_t *sampleCount);
    GPMF_LIB_ERROR GetSample(uint32_t fourcc, uint32_t index, GPMFSampleptr samplePtr);
    uint32_t GetSampleTimeRangeCount();
    GPMFTimeRange *GetSampleTimeRange(uint32_t index);
    GPMF_LIB_ERROR ExportTimeline(FILE *file, GPMF_EXPORT_FILE_TYPE fileType = GPMF_FILE_TYPE_CONSOLE);
    GPMF_LIB_ERROR ExportTimeline(FILE *file, uint32_t fourc, GPMF_EXPORT_FILE_TYPE fileType = GPMF_FILE_TYPE_CONSOLE);
    GPMF_LIB_ERROR ExportTimeline(FILE *file, uint32_t fourcc, double inTime, double outTime, GPMF_EXPORT_FILE_TYPE fileType = GPMF_FILE_TYPE_CONSOLE);
    GPMF_LIB_ERROR ExportTimeline(FILE *file, double inTime, double outTime, GPMF_EXPORT_FILE_TYPE fileType = GPMF_FILE_TYPE_CONSOLE);
    uint32_t GetSampleSegments(void);
    uint32_t GetSampleSegmentTime(uint32_t index, double *inTime, double *outTime);
    GPMF_LIB_ERROR ResetTimeline(void);

    double m_TrackSegmentInTime;
    double m_TrackSegmentOutTime;
    GPMFAssetSource *m_Asset;
    GPMFAssetTrackTimeline *m_AssetTrackTimeline;
    uint32_t m_TrackBufferSize;
};

#endif /* GPMF_ASSET_TRACK_H */
