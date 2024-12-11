# Waffle-16
An Emulator for a fictional computer.
### Instruction Set:
 - 00 - No Operation                    00()
 - 01 - Move Val To Reg                 01(w-reg, r-val)
 - 02 - Move Reg To Reg                 02(w-reg, r-reg)
 - 03 - Jump to Val                     03(addr-val x2)
 - 04 - Jump to Reg                     04(addr-reg x2)
 - 05 - Branch if Reg Equals Val        05(addr-reg x2, c-reg, c-val)
 - 06 - Branch if Reg !Equals Val       06(addr-reg x2, c-reg, c-val)
 - 07 - Store Reg to RAM @ Val          07(r-reg, addr-val x2)
 - 08 - Load RAM to Reg @ Val           08(w-reg, addr-val x2)
 - 09 - Store Reg to RAM @ Reg          09(r-reg, addr-reg x2)
 - 0A - Load RAM to Reg @ Reg           0A(w-reg, addr-reg x2)
 - 0B - Store Reg to Video RAM @ Val    0B(r-reg, addr-val x2)
 - 0C - Load Video RAM to Reg @ Val     0C(w-reg, addr-val x2)
 - 0D - Store Reg to Video RAM @ Reg    0D(r-reg, addr-reg x2)
 - 0E - Load Video RAM to Reg @ Reg     0E(w-reg, addr-reg x2)
 - 0F - Return                          0F()
 - 10 - 
 - FF - Halt!
