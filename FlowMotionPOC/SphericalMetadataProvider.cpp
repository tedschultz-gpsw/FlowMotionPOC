//
//  SphericalMetadataProvider.cpp
//  lib-sphericalMetadataProvider
//
//  Created by Daryl on 9/6/19.
//  Copyright © 2019 Daryl. All rights reserved.
//

#include "SphericalMetadataProvider.hpp"
#include "GPMFFrameMetadata.h"
#include <limits>

#define _USE_MATH_DEFINES
#include "math.h"

using namespace std;

enum RotationStrategy
{
    UNKNOWN = 0,
    NORTH_POLE = 1,
    SOUTH_POLE = -1
};

inline bool isCoriFilterNeeded(const GPMFFrameMetadata::FrameData &frameData) {
    return frameData.frameRate > -1;
}

inline float asinf_nan_avoid(float input)
{
    if (input > 1.0f)
    {
        return asinf(1.0);
    }
    else
    {
        return asinf(input);
    }
}

static bool coriFilter(
    const vector<GPMFFrameMetadata::FrameData> &input,
    vector<IJK::Quaternion> &coriFilteredOut)
{
    if (input.size() != coriFilteredOut.size()) {
        return false;
    }

    IJK::Quaternion prevQ = IJK::Quaternion(0, 0, 0, 1);
    int nSteady = 0;
    IJK::Quaternion cur = IJK::Quaternion(0, 0, 0, 1);
    
    for(int i = 0; i < input.size(); i++)
    {
        if (input[i].frameRate < 0) {
            continue;
        }

        IJK::Quaternion cori = input[i].cori;
        IJK::Quaternion r = cori * prevQ.inverted();

        prevQ = cori;
    
        IJK::Quaternion rot = r.toAxeAngle();
        
        float speed = (float)(rot.w() * 180.0 / M_PI * input[i].frameRate);
        
        if (speed < 0.5)
        {
            nSteady++;
        }
        else
        {
            nSteady=0;
        }
        
        if(nSteady < 5)
        {
            cur = r * cur;
        }

        coriFilteredOut[i] = cur;
    }
    return true;
}

static IJK::Quaternion getOrient(
    int frameIndex,
    const vector<GPMFFrameMetadata::FrameData> &input,
    const vector<IJK::Quaternion> &coriFiltered)
{
    IJK::Quaternion cori =
        isCoriFilterNeeded(input[frameIndex])
            ? coriFiltered[frameIndex]
            : input[frameIndex].cori;
    
    IJK::Quaternion iori = input[frameIndex].iori;
    IJK::Quaternion camOrient =  iori * cori;
    camOrient = camOrient.normalized();
    return camOrient.inverse();
}

static pair<IJK::Quaternion, float> processRollFromCameraOrient(
    IJK::Quaternion camOrient,
    IJK::Quaternion prevCamOrient,
    IJK::Quaternion prevRollOrient,
    float prevAngle,
    int index)
{
    float angle;
    float camAngle;

    float rollMax      = (float)(20*M_PI/180);
    float rollLock     = (float)(20*M_PI/180);
    float rollStrength = 0.8f;

    if(index > 0) {
        IJK::Quaternion d= prevRollOrient.inverse() * camOrient.inverse() * prevCamOrient * prevRollOrient;
        float x = 2*d.x()*d.y() - 2*d.z()*d.w();     // from matrix
        float y = 1 - 2*d.x()*d.x() -2*d.z()*d.z();
        camAngle = atan2(x,y);
        angle = prevAngle - camAngle;
    } else {
        camAngle = prevAngle;
        angle = prevAngle;
    }
    angle *= rollStrength;
    angle = max(-rollMax, min(rollMax, angle));
    if(abs(camAngle)>rollLock)
    {
        angle = prevAngle;
    }
    IJK::Quaternion newRoll = IJK::Quaternion(0.0f, 0.0f, sinf(angle/2.0f), cosf(angle/2.0f));
    return pair<IJK::Quaternion, float>(newRoll, angle);
}

