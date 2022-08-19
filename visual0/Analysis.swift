// -----------------------------------------------------------------------
//  Analysis.swift
// -----------------------------------------------------------------------


import Foundation
import SwiftUI
import AppKit
import CoreImage

// -----------------------------------------------------------------------
final class Analysis: ObservableObject {
    
    @Published var fImgIdx : Int = 0
    @Published var fImage : NSImage
    
    var imgArray = [
        NSImage(named: NSImage.Name("glass-20220401-5"))
        , NSImage(named: NSImage.Name("glass-20220401-6"))
    ]
        
    // -----------------------------------------------------------------------
    init() {
        fImage = imgArray[0]!
    }
    
    // -----------------------------------------------------------------------
    func getImage() -> NSImage {
        return fImage
    }
    
    // -----------------------------------------------------------------------
    func getIdx() -> Int {
        return fImgIdx
    }
     
    // -----------------------------------------------------------------------
    func changeImage() {
        if fImgIdx == imgArray.count-1 {
            fImgIdx = 0
        } else {
            fImgIdx += 1
        }
        fImage = imgArray[fImgIdx]!
    }
    

    // -----------------------------------------------------------------------
    // https://developer.apple.com/documentation/coreimage/processing_an_image_using_built-in_filters
    func runAna1() {
        let context = CIContext()
        let originalCIImage = NSImage.ciImage(fImage)!
        
        let sepiaCIImage = sepiaFilter(originalCIImage, intensity:0.9)!

        imgArray.append(NSImage.fromCIImage(sepiaCIImage))
        fImage = imgArray[imgArray.count-1]!
    }
    
    func sepiaFilter(_ input: CIImage, intensity: Double) -> CIImage?  {
        let sepiaFilter = CIFilter(name:"CISepiaTone")
        sepiaFilter?.setValue(input, forKey: kCIInputImageKey)
        sepiaFilter?.setValue(intensity, forKey: kCIInputIntensityKey)
        return sepiaFilter?.outputImage
    }
}
