# Core Flight System : Framework : App : IMU

This repository contains a imu application (imu_app), which is a framework component of the Core Flight System.

This application has been developed to obtain and process data from a 9dof IMU running on the cFS of a Raspberry Pi 2. This application needs the library mpu9dof_lib and library bcm2839_lib in order to use the GPIO pins and access the registers of the inertial unit.

The original software for the cFS can be found at https://github.com/nasa/cFS
