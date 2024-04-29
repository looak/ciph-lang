## Instruction Layout
Most Instructions will follow a simple layout consisting of at least 7 bits. Instructions where we can operate against either the stack or a register, the last bit will dictate which of these the instruction is operating against.

The following byte will hold the register or memory we're working against.

$$
    byte \to        
        [1bit]_{stack}
        [7bits]_{opcode}\\        
    byte \to    
        \begin{cases}    
        [4bits]_{reg}[4bits]_{reg}\\
        [8bits]_{memory || literal}    
        \end{cases}\\
        byte \to        
        [8bits]_{memory || literal}
$$


### List of Instructions:

Move instructions, first bit is dictating that we are using memory addresses or literals.

| *mnemonic and input* | *hex* | size | *description* |
|--|--|--|--|
|PUT, mem | `0x01` | 3 bytes | Takes the memory at the location and puts it in `imm` register.
|PUT, reg | `0x04` | 2 bytes | Takes value of `imm` and puts it in given register.
|PUT, lit | `0x05` | 3 bytes | Move literal into `imm` register.
|MOV, rX, rY| `0x02`| 2 bytes | Move data from register rY to register rX, if rY is unspecified moves rX into `imm` register. |
|MOV, mem | `0x03` | 3 bytes | Move data from `imm` register into given Memory. |


#### Basic arithmetic instructions

 | *mnemonic and input* | *hex* | size | *description* |
 |--|--|--|--|
 | ADD | `0x06` | 1 byte | Pops two values from the stack and adds them together. Result is pushed onto stack.
 | ADD, rX, rY | `0x07` | 2 byte | Takes values at registers, adds them together and puts result into rX. If rY is unspecified adds `imm` to rX.
 SUB | `0x08` | 1 byte | Subtracts second value on stack with first and pushes difference onto stack.
 SUB, rX, rY | `0x09` | 2 bytes | Subtracts rX with rY and puts difference into rX. If rY is unspecified uses `imm`.
 | MUL | `0x0A` | 1 byte | Pops two values from the stack and multiplies them together.
 | MUL, rX, rY | `0x0B` | 2 bytes | Takes values at registers, multiplies them together and puts product into rX. If rY is unspecified uses `imm` as multiplier, still stores product in rX.
 DIV | `0x0C` | 1 byte | Pops divisor of the stack, then pops dividen of the stack. Puts quotient of the divison onto stack. Remainder is lost.
 DIV, rX, rY | `0x0D` | 2 bytes | Uses register rX as dividen and rY as divisor. Quotient is put into rX. Remainder is lost. If rY is unspecified uses `imm` as divisor.
 INC, reg, 8bit lit | `0x0E` | 2 bytes | Increases the value at given register by one, if reg:sp is given, value on stack is incremented at given offset.
 DEC, reg, 8bit lit | `0x0F` | 2 bytes | Same as increment, just decreases the value by one.
 
#### Stack instructions
| *mnemonic and input* | *hex* | *description* |
|--|--|--|
| PSH | `0x10` | Pushes data from `imm` register onto stack.
| PSH, reg | `0x20` | Pushes data from reg onto stack.
| PSH, lit | `0x11` | Pushes literal onto stack. |
| PSH, mem | `0x21` | Takes value at memory adress and pushes it onto stack.
| POP | `0x30` | Pops top of stack to given `imm` register
| POP, reg | `0x40` | Pops top of stack to given register.
| PEK, reg | `0x50` | Copies top value of stack into given register |
| PEK, reg, 8bit lit | `0x51` | Copies value of stack at offset into given register, if reg:sp is given, value is pushed onto stack. |



#### Control flow instructions
| *mnemonic and input* | *hex* | *description* |
|--|--|--|
JMP, address | `0xC0` | Unconditionally jump to address.
JNZ, address | `0xC1` | Jump to address if `imm` is not zero.
RET, address | `0xC2` | Returns value in `ret` and terminates the program.

#### Other instructions
| *mnemonic and input* | *hex* | *description* |
|--|--|--|
NOP | `0x00` | No operation instruction, program counter should just tick pass this.

### Registers
| register | hex | description |
|--|--|--|
`imm` | `0x00` | immediate register
`r0` to `r7` | `0x01` to `0x08` | all purpose registers
| `pc` | `0xAA` | program counter
| `sp` | `0xAB` | stack pointer
| `fp` | `0xBA` | stack frame pointer



SWAP (R1, R2): Swap the contents of registers R1 and R2. (Can be implemented using PUSH and POP)

AND (R1, R2): Bitwise AND operation between R1 and R2, store result in R1. 
OR (R1, R2): Bitwise OR operation between R1 and R2, store result in R1. 
XOR (R1, R2): Bitwise XOR operation between R1 and R2, store result in R1. 
NOT (R1): Bitwise NOT operation on R1, store result in R1. 
2. Stack Operations (4 instructions):


3. Arithmetic and Logical Operations (4 instructions):

CMP (R1, R2): Compare data in R1 and R2, set status flags (e.g., Zero, Carry) based on the result. 
INC (R1): Increment the value in register R1 by 1. 
DEC (R1): Decrement the value in register R1 by 1. 
SHL (R1): Shift the data in R1 left by 1 bit.  (Optional)
SHR (R1): Shift the data in R1 right by 1 bit.  (Optional)
