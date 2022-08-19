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
                    Text("idx \(ana.getIdx())")
                    Button(action: ana.changeImage) {
                        Text("change image")
                    }
                    Button(action: ana.runAna1) {
                        Text("ana1")
                    }
                    Button(action: ana.runAna2) {
                        Text("ana2")
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

