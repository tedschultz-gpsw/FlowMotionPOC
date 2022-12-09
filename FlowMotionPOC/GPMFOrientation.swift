// 
//  GPMFOrientation.swift
//
//  Copyright (c) 2022 GoPro, Inc. All rights reserved.
//  THIS SOURCE CODE IS THE PROPRIETARY INTELLECTUAL PROPERTY AND CONFIDENTIAL
//  INFORMATION OF GOPRO, INC. AND IS PROTECTED UNDER U.S. AND INTERNATIONAL
//  LAW. ANY USE OF THIS SOURCE CODE WITHOUT THE PRIOR WRITTEN AUTHORIZATION OF
//  GOPRO IS STRICTLY PROHIBITED.
//

import Foundation
import Cocoa
import AVFoundation


// #include "GPMFAssetSourceVideo.h"
// #include "sphericalHorizonLevelingProvider.hpp"

class GPMFOrientation {
    
    let metadataProvider = GPSphericalMetadataProvider()
    let filePath: String
    let videoAsset: AVAsset
    let videoTrack: AVAssetTrack
    let framerate: Float
    var totalFrames: Int = 0
    
    init() {
        let panel = NSOpenPanel()
        panel.runModal()
        filePath = panel.url!.absoluteString
        videoAsset = AVAsset(url: URL(string: filePath)!)
        videoTrack = videoAsset.tracks(withMediaType: .video).first!
        framerate = videoTrack.nominalFrameRate
        totalFrames = Int((Float((videoTrack.asset?.duration.seconds)!)*framerate).rounded())
    }
    
    func getOrientationArray() -> Float? {
        
        let quaternion = metadataProvider.objc_meld_cori_grav(filePath)
        print(quaternion)

        guard totalFrames > 0 else { return nil }
                
        for frame in 1...totalFrames {
            // uses frame/framerate to get time in seconds as long as frame rate is fps
            let timeOfFrame = CMTimeMake(value: Int64(frame), timescale: Int32(framerate))
            let offset = CMTimeMake(value: 1, timescale: Int32(framerate))
            let quaternionFrame = metadataProvider.objc_meld_cori_grav(filePath)
            //print(quaternionFrame)
        }
        
        return 1
    }
    
}