static bool generateAntiShakeToIndex(
    const vector<GPMFFrameMetadata::FrameData> &input,
    const vector<IJK::Quaternion> &coriFiltered,
    vector<IJK::Quaternion> &stabilizationStateQuatOut)
{
    if (input.size() != stabilizationStateQuatOut.size()) {
        return false;
    }

    IJK::Quaternion prevCamOrient = IJK::Quaternion(0, 0, 0, 1);
    IJK::Quaternion prevRollOrient = IJK::Quaternion(0, 0, 0, 1);
    
    IJK::Quaternion camOrient;
    IJK::Quaternion roll;
    float prevAngle = 0.0;

    for(int i = 0; i < input.size(); i++)
    {
        camOrient = getOrient(i, input, coriFiltered);
        auto result = processRollFromCameraOrient(camOrient, prevCamOrient, prevRollOrient, prevAngle, i);
        roll = result.first;
        prevAngle = result.second;
        
        prevCamOrient = camOrient;
        prevRollOrient = roll;
                
        stabilizationStateQuatOut[i] = camOrient * roll;
    }
    
    return true;
}

static IJK::Quaternion horizonLevelQuatForIndex(
    int index,
    const vector<GPMFFrameMetadata::FrameData> &input,
    const vector<IJK::Quaternion> &coriFiltered)
{
    IJK::Vector grav = input[index].grav.normalized();
    IJK::Vector x = IJK::crossProduct(IJK::Vector(0, 0, 1), grav).normalized();
    IJK::Vector z = IJK::crossProduct(grav, x).normalized();
    
    IJK::Quaternion q = IJK::fromAxes(-x, grav, z);
    IJK::Quaternion cori =
        isCoriFilterNeeded(input[index])
            ? coriFiltered[index]
            : input[index].cori;
    
    return cori.inverted() * q;
}

static bool generateWorldLockStabToIndex(
    const vector<GPMFFrameMetadata::FrameData> &input,
    const vector<IJK::Quaternion> &coriFiltered,
    vector<IJK::Quaternion> &stabilizationStateQuatOut)
{
    if (input.size() != stabilizationStateQuatOut.size()) {
        return false;
    }

    IJK::Quaternion orientFollow = IJK::Quaternion();
    IJK::Quaternion stabIn;
    
    for(int i = 0; i < input.size(); i++)
    {
        stabIn = horizonLevelQuatForIndex(i, input, coriFiltered);
        
        float d1 = (orientFollow - stabIn).magnitude();
        float d2 = (orientFollow + stabIn).magnitude();
        
        if (d2<d1)
        {
            stabIn = -stabIn;
        }
        
        orientFollow = stabIn;
        stabilizationStateQuatOut[i] = stabIn;
    }
    return true;
}

static SphericalMetadataProvider::Status antiShake(
    int frameIndex,
    const IJK::Quaternion &cori,
    const IJK::Quaternion &iori,
    const vector<GPMFFrameMetadata::FrameData> &input,
    const vector<IJK::Quaternion> &stabilizationStateQuat,
    std::vector<IJK::Quaternion> &output)
{
    SphericalMetadataProvider::Status statusOut = SphericalMetadataProvider::Status::OK;
    
    IJK::Quaternion camOrient =  iori * cori;
    
    int halfIntegTime = 30;
    int iMin = max(0, frameIndex - halfIntegTime);
    int iMax = min((int)input.size() - 2, frameIndex + halfIntegTime);
    
    IJK::Quaternion q = IJK::Quaternion(0, 0, 0, 0);
    
    float weight;
    for (int n = iMin; n<=iMax; n++)
    {
        weight = (float)(halfIntegTime + 1 - abs(n-frameIndex));
        q = q + stabilizationStateQuat[n].scale(weight);
    }
        
    //Don't need inverse here like their code, cause iori * cori is inverse of the one they used
    IJK::Quaternion o = camOrient * q.normalized();
        
    output.push_back(o);

    return statusOut;
}

