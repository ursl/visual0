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
