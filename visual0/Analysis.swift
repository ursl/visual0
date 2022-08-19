//
//  Analysis.swift
//  visual0
//
//  Created by Urs Langenegger on 17.08.22.
//

import Foundation
import SwiftUI
import AppKit

// -----------------------------------------------------------------------
final class Analysis: ObservableObject {
    
    @Published var fStatus: Int
    @Published var fImage : NSImage
    
    let imgArray = [
        NSImage(named: NSImage.Name("glass-20220401-5"))
        , NSImage(named: NSImage.Name("glass-20220401-6"))
    ]
    
    
    
    init() {
        fStatus = 0
        fImage = imgArray[0]!
    }
    
    func getImage() -> NSImage {
        return fImage
    }
    
    func getStatus() -> Int {
        return fStatus
    }
    
    func incStatus() {
        fStatus += 1
    }
}
