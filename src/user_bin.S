# RISC-V Kernel - Embedded User Program
# This file includes the user program binary into the kernel's data section.

.section .data
.align 8
.global _user_prog_start
.global _user_prog_end

_user_prog_start:
    .incbin "../user/user_prog.bin"
_user_prog_end:
