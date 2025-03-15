#include <stdio.h>

// Global control signals and data bus
unsigned char CONTROL;    // Control signals for ALU operations
unsigned char BUS;       // Data bus
unsigned char MBR;       // Memory Buffer Register
unsigned char FLAGS;     // Flags register [OF -- -- -- SF CF ZF]
unsigned char IOM;       // I/O or Memory select
unsigned char RW;        // Read/Write control
unsigned char OE;        // Output Enable
unsigned char Memory;    // Memory operation flag
unsigned char IO;        // I/O operation flag
unsigned char Fetch;     // Fetch operation flag
unsigned short PC;       // Program Counter

// Memory arrays (simulated)
unsigned char memory[0x1000];  // 4KB of main memory
unsigned char io_memory[0x100]; // 256B of I/O memory

// Operation codes
#define ADD  0x1E    // Addition
#define SUB  0x1D    // Subtraction
#define MUL  0x1B    // Multiplication
#define DIV  0x1C    // Division
#define AND  0x19    // Logical AND
#define OR   0x18    // Logical OR
#define XOR  0x17    // Logical XOR
#define NOT  0x16    // Logical NOT
#define SHL  0x1A    // Shift Left
#define SHR  0x15    // Shift Right
#define WACC 0x30    // Write to ACC
#define RACC 0x31    // Read from ACC
#define WB   0x32    // Write Byte to MBR
#define WM   0x33    // Write MBR to Memory
#define RM   0x34    // Read Memory to MBR
#define CMP  0x14    // Compare
#define BRE  0x40    // Branch if Equal
#define BRNE 0x41    // Branch if Not Equal
#define BRGT 0x42    // Branch if Greater Than
#define BRLT 0x43    // Branch if Less Than
#define EOP  0xFF    // End of Program

// Function to calculate two's complement
unsigned char twosComp(unsigned char value) {
    return ~value + 1;
}

// Function to set flags based on ALU operation result
void setFlags(int result) {
    // Clear all flags first
    FLAGS &= 0x40;  // Clear all flags except bit 6 which is unused
    
    // Set Zero Flag (ZF) - bit 0
    if((result & 0xFF) == 0) {
        FLAGS |= 0x01;
    }
    
    // Set Carry Flag (CF) - bit 1
    if(result > 0xFF) {
        FLAGS |= 0x02;
    }
    
    // Set Sign Flag (SF) - bit 2
    if(result & 0x80) {
        FLAGS |= 0x04;
    }
    
    // Set Overflow Flag (OF) - bit 7
    if(result > 0xFF || result < 0) {
        FLAGS |= 0x80;
    }
}

// Main Memory Function
void MainMemory(void) {
    if(Memory) {
        if(RW) {  // Write operation
            memory[MBR] = BUS;
        } else {  // Read operation
            if(OE) {
                BUS = memory[MBR];
            }
        }
    }
}

// I/O Memory Function
void IOMemory(void) {
    if(IO) {
        if(RW) {  // Write operation
            io_memory[MBR] = BUS;
        } else {  // Read operation
            if(OE) {
                BUS = io_memory[MBR];
            }
        }
    }
}

// ALU Function
int ALU(void) {
    static unsigned int ACC;  // Accumulator as static unsigned int
    int temp_ACC;            // Temporary accumulator for calculations
    unsigned char temp_OP2;  // Temporary storage for second operand

    switch(CONTROL) {
        case ADD:  // Addition
            temp_ACC = (int)ACC + BUS;
            ACC = (unsigned char)temp_ACC;
            break;
            
        case SUB:  // Subtraction
            temp_OP2 = twosComp(BUS);
            temp_ACC = (int)ACC + temp_OP2;
            ACC = (unsigned char)temp_ACC;
            break;
            
        case MUL:  // Multiplication
            temp_ACC = (int)ACC * BUS;
            ACC = (unsigned char)temp_ACC;
            break;
            
        case DIV:  // Division
            if(BUS != 0) {
                temp_ACC = (int)ACC / BUS;
                ACC = (unsigned char)temp_ACC;
            }
            break;
            
        case AND:  // Logical AND
            temp_ACC = ACC & BUS;
            ACC = (unsigned char)temp_ACC;
            break;
            
        case OR:   // Logical OR
            temp_ACC = ACC | BUS;
            ACC = (unsigned char)temp_ACC;
            break;
            
        case XOR:  // Logical XOR
            temp_ACC = ACC ^ BUS;
            ACC = (unsigned char)temp_ACC;
            break;
            
        case NOT:  // Logical NOT
            temp_ACC = ~ACC;
            ACC = (unsigned char)temp_ACC;
            break;
            
        case SHL:  // Shift Left
            temp_ACC = (int)ACC << 1;
            ACC = (unsigned char)temp_ACC;
            break;
            
        case SHR:  // Shift Right
            temp_ACC = (int)ACC >> 1;
            ACC = (unsigned char)temp_ACC;
            break;
            
        case CMP:  // Compare
            temp_ACC = (int)ACC - BUS;
            // Only set flags, don't modify ACC
            setFlags(temp_ACC);
            return ACC;
            
        case WACC: // Write to ACC
            ACC = BUS;
            temp_ACC = ACC;
            break;
            
        case RACC: // Read from ACC
            MBR = ACC;
            temp_ACC = ACC;
            break;
    }
    
    setFlags(temp_ACC);
    return ACC;
}

