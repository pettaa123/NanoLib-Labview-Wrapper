This Project contains a wrapper dll for NanoLIB 1.0.1.
Furthermore it consists a VI, which covers the most important
settings for ProfilePosition and Velocity Mode.

#UI:

![Nanotec Wrapper](https://github-production-user-asset-6210df.s3.amazonaws.com/31046837/289487454-8fa34cff-83ad-47a4-b2d9-93ce07b936ac.PNG?raw=true)

#Prerequisites:
In order to build the wrapper dll one need the magic-enum header only library.
The VI is LabVIEW 2023 Q3 64-bit

Enable USB control of controller by inserting those 2 lines into CFG.txt

2102|=0x100000
4015:01=0

Tested with Firmware: C5-E-x-09-FIR-v2213-B1031134.fw
