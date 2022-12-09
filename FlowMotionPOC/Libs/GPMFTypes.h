/*! @file GPMFTypes.h
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

#ifndef GPMF_TYPES_H
#define GPMF_TYPES_H

#include "GPMFRational.h"
#include "GPMFURI.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <map>
#include <memory>

#ifdef DBG_GPMF
#if _WINDOWS
#define DBG_MSG_GPMF printf
#else
#define DBG_MSG_GPMF printf
#endif
#else
#define DBG_MSG_GPMF(...)
#endif

#define FOURCC(s) (uint32_t) ((s[0]<<0)|(s[1]<<8)|(s[2]<<16)|(s[3]<<24))

#define FOURCC_PRINTF(k) ((k) >> 0) & 0xff, ((k) >> 8) & 0xff, ((k) >> 16) & 0xff, ((k) >> 24) & 0xff


typedef enum GPMF_FILE_TYPE
{
    GPMF_FILE_TYPE_VIDEO = 0,
    GPMF_FILE_TYPE_IMAGE,
    GPMF_FILE_TYPE_GLOBAL_BUFFER,
    GPMF_FILE_TYPE_NOT_SUPPORTED,
    GPMF_FILE_TYPE_BAD_FILE_NAME
} GPMF_FILE_TYPE;

typedef enum GPMF_PARSE_TYPE : int
{
    GPMF_FILE_TYPE_ALL = 0,
    GPMF_FILE_TYPE_UDATA_ONLY,
    GPMF_FILE_TYPE_GPMF_ONLY,
    GPMF_FILE_TYPE_BUFFER
} GPMF_PARSE_TYPE;

typedef enum GPMF_EXPORT_FILE_TYPE
{
    GPMF_FILE_TYPE_MP4 = 0,
    GPMF_FILE_TYPE_LRV,
    GPMF_FILE_TYPE_JPG,
    GPMF_FILE_TYPE_JSON,
    GPMF_FILE_TYPE_RAW,
    GPMF_FILE_TYPE_CONSOLE
} GPMF_EXPORT_FILE_TYPE;

typedef enum GPMF_LIB_ERROR
{
    GPMF_LIB_OK = 0,
    GPMF_LIB_FAIL,
    GPMF_LIB_ERROR_MEMORY,
    GPMF_LIB_ERROR_BAD_STRUCTURE,
    GPMF_LIB_ERROR_BUFFER_END,
    GPMF_LIB_ERROR_FIND,
    GPMF_LIB_ERROR_LAST,
    GPMF_LIB_ERROR_TYPE_NOT_SUPPORTED,
    GPMF_LIB_ERROR_SCALE_NOT_SUPPORTED,
    GPMF_LIB_ERROR_SCALE_COUNT,
    GPMF_LIB_ERROR_NOT_IMPLEMENTED,
    GPMF_LIB_ERROR_NOT_VALID_FOR_TYPE,
    GPMF_LIB_ERROR_FILE_OPEN_FAILED,
    GPMF_LIB_ERROR_TIMERANGE_NOT_FOUND,
    GPMF_LIB_ERROR_TYPE_NOT_FOUND,
    GPMF_LIB_ERROR_RESERVED
} GPMF_LIB_ERROR;

typedef struct GPMFSample {
    GPMFRational rationalTime;          // Sample time in seconds as rationa. number
    double sampleTime;                  // Sample time in seconds
    uint64_t sampleSTMP;                // STMP
    uint32_t sampleKey;                 // FourCC
    uint32_t sampleType;                // GPMF_Type
    uint32_t sampleTypeSize;            // Size of GPMF_Type
    uint32_t sampleStructSize;          // Sample structure size
    uint32_t sampleRepeat;              // Repeat or the number of samples of this structure
    uint32_t sampleCount;               // number of samples of this structure, supporting multisample entries
    uint32_t sampleElementsInStruct;    // Number elements within the structure (e.g. 3-axis gyro)
    uint32_t sampleTSMP;                // TSMP
    uint32_t sampleNest;                // Nest level
    uint32_t sampleSCAL;                // SCAL
    char deviceName[32];                // Device name (DVNM)
    uint32_t deviceID;                  // Device ID (DVID)
    char complexType[32];               // Complex type
    double sampleRate;                  // Sample Rate
    void *sampleBuffer;                 // Sample Buffer
    uint32_t sampleBufferSize;          // Data size
    
    GPMFSample()
    {
        sampleTime = 0.0;
        sampleSTMP = 0;
        sampleKey = 0;
        sampleTypeSize = 0;
        sampleStructSize = 0;
        sampleRepeat = 0;
        sampleCount = 0;
        sampleElementsInStruct = 0;
        sampleTSMP = 0;
        sampleNest = 0;
        sampleSCAL = 0;
        memset(&deviceName, 0, sizeof(deviceName));
        deviceID = 0;
        memset(&complexType, 0, sizeof(complexType));
        sampleRate = 1.0;
        sampleBuffer = NULL;
        sampleBufferSize = 0;
    }
} GPMFSample, *GPMFSampleptr;

typedef enum
{
    GPMF_STRING = 'c',                  // single byte 'c' style character string
    GPMF_STRING_ASCII = 'c',            // single byte 'c' style character string
    GPMF_SIGNED_BYTE = 'b',             // single byte signed number
    GPMF_UNSIGNED_BYTE = 'B',           // single byte unsigned number
    GPMF_DOUBLE = 'd',                  // 64-bit double precision float (IEEE 754)
    GPMF_SIGNED_64BIT_INT = 'j',        // 64-bit integer
    GPMF_UNSIGNED_64BIT_INT = 'J',      // 64-bit integer
    GPMF_FLOAT = 'f',                   // 32-bit single precision float (IEEE 754)
    GPMF_FOURCC = 'F',                  // 32-bit four character tag
    GPMF_GUID = 'G',                    // 128-bit ID (like UUID)
    GPMF_HIDDEN = 'h',                  // internal data not displayed (formatting not reported)
    GPMF_UNSIGNED_LONG_HEX = 'H',       // 32-bit integer to be displayed 0xaabbccdd
    GPMF_SIGNED_LONG = 'l',             // 32-bit integer
    GPMF_UNSIGNED_LONG = 'L',           // 32-bit integer
    GPMF_Q15_16_FIXED_POINT = 'q',      // Q number Q15.16 - 16-bit signed integer (A) with 16-bit fixed point (B)
                                        // for A.B value (range -32768.0 to 32767.99998).
    GPMF_Q31_32_FIXED_POINT = 'Q',      // Q number Q31.32 - 32-bit signed integer (A) with 32-bit fixed point (B) for A.B value.
    GPMF_SIGNED_SHORT = 's',            // 16-bit integer
    GPMF_UNSIGNED_SHORT = 'S',          // 16-bit integer
    GPMF_STRING_UTF8 = 'u',             // UTF-8 formatted text string.  As the character storage size varies,
                                        // the size is in bytes, not UTF characters.
    GPMF_UTC_DATE_TIME = 'U',           // Date + UTC Time format yymmddhhmmss.sss - 16 bytes ASCII (years 20xx covered)
    GPMF_XML = 'x',                     // XML, support other systems metadata
    GPMF_COMPLEX= '?',                  // for sample with complex data structures, base size in bytes.
                                        // Data is either opaque, or the stream has a TYPE structure field for the sample.
    GPMF_COMPLEX_SIZE_1 = '1',          // 1 byte of opaque data
    GPMF_COMPLEX_SIZE_2 = '2',          // 2 bytes of opaque data
    GPMF_COMPLEX_SIZE_3 = '3',          // 3 bytes of opaque data
    GPMF_COMPLEX_SIZE_4 = '4',          // 4 bytes of opaque data
    GPMF_COMPLEX_SIZE_8 = '8',          // 8 bytes of opaque data
    GPMF_NEST = 0,                      // used to nest more GPMF formatted metadata
}  GPMF_METADATA_TYPE;

#endif /* GPMF_TYPES_H */
