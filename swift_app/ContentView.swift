//
//  ContentView.swift
//  Glucose Monitor
//
//  Created by Dhruv Nellutla on 3/31/25.
//

import SwiftUI
import CoreBluetooth

class BluetoothViewModel: NSObject, ObservableObject, CBCentralManagerDelegate, CBPeripheralDelegate {
    @Published var glucoseLevel: String = "Searching..."
    private var centralManager: CBCentralManager!
    private var glucosePeripheral: CBPeripheral?

    // Replace with your DA14531 BLE UUIDs
    private let targetDeviceName = "GlucoseMonitor"
    private let glucoseServiceUUID = CBUUID(string: "YOUR_SERVICE_UUID")
    private let glucoseCharacteristicUUID = CBUUID(string: "YOUR_CHARACTERISTIC_UUID")

    override init() {
        super.init()
        centralManager = CBCentralManager(delegate: self, queue: nil)
    }

    // Bluetooth State
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        if central.state == .poweredOn {
            centralManager.scanForPeripherals(withServices: nil, options: nil)
        } else {
            glucoseLevel = "Bluetooth Off"
        }
    }

    // Discover Peripheral
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String: Any], rssi RSSI: NSNumber) {
        if peripheral.name == targetDeviceName {
            glucoseLevel = "Connecting..."
            glucosePeripheral = peripheral
            glucosePeripheral?.delegate = self
            centralManager.stopScan()
            centralManager.connect(peripheral, options: nil)
        }
    }

    // Peripheral Connected
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        glucoseLevel = "Connected!"
        peripheral.discoverServices([glucoseServiceUUID])
    }

    // Discover Services
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        guard let services = peripheral.services else { return }
        for service in services {
            if service.uuid == glucoseServiceUUID {
                peripheral.discoverCharacteristics([glucoseCharacteristicUUID], for: service)
            }
        }
    }

    // Discover Characteristics
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        guard let characteristics = service.characteristics else { return }
        for characteristic in characteristics {
            if characteristic.uuid == glucoseCharacteristicUUID {
                peripheral.setNotifyValue(true, for: characteristic)
            }
        }
    }

    // Read Glucose Data
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        guard let data = characteristic.value else { return }
        let glucoseValue = data.first ?? 0  // Example: First byte as glucose level
        DispatchQueue.main.async {
            self.glucoseLevel = "Glucose Level: \(glucoseValue) mg/dL"
        }
    }
}

struct ContentView: View {
    @StateObject private var bluetoothVM = BluetoothViewModel()

    var body: some View {
        VStack {
            Text(bluetoothVM.glucoseLevel)
                .font(.title)
                .padding()
            Spacer()
        }
    }
}

@main
struct GlucoseMonitorApp: App {
    var body: some Scene {
        WindowGroup {
            ContentView()
        }
    }
}
