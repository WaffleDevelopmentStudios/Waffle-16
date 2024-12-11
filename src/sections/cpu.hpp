class CPU_Core {
public:
	u16 registers[32];
	u16 stack[65536];
	u16 bookkeepingStack[256];

	void execute() {
		u32 epc = (this->registers[0x1E] << 16) + this->registers[0x1F]; // Effective Program Counter
		u16 ins = ram[this->registers[0x1F] + 0];                  // Instruction
		u16 p1  = ram[this->registers[0x1F] + 1];                  // Parameter 1
		u16 p2  = ram[this->registers[0x1F] + 2];                  // Parameter 2
		u16 p3  = ram[this->registers[0x1F] + 3];                  // Parameter 3
		if   (ins == 0x0000) {      // No Operation
			epc += 1;
		} else if (ins == 0x0001) { // Move Val to Reg
			this->registers[p1] = p2;
			epc += 3;
		} else if (ins == 0x0002) { // Move Reg To Reg
			this->registers[p1] = this->registers[p2];
			epc += 3;
		} else if (ins == 0x0003) { // Jump to Val
			bookkeepingStack[registers[0x1C]] = epc;
			registers[0x1C]++;
			epc = p1;
		} else if (ins == 0x0004) { // Jump to Reg
			bookkeepingStack[registers[0x1C]] = epc;
			registers[0x1C]++;
			epc = this->registers[p1];
		} else if (ins == 0x0005) { // Branch if Reg Equals Val
			if (this->registers[p2] == p3) { epc = registers[p1]; bookkeepingStack[registers[0x1C]] = epc; registers[0x1C]++; }
			else { epc += 4; }
		} else if (ins == 0x0006) { // Branch if Reg !Equals Val
			if (!(this->registers[p2] == p3)) { epc = registers[p1]; bookkeepingStack[registers[0x1C]] = epc; registers[0x1C]++; }
			else { epc += 4; }
		} else if (ins == 0x0007) { // Store Reg to RAM @ Val
			u32 ela = (p2 << 16) + p3;
			ram[ela] = this->registers[p1];
			epc += 4;
		} else if (ins == 0x0008) { // Load RAM to Reg @ Val
			u32 ela = (p2 << 16) + p3;
			this->registers[p1] = ram[ela];
			epc += 4;
		} else if (ins == 0x0009) { // Store Reg to RAM @ Reg
			u32 ela = (this->registers[p2] << 16) + this->registers[p3];
			ram[ela] = this->registers[p1];	
			epc += 4;
		} else if (ins == 0x000A) { // Load RAM to Reg @ Reg
			u32 ela = (this->registers[p2] << 16) + this->registers[p3];
			this->registers[p1] = ram[ela];
			epc += 4;
		} else if (ins == 0x000B) { // Store Reg to Video RAM @ Val
			u32 ela = (p2 << 16) + p3;
			vram[ela] = this->registers[p1];
			epc += 4;
		} else if (ins == 0x000C) { // Load Video RAM to Reg @ Val
			u32 ela = (p2 << 16) + p3;
			this->registers[p1] = vram[ela];
			epc += 4;
		} else if (ins == 0x000D) { // Store Reg to Video RAM @ Reg
			u32 ela = (this->registers[p2] << 16) + this->registers[p3];
			vram[ela] = this->registers[p1];
			epc += 4;
		} else if (ins == 0x000E) { // Load Video RAM to Reg @ Reg
			u32 ela = (this->registers[p2] << 16) + this->registers[p3];
			this->registers[p1] = vram[ela];
			epc += 4;
		} else if (ins == 0x000F) { // Return
			epc = bookkeepingStack[registers[0x1C]];
			registers[0x1C]--;
		} else if (ins == 0x0010) { // Push
			stack[registers[0x1D]] = registers[p1];
			registers[0x1D]++;
			epc += 2;
		} else if (ins == 0x0011) { // Pop
			registers[p1] = stack[registers[0x1D]];
			registers[0x1D]--;
			epc += 2;
		}

		// Halt
		if (ins == 0xFF) {
			std::cout << "Halt Instruction Called. Stopping." << std::endl;
			state.quit = true;
		}
		if (epc >= (ramWidth / 2)) epc = 0;
		this->registers[0x1F] = epc & 0x0000FFFF;
		this->registers[0x1E] = (epc & 0xFFFF0000) >> 16;
        return;
	}
};
