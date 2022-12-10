// 
//  CIImage+JPG.swift
//
//  Copyright (c) 2022 GoPro, Inc. All rights reserved.
//  THIS SOURCE CODE IS THE PROPRIETARY INTELLECTUAL PROPERTY AND CONFIDENTIAL
//  INFORMATION OF GOPRO, INC. AND IS PROTECTED UNDER U.S. AND INTERNATIONAL
//  LAW. ANY USE OF THIS SOURCE CODE WITHOUT THE PRIOR WRITTEN AUTHORIZATION OF
//  GOPRO IS STRICTLY PROHIBITED.
//

import CoreImage

extension CIImage {
    
    /// Saves the jpg to the app's Documents directory.
    func saveJPG(_ name: String) -> String? {
        
        guard let documentDirectory = try? FileManager.default.url(for: .documentDirectory, in: .userDomainMask, appropriateFor: nil, create: true) else {
            return nil
        }
        
        let destinationURL = documentDirectory.appendingPathComponent(name)
        
        guard let colorSpace = CGColorSpace(name: CGColorSpace.sRGB) else { return nil }
        
        do {
            
            let context = CIContext()
            
            try context.writeJPEGRepresentation(of: self,
                                                to: destinationURL,
                                                colorSpace: colorSpace,
                                                options: [kCGImageDestinationLossyCompressionQuality as CIImageRepresentationOption : 1.0])
            
            return destinationURL.path
            
        } catch {
            return nil
        }
    }
}
