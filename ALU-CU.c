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

// Memory array (simulated)
unsigned char memory[0x1000];  // 4KB of memory

// Operation codes
#define ADD  0x1E    // Addition
#define SUB  0x1D    // Subtraction
#define MUL  0x1B    // Multiplication
#define SHL  0x1A    // Shift Left
#define WACC 0x30    // Write to ACC
#define RACC 0x31    // Read from ACC
#define WB   0x32    // Write Byte to MBR
#define WM   0x33    // Write MBR to Memory
#define RM   0x34    // Read Memory to MBR
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
            
        case SHL:  // Shift Left
            temp_ACC = (int)ACC << 1;
            ACC = (unsigned char)temp_ACC;
            break;
            
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
            
        case ADD:
        case SUB:
        case MUL:
        case SHL:
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