static SphericalMetadataProvider::Status worldLockOnly(
    int frameIndex,
    const IJK::Quaternion &cori,
    const IJK::Quaternion &iori,
    const vector<GPMFFrameMetadata::FrameData> &input,
    std::vector<IJK::Quaternion> &output)
{
    SphericalMetadataProvider::Status statusOut = SphericalMetadataProvider::Status::OK;
    
    IJK::Quaternion camOrient =  iori * cori;
    camOrient = camOrient.normalized();
    
    output.push_back(camOrient);

    return statusOut;
}

static SphericalMetadataProvider::Status horizonLevelOnly(
    int frameIndex,
    const IJK::Quaternion &cori,
    const IJK::Quaternion &iori,
    const vector<GPMFFrameMetadata::FrameData> &input,
    const vector<IJK::Quaternion> &stabilizationStateQuat,
    vector<IJK::Quaternion> &output)
{
    SphericalMetadataProvider::Status statusOut = SphericalMetadataProvider::Status::OK;
    
    int halfIntegTime = 20;
    int iMin = max(0, frameIndex - halfIntegTime);
    int iMax = min((int)input.size() - 2, frameIndex + halfIntegTime);
    
    IJK::Quaternion q = IJK::Quaternion(0, 0, 0, 0);
        
    float weight;
    for (int n = iMin; n<=iMax; n++)
    {
        weight = (float)(halfIntegTime + 1 - abs(n-frameIndex));
        q = q + stabilizationStateQuat[n].scale(weight);
    }
    
    IJK::Quaternion o = iori * cori * q.normalized();
    
    output.push_back(o);

    return statusOut;
}

static SphericalMetadataProvider::Status allOn(
    int frameIndex,
    const IJK::Quaternion &cori,
    const IJK::Quaternion &iori,
    const vector<GPMFFrameMetadata::FrameData> &input,
    vector<IJK::Quaternion> &output)
{
    SphericalMetadataProvider::Status statusOut = SphericalMetadataProvider::Status::OK;
    
    IJK::Quaternion camOrient = iori * cori;
    camOrient = camOrient.normalized();
    
    IJK::Vector grav = input[frameIndex].grav;
    
    IJK::Vector axe;
    
    IJK::Vector gravInit = cori.inverted().rotatedVector(grav).normalized();
    
    IJK::Quaternion gravRot;
    
    if(input[0].hlRotStrategy == RotationStrategy::UNKNOWN)
    {
        // at init we choose a rotation strategy, and we must keep this strategy until the end of the video
        // If we don't do this, there can be discontinuities during the video
        if (gravInit.y() > 0.f)
        {
            input[0].hlRotStrategy = RotationStrategy::NORTH_POLE;
        }
        else
        {
            input[0].hlRotStrategy = RotationStrategy::SOUTH_POLE;
        }
    }
    
    if (input[0].hlRotStrategy == RotationStrategy::NORTH_POLE)
    {
        axe = IJK::crossProduct(IJK::Vector(0,1,0), gravInit);
        float angle =  asinf_nan_avoid(axe.length());
        
        if(gravInit.y() < 0.f)
        {
            angle = (float)(M_PI - angle);
        }
        gravRot = IJK::Quaternion(axe.normalized(), angle);
        
    } else {
        // upside down
        axe = IJK::crossProduct(IJK::Vector(0,-1,0), gravInit);
        float angle =  asinf_nan_avoid(axe.length());
        
        if(gravInit.y() > 0.f)
        {
            angle = (float)(M_PI - angle);
        }
        
        gravRot = IJK::Quaternion(axe.normalized(), angle) * IJK::Quaternion(0,0,1,0);
    }
    
    IJK::Quaternion o = camOrient * gravRot.normalized();
    
    output.push_back(o);
    
    return statusOut;

}

