/*! @file GPMFAssetSourceFFMPEG.h
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
#ifdef USE_FFMPEG

#ifndef GPMF_ASSET_SOURCE_FFMPEG_H
#define GPMF_ASSET_SOURCE_FFMPEG_H

#include "GPMFAssetSourceVideo.h"
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
}
#endif

typedef struct
{
    const unsigned char *data;
    int length;
} gpmfBuffer;

class GPMFAssetSourceFFMPEG : GPMFAssetSourceVideo
{
public:
    GPMFAssetSourceFFMPEG();
    virtual ~GPMFAssetSourceFFMPEG();
    
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

private:
    uint32_t OpenVideo(const char *path, AVDictionary *options);
    uint32_t OpenVideoUDTA(const char *path, AVDictionary *options);
    int ParseTrackTimes();
    uint32_t ParseUDTATrackTimes();
    uint32_t GetPayLoadSizePrivate(double time);
    uint32_t *GetPayLoadBuffer(double time);
    char *GetPayLoadBufferUDTA(AVFormatContext *fmt_ctx, int *buffer_size);
    int BufferReadUint32(gpmfBuffer *buffer, int *position, uint32_t *value);
    char *FourCCToSring(char *buf, uint32_t fourcc);
    AVRational GetFPS(void);

    std::vector<std::pair<double, double> > m_FramesStartAndDurationTimes;
    std::vector<std::pair<GPMFRational, GPMFRational> > m_FrameStartAndDurationRationalTimes;
    AVRational TimebaseForStream(AVStream *stream);
    AVFormatContext *m_FmtCtx;
    AVStream *m_Stream;
    char *m_Firmware;
    char *m_GlobalDataBuffer;
    uint32_t m_GlobalDataBufferSize;
    double m_Duration;
    double m_VideoDuration;
    AVRational m_VideoFPSRational;
    int64_t m_VideoNumFrames;
};

#endif /* GPMF_ASSET_SOURCE_FFMPEG_H */
#endif
