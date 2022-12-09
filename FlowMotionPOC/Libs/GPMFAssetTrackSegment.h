/*! @file GPMFAssetTrackSegment.h
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

#ifndef GPMF_ASSET_TRACK_SEGMENT_H
#define GPMF_ASSET_TRACK_SEGMENT_H

#include "GPMFAssetTrack.h"
#include "GPMFAssetSamples.h"

class GPMFAssetSamples;

class GPMFAssetTrackSegment
{
public:
    GPMFAssetTrackSegment(GPMFAssetSource& gpmfasset, uint32_t trackType);
    virtual ~GPMFAssetTrackSegment();
    GPMF_LIB_ERROR GetSamples(GPMFAssetTrack *trackSample);
    GPMF_LIB_ERROR GetAllSamples(GPMFAssetTrack *trackSample);
    GPMF_LIB_ERROR GetSamples(GPMFAssetTrack *trackSample, uint32_t trackType, double inTime, double outTime);
    double GetSampleRate(uint32_t fourcc, uint32_t tracks, double *firstSampletime, double *lastSampletime);

    GPMFAssetSource* m_Asset;
    GPMFAssetTrack* m_AssetTrack;
    GPMFAssetSamples* m_SampleSegment;
    GPMFSample m_Sample;
    double m_FirstSampleTime;
    double m_LastSampleTime;
};

#endif /* GPMF_ASSET_TRACK_SEGMENT_H */
