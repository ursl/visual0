//
//  ContentView.swift
//  ImageFilterMac
//
//  Created by Alfian Losari on 23/02/20.
//  Copyright © 2020 Alfian Losari. All rights reserved.
//

import SwiftUI

struct ContentView: View {
    
    @EnvironmentObject var appState: AppState
    @State var imageLoaded = false
    
    var body: some View {
        let iv = InputView(image: self.$appState.image, imgLoaded: $imageLoaded)

        HStack(spacing: 16) {
            VStack{
                Button("Start") {
                    print("Start button was tapped")
                }
                Button("Remove") {
                    //                    Text("Remove")
                    if !imageLoaded {
                        print("Remove button should not be tapped")
                    } else {
                        print("Remove button was tapped")
                        iv.image = nil
                    }
                }
            }
            iv
        }
        
        .padding(.top, 32)
        .padding(.bottom, 16)
        .frame(minWidth: 700, idealWidth: 700, maxWidth: 700, minHeight: 1000, maxHeight: 1100)
    }
}

struct InputView: View {
    
    @Binding var image: NSImage?
    @Binding var imgLoaded : Bool
    
    var body: some View {
        VStack(spacing: 16) {
            HStack {
                Text("Input image")
                    .font(.headline)
                Button(action: selectFile) {
                    Text("Select image")
                }
            }
            InputImageView(image: self.$image, imgLoaded: self.$imgLoaded)
            
        }
    }
    
    private func selectFile() {
        NSOpenPanel.openImage { (result) in
            if case let .success(image) = result {
                self.image = image
                self.imgLoaded = true
            }
        }
    }
    
}


struct InputImageView: View {
    
    @Binding var image: NSImage?
    @Binding var imgLoaded : Bool
    
    var body: some View {
        ZStack {
            if image != nil {
                Image(nsImage: image!)
                    .resizable()
                    .aspectRatio(contentMode: .fit)
            } else {
                Text("Drag and drop image file")
                    .frame(width: 600)
            }
        }
        .frame(height: 900)
        .background(Color.black.opacity(0.5))
        .cornerRadius(8)
        
        .onDrop(of: ["public.file-url"], isTargeted: nil, perform: handleOnDrop(providers:))
    }
    
    private func handleOnDrop(providers: [NSItemProvider]) -> Bool {
        if let item = providers.first {
            item.loadItem(forTypeIdentifier: "public.file-url", options: nil) { (urlData, error) in
                DispatchQueue.main.async {
                    if let urlData = urlData as? Data {
                        let url = NSURL(absoluteURLWithDataRepresentation: urlData, relativeTo: nil) as URL
                        guard let image = NSImage(contentsOf: url) else {
                            return
                        }
                        self.image = image
                        self.imgLoaded = true
                    }
                }
            }
            return true
        }
        return false
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