SphericalMetadataProvider::Status meld_cori_iori_grav_internal(
    const std::vector<GPMFFrameMetadata::FrameData>& frameData,
    SphericalMetadataProvider::GPMFStabilizationState state,
    bool applyIORI,
    int32_t startFrameIdx,
    int32_t requestedCount,
    std::vector<IJK::Quaternion>& output)
{
    //All Off Ends Without Calculation
    vector<IJK::Quaternion> stabilizationStateQuat;
    stabilizationStateQuat.resize(frameData.size());

    vector<IJK::Quaternion> coriFiltered;
    coriFiltered.resize(frameData.size());
    coriFilter(frameData, coriFiltered);

    if (state == SphericalMetadataProvider::GPMFStabilizationState::kGPMFStabilizationStateAntiShake)
    {
        generateAntiShakeToIndex(frameData, coriFiltered, stabilizationStateQuat);
    }

    if (state == SphericalMetadataProvider::GPMFStabilizationState::kGPMFStabilizationStateHorizonLevel)
    {
        generateWorldLockStabToIndex(frameData, coriFiltered, stabilizationStateQuat);
    }

    IJK::Quaternion identity = IJK::Quaternion(0.0, 0.0, 0.0, 1.0);

    auto frameCount = std::min(requestedCount, (int32_t)frameData.size() - startFrameIdx);
    auto endFrameIdx = startFrameIdx + frameCount;
    output.reserve(frameCount);

    // Make sure we cache the rot orientation using the first frame and "AllOn"
    if (frameData[0].hlRotStrategy == RotationStrategy::UNKNOWN)
    {
        IJK::Quaternion cori = frameData[0].cori;
        IJK::Quaternion iori = frameData[0].iori;

        SphericalMetadataProvider::Status status = SphericalMetadataProvider::Status::FAILED;
        std::vector<IJK::Quaternion> discard;
        status = allOn(0, cori, iori, frameData, discard);
        if (status != SphericalMetadataProvider::Status::OK)
            return status;
    }

    for (int frameIdx = startFrameIdx; frameIdx < endFrameIdx; frameIdx++)
    {
        IJK::Quaternion cori =
            isCoriFilterNeeded(frameData[frameIdx])
            ? coriFiltered[frameIdx]
            : frameData[frameIdx].cori;
        IJK::Quaternion iori = applyIORI ? frameData[frameIdx].iori : identity;

        SphericalMetadataProvider::Status status = SphericalMetadataProvider::Status::FAILED;
        switch (state) {
        case SphericalMetadataProvider::GPMFStabilizationState::kGPMFStabilizationStateAllOff:
            output.push_back(identity);
            status = SphericalMetadataProvider::Status::OK;
            break;
        case SphericalMetadataProvider::GPMFStabilizationState::kGPMFStabilizationStateAntiShake:
            status = antiShake(frameIdx, cori, iori, frameData, stabilizationStateQuat, output);
            break;
        case SphericalMetadataProvider::GPMFStabilizationState::kGPMFStabilizationStateWorldLock:
            status = worldLockOnly(frameIdx, cori, iori, frameData, output);
            break;
        case SphericalMetadataProvider::GPMFStabilizationState::kGPMFStabilizationStateHorizonLevel:
            status = horizonLevelOnly(frameIdx, cori, iori, frameData, stabilizationStateQuat, output);
            break;
        case SphericalMetadataProvider::GPMFStabilizationState::kGPMFStabilizationStateAllOn:
            status = allOn(frameIdx, cori, iori, frameData, output);
            break;
        }

        if (status != SphericalMetadataProvider::Status::OK)
            return status;
    }

    return SphericalMetadataProvider::Status::OK;
}

std::vector<IJK::Quaternion> SphericalMetadataProvider::objc_meld_cori_grav(
    const char* filepath,
    GPMFStabilizationState state)
{
    std::vector<IJK::Quaternion> output;
    meld_cori_grav(filepath, state, 0, std::numeric_limits<int>::max(), output);
    return output;
}

