//
//  DetectorViewModel.swift
//  Detected-in-Still-Image
//
//  Created by satoutakeshi on 2021/06/17.
//

import Foundation
import SwiftUI
import AppKit
import Vision
import Combine
import CoreImage

final class DetectorViewModel: ObservableObject {

    @Published var image: Image = Image("bla")
    @Published var detectedFrame: [CGRect] = []
    @Published var detectedPoints: [(closed: Bool, points: [CGPoint])] = []
    @Published var detectedInfo: [[String: String]] = []
    private var cancellables: Set<AnyCancellable> = []
    private var errorCancellables: Set<AnyCancellable> = []
    private let visionClient = VisionClient()
    private var imageViewFramePublisher = PassthroughSubject<CGRect, Never>()
    private var originImagePublisher = PassthroughSubject<(CGImage, VisionRequestTypes.Set), Never>()

    init() {
        visionClient.$result
            .receive(on: RunLoop.main)
            .sink { type in
                switch type {
                    case .faceLandmarks(let drawPoints, let info):
                        self.detectedPoints = drawPoints
                        self.detectedInfo = info
                    case .faceRect(let rectBox, let info):
                        self.detectedFrame = rectBox
                        self.detectedInfo = info
                    case .word(let rectBoxes, let info):
                        self.detectedFrame += rectBoxes
                        self.detectedInfo = info
                    case .character(let rectBox, let info):
                        self.detectedFrame += rectBox
                        self.detectedInfo = info
                    case .textRecognize(let info):
                        self.detectedInfo = info
                    case .barcode(let rectBoxes, let info):
                        self.detectedFrame = rectBoxes
                        self.detectedInfo = info
                    case .rect(let drawPoints, let info):
                        self.detectedPoints = drawPoints
                        self.detectedInfo = info
                    case .rectBoundingBoxes(let rectBoxes):
                        self.detectedFrame = rectBoxes
                    default:
                        break
                }
            }
            .store(in: &cancellables)

        visionClient.$error
            .receive(on: RunLoop.main)
            .sink { error in
                print(error?.localizedDescription ?? "")
            }
            .store(in: &errorCancellables)

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
//                let cgOrientation = CGImagePropertyOrientation(self.image.imageOrientation)

                // clear info
                self.clearAllInfo()
                self.visionClient.performVisionRequest(image: cgImage, orientation: .up)
            }
            .store(in: &cancellables)
    }

    func onAppear(image: Image, detectType: VisionRequestTypes.Set) {
        self.image = image
       // guard let resizedImage = resize(image: image) else { return }
       // print(resizedImage.description)
        // Transform image to fit screen.
        
        let image = NSImage(named:"image")
        if let image = image {
            let cgimg = image.cgImage(forProposedRect: nil, context: nil, hints: nil)
            originImagePublisher.send((cgimg!, detectType))
        } else  {
            print("Trying to show an image not backed by CGImage!")
            return
        }
    }

    func input(imageFrame: CGRect) {

       imageViewFramePublisher.send(imageFrame)
    }

    private func clearAllInfo() {
        detectedFrame.removeAll()
        detectedPoints.removeAll()
        detectedInfo.removeAll()
    }
}

//// Convert UIImageOrientation to CGImageOrientation for use in Vision analysis.
//extension CGImagePropertyOrientation {
//    init(_ uiImageOrientation: UIImage.Orientation) {
//        switch uiImageOrientation {
//            case .up: self = .up
//            case .down: self = .down
//            case .left: self = .left
//            case .right: self = .right
//            case .upMirrored: self = .upMirrored
//            case .downMirrored: self = .downMirrored
//            case .leftMirrored: self = .leftMirrored
//            case .rightMirrored: self = .rightMirrored
//            @unknown default:
//                fatalError()
//        }
//    }
//}
