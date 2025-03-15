#include <stdio.h>

// members:
// Nino Angelo Balagtas - Leader
// Mac Dylan Philippe Balagtas

// Global control signals and data bus
unsigned char CONTROL; // Control signals for ALU operations
unsigned char BUS;     // Data bus
unsigned char MBR;     // Memory Buffer Register
unsigned char IOBR;    // I/O Buffer Register
unsigned char FLAGS;   // Flags register [OF -- -- -- SF CF ZF]
unsigned char IOM;     // I/O or Memory select
unsigned char RW;      // Read/Write control
unsigned char OE;      // Output Enable
unsigned char Memory;  // Memory operation flag
unsigned char IO;      // I/O operation flag
unsigned char Fetch;   // Fetch operation flag
unsigned short PC;     // Program Counter

// Memory arrays (simulated)
unsigned char memory[0x1000];   // 4KB of main memory
unsigned char io_memory[0x100]; // 256B of I/O memory

// Operation codes (based on Table 1)
// Arithmetic & Logical Operations
#define ADD 0x3C // 111100 - Add
#define SUB 0x3A // 111010 - Subtract
#define MUL 0x36 // 110110 - Multiply
#define AND 0x34 // 110100 - AND
#define OR 0x32  // 110010 - OR
#define NOT 0x30 // 110000 - NOT
#define XOR 0x2E // 101110 - XOR
#define SHL 0x2C // 101100 - Shift Left
#define SHR 0x2A // 101010 - Shift Right

// Data Movement
#define WM 0x02   // 000010 - Write Memory
#define RM 0x04   // 000100 - Read Memory
#define RIO 0x08  // 001000 - Read IO
#define WIO 0x0A  // 001010 - Write IO
#define WB 0x0C   // 001100 - Write Byte to MBR
#define WIB 0x0E  // 001110 - Write Byte to IOBR
#define WACC 0x12 // 010010 - Write to ACC
#define RACC 0x16 // 010110 - Read from ACC
#define SWAP 0x1C // 011100 - Swap MBR and IOBR

// Program Control
#define BR 0x06   // 000110 - Branch
#define BRE 0x28  // 101000 - Branch if Equal
#define BRNE 0x26 // 100110 - Branch if Not Equal
#define BRGT 0x24 // 100100 - Branch if Greater Than
#define BRLT 0x22 // 100010 - Branch if Less Than
#define EOP 0x3E  // 111110 - End of Program

// Function to calculate two's complement
unsigned char twosComp(unsigned char value)
{
    return ~value + 1;
}

// Function to set flags based on ALU operation result
void setFlags(int result)
{
    // Clear all flags first
    FLAGS &= 0x40; // Clear all flags except bit 6 which is unused

    // Set Zero Flag (ZF) - bit 0
    if ((result & 0xFF) == 0)
    {
        FLAGS |= 0x01;
    }

    // Set Carry Flag (CF) - bit 1
    if (result > 0xFF)
    {
        FLAGS |= 0x02;
    }

    // Set Sign Flag (SF) - bit 2
    if (result & 0x80)
    {
        FLAGS |= 0x04;
    }

    // Set Overflow Flag (OF) - bit 7
    if (result > 0xFF || result < 0)
    {
        FLAGS |= 0x80;
    }
}

// Main Memory Function
void MainMemory(void)
{
    if (Memory)
    {
        if (RW)
        { // Write operation
            memory[MBR] = BUS;
        }
        else
        { // Read operation
            if (OE)
            {
                BUS = memory[MBR];
            }
        }
    }
}

// I/O Memory Function
void IOMemory(void)
{
    if (IO)
    {
        if (RW)
        { // Write operation
            io_memory[MBR] = BUS;
        }
        else
        { // Read operation
            if (OE)
            {
                BUS = io_memory[MBR];
            }
        }
    }
}