SphericalMetadataProvider::Status SphericalMetadataProvider::meld_cori_grav(
    const char* filepath,
    GPMFStabilizationState state,
    std::vector<IJK::Quaternion>& output)
{
    return meld_cori_grav(filepath, state, 0, std::numeric_limits<int>::max(), output);
}

SphericalMetadataProvider::Status SphericalMetadataProvider::meld_cori_grav(
    const char* filepath,
    GPMFStabilizationState state,
    int32_t startFrameIdx,
    int32_t requestedCount,
    std::vector<IJK::Quaternion>& output)
{
    std::vector<GPMFFrameMetadata::FrameData> frameData;
    GPMFFrameMetadata::Status status = GPMFFrameMetadata::extract_cori_iori_grav(filepath, frameData);
    switch (status) {
    case GPMFFrameMetadata::Status::OK:
        break;
        // default to antishake when we detect invalid grav metadata
    case GPMFFrameMetadata::Status::METADATA_GRAV_INVALID:
        state = GPMFStabilizationState::kGPMFStabilizationStateAntiShake;
        break;
    default:
        return Status::FAILED;
    }

    return meld_cori_iori_grav(frameData, state, 0, requestedCount, output);
}

SphericalMetadataProvider::Status SphericalMetadataProvider::meld_cori_grav(
    const std::vector<GPMFFrameMetadata::FrameData>& frameData,
    GPMFStabilizationState state,
    int32_t startFrameIdx,
    int32_t requestedCount,
    std::vector<IJK::Quaternion>& output)
{
    return meld_cori_iori_grav_internal(frameData, state, false, startFrameIdx, requestedCount, output);
}

SphericalMetadataProvider::Status SphericalMetadataProvider::meld_cori_iori_grav(
    const char *filepath,
    GPMFStabilizationState state,
    std::vector<IJK::Quaternion> &output)
{
    return meld_cori_iori_grav(filepath, state, 0, std::numeric_limits<int>::max(), output);
}

SphericalMetadataProvider::Status SphericalMetadataProvider::meld_cori_iori_grav(
    const char *filepath, GPMFStabilizationState state,
    int32_t startFrameIdx,
    int32_t requestedCount,
    std::vector<IJK::Quaternion> &output)
{
    std::vector<GPMFFrameMetadata::FrameData> frameData;
    GPMFFrameMetadata::Status status = GPMFFrameMetadata::extract_cori_iori_grav(filepath, frameData);
    switch (status) {
        case GPMFFrameMetadata::Status::OK:
            break;
        // default to antishake when we detect invalid grav metadata
        case GPMFFrameMetadata::Status::METADATA_GRAV_INVALID:
            state = GPMFStabilizationState::kGPMFStabilizationStateAntiShake;
            break;
        default:
            return Status::FAILED;
    }

    return meld_cori_iori_grav(frameData, state, 0, requestedCount, output);
}

SphericalMetadataProvider::Status SphericalMetadataProvider::meld_cori_iori_grav(
    const std::vector<GPMFFrameMetadata::FrameData> &frameData,
    GPMFStabilizationState state,
    int32_t startFrameIdx,
    int32_t requestedCount,
    std::vector<IJK::Quaternion> &output)
{
    return meld_cori_iori_grav_internal(frameData, state, true, startFrameIdx, requestedCount, output);
}

