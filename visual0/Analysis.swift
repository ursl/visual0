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

final class Analysis: ObservableObject {
    
    @Published var fImage: Image = Image("bla")
    @Published var detectedFrame: [CGRect] = []
    @Published var detectedPoints: [(closed: Bool, points: [CGPoint])] = []
    @Published var detectedInfo: [[String: String]] = []

    @Published var fStatus: Int
    
    private let visionClient = VisionClient()

    private var detectType: VisionRequestTypes.Set = .rect
    private var cancellables: Set<AnyCancellable> = []
    private var errorCancellables: Set<AnyCancellable> = []
    private var imageViewFramePublisher = PassthroughSubject<CGRect, Never>()
    private var originImagePublisher = PassthroughSubject<(CGImage, VisionRequestTypes.Set), Never>()

    init() {
        fStatus = 0
        
        imageViewFramePublisher
            .removeDuplicates()
            .prefix(2)
            .last()
            .combineLatest(originImagePublisher)
            .sink { (imageRect, originImageArg) in
                let (cgImage, detectType) = originImageArg
                let fullImageWidth = CGFloat(cgImage.width)
                let fullImageHeight = CGFloat(cgImage.height)
                let targetWidh = imageRect.width
                let ratio = fullImageWidth / targetWidh

                let imageFrame = CGRect(x: 0, y: 0, width: imageRect.width, height: fullImageHeight / ratio)
                self.visionClient.configure(type: detectType, imageViewFrame: imageFrame)

                print(cgImage)

                // clear info
                self.clearAllInfo()
                self.visionClient.performVisionRequest(image: cgImage, orientation: .up)
            }
            .store(in: &cancellables)

    }
    
    func input(imageFrame: CGRect) {
       imageViewFramePublisher.send(imageFrame)
    }
    
    func incStatus() {
        print("Analysis:incStatus() called")
        fStatus += 1
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
