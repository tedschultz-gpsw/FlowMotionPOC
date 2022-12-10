//
//  VideoWriter.swift
//  FlowMotionPOC
//
//  Created by Robert Harrison on 12/9/22.
//

import Foundation
import AVFoundation
import CoreImage

final class VideoWriter {
    
    private let framesPerSecond = 30
    private var frameCount = 0
    
    private var assetWriter: AVAssetWriter?
    private var assetWriterInput: AVAssetWriterInput?
    private var assetWriterAdaptor: AVAssetWriterInputPixelBufferAdaptor?
    
    var documentsDirectory: URL {
        try! FileManager.default.url(for: .documentDirectory, in: .userDomainMask, appropriateFor: nil, create: true)
    }
    
    var outputDirectory: URL {
        documentsDirectory.appendingPathComponent("out")
    }
    
    var outputURL: URL {
        return outputDirectory.appendingPathComponent("output.mov")
    }
    
    func start() throws {
        
        reset()
        
        assetWriter = try AVAssetWriter(outputURL: outputURL, fileType: .mov)
        let videoSettings = AVOutputSettingsAssistant(preset: .preset1920x1080)?.videoSettings
        assetWriterInput = AVAssetWriterInput(mediaType: .video, outputSettings: videoSettings)
        assetWriterAdaptor = AVAssetWriterInputPixelBufferAdaptor(assetWriterInput: assetWriterInput!, sourcePixelBufferAttributes: nil)
        
        if assetWriter!.canAdd(assetWriterInput!) {
            assetWriter!.add(assetWriterInput!)
        } else {
            print("CANNOT ADD ASSET WRITER INPUT")
        }
        
        assetWriter!.startWriting()
        assetWriter!.startSession(atSourceTime: .zero)
        
        
    }
    
    func write(pixelBuffer: CVPixelBuffer) async {
        
        guard let assetWriterInput, let assetWriterAdaptor, let assetWriter else { return }
        
        // Determine how many frames we need to generate
        
        // Duration is the number of seconds for the final video
        let duration = 1
        
        let totalFrames = duration * framesPerSecond
        var frameCount = 0
        while frameCount < totalFrames {
            if assetWriterInput.isReadyForMoreMediaData {
                let frameTime = CMTimeMake(value: Int64(frameCount), timescale: Int32(framesPerSecond))
                // Append the the pixelBuffer at the specified time.
                
                
                
                let success = assetWriterAdaptor.append(pixelBuffer, withPresentationTime: frameTime)
                if !success {
                    if assetWriter.status == .failed {
                        print(assetWriter.error!)
                    }
                    
                    
                }
                
                frameCount += 1
            }
        }
        
    }
    
    func finishWriting() async {
        guard let assetWriterInput, let assetWriter else { return }
        assetWriterInput.markAsFinished()
        await assetWriter.finishWriting()
    }
    
    private func reset() {
        frameCount = 0
        
        if FileManager.default.fileExists(atPath: outputDirectory.path) {
            try? FileManager.default.removeItem(at: outputDirectory)
        }
        
        try? FileManager.default.createDirectory(atPath: outputDirectory.path, withIntermediateDirectories: true, attributes: nil)
    }
    
}
