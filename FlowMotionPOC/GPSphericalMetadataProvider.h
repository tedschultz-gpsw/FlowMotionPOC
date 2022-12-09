// 
//  GPSphericalMetadataProvider.h
//
//  Copyright (c) 2022 GoPro, Inc. All rights reserved.
//  THIS SOURCE CODE IS THE PROPRIETARY INTELLECTUAL PROPERTY AND CONFIDENTIAL
//  INFORMATION OF GOPRO, INC. AND IS PROTECTED UNDER U.S. AND INTERNATIONAL
//  LAW. ANY USE OF THIS SOURCE CODE WITHOUT THE PRIOR WRITTEN AUTHORIZATION OF
//  GOPRO IS STRICTLY PROHIBITED.
//

#import <Foundation/Foundation.h>

#import "SphericalMetadataProvider.hpp"

@interface GPSphericalMetadataProvider : NSObject

enum class GPMFStabilizationState;

-(std::vector<IJK::Quaternion>) objc_meld_cori_grav: (NSString*)filepath;

-(IJK::Quaternion) objc_meld_cori_grav: (NSString*)filepath : (int) frame;

@end
