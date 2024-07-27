# c2system
Master-Slave Stepper Motor Control with NodeMCU

## Overview

This project involves controlling multiple stepper motors using NodeMCU modules in a master-slave configuration. The master NodeMCU hosts a web server that allows users to control the speed and maximum rotation angle of each motor. The slave NodeMCUs receive these settings and control the motors accordingly, with additional functionality to calibrate the zero position using end switches and handle end switch triggers during operation.

## Components

- Master NodeMCU
- Slave NodeMCUs (up to 11)
- Stepper Motors
- Motor Drivers
- End Switches
- WiFi Network

## Features

- Web interface to control motor speed and rotation angle
- Periodic heartbeat check to monitor the status of each slave NodeMCU
- Calibration of motor zero position using end switches
- UDP communication for synchronized motor control

## Master NodeMCU Setup

1. **Connect to WiFi**: Set the SSID and password for your WiFi network.
    ```cpp
    const char* ssid = "ceilc2";
    const char* password = "idrf2024";
    ```

2. **Initialize Web Server**: Create a web server with routes to handle root and setting updates.
    - Define routes:
        ```cpp
        server.on("/", handleRoot);
        server.on("/set", handleSet);
        ```
    - Serve the HTML page with sliders for speed and angle control.

3. **Send Settings via UDP**: When settings are updated via the web interface, send the speed and angle to the respective slave NodeMCU using their hardcoded IP addresses.

4. **Heartbeat Check**: Implement a function to check for heartbeat messages from the slaves and update their status on the web interface.

## Slave NodeMCU Setup

1. **Connect to WiFi with Static IP**: Set the SSID, password, and static IP configuration for each slave.
    ```cpp
    const char* ssid = "ceilc2";
    const char* password = "idrf2024";
    IPAddress local_IP(192, 168, 1, 2); // Change this for each slave
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    ```

2. **Initialize Web Server**: Create a server to handle updates from the master.
    - Define route:
        ```cpp
        server.on("/update", handleUpdate);
        ```

3. **Calibrate Zero Position**: Implement a function to calibrate the zero position using end switches.
    - Move motor to hit both end switches and set the zero position at the midpoint.

4. **Move Motor**: Implement functions to move the motor based on received speed and angle settings.
    - Handle end switch hits during movement to reverse direction if necessary.

5. **Send Heartbeat**: Periodically send a heartbeat message to the master to indicate that the slave is operational.

## Putting It All Together

1. **Upload Code to Master**:
    - Configure WiFi and server settings.
    - Implement functions to handle web requests, send UDP messages, and check heartbeats.

2. **Upload Code to Slaves**:
    - Configure WiFi with static IPs.
    - Implement functions to handle updates, calibrate zero position, move the motor, and send heartbeats.

3. **Network Configuration**:
    - Ensure all NodeMCUs are connected to the same WiFi network.
    - Verify the IP address assignments and ensure no conflicts.

4. **Testing**:
    - Access the web interface hosted by the master NodeMCU.
    - Adjust the speed and angle settings for each motor and observe the responses from the slaves.
    - Verify that the heartbeat status is updated correctly on the web interface.

## Example IP Configuration

- Master NodeMCU: `192.168.1.99`
- Slave NodeMCUs:
  - Slave 1: `192.168.1.2`
  - Slave 2: `192.168.1.3`
  - ...
  - Slave 11: `192.168.1.12`

By following these instructions, you should be able to configure your NodeMCU modules to control multiple stepper motors in a synchronized master-slave setup.
