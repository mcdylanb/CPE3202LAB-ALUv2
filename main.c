// Global control signals and data bus
unsigned char CONTROL;  // Control signals for ALU operations
unsigned char BUS;      // Data bus
unsigned char MBR;      // Memory Buffer Register
unsigned char FLAGS;    // Flags register [OF -- -- -- SF CF ZF]

// Function to calculate two's complement of a number
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
    // Overflow occurs when result cannot be represented in 8 bits
    if(result > 0xFF || result < 0) {
        FLAGS |= 0x80;
    }
}

int ALU(void) {
    static unsigned int ACC;  // Accumulator as static unsigned int for unsigned operations only
    int temp_ACC;            // Temporary accumulator for calculations
    unsigned char temp_OP2;  // Temporary storage for second operand

    // ADD (0x1E) or SUB (0x1D) operations
    if(CONTROL == 0x1E || CONTROL == 0x1D) {
        // Check if operation is SUB
        if(CONTROL == 0x1D) {
            temp_OP2 = twosComp(BUS);  // 2's complement for subtraction
        } else {
            temp_OP2 = BUS;  // Direct value for addition
        }
        
        // Perform addition
        temp_ACC = (int)ACC + temp_OP2;
        ACC = (unsigned char)temp_ACC;
    }
    // Other operations will be implemented here
    
    setFlags(temp_ACC);  // Set flags based on operation result
    return ACC;  // Return the accumulator value
}
