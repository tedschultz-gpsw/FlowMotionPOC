//
//  ViewController.swift
//  FlowMotionPOC
//
//  Created by tedschultz on 12/9/22.
//

import Cocoa

class ViewController: NSViewController {
    
    func getVideo() {
        let panel = NSOpenPanel()
        panel.runModal()
        guard let url = panel.urls.first else { return }
        let data = try? Data(contentsOf: url)
    }
    
    func getCSVFile() {
        let panel = NSOpenPanel()
        panel.runModal()
        guard let url = panel.urls.first else { return }
        let data = try? Data(contentsOf: url)
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        getVideo()
        getCSVFile()
        // Do any additional setup after loading the view.
    }

    override var representedObject: Any? {
        didSet {
        // Update the view, if already loaded.
        }
    }


}

