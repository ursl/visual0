import Combine
import Cocoa

// -----------------------------------------------------------------------
class AppState: ObservableObject {
    
    // 1
    static let shared = AppState()
    private init() {}
    
    // 2
    @Published var image: NSImage? {
        didSet {
            // 4
            self.filteredImage = nil
        }
    }
        
    // 3
    @Published var filteredImage: NSImage?
}
