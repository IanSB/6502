##########################################################################
#
# Processor specific code

CPU = "6811"
Description = "FreeScale 68HC11 8-bit microcontroller."
DataWidth = 8 # 8-bit data
AddressWidth = 16 # 16-bit addresses

# Maximum length of an instruction (for formatting purposes)
maxLength = 5;

# Leadin bytes for multbyte instructions
leadInBytes = [0x18, 0x1a, 0xcd]

# Op Code Table
# Key is numeric opcode (possibly multiple bytes)
# Value is a list:
#   # bytes
#   mnemonic
#   addressing mode.
opcodeTable = {
0x00   :  [ 1, "test", "inherent"        ],
0x01   :  [ 1, "nop",  "inherent"        ],
0x02   :  [ 2, "ora",  "direct"          ],
0x03   :  [ 3, "jmp",  "extended"        ],
0x183a :  [ 2, "aby",  "inherent"        ],
0x18a9 :  [ 5, "adca", "indirecty"       ],
}

# Addressing mode table
addressmodeTable = {
"inherent"  : "",
"immediate" : "#${0:02X}",
"direct"    : "${0:02X}{1:02X}",
"extended"  : "",
"indirectx" : "",
"indirecty" : "($:0:02X)),Y",
"relative"  : "",
}


# End of processor specific code
##########################################################################
