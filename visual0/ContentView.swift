//
//  ContentView.swift
//  visual0
//
//  Created by Urs Langenegger on 15.08.22.
//

import SwiftUI

struct ContentView: View {
    //    var body: some View {
    //        Text("Hello, world!")
    //            .padding()
    //    }
    @State private var tapCount = 0
    
    var body: some View {
        
        NavigationView {
            
            Form {
                Section {
                    Text("Hello, world!")
                        .padding()
                }
                Button("Tap Count: \(tapCount)") {
                    tapCount += 1
                }
            }
            
        }
        .navigationSubtitle("SwiftUI")
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        Group {
            ContentView()
        }
    }
}
