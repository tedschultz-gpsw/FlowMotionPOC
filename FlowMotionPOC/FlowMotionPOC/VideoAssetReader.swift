// 
//  VideoAssetReader.swift
//
//  Copyright (c) 2022 GoPro, Inc. All rights reserved.
//  THIS SOURCE CODE IS THE PROPRIETARY INTELLECTUAL PROPERTY AND CONFIDENTIAL
//  INFORMATION OF GOPRO, INC. AND IS PROTECTED UNDER U.S. AND INTERNATIONAL
//  LAW. ANY USE OF THIS SOURCE CODE WITHOUT THE PRIOR WRITTEN AUTHORIZATION OF
//  GOPRO IS STRICTLY PROHIBITED.
//

import Foundation
import AVFoundation

final class VideoAssetReader {
    
    private var asset: AVAsset
    private var track: AVAssetTrack?
    private var assetReader: AVAssetReader?
    private var assetReaderOutput: AVAssetReaderOutput?
    
    init(videoAsset: AVAsset) {
        self.asset = videoAsset
    }
    
    func start() async -> Bool {
        
        do {
            try await loadInitialTrack()
        } catch let error {
            print(error.localizedDescription)
            return false
        }
        
        guard let track else {
            print("track is nil!")
            return false
        }
        
        do {
            assetReader = try AVAssetReader(asset: asset)
        } catch let error {
            print(error.localizedDescription)
            return false
        }
        
        //kCVPixelFormatType_32ARGB
        //kCVPixelFormatType_420YpCbCr8BiPlanarFullRange
        assetReaderOutput = AVAssetReaderTrackOutput(track: track,
                                                     outputSettings: [kCVPixelBufferPixelFormatTypeKey as String: kCVPixelFormatType_4444AYpCbCr16])
        guard assetReaderOutput != nil else {
            print("assetReaderOutput is nil!")
            return false
        }
        
        assetReaderOutput?.alwaysCopiesSampleData = true
        
        guard assetReader!.canAdd(assetReaderOutput!) else {
            print("cannot add assetReaderOutput!")
            return false
        }
        
        assetReader!.add(assetReaderOutput!)
        
        return assetReader!.startReading()
    }
    
    private func loadInitialTrack() async throws {
        return try await withCheckedThrowingContinuation { continuation in
            
            self.asset.loadTracks(withMediaType: .video) { tracks, error in
                
                if let error {
                    print("Error loading intial track: \(error.localizedDescription)")
                    continuation.resume(throwing: error)
                    return
                }
                
                self.track = tracks?.first
                
                print("loaded track")
                
                continuation.resume()
            }
        }
    }
    
    func nextFrame() -> CVPixelBuffer? {
        guard let assetReaderOutput else { return nil }
        guard let sampleBuffer = assetReaderOutput.copyNextSampleBuffer() else { return nil }
        return CMSampleBufferGetImageBuffer(sampleBuffer)
    }
    
}
