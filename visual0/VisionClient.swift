//
//  VisionClient.swift
//  Detected-in-Still-Image
//
//  Created by satoutakeshi on 2021/06/20.
//
import Foundation
import Vision

enum VisionRequestTypes {
    case unknown
    case rectBoundingBoxes(rectBox: [CGRect])
    case rect(drawPoints: [(closed: Bool, points: [CGPoint])], info: [[String: String]])

    struct Set: OptionSet {
        typealias Element = VisionRequestTypes.Set
        let rawValue: Int8
        init(rawValue: Int8) {
            self.rawValue = rawValue
        }
        static let rect             = Set(rawValue: 1 << 5)
        static let all: Set         = [.rect]
    }
}

final class VisionClient: ObservableObject {

    enum VisionError: Error {
        case typeNotSet
        case visionError(error: Error)
    }

    private var requestTypes: VisionRequestTypes.Set = []
    private var imageViewFrame: CGRect = .zero

    @Published var result: VisionRequestTypes = .unknown
    @Published var error: VisionError?

    func configure(type: VisionRequestTypes.Set, imageViewFrame: CGRect) {
        self.requestTypes = type
        self.imageViewFrame = imageViewFrame
    }

    func performVisionRequest(image: CGImage,
                              orientation: CGImagePropertyOrientation) {
        guard !requestTypes.isEmpty else {
            error = VisionError.typeNotSet
            return
        }
        let imageRequestHandler = VNImageRequestHandler(cgImage: image,
                                                        orientation: orientation,
                                                        options: [:])

        let requests = makeImageRequests()
        do {
            try imageRequestHandler.perform(requests)
        } catch {
            self.error = VisionError.visionError(error: error)
        }
    }

    private func makeImageRequests() -> [VNRequest] {
        var requests: [VNRequest] = []

        if requestTypes.contains(.rect) {
            requests.append(rectDetectionRequest)
        }

        return requests
    }

    lazy var rectDetectionRequest: VNDetectRectanglesRequest = {
        let rectDetectRequest = VNDetectRectanglesRequest { [weak self] request, error in
            guard let self = self else { return }
            if let error = error {
                print(error.localizedDescription)
                self.error = VisionError.visionError(error: error)
                return
            }

            guard let results = request.results as? [VNRectangleObservation] else {
                return
            }

            let rectBoxes = results.map { observation -> CGRect in
                let rectBox = self.boundingBox(forRegionOfInterest: observation.boundingBox, withinImageBounds: self.imageViewFrame)
                return rectBox
            }

            let points = self.makeRectanglePoints(observations: results, onImageWithBounds: self.imageViewFrame)
            let info = ["detected count": "\(results.count)"]
            self.result = .rect(drawPoints: points, info: [info])
            self.result = .rectBoundingBoxes(rectBox: rectBoxes)
        }
        rectDetectRequest.maximumObservations = 0
        rectDetectRequest.minimumSize = 0.4
        return rectDetectRequest
    }()

    // MARK: - Private
    private func boundingBox(forRegionOfInterest: CGRect,
                     withinImageBounds bounds: CGRect) -> CGRect {

        let imageWidth = bounds.width
        let imageHeight = bounds.height

        // Begin with input rect.
        var rect = forRegionOfInterest

        // Reposition origin.
        rect.origin.x = rect.origin.x * imageWidth + bounds.origin.x
        rect.origin.y = rect.origin.y * imageHeight + bounds.origin.y

        // Rescale normalized coordinates.
        rect.size.width *= imageWidth
        rect.size.height *= imageHeight

        return rect
    }


    private func makeRectanglePoints(observations : [VNRectangleObservation],
                                     onImageWithBounds bounds: CGRect) -> [(closed: Bool, points: [CGPoint])] {
        var rectPoints: [(closed: Bool, points: [CGPoint])] = []

        for obs in observations {
            let topLeftX = obs.topLeft.x * bounds.width
            let topLeftY = obs.topLeft.y * bounds.height
            let topLeft = CGPoint(x: topLeftX, y: topLeftY)

            let topRightX = obs.topRight.x * bounds.width
            let topRightY = obs.topRight.y * bounds.height
            let topRight = CGPoint(x: topRightX, y: topRightY)

            let bottomLeftX = obs.bottomLeft.x * bounds.width
            let bottomLeftY = obs.bottomLeft.y * bounds.height
            let bottomLeft = CGPoint(x: bottomLeftX, y: bottomLeftY)

            let bottomRightX = obs.bottomRight.x * bounds.width
            let bottomRightY = obs.bottomRight.y * bounds.height
            let bottomRight = CGPoint(x: bottomRightX, y: bottomRightY)

            rectPoints.append((closed: true, points: [topLeft, topRight, bottomRight, bottomLeft]))

        }
        return rectPoints
    }
}
