// -----------------------------------------------------------------------
//  ContentView.swift
// -----------------------------------------------------------------------

import SwiftUI
import CoreImage
import Vision

// -----------------------------------------------------------------------
struct ContentView: View {
    
    @EnvironmentObject var appState: AppState
    
    
    @State var imageLoaded = false
    
    @StateObject var ana = Analysis()
    
    var body: some View {
        
        HStack(spacing: 16) {
            VStack{
                Text("Status \(ana.getStatus())")
                Button("Button 1") {
                }
                Button("Button 2") {
                }
            }
        }
        
        .padding(.top, 32)
        .padding(.bottom, 16)
        .frame(minWidth: 700, idealWidth: 700, maxWidth: 700, minHeight: 1000, maxHeight: 1100)
    }
    
}


struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}

