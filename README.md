# AutoVP

This software is the outcome of our accademic research.

If you use this code, please cite our accademic paper as:

```
@inproceedings{AutoVP,
 author = {Lin Jiang, Feiyu Zhang, Jiang Ming},
 title = {{Towards Intelligent Automobile Cockpit via A New Container Architecture}},
 booktitle = {NSDI 2024 : The 21st USENIX Symposium on Networked Systems Design and Implementation},
 year = {2024}
} 
```

# AutoVP

In a nutshell, we make the following key contributions:

- We propose a mixed-criticality decoupled architecture to address the need for centralized deployment of various business subsystems within the intelligent cockpit domain, all while offering ease of construction, low production costs, and small performance overhead.

- We present a new Android container framework tailed for intelligent cockpits. Our work represents the latest progress in mobile container-based virtualization, and it is an ideal solution to host the non-safety-critical subsystems that require display screens for user interaction.

- Our evaluation and real-world deployment demonstrate that AutoVP is a viable in-vehicle virtualization alternative to mitigate the ongoing automotive chip crisis.

## Overview of AutoVP's Architecture

<img src="https://github.com/jianglin-code/AutoVP/blob/main/autovp.png" width="800">

The figure provides an overview of AutoVP’s architecture. AutoVP supports multiple Android containers. For example, both driver and co-driver can have their own in-vehicle infotainment systems running on isolated Android containers.

Video: https://github.com/jianglin-code/AutoVP/blob/main/autovp.mp4

## Code Introduction  (Android 13, Rockchip rk3528 SOC, Demo)

### autovp
  - autovp/: VP manager daemons

### system
  - cd system/ & git log .

### kernel-5.10
  - cd kernel-5.10/ & git log .

### frameworks
  - cd frameworks/ & git log .

### basic code 

- The Android source code is:  Rockchip rk3528 

## System Prerequisites

- Operating System: Ubuntu 20.04 LTS

- JDK version：openJDK version 9

## Compile Command

`source build/envsetup.sh`

`lunch rk3528_box-userdebug`

`make -j4`


## Benchmarks and Samples

### Benchmarks 

- Linpack (v1.1) for CPU; 

- Quadrant advanced edition (v2.1.1) for 2D graphics and file I/O; 

- 3DMark (v2.0.4646) for 3D graphics;

- BusyBox wget (v1.21.1) for networking.

- GPS Test app (v1.6.3) for GPS.

- Bluetooth Setting for Bluetooth. 

### Samples  

We have used nine families of malware to evaluate virtualization environment detection.









