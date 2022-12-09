//
//  SphericalMetadataProvider.hpp
//  lib-sphericalMetadataProvider
//
//  Created by Daryl on 9/6/19.
//  Copyright © 2019 Daryl. All rights reserved.
//

#ifndef SphericalMetadataProvider_hpp
#define SphericalMetadataProvider_hpp

#include "Quaternion.hpp"
#include <vector>
#include <stdio.h>
#include "GPMFFrameMetadata.h"

class SphericalMetadataProvider
{
public:
    enum class Status {
        OK = 0,
        NOT_AVAILABLE,
        FAILED
    };
    
    enum class GPMFStabilizationState {
        kGPMFStabilizationStateAllOff,             //World Lock OFF,   Horizon Leveling OFF,   Stabilization OFF
        kGPMFStabilizationStateAntiShake,          //World Lock OFF,   Horizon Leveling OFF,   Stabilization ON
        kGPMFStabilizationStateWorldLock,          //World Lock ON,    Horizon Leveling OFF,   Stabilization ON
        kGPMFStabilizationStateHorizonLevel,       //World Lock OFF,   Horizon Leveling ON,    Stabilization ON
        kGPMFStabilizationStateAllOn,              //World Lock ON,    Horizon Leveling ON,    Stabilization ON
    };
    
    static std::vector<IJK::Quaternion> objc_meld_cori_grav(const char* filepath, GPMFStabilizationState state);
    static Status meld_cori_grav(const char* filepath, GPMFStabilizationState state, std::vector<IJK::Quaternion>& output);
    static Status meld_cori_grav(const char* filepath, GPMFStabilizationState state, int32_t startFrameIdx, int32_t requestedCount, std::vector<IJK::Quaternion>& output);
    static Status meld_cori_grav(const std::vector<GPMFFrameMetadata::FrameData>& frameData, GPMFStabilizationState state, int32_t startFrameIdx, int32_t requestedCount, std::vector<IJK::Quaternion>& output);

    static Status meld_cori_iori_grav_old(const char *filepath, GPMFStabilizationState state, std::vector<IJK::Quaternion> &output);
    static Status meld_cori_iori_grav(const char *filepath, GPMFStabilizationState state, std::vector<IJK::Quaternion> &output);
    static Status meld_cori_iori_grav(const char *filepath, GPMFStabilizationState state, int32_t startFrameIdx, int32_t requestedCount, std::vector<IJK::Quaternion> &output);
    static Status meld_cori_iori_grav(const std::vector<GPMFFrameMetadata::FrameData> &frameData, GPMFStabilizationState state, int32_t startFrameIdx, int32_t requestedCount, std::vector<IJK::Quaternion> &output);
};

#endif /* SphericalMetadataProvider_hpp */
