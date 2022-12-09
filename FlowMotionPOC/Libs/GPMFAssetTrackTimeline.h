/*! @file GPMFAssetTrackTimeline.h
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

#ifndef GPMF_ASSET_TRACK_TIMELINE_H
#define GPMF_ASSET_TRACK_TIMELINE_H

#include <vector>
#include <algorithm>
#include "GPMFAssetSource.h"
#include "GPMFAssetSample.h"
#include "GPMFTimeRange.h"
#include "GPMFAssetTrack.h"

class GPMFAssetSource;


class GPMFAssetTrackTimeline
{
public:
    GPMFAssetTrackTimeline(GPMFAssetSource& gpmfasset);
    virtual ~GPMFAssetTrackTimeline();
    void AddAssetTimeSample(GPMFAssetSample *assetSample);
    void AddSegmentTime(double inTime, double outTime);
    GPMF_LIB_ERROR GetSampleCount(uint32_t fourcc, uint32_t *sampleCount);
    GPMF_LIB_ERROR GetSample(uint32_t fourcc, uint32_t index, GPMFSampleptr samplePtr);
    uint32_t GetSampleTimeRangeCount();
    GPMFTimeRange *GetSampleTimeRange(uint32_t index);
    GPMF_LIB_ERROR ExportTimeline(FILE *file, GPMF_EXPORT_FILE_TYPE fileType = GPMF_FILE_TYPE_CONSOLE);
    GPMF_LIB_ERROR ExportTimeline(FILE *file, uint32_t fourc, GPMF_EXPORT_FILE_TYPE fileType = GPMF_FILE_TYPE_CONSOLE);
    GPMF_LIB_ERROR ExportTimeline(FILE *file, uint32_t fourcc, double inTime, double outTime, GPMF_EXPORT_FILE_TYPE fileType = GPMF_FILE_TYPE_CONSOLE);
    GPMF_LIB_ERROR ExportTimeline(FILE *file, double inTime, double outTime, GPMF_EXPORT_FILE_TYPE fileType = GPMF_FILE_TYPE_CONSOLE);
    GPMF_LIB_ERROR ResetTimeline(void);

private:
    std::vector<std::pair<double, GPMFAssetSample*> > GetFourCCSet(uint32_t fourcc, std::vector<std::pair<double, GPMFAssetSample*> > set);
    std::vector<std::pair<double, GPMFAssetSample*> > GetFourCCSet(uint32_t fourcc, uint32_t in, uint32_t out, std::vector<std::pair<double, GPMFAssetSample*> > set);
    std::vector<std::pair<double, GPMFAssetSample*> > GetTimedSet(uint32_t in, uint32_t out, std::vector<std::pair<double, GPMFAssetSample*> > set);
    void printJsonTimeline(std::vector<std::pair<double, GPMFAssetSample*> > timeLine);
    void PrintfFormattedData(uint32_t type, uint32_t structsize, uint32_t repeat, void *data);
    std::string EscapeJSON(const std::string& input);
    const char *Type2String(int32_t type);
    void PrintGPMFInternal(void *msv);

    GPMFAssetSource *m_Asset;
    std::vector<std::pair<double, GPMFAssetSample*> > m_TrackTimeline;
    std::vector<std::pair<double, GPMFAssetSample*> > m_FourccSubset;
    std::vector<std::pair<double, double> > m_SegmentTimes;
    std::vector<GPMFTimeRange*> m_Timeranges;
    uint32_t m_CachedFourcc;
    FILE *m_FP;
    bool m_MinimalJSON;
};

#endif /* GPMF_ASSET_TRACK_TIMELINE_H */
