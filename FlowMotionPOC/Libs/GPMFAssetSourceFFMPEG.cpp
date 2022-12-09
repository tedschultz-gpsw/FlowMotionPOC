/*! @file GPMFAssetSourceFFMPEG.cpp
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

#include "GPMFAssetSourceFFMPEG.h"
#include "GPMFAssetSource.h"

#define AV_FOURCC_MAX_STRING_SIZE 32
#define GPMF_MAKETAG(a, b, c, d) (((a & 0xFF) << 24) | ((b & 0xFF) << 16) | ((c & 0xFF) << 8) | (d & 0xFF))

GPMFAssetSourceFFMPEG::GPMFAssetSourceFFMPEG()
{
    m_FmtCtx = NULL;
    m_Stream = NULL;
    m_GlobalDataBuffer = NULL;
    m_GlobalDataBufferSize = 0;
    m_Firmware = NULL;
    m_Duration = 0;
    m_VideoDuration = 0;
    m_VideoFPSRational.den = 0;
    m_VideoFPSRational.num = 0;
    m_VideoNumFrames = 0;
}

GPMFAssetSourceFFMPEG::~GPMFAssetSourceFFMPEG()
{
    if(m_GlobalDataBuffer){
        free(m_GlobalDataBuffer);
    }
}

GPMF_LIB_ERROR GPMFAssetSourceFFMPEG::Open(char* filename)
{
    uint32_t ret;
    AVRational gpmf_time_base;
    AVDictionary *options;
    AVDictionaryEntry *firmware_tag = NULL;
    
    options = NULL;
    ret = OpenVideo((const char *)filename, options);
    if (ret != 0 || m_Stream == NULL) {
        return GPMF_LIB_ERROR_FILE_OPEN_FAILED;
    }
    m_VideoFPSRational = GetFPS();

    gpmf_time_base = TimebaseForStream(m_Stream);
    m_Duration = (m_Stream)->duration * av_q2d(gpmf_time_base);
    m_VideoDuration = (m_FmtCtx)->duration / (double)AV_TIME_BASE;
    
    if (m_Firmware) {
        firmware_tag = av_dict_get((*m_FmtCtx).metadata, "firmware", NULL, 0);
        if (firmware_tag) {
            m_Firmware = firmware_tag->value;
        } else {
            m_Firmware = NULL;
        }
    }
    
    ret = (uint32_t)ParseTrackTimes();

    return GPMF_LIB_OK;
}

GPMF_LIB_ERROR GPMFAssetSourceFFMPEG::OpenUDTA(char* filename)
{
#ifdef USE_FFMPEG_NOUDATA
    return GPMF_LIB_OK;
#endif
    uint32_t ret;
    AVDictionary *options;
    // UDTA
    avformat_network_init();
    
    options = NULL;
    ret = OpenVideoUDTA((const char *)filename, options);
    if (ret < 0 || m_Stream == NULL) {
        return GPMF_LIB_ERROR_MEMORY;
    }
    
    m_VideoFPSRational = GetFPS();

    char *buffer = GetPayLoadBufferUDTA(m_FmtCtx, (int *)&m_GlobalDataBufferSize);
   
    if(buffer &&  m_GlobalDataBufferSize > 0)
    {
        m_GlobalDataBuffer = (char *)malloc(m_GlobalDataBufferSize);
        memcpy(m_GlobalDataBuffer, buffer, m_GlobalDataBufferSize);
        ret = ParseUDTATrackTimes();
    }
    
    return GPMF_LIB_OK;
}

void GPMFAssetSourceFFMPEG::Close()
{
    if(m_FmtCtx != NULL)
        avformat_close_input(&m_FmtCtx);
    m_FmtCtx = NULL;
}

float GPMFAssetSourceFFMPEG::GetAssetDuration()
{
    return m_Duration;
}

GPMFRational GPMFAssetSourceFFMPEG::GetVideoFrameRateAndCount(GPMF_LIB_ERROR& error, uint32_t *count)
{
    GPMFRational rational;
    rational.denominator = m_VideoFPSRational.den;
    rational.quotient = 0;
    rational.numerator = m_VideoFPSRational.num;
    *count = (uint32_t)m_VideoNumFrames;
    error = GPMF_LIB_OK;
    return rational;
}

GPMF_LIB_ERROR GPMFAssetSourceFFMPEG::GetAssetDuration(GPMFRational *rationalTime)
{
    //Double2Rational(gpmf_duration, 100, rationalTime);
    *rationalTime = (double)m_Duration;
    
    return GPMF_LIB_OK;
}

double GPMFAssetSourceFFMPEG::GetSampleRate(uint32_t fourcc)
{
    double firstsampletime = 0;
    double lastsampletime = 0;
    return GetSampleRate(fourcc, &firstsampletime, &lastsampletime);
}

double GPMFAssetSourceFFMPEG::GetSampleRate(uint32_t fourcc, double *firstsampletime, double *lastsampletime)
{
    double sampleRate = 0;
    
    if(m_FramesStartAndDurationTimes[0].first == GPMF_GLOBAL_TIME)
    {
        *firstsampletime = 0;
        *lastsampletime = 0;
    } else {
        uint32_t last = GetPayloadCount() - 1;

//        *firstsampletime = frameStartAndDurationRationalTimes[0].first.numerator / frameStartAndDurationRationalTimes[0].first.denominator;
//        *lastsampletime = frameStartAndDurationRationalTimes[last].second.numerator / frameStartAndDurationRationalTimes[last].second.denominator;
        *firstsampletime = m_FramesStartAndDurationTimes[0].first;
        *lastsampletime = m_FramesStartAndDurationTimes[last].second;
    }
    return sampleRate;
}

uint32_t *GPMFAssetSourceFFMPEG::GetPayload(uint32_t *lastpayload, uint32_t index)
{
    uint32_t *ret;
    if(m_FramesStartAndDurationTimes[index].first < 0)
    {
        return (uint32_t *)m_GlobalDataBuffer;
    }
    
    ret = GetPayLoadBuffer(m_FramesStartAndDurationTimes[index].first);
    return ret;
}

uint32_t GPMFAssetSourceFFMPEG::GetPayloadCount()
{
    return (uint32_t)m_FramesStartAndDurationTimes.size();
}

uint32_t GPMFAssetSourceFFMPEG::GetPayloadSize(uint32_t index)
{
    if(m_FramesStartAndDurationTimes[index].first < 0)
    {
        return m_GlobalDataBufferSize;
    }
    
    return GetPayLoadSizePrivate(m_FramesStartAndDurationTimes[index].first);
}

uint32_t GPMFAssetSourceFFMPEG::GetPayloadTime(uint32_t index, double *in, double *out)
{
    if(m_FramesStartAndDurationTimes[index].first < 0)
    {
        *in = GPMF_GLOBAL_TIME;
        *out = 0;
        return 0;
    }
    *in = m_FramesStartAndDurationTimes[index].first;
    *out = m_FramesStartAndDurationTimes[index].first + m_FramesStartAndDurationTimes[index].second;
    return 0;
}

uint32_t GPMFAssetSourceFFMPEG::OpenVideo(const char *path, AVDictionary *options)
{
    uint32_t ret, i;
    
    m_FmtCtx = NULL;
    m_Stream = NULL;
    ret = (uint32_t)avformat_open_input(&m_FmtCtx, path, NULL, &options);
    if (ret != 0) {
        return ret;
    }
    
    m_Stream = NULL;
    for (i = 0; i < (m_FmtCtx)->nb_streams; ++i)
    {
        char buf[AV_FOURCC_MAX_STRING_SIZE] = {0};
        FourCCToSring((char *)buf, (uint32_t)(m_FmtCtx)->streams[i]->codecpar->codec_tag);
        
        if (!strcmp(buf, "gpmd")) {
            m_Stream = (m_FmtCtx)->streams[i];
        } else {
            (m_FmtCtx)->streams[i]->discard = AVDISCARD_ALL;
        }
    }
    
    return 0;
}

 AVRational GPMFAssetSourceFFMPEG::GetFPS(void)
{
    AVRational rational;
    
    for (unsigned int i = 0; i < m_FmtCtx->nb_streams; i++) {
        AVStream *avstream = m_FmtCtx->streams[i];
        if(avstream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            rational = av_guess_frame_rate(m_FmtCtx, avstream, NULL);
            
            m_VideoNumFrames = avstream->nb_frames;
            
            return rational;
        }
    }
    return rational;
}

uint32_t GPMFAssetSourceFFMPEG::OpenVideoUDTA(const char *path, AVDictionary *options)
{
    uint32_t ret, i;
    m_FmtCtx = NULL;
    m_Stream = NULL;
    
    AVDictionary *opts = 0;
    av_dict_set(&opts, "export_udta", "1", 0);
    m_FmtCtx = NULL;
    m_Stream = NULL;
    ret = (uint32_t)avformat_open_input(&m_FmtCtx, path, NULL, &opts);
    if (ret < 0) {
        return (uint32_t)-2;
    }
    
    m_Stream = NULL;
    for (i = 0; i < (m_FmtCtx)->nb_streams; ++i)
    {
        char buf[AV_FOURCC_MAX_STRING_SIZE] = {0};
        FourCCToSring((char *)buf, (uint32_t)(m_FmtCtx)->streams[i]->codecpar->codec_tag);
        
        if (!strcmp(buf, "gpmd")) {
            m_Stream = (m_FmtCtx)->streams[i];
        } else {
            (m_FmtCtx)->streams[i]->discard = AVDISCARD_ALL;
        }
    }
    
    return 0;
}

int GPMFAssetSourceFFMPEG::ParseTrackTimes()
{
    int ret;
    AVPacket pkt;
    AVRational time_base;
    
    time_base = TimebaseForStream(m_Stream);
    avformat_flush(m_FmtCtx);
    av_init_packet(&pkt);
    
    while (1)
    {
        ret = av_read_frame(m_FmtCtx, &pkt);
        if (ret < 0)
        {
            if (ret == AVERROR_EOF) {
                return 0;
            }
            return ret;
        }
        
        if (pkt.stream_index == m_Stream->index)
        {
            double packet_time = (pkt.pts * time_base.num) / (double)time_base.den;
            double packet_duration = (pkt.duration * time_base.num) / (double)time_base.den;
            m_FramesStartAndDurationTimes.push_back( std::pair<double, double>(packet_time, packet_duration));
            GPMFRational rationalIn (time_base.num, time_base.den);
            GPMFRational rationalOut ((int) pkt.duration * time_base.num, time_base.den);
            m_FrameStartAndDurationRationalTimes.push_back( std::pair<GPMFRational, GPMFRational>(rationalIn, rationalOut));
        }
        
        av_packet_unref(&pkt);
    }
    
    return 0;
}

uint32_t GPMFAssetSourceFFMPEG::ParseUDTATrackTimes()
{
    m_FramesStartAndDurationTimes.push_back( std::pair<double, double>(GPMF_GLOBAL_TIME, 0));
    return 0;
}

uint32_t GPMFAssetSourceFFMPEG::GetPayLoadSizePrivate(double time)
{
    uint32_t ret;
    AVPacket pkt;
    AVRational time_base;
    
    time_base = TimebaseForStream(m_Stream);
    time = time * time_base.den;
    avformat_flush(m_FmtCtx);
    av_init_packet(&pkt);
    
    if(av_seek_frame(m_FmtCtx, m_Stream->index,  time, AVSEEK_FLAG_FRAME) >= 0)
    {
        ret = (uint32_t)av_read_frame(m_FmtCtx, &pkt);
        if (ret < 0)
        {
            if (ret == (uint32_t)AVERROR_EOF) {
                return 0;
            }
            return ret;
        }
        
        if (pkt.stream_index == m_Stream->index)
        {
            return (uint32_t)pkt.size;
        }
        
        av_packet_unref(&pkt);
    }
    
    return 0;
}

uint32_t *GPMFAssetSourceFFMPEG::GetPayLoadBuffer(double time)
{
    int ret;
    AVPacket pkt;
    AVRational time_base;
    
    time_base = TimebaseForStream(m_Stream);
    time = time * time_base.den;
    avformat_flush(m_FmtCtx);
    av_init_packet(&pkt);
    
    if(av_seek_frame(m_FmtCtx, m_Stream->index,  time, AVSEEK_FLAG_FRAME) >= 0)
    {
        ret = av_read_frame(m_FmtCtx, &pkt);
        if (ret < 0)
        {
            if (ret == AVERROR_EOF) {
                return 0;
            }
            return NULL;
        }
        
        if (pkt.stream_index == m_Stream->index)
        {
            return (uint32_t*)pkt.data;
        }
        
        av_packet_unref(&pkt);
    }
    
    return 0;
}

char *GPMFAssetSourceFFMPEG::FourCCToSring(char *buf, uint32_t fourcc)
{
    int i;
    char *orig_buf = buf;
    uint32_t buf_size = AV_FOURCC_MAX_STRING_SIZE;
    
    for (i = 0; i < 4; i++) {
        const int c = fourcc & 0xff;
        const int print_chr = (c >= '0' && c <= '9') ||
        (c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c && strchr(". -_", c));
        const int len = snprintf(buf, buf_size, print_chr ? "%c" : "[%d]", c);
        if (len < 0)
            break;
        buf += len;
        buf_size = buf_size > len ? buf_size - len : 0;
        fourcc >>= 8;
    }
    
    return orig_buf;
}

AVRational GPMFAssetSourceFFMPEG::TimebaseForStream(AVStream *stream)
{
    AVRational result;
    
    if (stream->time_base.num == 1 && stream->time_base.den == 1) {
        /* fix invalid time scale for video re-encoded by the camera */
        fprintf(stderr, "gpmf: invalid time_scale, defaulting to 1/1000\n");
        result.num = 1;
        result.den = 1000;
    } else {
        result = stream->time_base;
    }
    return result;
}

