[BITS 32]			; 32 bit code
[global start]		; make 'start' function global
[extern kmain]		; our C kernel main

[extern start_ctors]	; beginning and end
[extern end_ctors]		; of the respective
[extern start_dtors]	; ctors and dtors section,
[extern end_dtors]		; declare by the linker script

; setting up the Multiboot header - see GRUB docs for details
MULTIBOOT_PAGE_ALIGN	equ 1<<0
MULTIBOOT_MEMORY_INFO	equ 1<<1
MULTIBOOT_HEADER_MAGIC	equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS	equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
MULTIBOOT_CHECKSUM		equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

; Multiboot header (needed to boot from GRUB)
ALIGN 4
multiboot_header:
	dd MULTIBOOT_HEADER_MAGIC
	dd MULTIBOOT_HEADER_FLAGS
	dd MULTIBOOT_CHECKSUM
	
section .setup
; the kernel entry point
start:
	; here's the trick: we load a GDT with a base address
	; of 0x40000000 for the code (0x08) and data (0x10) segments
	lgdt [trickgdt]
	mov cx, 0x10
	mov ds, cx
	mov es, cx
	mov fs, cx
	mov gs, cx
	mov ss, cx
	
	; jump to the higher half kernel
	jmp 0x08:higherhalf
	
section .text
higherhalf:
	; from now the CPU will translate automatically every address
	; by adding the base 0x40000000
	
	mov esp, sys_stack	; set up a new stack for our kernel
	
	push eax			; pass multiboot magic number
	push ebx			; pass multiboot header pointer
	
	mov ebx, start_ctors	; call the constructors
    jmp .ctors_until_end
.call_constructor:
    call [ebx]
    add ebx,4
.ctors_until_end:
    cmp ebx, end_ctors
    jb .call_constructor

	call kmain			; jump to our c kernel
	
	mov ebx, end_dtors	; call the destructors
    jmp .dtors_until_end
.call_destructor:
    sub ebx, 4
    call [ebx]
.dtors_until_end:
    cmp ebx, start_dtors
    ja .call_destructor

    cli
	jmp $				; just a simple protection
	
[global gdt_flush]		; make gdt_flush accessible from C code

; this function does the same thing of the 'start' one, this time with
; the real GDT
gdt_flush:
	mov eax, [esp+4]	; Get the pointer to the GDT, passed as a parameter.
	lgdt [eax]			; Load the new GDT pointer.
	
	mov ax, 0x10		; 0x10 is the offset in the GDT to our data segment
	mov ds, ax			; Load all data segment selectors
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	jmp 0x08:.flush		; 0x08 is the offset to our code segment: Far jump!
.flush:
	ret
	
[global idt_flush]		; make idt_flush accessible from C code

idt_flush:
	mov eax, [esp+4]	; Get the pointer to the IDT, passed as a parameter.
	lidt [eax]			; Load the IDT pointer.
	ret

[section .setup]	; tells the assembler to include this data in the '.setup' section

trickgdt:
	dw gdt_end - gdt	; size of the GDT
	dd gdt				; linear address of GDT
	
gdt:
	dd 0, 0
	db 0xFF, 0xFF, 0, 0, 0, 10011010b, 11001111b, 0x40	; code selector 0x08: base 0x40000000, limit 0xFFFFFFFF, type 0x9A, granularity 0xCF
	db 0xFF, 0xFF, 0, 0, 0, 10010010b, 11001111b, 0x40	; data selector 0x10: base 0x40000000, limit 0xFFFFFFFF, type 0x92, granularity 0xCF
 
gdt_end:

[section .bss]

resb 0x4000
sys_stack:
	; our kernel stack
	
; reserve initial kernel stack space
;STACKSIZE equ 0x4000		; that's 16k.
;
;loader:
;	mov esp, stack+STACKSIZE	; set up the stack
;	push eax			; pass Multiboot magic number
;	push ebx			; pass Multiboot info structure
;	
;static_ctors_loop:
;	mov ebx, start_ctors
;	jmp .test
;	
;.body:
;	call [ebx]
;	add ebx, 4
;	
;.test:
;	cmp ebx, end_ctors
;	jb .body
;	
;	call kmain			; call kernel proper
;	
;static_dtors_loop:
;	mov ebx, start_dtors
;	jmp .test
;	
;.body:
;	call [ebx]
;	add ebx, 4
;	
;.test:
;	cmp ebx, end_dtors
;	jb .body
;	
;	cli
;hang:
;	hlt				; halt machine should kernel return
;	jmp hang
;	
;section .bss
;align 4
;stack:
;	resb STACKSIZE			; reserve 16k stack on a doubleword boundary
