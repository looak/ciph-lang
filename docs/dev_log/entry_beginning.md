## Entry Beginning
### 2024-04-15: Created the dev log

#### new entry @2024-04-15
Thinking I will going deeper with my virtual machine and simulate a CPU at a bit lower level with.

Thinking way too hard on the instruction set of the virtual CPU I want to create. Is this a waste of time or actually valuable?

#### new entry @2024-04-15
Yes, spent quite some time on this topic, and it will most likely change. I gave myself space to grow, hopefully I'll never need more than 255 instructions.

#### new entry @5pm
Didn't take long for me to realize my misstake. I've been thinking that my instructions will be no longer than 2 bytes, but they can be 3 bytes, since I'm dealing with a 16bit system, my memory addresses will be 2 bytes.

PSH = 1byte (only the instruction)
MOV_REG = 2 byte (1 byte for instruction, 1 byte for register identifiers)
PUT_MEM = 3 byte (1 byte for instruction, 2 byte for memory address)

#### new entry @2024-04-16
Been hacking away at the new architecture, and I'm pleased to have gotten push, pop and some of the arithmetic instructions against the stack to work.
Gonna address the peek instruction now.