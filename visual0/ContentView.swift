//
//  ContentView.swift
//  visual0
//
//  Created by Urs Langenegger on 15.08.22.
//

import SwiftUI

struct ContentView: View {
    
    @State private var tapCount = 0
    @State private var name = ""
    
    let students = ["Harry", "Hermione", "Ron"]
    @State private var selectedStudent = "Harry"
    
    var body: some View {
        
        NavigationView {
            
            VStack(alignment: .leading) {
                Section {
                    Text("Hello, world!")
                    // .padding()
                }
                Button("Tap Count: \(tapCount)") {
                    tapCount += 1
                }
                Text("Enter your name:")
                TextField("", text: $name)
                Text("Your name is \(name)")
                Picker("Select your student", selection: $selectedStudent) {
                    ForEach(students, id: \.self) {
                        Text($0)
                    }
                }
                Text("You chose student \(selectedStudent)")
                ZStack {
                    
                }
                .padding(50)
                .background(.ultraThinMaterial)
    
            }
            
            HStack {
                Button {
                    print("Edit button was tapped")
                } label: {
                    Image(systemName: "pencil")
                }
                Button("Button 2", role: .destructive) { }
                    .buttonStyle(.bordered)
                Button("Button 3") { }
                    .buttonStyle(.borderedProminent)
                Button("Button 4", role: .destructive) { }
                    .buttonStyle(.borderedProminent)
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
