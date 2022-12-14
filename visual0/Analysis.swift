// -----------------------------------------------------------------------
//  Analysis.swift
// -----------------------------------------------------------------------


import Foundation
import SwiftUI
import AppKit
import CoreImage
import Vision

// -----------------------------------------------------------------------
final class Analysis: ObservableObject {
    
    @Published var fImgIdx : Int = 0
    @Published var fImage : NSImage
    
    var imgArray = [
        NSImage(named: NSImage.Name("glass-20220401-5"))
        , NSImage(named: NSImage.Name("glass-20220401-6"))
    ]
    
    let fContext = CIContext()
    var fLocalImageView: NSImageView!
    var fLocalCIImage : CIImage
    var frectangles: [VNRectangleObservation]?
    
    // -----------------------------------------------------------------------
    init() {
        fImage = imgArray[0]!
        fLocalImageView = NSImageView(frame: NSRect())
        fLocalImageView.wantsLayer = true
        fLocalCIImage = NSImage.ciImage(imgArray[0]!)!
        
        
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
    func runAna1() {
        fLocalCIImage = NSImage.ciImage(fImage)!
        
        let sepiaCIImage = sepiaFilter(fLocalCIImage, intensity:0.9)!
        
        
        imgArray.append(NSImage.fromCIImage(sepiaCIImage))
        fImgIdx = imgArray.count-1
        fImage = imgArray[fImgIdx]!
    }
    
    // -----------------------------------------------------------------------
    func runAna2() {
        print("runAna2")
        fLocalCIImage = NSImage.ciImage(fImage)!
        let requestHandler = VNImageRequestHandler(ciImage: fLocalCIImage)
        let request = VNDetectRectanglesRequest { request, error in
            self.completedVisionRequest(request, error)
        }
        
        // -- perform additional request configuration
        request.usesCPUOnly = false //allow Vision to utilize the GPU
        request.maximumObservations = 0
        request.maximumAspectRatio = 1.0
        request.minimumAspectRatio = 0.0
        request.minimumSize = 0.1
        request.minimumConfidence = 0.5
        
        DispatchQueue.global().async {
            do {
                print("try requestHandler")
                try requestHandler.perform([request])
            } catch {
                print("Error: Rectangle detection failed - vision request failed.")
            }
        }
        
    }
    
    
    // -----------------------------------------------------------------------
    func sepiaFilter(_ input: CIImage, intensity: Double) -> CIImage?  {
        let sepiaFilter = CIFilter(name:"CISepiaTone")
        sepiaFilter?.setValue(input, forKey: kCIInputImageKey)
        sepiaFilter?.setValue(intensity, forKey: kCIInputIntensityKey)
        return sepiaFilter?.outputImage
    }
    
    // -----------------------------------------------------------------------
    func completedVisionRequest(_ request: VNRequest?, _ error: Error?) {
        print("completedVisionRequest")
        // -- Only proceed if a rectangular image was detected.
        guard let lrectangles = request?.results as? [VNRectangleObservation] else {
            guard let error = error else {
                print("hm?")
                return
            }
            print("Error: Rectangle detection failed: \(error.localizedDescription)")
            return
        }
        
        frectangles = lrectangles
        
        // -- this is required to run in main thread
        DispatchQueue.main.async { [weak self] in
            guard let self = self else { return }
            self.addRectangleOutlinesToInputImage()
        }

        // -----------------------------------------------------------------------
        // -- testBox
        print("call testBox")
        DispatchQueue.main.async { [weak self] in
            guard let self = self else { return }
            self.testBox()
        }

        
        
        // -- do stuff with your rectangles
        print("found \(frectangles!.count) rectangles")
    }
    
    // -----------------------------------------------------------------------
    func addRectangleOutlinesToInputImage() {
        print("running addRectangleOutlinesToInputImage")
        
        fLocalCIImage = NSImage.ciImage(fImage)!
        print("running testBox, ciimage = \(fLocalCIImage.extent)")
        // https://stackoverflow.com/questions/13272512/add-uiimage-in-calayer
        let nsi = fImage
        fLocalImageView = NSImageView(frame: NSRect(origin: .zero, size: nsi.size))
        fLocalImageView.wantsLayer = true
        fLocalImageView.canDrawSubviewsIntoLayer = true
        
        let myLayer = CALayer()
        myLayer.frame = fLocalImageView.bounds
        myLayer.contents = fImage
        fLocalImageView.layer?.addSublayer(myLayer)
        
        let shapeLayer = CAShapeLayer()
        
        if let layer = fLocalImageView.layer {
            print("testBox layer OK")
            
            shapeLayer.frame = CGRect(x: 0, y: 0, width: 600, height: 900)
            var transform  = CGAffineTransform(rotationAngle: 0)
                .concatenating(CGAffineTransform(translationX: 0.0, y: 0.0))
            
            let width = 0.9*fLocalCIImage.extent.width
            let height = 0.9*fLocalCIImage.extent.height
            let rechteck = CGPath(rect: CGRect(x: 100, y: 200, width: width, height: height),
                                  transform: &transform)
            let path = CGMutablePath()
            path.addPath(rechteck)
            shapeLayer.path = path
        }
        
        shapeLayer.strokeColor = CGColor(red: 0, green: 1, blue: 1, alpha: 1.0)
        shapeLayer.fillColor = CGColor(red: 0.0, green: 1, blue: 1, alpha: 0.2)
        shapeLayer.zPosition = CGFloat(-1)
        shapeLayer.isHidden = false
        
        fLocalImageView.layer!.contents = nsi
        fLocalImageView.layer!.addSublayer(shapeLayer)
        
        fImage = fLocalImageView.image()
        
        imgArray.append(fImage)
        fImgIdx = imgArray.count-1
        
        
        
//        let nsi = NSImage.fromCIImage(fLocalCIImage)
//        fLocalImageView = NSImageView(frame: NSRect(origin: .zero, size: nsi.size))
//        fLocalImageView.wantsLayer = true
//        fLocalImageView.image = nsi
//        if let layer = fLocalImageView.layer {
//            print("layer OK")
//            if let rectangles = frectangles {
//                print("looping over rectangles")
//                for rectangle in rectangles {
//                    print(rectangle.boundingBox)
//                    let shapeLayer = shapeLayerForObservation(rectangle)
//                    layer.addSublayer(shapeLayer)
//                }
//            }
//        } else {
//            print("no layer in fLocalImageView")
//        }
//
//        fImage = fLocalImageView.image!
        
    }
    
    // -----------------------------------------------------------------------
    func testBox() {
        fLocalCIImage = NSImage.ciImage(fImage)!
        print("running testBox, ciimage = \(fLocalCIImage.extent)")
        // https://stackoverflow.com/questions/13272512/add-uiimage-in-calayer
        let nsi = fImage
        fLocalImageView = NSImageView(frame: NSRect(origin: .zero, size: nsi.size))
        fLocalImageView.wantsLayer = true
        fLocalImageView.canDrawSubviewsIntoLayer = true
        
        let myLayer = CALayer()
        myLayer.frame = fLocalImageView.bounds
        myLayer.contents = fImage
        fLocalImageView.layer?.addSublayer(myLayer)
        
        let shapeLayer = CAShapeLayer()
        
        if let layer = fLocalImageView.layer {
            print("testBox layer OK")
            
            shapeLayer.frame = CGRect(x: 0, y: 0, width: 600, height: 900)
            var transform  = CGAffineTransform(rotationAngle: 0)
                .concatenating(CGAffineTransform(translationX: 0.0, y: 0.0))
            
            let width = 0.9*fLocalCIImage.extent.width
            let height = 0.9*fLocalCIImage.extent.height
            let rechteck = CGPath(rect: CGRect(x: 100, y: 200, width: width, height: height),
                                  transform: &transform)
            let path = CGMutablePath()
            path.addPath(rechteck)
            shapeLayer.path = path
        }
        
        shapeLayer.strokeColor = CGColor(red: 0, green: 1, blue: 1, alpha: 1.0)
        shapeLayer.fillColor = CGColor(red: 0.0, green: 1, blue: 1, alpha: 0.2)
        shapeLayer.zPosition = CGFloat(-1)
        shapeLayer.isHidden = false
        
        fLocalImageView.layer!.contents = nsi
        fLocalImageView.layer!.addSublayer(shapeLayer)
        
        fImage = fLocalImageView.image()
        
        imgArray.append(fImage)
        fImgIdx = imgArray.count-1
    }
    
    // -----------------------------------------------------------------------
    func shapeLayerForObservation(_ rectangle: VNRectangleObservation) -> CAShapeLayer {
        guard let image = fLocalImageView.layer?.contents as? NSImage else { return CAShapeLayer() }
        let transformProperties = CGSize.aspectFit(aspectRatio: image.size, boundingSize: fLocalImageView.bounds.size)
        let shapeLayer = CAShapeLayer()
        let frame = frameForRectangle(rectangle, withTransformProperties: transformProperties)
        shapeLayer.frame = frame
        shapeLayer.path = pathForRectangle(rectangle, withTransformProperties: transformProperties, andBoundingBox: shapeLayer.bounds)
        shapeLayer.strokeColor = CGColor(red: 1.0, green: 0, blue: 0, alpha: 1.0)
        shapeLayer.lineWidth = 4.0
        shapeLayer.fillColor = CGColor(red: 1.0, green: 1.0, blue: 1.0, alpha: 0.0)
        shapeLayer.name = "rectangle"
        return shapeLayer
    }
    
    // -----------------------------------------------------------------------
    func frameForRectangle(_ rectangle: VNRectangleObservation, withTransformProperties properties: (size: CGSize, xOffset: CGFloat, yOffset: CGFloat)) -> NSRect {
        // Use aspect fit to determine scaling and X & Y offsets
        let transform = CGAffineTransform.identity
            .translatedBy(x: properties.xOffset, y: properties.yOffset)
            .scaledBy(x: properties.size.width, y: properties.size.height)
        
        // Convert normalized coordinates to display coordinates
        let convertedTopLeft = rectangle.topLeft.applying(transform)
        let convertedTopRight = rectangle.topRight.applying(transform)
        let convertedBottomLeft = rectangle.bottomLeft.applying(transform)
        let convertedBottomRight = rectangle.bottomRight.applying(transform)
        
        // Calculate bounds of bounding box
        let minX = min(convertedTopLeft.x, convertedTopRight.x, convertedBottomLeft.x, convertedBottomRight.x)
        let maxX = max(convertedTopLeft.x, convertedTopRight.x, convertedBottomLeft.x, convertedBottomRight.x)
        let minY = min(convertedTopLeft.y, convertedTopRight.y, convertedBottomLeft.y, convertedBottomRight.y)
        let maxY = max(convertedTopLeft.y, convertedTopRight.y, convertedBottomLeft.y, convertedBottomRight.y)
        let frame = NSRect(x: minX , y: minY, width: maxX - minX, height: maxY - minY)
        return frame
    }
    
    // -----------------------------------------------------------------------
    func pathForRectangle(_ rectangle: VNRectangleObservation,withTransformProperties properties: (size: CGSize, xOffset: CGFloat, yOffset: CGFloat),andBoundingBox size: CGRect) -> CGPath {
        //Convert to appropriate scale
        let scaleTransform = CGAffineTransform.identity
            .scaledBy(x: properties.size.width, y: properties.size.height)
        
        // Convert normalized coordinates to adjust for size of bounding box
        let scaledTopLeft = rectangle.topLeft.applying(scaleTransform)
        let scaledTopRight = rectangle.topRight.applying(scaleTransform)
        let scaledBottomLeft = rectangle.bottomLeft.applying(scaleTransform)
        let scaledBottomRight = rectangle.bottomRight.applying(scaleTransform)
        
        // translate to make bottom left corner of bounding box 0, 0
        let minX = min(scaledTopLeft.x, scaledTopRight.x, scaledBottomRight.x, scaledBottomLeft.x)
        let minY = min(scaledTopLeft.y, scaledTopRight.y, scaledBottomRight.y, scaledBottomLeft.y)
        
        let translateTransform = CGAffineTransform.identity
            .translatedBy(x: -minX, y: -minY)
            .scaledBy(x: properties.size.width, y: properties.size.height)
        
        let convertedTopLeft = rectangle.topLeft.applying(translateTransform)
        let convertedTopRight = rectangle.topRight.applying(translateTransform)
        let convertedBottomLeft = rectangle.bottomLeft.applying(translateTransform)
        let convertedBottomRight = rectangle.bottomRight.applying(translateTransform)
        
        let path = CGMutablePath()
        
        path.addLines(between: [convertedTopLeft, convertedTopRight, convertedBottomRight, convertedBottomLeft, convertedTopLeft])
        return path
    }
    
    
    // -----------------------------------------------------------------------
    func runAna3() {
        print("runAna3")
        
        let width: CGFloat = 600
        let height: CGFloat = 900
        
        let localView = NSView(frame: NSRect(x: 0, y: 0, width: width, height: height))
        
        localView.wantsLayer = true
        localView.canDrawSubviewsIntoLayer = true
        
        let shapeLayer = CAShapeLayer()
        shapeLayer.frame = CGRect(x: 0, y: 0,
                                  width: width, height: height)
        
        
        let path = CGMutablePath()
        
        stride(from: 0, to: CGFloat.pi * 2, by: CGFloat.pi / 6).forEach {
            angle in
            var transform  = CGAffineTransform(rotationAngle: angle)
                .concatenating(CGAffineTransform(translationX: 0, y: 0))
            var idx = fImgIdx
            if idx == 0 {
                idx = 1
            }
            let petal = CGPath(ellipseIn: CGRect(x: -20, y: 0,
                                                 width: 200/idx, height: 400/idx),
                               transform: &transform)
            
            path.addPath(petal)
        }
        
        shapeLayer.path = path
        shapeLayer.strokeColor = CGColor(red: 1.0, green: 0, blue: 0, alpha: 1.0)
        shapeLayer.fillColor = CGColor(red: 0.0, green: 1, blue: 1, alpha: 1.0)
        
        localView.layer!.addSublayer(shapeLayer)
        
        // -- the crucial difference to the original ana2()
        fImage = localView.image()
        imgArray.append(fImage)
        fImgIdx = imgArray.count-1
    }
    
    
    // -----------------------------------------------------------------------
    // -- keep the petal drawing code around for a while
    func runAna3Old() {
        print("runAna3")
        
        let width: CGFloat = 600
        let height: CGFloat = 900
        let localView = NSView(frame: NSRect(x: 0, y: 0, width: width, height: height))
        localView.wantsLayer = true
        localView.canDrawSubviewsIntoLayer = true
        let shapeLayer = CAShapeLayer()
        shapeLayer.frame = CGRect(x: 0, y: 0, width: width, height: height)
                
        let path = CGMutablePath()
        stride(from: 0, to: CGFloat.pi * 2, by: CGFloat.pi / 6).forEach {
            angle in
            var transform  = CGAffineTransform(rotationAngle: angle)
                .concatenating(CGAffineTransform(translationX: 0, y: 0))
            var idx = fImgIdx
            if idx == 0 {
                idx = 1
            }
            let petal = CGPath(ellipseIn: CGRect(x: -20, y: 0, width: 200/idx, height: 400/idx),
                               transform: &transform)
            path.addPath(petal)
        }
        
        shapeLayer.path = path
        shapeLayer.strokeColor = CGColor(red: 1.0, green: 0, blue: 0, alpha: 1.0)
        shapeLayer.fillColor = CGColor(red: 0.0, green: 1, blue: 1, alpha: 1.0)

        localView.layer!.addSublayer(shapeLayer)
        // -- the crucial difference to the original ana2()
        fImage = localView.image()
        imgArray.append(fImage)
        fImgIdx = imgArray.count-1
    }
    
}
