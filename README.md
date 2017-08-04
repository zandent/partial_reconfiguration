# Partial Reconfiguration
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

## Create project with partial reconfiguration
### partial reconfig non-ip type 
Normally, create a PR wizard and draw pblock, then generate full and partial bitstreams
### partial reconfig customized IP type module(such as HLS IP core)
  (1) Synthesis the project and draw proper pblock for selected IP type module
  
  (2) Set IP module as PR, reset as 1 and snapping mode as on
```    
set_property HD.RECONFIGURABLE 1 [get_cells <block design module>/<IP module>]
set_property RESET_AFTER_RECONFIG 1 [get_pblocks <IP module pblock name>]
set_property SNAPPING_MODE ON [get_pblocks <IP module pblock name>]
```    
  (3) Implement the design
```
opt_design
place_design
route_design
```
  (4) Set the IP module as black box and lock the design
```
lock_design -level routing
```
  (5) Write the checkpoint as static design
```
write_checkpoint -force <checkpointname1>.dcp
```
  (6) Generate full and partial bitstream (Do not click "generate bitstream" on the left. It cannot generate partial bitstream. Only to do is unput the following command in the console). After this step, the full and partial bitstream will be generated in current work reopsitory
```
write_bitstream <fullbitstreamname>.bit
```
  (7) Next copy this project as a new project, replace the PR region with a new IP core






