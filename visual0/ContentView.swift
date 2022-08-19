// -----------------------------------------------------------------------
//  ContentView.swift
// -----------------------------------------------------------------------

import SwiftUI
import CoreImage
import Vision


// -----------------------------------------------------------------------
struct ContentView: View {
    @ObservedObject var ana = Analysis()
    
    var body: some View {
        
        HStack(spacing: 16) {
            VStack{
                HStack{
                    Text("getStatus \(ana.getStatus())")
                    Button(action: ana.incStatus) {
                        Text("inc status")
                    }
                    Button(action: ana.changeImage) {
                        Text("change image")
                    }
                }
                Image(nsImage: ana.fImage)
                    .resizable()
            }
        }
        .padding(.top, 32)
        .padding(.bottom, 16)
        .frame(minWidth: 600, idealWidth: 600, maxWidth: 600, minHeight: 900, maxHeight: 900)
    }
    
}

// -----------------------------------------------------------------------
struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}