SphericalMetadataProvider::Status SphericalMetadataProvider::meld_cori_iori_grav_old(const char *filepath, GPMFStabilizationState state, std::vector<IJK::Quaternion> &output)
{
    std::vector<GPMFFrameMetadata::FrameData> outputVector;

    
    SphericalMetadataProvider::Status statusOut = Status::FAILED;

    GPMFFrameMetadata::Status status = GPMFFrameMetadata::extract_cori_iori_grav(filepath, outputVector);
    switch (status) {
        case GPMFFrameMetadata::Status::OK:
            statusOut = SphericalMetadataProvider::Status::OK;
            break;
        // default to antishake when we detect invalid grav metadata
        case GPMFFrameMetadata::Status::METADATA_GRAV_INVALID:
            state = GPMFStabilizationState::kGPMFStabilizationStateAntiShake;
            statusOut = SphericalMetadataProvider::Status::OK;
            break;
        default:
            break;
    }

    vector<IJK::Quaternion> coriFiltered;
    if(outputVector.size() > 0)
    {
        coriFiltered.resize(outputVector.size());
        if (isCoriFilterNeeded(outputVector[0]))
        {
            coriFilter(outputVector, coriFiltered);
        }
    }
    
    short hlRotStrategy = 0;

    
    for (int frameIdx = 0; frameIdx < outputVector.size(); frameIdx++)
    {
        
        bool removeWorldLock = false;
        bool removeHorizonLevel = false;
        bool stabOff = false;

        switch (state) {
            case GPMFStabilizationState::kGPMFStabilizationStateAllOff:
                stabOff = true;
                break;
            case GPMFStabilizationState::kGPMFStabilizationStateAntiShake:
                removeWorldLock = true;
                removeHorizonLevel = true;
                break;
            case GPMFStabilizationState::kGPMFStabilizationStateWorldLock:
                removeHorizonLevel = true;
                break;
            case GPMFStabilizationState::kGPMFStabilizationStateHorizonLevel:
                removeWorldLock = true;
                break;
            case GPMFStabilizationState::kGPMFStabilizationStateAllOn:
                
                break;
        }
        
        if(stabOff)
        {
            IJK::Quaternion o = IJK::Quaternion(0.0, 0.0, 0.0, 1.0);
            
            output.push_back(o);
        }
        else
        {

            IJK::Vector grav = outputVector[frameIdx].grav;
            
            IJK::Quaternion cori =
                isCoriFilterNeeded(outputVector[frameIdx])
                    ? coriFiltered[frameIdx]
                    : outputVector[frameIdx].cori;
            
            if (removeWorldLock && frameIdx > 0)
            {
                IJK::Quaternion diff = isCoriFilterNeeded(outputVector[frameIdx]) ? coriFiltered[0] : outputVector[0].cori * inverse(cori);
                
                //Isolate yaw portion
                diff = normalize(IJK::Quaternion(0.0, diff.y(), 0.0, diff.w()));
                
                cori = cori * diff;
            }
            
            
            IJK::Quaternion iori = outputVector[frameIdx].iori;
            
            IJK::Quaternion camOrient =  iori * cori;
            camOrient = camOrient.normalized();

            
            if (removeHorizonLevel)
            {
                output.push_back(camOrient);

                continue;
            }
            
            IJK::Vector axe;
            
            IJK::Vector gravInit = cori.inverted().rotatedVector(grav).normalized();
            
            
            IJK::Quaternion gravRot;
            
            if(hlRotStrategy == 0)
            {
                // at init we choose a rotation strategy, and we must keep this strategy until the end of the video
                // If we don't do this, there can be discontinuities during the video
                if (gravInit.y() > 0.f)
                {
                    hlRotStrategy = 1;
                }
                else
                {
                    hlRotStrategy = -1;
                }
            }
            
            if (hlRotStrategy == 1)
            {
                axe = IJK::crossProduct(IJK::Vector(0,1,0), gravInit);
                float angle =  asinf_nan_avoid(axe.length());
                
                if(gravInit.y() < 0.f)
                {
                    angle = (float)(M_PI - angle);
                }
                gravRot = IJK::Quaternion(axe.normalized(), angle);
                
            } else {
                // upside down
                axe = IJK::crossProduct(IJK::Vector(0,-1,0), gravInit);
                float angle =  asinf_nan_avoid(axe.length());
                
                if(gravInit.y() > 0.f)
                {
                    angle = (float)(M_PI - angle);
                }
                
                gravRot = IJK::Quaternion(axe.normalized(), angle) * IJK::Quaternion(0,0,1,0);
            }
            
            IJK::Quaternion o = camOrient * gravRot.normalized();
            
            output.push_back(o);
        }
        

      
        statusOut = Status::OK;
        
    }

    
   

    return statusOut;
}




