# Installation Manual for NodeMCU Modules

## Overview

This manual provides step-by-step instructions for setting up the master and slave NodeMCU modules, including the installation of the Arduino IDE, drivers for the NodeMCU, and uploading the required code to the modules.

## Requirements

- NodeMCU modules (1 master, up to 11 slaves)
- USB cables for NodeMCU modules
- Stepper Motors and Drivers
- End Switches
- Computer with internet access

## Step 1: Install Arduino IDE

1. **Download Arduino IDE**:
   - Visit the [Arduino Software page](https://www.arduino.cc/en/software) and download the latest version of the Arduino IDE for your operating system.

2. **Install Arduino IDE**:
   - Follow the installation instructions specific to your operating system:
     - **Windows**: Run the downloaded installer and follow the prompts.
     - **Mac OS X**: Open the downloaded `.dmg` file and drag the Arduino application into your Applications folder.
     - **Linux**: Extract the downloaded tarball and run the `install.sh` script.

## Step 2: Install NodeMCU Drivers

1. **Identify the Serial Chip**:
   - Most NodeMCU boards use either the CH340 or CP2102 USB-to-serial chip. Check your NodeMCU board to identify the chip type.

2. **Install Drivers for CH340**:
   - **Windows**: Download the CH340 driver from [this link](http://www.wch.cn/download/CH341SER_EXE.html) and run the installer.
   - **Mac OS X**: Download the CH340 driver from [this link](http://www.wch.cn/download/CH341SER_MAC_ZIP.html), unzip it, and run the installer package.
   - **Linux**: CH340 drivers are usually included in the Linux kernel. If not, follow [this guide](https://github.com/juliagoda/CH341SER).

3. **Install Drivers for CP2102**:
   - **Windows**: Download the CP2102 driver from [this link](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) and run the installer.
   - **Mac OS X**: Download the CP2102 driver from [this link](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers), unzip it, and run the installer package.
   - **Linux**: CP2102 drivers are usually included in the Linux kernel. If not, follow [this guide](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers/linux).

## Step 3: Configure Arduino IDE for NodeMCU

1. **Open Arduino IDE**:
   - Launch the Arduino IDE on your computer.

2. **Install ESP8266 Board Package**:
   - Go to `File` > `Preferences`.
   - In the `Additional Board Manager URLs` field, add the following URL: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`.
   - Click `OK`.
   - Go to `Tools` > `Board` > `Boards Manager`.
   - Search for `esp8266` and install the `esp8266 by ESP8266 Community` package.

3. **Select NodeMCU Board**:
   - Go to `Tools` > `Board` and select `NodeMCU 1.0 (ESP-12E Module)`.

## Step 4: Upload Code to Master NodeMCU

1. **Connect Master NodeMCU**:
   - Use a USB cable to connect the master NodeMCU to your computer.

2. **Open Master Code**:
   - Create a new sketch in Arduino IDE and copy the master code into the sketch.

3. **Upload Master Code**:
   - Select the correct port: `Tools` > `Port` and choose the port corresponding to your NodeMCU.
   - Click the upload button (right arrow) in the Arduino IDE to upload the code to the master NodeMCU.

## Step 5: Upload Code to Slave NodeMCUs

1. **Configure Static IP Addresses**:
   - Modify the `local_IP` variable in the slave code for each slave NodeMCU to assign a unique IP address.

2. **Connect Slave NodeMCU**:
   - Use a USB cable to connect each slave NodeMCU to your computer, one at a time.

3. **Open Slave Code**:
   - Create a new sketch in Arduino IDE and copy the slave code into the sketch.

4. **Upload Slave Code**:
   - Select the correct port: `Tools` > `Port` and choose the port corresponding to your NodeMCU.
   - Click the upload button (right arrow) in the Arduino IDE to upload the code to the slave NodeMCU.
   - Repeat this process for each slave NodeMCU, ensuring each one has a unique static IP address.

## Network Configuration

1. **Ensure WiFi Network**:
   - Make sure all NodeMCUs are connected to the same WiFi network as specified in the code.

2. **IP Address Assignment**:
   - Verify that each NodeMCU (master and slaves) has the correct IP address assignment as per your network configuration.

## Testing and Troubleshooting

1. **Access Web Interface**:
   - Open a web browser and navigate to the IP address of the master NodeMCU (e.g., `http://192.168.1.99`).

2. **Control Motors**:
   - Use the web interface to adjust the speed and angle settings for each motor and observe the responses from the slave NodeMCUs.

3. **Check Heartbeat Status**:
   - Verify that the heartbeat status of each slave is updated correctly on the web interface.

4. **Troubleshooting**:
   - If a NodeMCU does not connect or respond, check the serial monitor in the Arduino IDE for debug messages.
   - Ensure the correct drivers are installed and the NodeMCU is using the correct port.

By following these instructions, you should be able to set up and configure your NodeMCU modules to control multiple stepper motors in a synchronized master-slave setup.
