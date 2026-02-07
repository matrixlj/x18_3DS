ENTRY(_start)

SECTIONS
{
    . = 0;
    
    .text : {
        KEEP(*(.text.start))
        *(.text*)
        *(.rodata*)
    }
    
    .data : {
        *(.data*)
    }
    
    .bss : {
        *(.bss*)
        *(COMMON)
    }
    
    /DISCARD/ : {
        *(.comment)
        *(.gnu*)
    }
}


