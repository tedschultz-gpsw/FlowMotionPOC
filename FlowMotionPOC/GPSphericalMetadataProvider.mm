// 
//  GPSphericalMetadataProvider.m
//
//  Copyright (c) 2022 GoPro, Inc. All rights reserved.
//  THIS SOURCE CODE IS THE PROPRIETARY INTELLECTUAL PROPERTY AND CONFIDENTIAL
//  INFORMATION OF GOPRO, INC. AND IS PROTECTED UNDER U.S. AND INTERNATIONAL
//  LAW. ANY USE OF THIS SOURCE CODE WITHOUT THE PRIOR WRITTEN AUTHORIZATION OF
//  GOPRO IS STRICTLY PROHIBITED.
//

#import <Foundation/Foundation.h>

#import "GPSphericalMetadataProvider.h"
#import "SphericalMetadataProvider.hpp"

@implementation GPSphericalMetadataProvider

enum class GPMFStabilizationState {
    kGPMFStabilizationStateAllOff,             //World Lock OFF,   Horizon Leveling OFF,   Stabilization OFF
    kGPMFStabilizationStateAntiShake,          //World Lock OFF,   Horizon Leveling OFF,   Stabilization ON
    kGPMFStabilizationStateWorldLock,          //World Lock ON,    Horizon Leveling OFF,   Stabilization ON
    kGPMFStabilizationStateHorizonLevel,       //World Lock OFF,   Horizon Leveling ON,    Stabilization ON
    kGPMFStabilizationStateAllOn,              //World Lock ON,    Horizon Leveling ON,    Stabilization ON
};

-(std::vector<IJK::Quaternion>) objc_meld_cori_grav: (NSString*)filepath {
    const char* cFilepath = filepath.UTF8String;
    SphericalMetadataProvider sphericalMetadataProvider;
    SphericalMetadataProvider::GPMFStabilizationState state = SphericalMetadataProvider::GPMFStabilizationState::kGPMFStabilizationStateAllOn;
    std::vector<IJK::Quaternion> quaternionVectorArray = sphericalMetadataProvider.objc_meld_cori_grav(cFilepath, state);
    return sphericalMetadataProvider.objc_meld_cori_grav(cFilepath, state);
}


-(IJK::Quaternion) objc_meld_cori_grav: (NSString*)filepath : (int) frame {
    const char* cFilepath = filepath.UTF8String;
    SphericalMetadataProvider sphericalMetadataProvider;
    SphericalMetadataProvider::GPMFStabilizationState state = SphericalMetadataProvider::GPMFStabilizationState::kGPMFStabilizationStateAllOn;
    std::vector<IJK::Quaternion> quaternionVectorArray = sphericalMetadataProvider.objc_meld_cori_grav(cFilepath, state);
    return quaternionVectorArray[frame];
}


@end