// ALU Function
int ALU(void)
{
    static unsigned int ACC; // Accumulator as static unsigned int
    int temp_ACC;            // Temporary accumulator for calculations
    unsigned char temp_OP2;  // Temporary storage for second operand

    switch (CONTROL)
    {
    case ADD: // Addition
        temp_ACC = (int)ACC + BUS;
        ACC = (unsigned char)temp_ACC;
        break;

    case SUB: // Subtraction
        temp_OP2 = twosComp(BUS);
        temp_ACC = (int)ACC + temp_OP2;
        ACC = (unsigned char)temp_ACC;
        break;

    case MUL: // Multiplication
        temp_ACC = (int)ACC * BUS;
        ACC = (unsigned char)temp_ACC;
        break;

    case AND: // Logical AND
        temp_ACC = ACC & BUS;
        ACC = (unsigned char)temp_ACC;
        break;

    case OR: // Logical OR
        temp_ACC = ACC | BUS;
        ACC = (unsigned char)temp_ACC;
        break;

    case XOR: // Logical XOR
        temp_ACC = ACC ^ BUS;
        ACC = (unsigned char)temp_ACC;
        break;

    case NOT: // Logical NOT
        temp_ACC = ~ACC;
        ACC = (unsigned char)temp_ACC;
        break;

    case SHL: // Shift Left
        temp_ACC = (int)ACC << 1;
        ACC = (unsigned char)temp_ACC;
        break;

    case SHR: // Shift Right
        temp_ACC = (int)ACC >> 1;
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
void CU(unsigned char inst_code, unsigned short address)
{
    switch (inst_code)
    {
    case WB: // Write Byte to MBR
        Fetch = 1;
        Memory = 0;
        IO = 0;
        // MBR will be set by the next instruction byte
        break;

    case WIB: // Write Byte to IOBR
        Fetch = 1;
        Memory = 0;
        IO = 0;
        // IOBR will be set by the next instruction byte
        break;

    case WM: // Write MBR to Memory
        Fetch = 0;
        Memory = 1;
        IO = 0;
        IOM = 0;
        RW = 1;
        OE = 0;
        if (Memory)
        {
            memory[address] = MBR;
        }
        break;

    case RM: // Read Memory to MBR
        Fetch = 0;
        Memory = 1;
        IO = 0;
        IOM = 0;
        RW = 0;
        OE = 1;
        if (Memory)
        {
            MBR = memory[address];
        }
        break;

    case RIO: // Read IO
        Fetch = 0;
        Memory = 0;
        IO = 1;
        IOM = 1;
        RW = 0;
        OE = 1;
        if (IO)
        {
            IOBR = io_memory[address];
        }
        break;

    case WIO: // Write IO
        Fetch = 0;
        Memory = 0;
        IO = 1;
        IOM = 1;
        RW = 1;
        OE = 0;
        if (IO)
        {
            io_memory[address] = IOBR;
        }
        break;

    case SWAP: // Swap MBR and IOBR
    {
        unsigned char temp = MBR;
        MBR = IOBR;
        IOBR = temp;
    }
    break;

    case BR: // Unconditional Branch
        PC = address;
        break;

    // Arithmetic and Logic Operations
    case ADD:
    case SUB:
    case MUL:
    case AND:
    case OR:
    case XOR:
    case NOT:
    case SHL:
    case SHR:
        Fetch = 0;
        Memory = 1;
        IO = 0;
        CONTROL = inst_code;
        IOM = 0;
        RW = 0;
        OE = 0;
        if (Memory)
        {
            BUS = MBR;
            ALU();
        }
        break;

    // Branch Instructions
    case BRE: // Branch if Equal
        if (FLAGS & 0x01)
        { // Check Zero Flag
            PC = address;
        }
        break;

    case BRNE: // Branch if Not Equal
        if (!(FLAGS & 0x01))
        { // Check Zero Flag
            PC = address;
        }
        break;

    case BRGT: // Branch if Greater Than
        if (!(FLAGS & 0x01) && !(FLAGS & 0x04))
        { // Not Zero and Not Sign
            PC = address;
        }
        break;

    case BRLT: // Branch if Less Than
        if (FLAGS & 0x04)
        { // Check Sign Flag
            PC = address;
        }
        break;

    case WACC:
        Fetch = 0;
        Memory = 1;
        IO = 0;
        CONTROL = inst_code;
        if (Memory)
        {
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
int main()
{
    // Initialize system
    PC = 0;
    FLAGS = 0;

    // Program from Appendix A
    // First section: Initial operations and memory setup
    memory[0x000] = WB;
    memory[0x001] = 0x15; // WB 0x15
    memory[0x002] = WM;
    memory[0x003] = 0x04;
    memory[0x004] = 0x00; // WM 0x400
    memory[0x004] = WB;
    memory[0x005] = 0x05; // WB 0x05
    memory[0x006] = WACC; // WACC
    memory[0x008] = WB;
    memory[0x009] = 0x08; // WB 0x08
    memory[0x00A] = ADD;  // ADD
    memory[0x00C] = RM;
    memory[0x00D] = 0x04;
    memory[0x00E] = 0x00; // RM 0x400
    memory[0x00E] = MUL;  // MUL
    memory[0x010] = RACC; // RACC
    memory[0x012] = WM;
    memory[0x013] = 0x04;
    memory[0x014] = 0x01; // WM 0x401

    // I/O operations
    memory[0x014] = WIB;
    memory[0x015] = 0x0B; // WIB 0x0B
    memory[0x016] = WIO;
    memory[0x017] = 0x00;
    memory[0x018] = 0x00; // WIO 0x000
    memory[0x018] = WB;
    memory[0x019] = 0x10; // WB 0x10
    memory[0x01A] = SUB;  // SUB
    memory[0x01C] = RACC; // RACC
    memory[0x01E] = WIO;
    memory[0x01F] = 0x00;
    memory[0x020] = 0x01; // WIO 0x001

    // Shift operations
    memory[0x020] = SHL; // SHL
    memory[0x022] = SHL; // SHL
    memory[0x024] = RM;
    memory[0x025] = 0x04;
    memory[0x026] = 0x01; // RM 0x401
    memory[0x026] = SHR;  // SHR
    memory[0x028] = OR;   // OR
    memory[0x02A] = NOT;  // NOT

    // I/O and logical operations
    memory[0x02C] = RIO;
    memory[0x02D] = 0x00;
    memory[0x02E] = 0x01; // RIO 0x001
    memory[0x02E] = SWAP; // SWAP
    memory[0x030] = XOR;  // XOR
    memory[0x032] = WB;
    memory[0x033] = 0xFF; // WB 0xFF
    memory[0x034] = AND;  // AND

    // Branch operations
    memory[0x036] = RM;
    memory[0x037] = 0x04;
    memory[0x038] = 0x01; // RM 0x401
    memory[0x038] = BRE;
    memory[0x039] = 0x03;
    memory[0x03A] = 0x0C; // BRE 0x03C
    memory[0x03A] = WM;
    memory[0x03B] = 0x00;
    memory[0x03C] = 0xF0; // WM 0xF0
    memory[0x03C] = BRGT;
    memory[0x03D] = 0x04;
    memory[0x03E] = 0x00; // BRGT 0x040
    memory[0x03E] = BRLT;
    memory[0x03F] = 0x04;
    memory[0x040] = 0x04; // BRLT 0x044
    memory[0x040] = WB;
    memory[0x041] = 0x00; // WB 0x00 (unreachable)
    memory[0x042] = WACC; // WACC (unreachable)
    memory[0x044] = WB;
    memory[0x045] = 0x03; // WB 0x03
    memory[0x046] = WACC; // WACC

    // Controlled loop
    memory[0x048] = WB;
    memory[0x049] = 0x00; // WB 0x00
    memory[0x04A] = BRE;
    memory[0x04B] = 0x05;
    memory[0x04C] = 0x02; // BRE 0x052
    memory[0x04C] = WB;
    memory[0x04D] = 0x01; // WB 0x01
    memory[0x04E] = SUB;  // SUB
    memory[0x050] = BR;
    memory[0x051] = 0x04;
    memory[0x052] = 0x08; // BR 0x048
    memory[0x052] = EOP;  // EOP

    // Execute program
    unsigned char inst;
    unsigned short addr;

    while (1)
    {
        inst = memory[PC++];
        if (inst == EOP)
            break;

        // For instructions that need address
        if (inst == WM || inst == RM || inst == WIO || inst == RIO ||
            inst == BR || inst == BRE || inst == BRNE || inst == BRGT || inst == BRLT)
        {
            addr = (memory[PC] << 8) | memory[PC + 1];
            PC += 2;
        }

        CU(inst, addr);

        // Debug output
        printf("PC: 0x%03X, Inst: 0x%02X, ACC: 0x%02X, Flags: 0x%02X\n",
               PC, inst, ALU(), FLAGS);
    }

    // Print final results
    printf("\nFinal memory contents:\n");
    printf("Memory 0x400: 0x%02X\n", memory[0x400]);
    printf("Memory 0x401: 0x%02X\n", memory[0x401]);
    printf("I/O Buffer 0x000: 0x%02X\n", io_memory[0x000]);
    printf("I/O Buffer 0x001: 0x%02X\n", io_memory[0x001]);
    printf("Final Flags: ZF=%d CF=%d SF=%d OF=%d\n",
           (FLAGS & 0x01) ? 1 : 0,
           (FLAGS & 0x02) ? 1 : 0,
           (FLAGS & 0x04) ? 1 : 0,
           (FLAGS & 0x80) ? 1 : 0);

    return 0;
}