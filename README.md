# partial_reconfiguration
Based on microblaze, load partial bitstream from BRAM to ICAP to partial reconfigures FPGA

#Before started
1. install minicom on PC
```
  sudo apt-get install minicom
```
2. install binary file transfer utility for UART (ONLY for loading bitstream through UART terminal)
  (1) Install pv utility:
 ```
    sudo apt-get install pv
 ```
  (2) Import the script "bin_transfer" in ~/bin/ directory
    
