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
The necessary IP cores (Example block design based on Nexy4DDR board shown in "example_bd.png"):

  * microblaze
  * AXI interconnect
  * AXi HWICAP
  * AXI BRAM controller
  * AXI UART (depands on board)
  
### Partial reconfigure non-IP type 
Normally, create a PR wizard and draw pblock, then generate full and partial bitstreams
### Partial reconfigure customized IP type module(such as HLS IP core)
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
  (4) Set the modified IP module as black box
```
update_design -cell <block design module>/<IP module> -black_box
```
  (5) Set the IP module as black box and lock the design
```
lock_design -level routing
```
  (6) Write the checkpoint as static design
```
write_checkpoint -force <checkpointname1>.dcp
```
  (7) Generate full and partial bitstream (Do not click "generate bitstream" on the left. It cannot generate partial bitstream. Only to do is unput the following command in the console). After this step, the full and partial bitstream will be generated in current work reopsitory
```
write_bitstream <fullbitstreamname>.bit
```
  (8) Next copy this project as a new project, replace the PR region with a new IP core, which is the same as partial reconfiguring non_ip module. Then generate output products for the block design and synthesis the design
  
  (9) Open previous project checkpoint. Vivado will pop out a new window for the implement design of previous project
```
open_checkpoint <static design checkpoint directory>/<checkpointname1>.dcp
```
  (10) Set the modified IP module as black box, which is the same as (4)

  (11) In the new window console (Caution: Even if the terminals of the two windows' consoles are the same, the following command must be based on the previous project in the new window!!!), read the checkpoint of the new project's modified IP module
```
read_checkpoint -cell <block design module>/<IP module> <child design synthsis directory>/<IP module's dcp file>
``` 
  (11) Implement the design and generate bitstreams, which is the same as (3) and (7)
  
  (12) Convert partial bitstram files (*.bit) to binary files (*.bin)
```
vivado -mode tcl
write_cfgmem -format bin -size 32 -loadbit "up 0x0 <PR bitfile directory>/<PR bitfiles>" -file <binfile name>.bin disablebitswap
```
## Programming on board
###1. After export hardware, launch SDK. In SDK, import partial_reconfig.c in the software application and run
###2. open minicom to connect UART terminal
```
sudo minicom -D /dev/<UART terminal> -b 115200 -8 -o
```
###3. Follow the menu to load binary files. (Note: hit crtl & a then hit s, select Binary, then select partial binary file to load)
