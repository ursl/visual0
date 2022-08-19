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
    @EnvironmentObject var appState: AppState
    
    @Published var fStatus: Int = 0
 
    func getStatus() -> Int {
        return fStatus
    }
}
