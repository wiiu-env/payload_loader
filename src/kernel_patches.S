    .globl SC_0x09_SETIBAT0
SC_0x09_SETIBAT0:
    li r0, 0x0900
    sc
    blr

    .globl SC_0x36_SETBATS
SC_0x36_SETBATS:
    li r0, 0x3600
    sc
    blr
