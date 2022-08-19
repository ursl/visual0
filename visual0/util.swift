import Foundation
import Cocoa


extension NSImage {
    // -----------------------------------------------------------------------
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
    
    // -----------------------------------------------------------------------
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

extension CGSize {
    static func aspectFit(aspectRatio : CGSize, boundingSize: CGSize) -> (size: CGSize, xOffset: CGFloat, yOffset: CGFloat)  {
        let mW = boundingSize.width / aspectRatio.width;
        let mH = boundingSize.height / aspectRatio.height;
        var fittedWidth = boundingSize.width
        var fittedHeight = boundingSize.height
        var xOffset = CGFloat(0.0)
        var yOffset = CGFloat(0.0)

        if( mH < mW ) {
            fittedWidth = boundingSize.height / aspectRatio.height * aspectRatio.width;
            xOffset = abs(boundingSize.width - fittedWidth)/2
            
        }
        else if( mW < mH ) {
            fittedHeight = boundingSize.width / aspectRatio.width * aspectRatio.height;
            yOffset = abs(boundingSize.height - fittedHeight)/2
            
        }
        let size = CGSize(width: fittedWidth, height: fittedHeight)
        
        return (size, xOffset, yOffset)
    }
    
    static func aspectFill(aspectRatio :CGSize, minimumSize: CGSize) -> CGSize {
        let mW = minimumSize.width / aspectRatio.width;
        let mH = minimumSize.height / aspectRatio.height;
        var minWidth = minimumSize.width
        var minHeight = minimumSize.height
        if( mH > mW ) {
            minWidth = minimumSize.height / aspectRatio.height * aspectRatio.width;
        }
        else if( mW > mH ) {
            minHeight = minimumSize.width / aspectRatio.width * aspectRatio.height;
        }
        
        return CGSize(width: minWidth, height: minHeight)
    }
}
