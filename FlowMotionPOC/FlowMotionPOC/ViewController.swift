//
//  ViewController.swift
//  FlowMotionPOC
//
//  Created by tedschultz on 12/9/22.
//

import Cocoa

class ViewController: NSViewController {
    
    private var selectedVideoURL: URL?
    private var selectedCSVURL: URL?
    
    // MARK: - View Life Cycle

    override func viewDidLoad() {
        super.viewDidLoad()
    }
    
    // MARK: - Actions
    
    @IBAction private func performSelectVideo(_ sender: AnyObject) {
        let panel = NSOpenPanel()
        panel.runModal()
        selectedVideoURL = panel.urls.first
    }
    
    @IBAction private func performSelectCSVFile(_ sender: AnyObject) {
        let panel = NSOpenPanel()
        panel.runModal()
        selectedCSVURL = panel.urls.first
    }
    
    @IBAction private func performStart(_ sender: AnyObject) {
        
        guard let selectedVideoURL, let selectedCSVURL else { return }
        
        let selectedVideoData = try? Data(contentsOf: selectedVideoURL)
        
        // TODO: parse/decode the CSV
        
        Task {
            do {
                // TODO: pass in the parsed data from the CSV
                try await OpticalFlowProcessor.shared.process(videoURL: selectedVideoURL)
            } catch let error {
                print(error.localizedDescription)
            }
        }
    }
}

