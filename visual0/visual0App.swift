//
//  visual0App.swift
//  visual0
//
//  Created by Urs Langenegger on 15.08.22.
//

import SwiftUI

@main
struct visual0App: App {
    var body: some Scene {
        let appState = AppState.shared
        WindowGroup {
            ContentView()
                .environmentObject(appState)
        }
    }
}

