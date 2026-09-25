# ESP32-Quadruped-Robot-IK-PCA9685-WiFi-Control
A four-legged walking robot built as an individual project during my first year of General Engineering at KCL. Each leg has 3 degrees of freedom — hip, thigh and knee — with each joint driven directly by its own servo, giving 12 degrees of freedom total, all coordinated from a single ESP32 microcontroller running a trot gait.

# Overview 
Goal was to design, build and program a quadruped form the ground up - covering mechanical leg design, electronics and gait control firmware,Each leg is a 3-DOF serial (open-chain) linkage where the servo horns directly actuate each joint in sequence, all while remaining lightweight and within a budget of £100

final weight-400g , final cost- £98

#  How does ot work

**mechanically**

as previosuly mentioned the 3 DOF legs are serially linked with direct s ervos at hip, knee and thigh. Link lengths and joint ranges were modelled in Fusion and iterated for a stable foot trajectory and good ground clearance.

**Electronics**

all 12 motors were powered by an ESP32 via a PCA9685 servo driver via PWM, and a dedicated power supply.

**Firmware**

Written in C++ Implements a trot gait with adjustable stride length, step height, speed, and phase offsets between legs, using inverse kinematics and interpolation to form smooth arcs. Servo calibration and gait parameters are tuned via bench testing.


**build**

3D-printed (FDM) chassis and leg links, assembled and iteratively tested to reduce foot slip., a multiude of m1.5-m2 screws to mount the servos and nano gel foot attachments to improve grip

# tools and tech used

ESP32 · C++ · PWM servo control · Fusion 360  · 3D printing (FDM)

# Demo

[![Quadruped Walking Demo](https://img.youtube.com/vi/dQw4w9WgXcQ/0.jpg)](https://www.youtube.com/watch?v=x4xqgKQPE7Q)

# Limitations and Future improvements

Quadruped tends to oscillate side ways during its gait: redesigned leg linkage to a 4 bar linkage , reduce limb inertia due to less mass along leg thus improving leg stability , and a higher mechancial advanatage

Implementation of IMU auto levelling in order to further improve stability, more gaits(bound , crawl), 

redesign of frame for cable management, and better ridgity

implement basic object detection (ultrasonic/ lidar)






