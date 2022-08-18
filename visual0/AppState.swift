import Combine
import Cocoa

// -----------------------------------------------------------------------
class AppState: ObservableObject {
    // -- singleton
    static let shared = AppState()
    private init() {}
    
    @Published var image: NSImage? {
        didSet {
            print("AppState: assigned to image")
        }
    }
    
    @Published var ciimage: CIImage? {
        didSet {
            print("AppState: assigned to ciimage")
        }
    }
}
