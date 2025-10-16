# Windows Driver Samples for Windows 11 (Fork)

This repository is a fork of [Microsoft/Windows-driver-samples](https://github.com/microsoft/Windows-driver-samples).

It is intended for testing and learning purposes related to Windows 11 driver development.

The original project is maintained by Microsoft and distributed under the MIT License. It contains official sample drivers demonstrating how to build, test, and deploy device drivers for Windows 11 using Visual Studio 2022 and Windows Driver Kit (WDK) 11.
Purpose

This fork is used to:

Experiment with building and testing drivers using Visual Studio 2022 and WDK 11

Learn about Universal Windows Drivers (UWD) and Windows Driver Frameworks (WDF)

Modify and compile sample drivers to better understand driver development workflows

# Note: This fork is for educational and experimental purposes only.
It is not intended for production driver releases.

# Project Overview

The original Microsoft repository provides driver samples for:

KMDF (Kernel-Mode Driver Framework)

UMDF (User-Mode Driver Framework)

Device types such as USB, Bluetooth, Camera, Display, NFC, and WLAN

Useful Microsoft documentation:

[Windows Driver Kit (WDK) documentation](https://learn.microsoft.com/windows-hardware/drivers/)

[Windows Driver Frameworks (WDF) guide](https://learn.microsoft.com/windows-hardware/drivers/wdf/)

[From Sample Code to Production Driver](https://learn.microsoft.com/windows-hardware/drivers/develop/from-sample-code-to-production-driver)

# Development Setup

To build and test the samples:

Install Visual Studio 2022

Install Windows Driver Kit (WDK) 11

Open a sample project in Visual Studio

Build and deploy the driver to a test environment

Official setup guide:
Download the WDK, WinDbg, and associated tools

# License

This project is licensed under the MIT License
.
The original source code is © Microsoft Corporation and contributors.

If you plan to use any part of these samples in production code, please review:
[From Sample Code to Production Driver - What to Change in the Samples](https://learn.microsoft.com/windows-hardware/drivers/develop/from-sample-code-to-production-driver)

# Acknowledgements

Thanks to Microsoft and the open-source community for providing high-quality driver samples and documentation that make Windows driver development accessible for learning and experimentation.
