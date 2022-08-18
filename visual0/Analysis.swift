//
//  Analysis.swift
//  visual0
//
//  Created by Urs Langenegger on 17.08.22.
//

import Foundation
import SwiftUI
import AppKit
import Vision
import Combine
import CoreImage
import CoreImage.CIFilterBuiltins

// -----------------------------------------------------------------------
final class Analysis: ObservableObject {
    @EnvironmentObject var appState: AppState
    @Published var ciimage: CIImage = CIImage.empty()
    @Published var detectedFrame: [CGRect] = []
    @Published var detectedPoints: [(closed: Bool, points: [CGPoint])] = []
    @Published var detectedInfo: [[String: String]] = []
    
    @Published var fStatus: Int = 0
    
    private let visionClient = VisionClient()
    
    private var detectType: VisionRequestTypes.Set = .rect
//    private var cancellables: Set<AnyCancellable> = []
//    private var errorCancellables: Set<AnyCancellable> = []
//    private var imageViewFramePublisher = PassthroughSubject<CGRect, Never>()
//    private var originImagePublisher = PassthroughSubject<(CGImage, VisionRequestTypes.Set), Never>()
//
    init() {
        //        imageViewFramePublisher
//            .removeDuplicates()
//            .prefix(2)
//            .last()
//            .combineLatest(originImagePublisher)
//            .sink { (imageRect, originImageArg) in
//                let (cgImage, detectType) = originImageArg
//                let fullImageWidth = CGFloat(cgImage.width)
//                let fullImageHeight = CGFloat(cgImage.height)
//                let targetWidh = imageRect.width
//                let ratio = fullImageWidth / targetWidh
//
//                let imageFrame = CGRect(x: 0, y: 0, width: imageRect.width, height: fullImageHeight / ratio)
//                self.visionClient.configure(type: detectType, imageViewFrame: imageFrame)
//
//                print(cgImage)
//
//                // clear info
//                self.clearAllInfo()
//                self.visionClient.performVisionRequest(image: cgImage, orientation: .up)
//            }
//            .store(in: &cancellables)
//
    }
    
    func setImage(img: NSImage) {
        ciimage = NSImage.ciImage(img)!
    }

    func filterImage()  {
        let context = CIContext()
        let currentFilter = CIFilter.sepiaTone()
        currentFilter.inputImage = ciimage
        currentFilter.intensity = 1

        // https://www.hackingwithswift.com/books/ios-swiftui/integrating-core-image-with-swiftui
        // get a CIImage from our filter or exit if that fails
        guard let outputImage = currentFilter.outputImage else { return }

        // attempt to get a CGImage from our CIImage
        if let cgimg = context.createCGImage(outputImage, from: outputImage.extent) {
            // convert that to a UIImage
            // = CIImage(cgImage: cgimg)
        }

        return
    }

    
    func incStatus() {
        print("Analysis:incStatus() called")
        fStatus += 1
        // variable "image" is UIImage object
        //  -- UIImage -> CGImage
        //  let cgImage: CGImage = image.cgImage!
        // after some process...
        //  let processedImage: UIImage = UIImage(cgImage: cgImage)
//        let myNsImage = NSImage(cgImage: image, size: .zero)
        
    }
    
    func getStatus() -> Int {
        return fStatus
    }
    
    private func clearAllInfo() {
        detectedFrame.removeAll()
        detectedPoints.removeAll()
        detectedInfo.removeAll()
    }
}
