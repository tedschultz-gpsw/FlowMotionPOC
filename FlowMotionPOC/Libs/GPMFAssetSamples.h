/*! @file GPMFAssetSamples.h
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

#ifndef GPMF_ASSET_SAMPLES_H
#define GPMF_ASSET_SAMPLES_H

#include "GPMFAssetSample.h"
#include "GPMFAssetTrackSegment.h"

class GPMFAssetSamples
{
public:
    GPMFAssetSamples(GPMFAssetSource& gpmfasset);
    virtual ~GPMFAssetSamples();
    uint32_t GetNextSampleSegment(void *assetTrackSample, void *trackSample, double inTime, double outTime);
    GPMFAssetSample *AddAssetTimeSample(void *assetTrackSample, double sampleTime, double sampleRate, void *sampleBuffer, uint32_t sampleSize, uint32_t sampleRepeat);
    
    GPMFAssetSource* m_Asset;
};

#endif /* GPMF_ASSET_SAMPLES_H */