// Control Unit Function
void CU(unsigned char inst_code, unsigned short address) {
    switch(inst_code) {
        case WB:    // Write Byte to MBR
            Fetch = 1;
            Memory = 0;
            IO = 0;
            // MBR will be set by the next instruction byte
            break;
            
        case WM:    // Write MBR to Memory
            Fetch = 0;
            Memory = 1;
            IO = 0;
            IOM = 0;
            RW = 1;
            OE = 0;
            if(Memory) {
                memory[address] = MBR;
            }
            break;
            
        case RM:    // Read Memory to MBR
            Fetch = 0;
            Memory = 1;
            IO = 0;
            IOM = 0;
            RW = 0;
            OE = 1;
            if(Memory) {
                MBR = memory[address];
            }
            break;
            
        // Arithmetic and Logic Operations
        case ADD:
        case SUB:
        case MUL:
        case DIV:
        case AND:
        case OR:
        case XOR:
        case NOT:
        case SHL:
        case SHR:
        case CMP:
            Fetch = 0;
            Memory = 1;
            IO = 0;
            CONTROL = inst_code;
            IOM = 0;
            RW = 0;
            OE = 0;
            if(Memory) {
                BUS = MBR;
                ALU();
            }
            break;
            
        // Branch Instructions
        case BRE:   // Branch if Equal
            if(FLAGS & 0x01) {  // Check Zero Flag
                PC = address;
            }
            break;
            
        case BRNE:  // Branch if Not Equal
            if(!(FLAGS & 0x01)) {  // Check Zero Flag
                PC = address;
            }
            break;
            
        case BRGT:  // Branch if Greater Than
            if(!(FLAGS & 0x01) && !(FLAGS & 0x04)) {  // Not Zero and Not Sign
                PC = address;
            }
            break;
            
        case BRLT:  // Branch if Less Than
            if(FLAGS & 0x04) {  // Check Sign Flag
                PC = address;
            }
            break;
            
        case WACC:
            Fetch = 0;
            Memory = 1;
            IO = 0;
            CONTROL = inst_code;
            if(Memory) {
                BUS = MBR;
                ALU();
            }
            break;
            
        case RACC:
            Fetch = 0;
            Memory = 1;
            IO = 0;
            CONTROL = inst_code;
            ALU();
            break;
    }
}

// Test program
int main() {
    // Initialize system
    PC = 0;
    FLAGS = 0;
    
    // Example program from Appendix A (machine code)
    memory[0x000] = WB;    memory[0x001] = 0x05;  // WB 0x05
    memory[0x002] = WM;    memory[0x003] = 0x04;  memory[0x004] = 0x00;  // WM 0x400
    memory[0x005] = WB;    memory[0x006] = 0x03;  // WB 0x03
    memory[0x007] = WM;    memory[0x008] = 0x04;  memory[0x009] = 0x01;  // WM 0x401
    memory[0x00A] = RM;    memory[0x00B] = 0x04;  memory[0x00C] = 0x00;  // RM 0x400
    memory[0x00D] = WACC;  // WACC
    memory[0x00E] = RM;    memory[0x00F] = 0x04;  memory[0x010] = 0x01;  // RM 0x401
    memory[0x011] = ADD;   // ADD
    memory[0x012] = RACC;  // RACC
    memory[0x013] = WM;    memory[0x014] = 0x04;  memory[0x015] = 0x02;  // WM 0x402
    memory[0x016] = EOP;   // EOP
    
    // Execute program
    unsigned char inst;
    unsigned short addr;
    
    while(1) {
        inst = memory[PC++];
        if(inst == EOP) break;
        
        // For instructions that need address
        if(inst == WM || inst == RM) {
            addr = (memory[PC] << 8) | memory[PC + 1];
            PC += 2;
        }
        
        CU(inst, addr);
    }
    
    // Print results
    printf("Final memory contents:\n");
    printf("0x400: %02X\n", memory[0x400]);
    printf("0x401: %02X\n", memory[0x401]);
    printf("0x402: %02X\n", memory[0x402]);
    
    return 0;
} 