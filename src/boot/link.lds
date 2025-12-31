/* RISC-V Linker Script
 * Define program layout in memory
 */

/* Program entry point */
ENTRY(_start)

/* Memory layout definition */
MEMORY {
    /* QEMU virt machine memory layout
     * 0x80000000-0x80200000: OpenSBI firmware
     * 0x80200000+: kernel space
     */
    RAM : ORIGIN = __LOAD_ADDR__, LENGTH = 128M
}

SECTIONS {
    /* Set starting address */
    . = __LOAD_ADDR__;

    /* Text section - contains executable instructions */
    .text : {
        /* Ensure _start is at the front */
        *(.text._start)
        *(.text)
        *(.text.*)
    } > RAM

    /* Syscall Gateway - Fixed at 4KB offset from start */
    . = __LOAD_ADDR__ + 0x4000;
    .syscall_gateway : {
        PROVIDE(__syscall_gateway_start = .);
        *(.syscall_gateway)
        . = ALIGN(4096);
        PROVIDE(__syscall_gateway_end = .);
    } > RAM

    /* Read-only data section - contains constants and strings */
    .rodata : {
        . = ALIGN(8);
        *(.rodata)
        *(.rodata.*)
        *(.srodata)
        *(.srodata.*)
    } > RAM

    /* Initialized data section - contains initialized global variables */
    .data : {
        . = ALIGN(8);
        *(.data)
        *(.data.*)
        *(.sdata)
        *(.sdata.*)
    } > RAM

    /* Uninitialized data section - contains uninitialized global variables */
    .bss : {
        . = ALIGN(8);
        
        /* Provide BSS section boundary symbols for clear_bss function */
        PROVIDE(__bss_start = .);
        *(.bss)
        *(.bss.*)
        *(.sbss)
        *(.sbss.*)
        
        /* Reserve space for symbols not explicitly allocated */
        *(COMMON)
        
        . = ALIGN(8);
        PROVIDE(__bss_end = .);
    } > RAM

    /* Heap space marker (reserved for dynamic memory allocation) */
    .heap : {
        . = ALIGN(4096);              /* Page alignment for future MMU */
        PROVIDE(__heap_start = .);
        /* Heap end is determined by runtime memory size, not defined here */
    } > RAM

    /* Discard unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.gnu*)
        *(.note*)
        *(.eh_frame*)
    }
}

/* Define some useful symbols */
PROVIDE(__text_start = ADDR(.text));
PROVIDE(__text_end = ADDR(.text) + SIZEOF(.text));
PROVIDE(__data_start = ADDR(.data));
PROVIDE(__data_end = ADDR(.data) + SIZEOF(.data));