int GPMFAssetSourceFFMPEG::BufferReadUint32(gpmfBuffer *buffer, int *position, uint32_t *value)
{
    if ((uint32_t)*position + (uint32_t)sizeof(*value) > (uint32_t)buffer->length || (uint32_t)*position < 0) {
        fprintf(stderr, "gpmf: buffer read out of bound\n");
        return -1;
    }
    
    /* big endian to host conversion */
    const unsigned char *data = buffer->data + *position;
    *value = (uint32_t)((data[3] << 0) | (data[2] << 8) | (data[1] << 16) | (data[0] << 24));
    *position += sizeof(*value);
    return 0;
}

char *GPMFAssetSourceFFMPEG::GetPayLoadBufferUDTA(AVFormatContext *fmt_ctx, int *buffer_size)
{
    AVFormatSideData *side_data = avformat_get_side_data(fmt_ctx, AVFMT_DATA_USER);

    if (side_data)
    {
        gpmfBuffer buffer;
        int pos, ret;
        uint32_t size, four_cc;
        buffer.data = side_data->data;
        buffer.length = side_data->size;
        
        pos = 0;
        while (pos < buffer.length)
        {
            ret = BufferReadUint32(&buffer, &pos, &size);
            if (ret < 0) {
                return NULL;
            }
            int next_pos = pos - 4 + size;
            ret = BufferReadUint32(&buffer, &pos, &four_cc);
            if (ret < 0) {
                return NULL;
            }
            if (four_cc == GPMF_MAKETAG('G', 'P', 'M', 'F'))
            {
                
                *buffer_size = next_pos - pos;
                return (char *)buffer.data + pos;
            }
            pos = next_pos;
        }
    }
    return NULL;
}

#endif
