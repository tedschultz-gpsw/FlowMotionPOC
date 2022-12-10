// 
//  OpticalFlowProcessor.swift
//
//  Copyright (c) 2022 GoPro, Inc. All rights reserved.
//  THIS SOURCE CODE IS THE PROPRIETARY INTELLECTUAL PROPERTY AND CONFIDENTIAL
//  INFORMATION OF GOPRO, INC. AND IS PROTECTED UNDER U.S. AND INTERNATIONAL
//  LAW. ANY USE OF THIS SOURCE CODE WITHOUT THE PRIOR WRITTEN AUTHORIZATION OF
//  GOPRO IS STRICTLY PROHIBITED.
//

import Foundation
import AVFoundation
import Vision
import CoreImage

final class OpticalFlowProcessor {
    
    static let shared = OpticalFlowProcessor()
    
    private var inputAsset: AVAsset?
    private var outputAsset: AVAsset?
    private var videoURL: URL?
    
    func process(videoURL: URL) async throws {
        self.videoURL = videoURL
        
        let asset = AVAsset(url: videoURL)
        self.inputAsset = asset
        let reader = VideoAssetReader(videoAsset: asset)
        
        let isReading = await reader.start()
        
        guard isReading else {
            print("Failed to start reader!")
            return
        }
        
        guard let initialFrame = reader.nextFrame() else {
            print("Failed to read initial frame!")
            return
        }
        
        print("Processing...")
            
        // Process only first two frames
        var previousFrame = initialFrame
        let nextFrame = reader.nextFrame()
        // TODO: pass the appropriate orientation
        await process(currentFrame: nextFrame!, currentFrameOrientation: .up, previousFrame: previousFrame)
            
        // Iteratate all frames
//        var previousFrame = initialFrame
//        while let nextFrame = reader.nextFrame() {
                // TODO: pass the appropriate orientation
//              await process(currentFrame: nextFrame, currentFrameOrientation: .up, previousFrame: previousFrame)
//              previousFrame = nextFrame
//        }
            
        print("Done")
        
    }
    
    private func process(currentFrame: CVPixelBuffer, currentFrameOrientation: CGImagePropertyOrientation, previousFrame: CVPixelBuffer) async {
        let visionRequest = VNGenerateOpticalFlowRequest(targetedCVPixelBuffer: currentFrame, orientation: currentFrameOrientation, options: [:])
        
        let requestHandler = VNSequenceRequestHandler()
        
        do {
            try requestHandler.perform([visionRequest], on: previousFrame)
            
            if let pixelBufferObservation = visionRequest.results?.first as? VNPixelBufferObservation {
                let image = CIImage(cvPixelBuffer: pixelBufferObservation.pixelBuffer)
                
                let path = image.saveJPG("test3.jpg")
                print("path = \(path!)")
            }
        } catch {
            print("Vision Request failed: \(error.localizedDescription)")
        }
    }
}

