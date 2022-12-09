#pragma once
#include <string>
#include <vector>
#include <functional>

#include "Vector.hpp"
#include "Quaternion.hpp"


class GPMFFrameMetadata
{
public:
    struct FrameData {
        int64_t timestamp;                          // timestamp in nanosecond
        IJK::Quaternion cori;                       // camera orientation
        IJK::Quaternion iori;                       // image orientation
        IJK::Vector grav;                           // grav orientation
        IJK::Quaternion coriFiltered;               // camera orientation
        IJK::Quaternion stabilizationStateQuat;     // value generated for current stabilization mode
        float frameRate;                            // timeWarp Framerate
        mutable uint32_t hlRotStrategy;             // stores cached orientation value being used for cori filtering.
    };
    
    struct FrameIQData { //superbank only
        int64_t timestamp;
        double frontShut;
        double frontIsog;
        double backShut;
        double backIsog;
    };
    
    enum class Status {OK, NOIMUCAL, METADATA_GRAV_INVALID, FAILED};

public:
    static Status extract_cori_iori_grav(const char *filepath, std::vector<FrameData> &output);
    static Status extract_cori_iori_grav(const char *filepath, std::vector<FrameData> &output, bool *sphericalTimeLapse, bool *superView, unsigned char *eisType);
    static Status extract_isog_shut(const char *frontFilepath, const char *backFilepath, std::vector<FrameIQData> &output);

private:
};

