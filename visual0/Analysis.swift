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
    @Published var fStatus: Int
    
    private let visionClient = VisionClient()
    
    init() {
        fStatus = 0
    }
    
    func incStatus() {
        print("Analysis:incStatus() called")
        fStatus += 1
    }
    
    func getStatus() -> Int {
        return fStatus
    }
}
