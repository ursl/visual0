//
//  FilePanel+Extension.swift
//  ImageFilterMac
//
//  Created by Alfian Losari on 25/02/20.
//  Copyright © 2020 Alfian Losari. All rights reserved.
//

import Cocoa

extension NSOpenPanel {
    
    static func openImage(completion: @escaping (_ result: Result<NSImage, Error>) -> ()) {
        let panel = NSOpenPanel()
        panel.allowsMultipleSelection = false
        panel.canChooseFiles = true
        panel.canChooseDirectories = false
        panel.allowedContentTypes = [.jpeg, .png, .heic]
        panel.canChooseFiles = true
        panel.begin { (result) in
            if result == .OK,
               let url = panel.urls.first,
               let image = NSImage(contentsOf: url) {
                completion(.success(image))
            } else {
                completion(.failure(
                    NSError(domain: "", code: 0, userInfo: [NSLocalizedDescriptionKey: "Failed to get file location"])
                ))
            }
        }
    }
}

extension NSSavePanel {
    
    static func saveImage(_ image: NSImage, completion: @escaping (_ result: Result<Bool, Error>) -> ()) {
        let savePanel = NSSavePanel()
        savePanel.canCreateDirectories = true
        savePanel.showsTagField = false
        savePanel.nameFieldStringValue = "image.jpg"
        savePanel.level = NSWindow.Level(rawValue: Int(CGWindowLevelForKey(.modalPanelWindow)))
        savePanel.begin { (result) in
            guard result == .OK,
                  let url = savePanel.url else {
                completion(.failure(
                    NSError(domain: "", code: 0, userInfo: [NSLocalizedDescriptionKey: "Failed to get file location"])
                ))
                return
            }
            
            DispatchQueue.global(qos: .userInitiated).async {
                guard
                    let data = image.tiffRepresentation,
                    let imageRep = NSBitmapImageRep(data: data) else { return }
                
                do {
                    let imageData = imageRep.representation(using: .jpeg, properties: [.compressionFactor: 1.0])
                    try imageData?.write(to: url)
                } catch {
                    completion(.failure(error))
                }
            }
        }
    }
}

extension NSImage {
    func rotated(by degrees : CGFloat) -> NSImage {
        var imageBounds = NSRect(x: 0, y: 0, width: size.width, height: size.height)
        let rotatedSize = AffineTransform(rotationByDegrees: degrees).transform(size)
        let newSize = CGSize(width: abs(rotatedSize.width), height: abs(rotatedSize.height))
        let rotatedImage = NSImage(size: newSize)
        
        imageBounds.origin = CGPoint(x: newSize.width / 2 - imageBounds.width / 2, y: newSize.height / 2 - imageBounds.height / 2)
        
        let otherTransform = NSAffineTransform()
        otherTransform.translateX(by: newSize.width / 2, yBy: newSize.height / 2)
        otherTransform.rotate(byDegrees: degrees)
        otherTransform.translateX(by: -newSize.width / 2, yBy: -newSize.height / 2)
        
        rotatedImage.lockFocus()
        otherTransform.concat()
        draw(in: imageBounds, from: CGRect.zero, operation: NSCompositingOperation.copy, fraction: 1.0)
        rotatedImage.unlockFocus()
        
        return rotatedImage
    }
}


extension NSImage {
    // -- generate a CIImage for this NSImage
    // -- returns: a CIImage optional
    static func ciImage(_ nsImage: NSImage) -> CIImage? {
        guard let data = nsImage.tiffRepresentation,
              let bitmap = NSBitmapImageRep(data: data) else {
            return nil
        }
        let ci = CIImage(bitmapImageRep: bitmap)
        return ci
    }
    
    // -- generates an NSImage from a CIImage
    // -- parameter ciImage: the CIImage
    // -- returns: an NSImage optional
    static func fromCIImage(_ ciImage: CIImage) -> NSImage {
        let rep = NSCIImageRep(ciImage: ciImage)
        let nsImage = NSImage(size: rep.size)
        nsImage.addRepresentation(rep)
        return nsImage
    }
}
