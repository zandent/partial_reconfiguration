# partial_reconfiguration
Based on microblaze, load partial bitstream from BRAM to ICAP to partial reconfigures FPGA

## Before started
### 1. install minicom on PC
```
sudo apt-get install minicom
```
### 2. install binary file transfer utility for minicom (ONLY for loading bitstream through UART terminal)
  (1) Install pv utility:
 ```
 sudo apt-get install pv
 ```
  (2) Import the script "bin_transfer" in ~/bin/ directory
  
  (3) Settings in minicom. In minicom terminal, hit ctrl & A, then hit 'o' to open minicom configuration. Enter File transfer protocols. Add a new section as following:
  
  * Name: Binary
  * Program: ~/bin/bin_transfer -o %l
  * Name: Y
  * U/D:  U
  * FullScr: Y
  * IO-Red: N
  * Multi: N

## create project with partial reconfiguration
### partial reconfig non-ip type 

### partial reconfig ip type (HLS core)
