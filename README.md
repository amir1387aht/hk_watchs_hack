# hk-watches-hack

> Reverse-engineering toolkit and project files for HK watches — includes SDK setup, build, and flashing instructions.

---

## 🎥 Demo
Watch the full video tutorial: [Click here to watch]()

---

## 📑 Table of Contents
- [Overview](#overview)
- [Prerequisites](#prerequisites)
- [Install SDK](#install-sdk)
- [Build / Compile](#build--compile)
- [Menu Configuration](#menu-configuration)
- [Flashing (UART)](#flashing-uart)
- [Wiring / Hardware Notes](#wiring--hardware-notes)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

---

## 🧩 Overview
This repository contains:
- SiFli SDK project files for HK watch firmware  
  *(example path: `projects/hk10ultra3_firmware/v1/watch/project`)*
- Build scripts and a UART flashing helper used in the demo video  
- Documentation for reproducing the reverse-engineering workflow

---

## ⚙️ Prerequisites
- Windows PC (SDK tools are Windows-oriented)  
- Installed **SiFli SDK**  
- **Python** and **scons** (usually included with SDK)  
- **UART-to-USB** adapter (e.g., CH340, FTDI)  
- Basic familiarity with serial ports / command line

---

## 📦 Install SDK
Follow the official SiFli SDK installation guide:

👉 [https://docs.sifli.com/projects/sdk/latest/sf32lb52x/quickstart/install/script/windows.html](https://docs.sifli.com/projects/sdk/latest/sf32lb52x/quickstart/install/script/windows.html)

> After installing, always use the **“SiFli-SDK Environment”** terminal — it configures all paths and tools for you.

---

## 🧰 Build / Compile
1. Open the **SiFli-SDK Environment** shell.  
2. Navigate to the project folder:
   ```bash
   cd projects\hk10ultra3_firmware\v1\watch\project

3. Compile:

   ```bash
   scons --board=eh-lb525 -j8
   ```

   * `--board=eh-lb525` sets the build target.
   * `-j8` runs 8 jobs in parallel (adjust based on CPU cores).

📁 Build outputs appear under `build_eh-lb525_hcpu`.

---

## 🧭 Menu Configuration

To open configuration UI:

```bash
scons --menuconfig --board=eh-lb525
```

Use **arrow keys** and **Enter** to modify settings.
Save and exit to apply your configuration.

---

## 🔌 Flashing (UART)

1. Connect the watch to your UART-USB adapter *(see video for pinout)*.
2. Run this command from the SDK shell:

   ```bash
   build_eh-lb525_hcpu\uart_download.bat
   ```
3. When prompted, choose your **COM port number** (e.g., `3`, `6`).

> 💡 If flashing fails, check wiring (TX↔RX), COM port, or boot mode.

---

## 🪛 Wiring / Hardware Notes

Typical UART wiring:

| Watch Pin | USB Adapter | Description      |
| --------- | ----------- | ---------------- |
| DBG_TX0        | RX          | Data transmit    |
| DBG_RX0        | TX          | Data receive     |
| GND            | GND         | Ground reference |

⚠️ **Do NOT power the watch from the adapter** incorrect voltage can destroy the device.
If your adapter is 1.8 V or 5 V, use a 3.3 V **level shifter**.

---

## 🧯 Troubleshooting

**Build errors**

* Make sure you’re using *SiFli-SDK Environment*.
* Check that `scons` and the toolchain are accessible.
* Review the console for missing files or dependency errors.

**Flashing issues**

* Verify correct COM port and close other serial tools.
* Swap TX/RX if flashing fails.
* Confirm solid GND and stable power.

**Device not responding**

* Ensure proper power and wiring.
* Retry after entering bootloader mode.
* Avoid using adapters that output 5 V to 3.3 V lines.

---

## 🧾 Quick Reference Commands

```bash
# Build project
scons --board=eh-lb525 -j8

# Open menuconfig
scons --menuconfig --board=eh-lb525

# Flash via UART
build_eh-lb525_hcpu\uart_download.bat
```

---

## 🤝 Contributing

Pull requests and improvements are welcome!
To contribute:

1. Open an issue describing your suggestion or bug.
2. Fork the repo and push your changes.
3. Create a Pull Request with testing notes (mention board & firmware version if possible).

---

## ⚖️ License

Apache License 2.0

---
