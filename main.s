/* =========================
Bit definitions
========================= */
.equ GPIOAEN, (1 << 0)
.equ GPIODEN, (1 << 3)
.equ USART2EN, (1 << 17)
.equ USART_SR_TXE, (1 << 7)
.equ USART_SR_RXNE, (1 << 5)
.equ USART_CR1_UE, (1 << 13)
.equ USART_CR1_TE, (1 << 3)
.equ USART_CR1_RE, (1 << 2)
.equ PD12_SET, (1 << 12)
.equ PD12_RESET, (1 << 28)
.equ CHAR_A, 0x41 // 'A'
.equ CHAR_C, 0x43 //'C'
.equ CHAR_G, 0x47 //'G'
.equ CHAR_U, 0x55 //'U'
.equ CHAR_NL, 0x0A //'\n'
.equ CHAR_T, 0x54 //'T'
.equ USART2_BRR_115200_16MHZ, 0x008B
// now my actual code
main_asm:
bl gpio_init
bl uart2_init
bl uart2_flush
LDR R0, =msg_test
BL uart2_send_string
LDR R1, =rna_sequence // R1 is the pointer to my RNA file
LDR R4, =user_input_data // i put in R4 the mem adress of the buffer
// I now take input
poll_uart:
BL uart2_char_available
CMP R0, #0
BEQ poll_uart
BL uart2_read_char
// now we check if valid
CMP R0, #CHAR_NL
BEQ input_complete
CMP R0, #0x0D // Check for ('\r')
BEQ input_complete // If found, just skip it and wait for the '\n' right behind it
CMP R0, #CHAR_A
BEQ store
CMP R0, #CHAR_C
BEQ store
CMP R0, #CHAR_G
BEQ store
CMP R0, #CHAR_U
BEQ store
B poll_uart
store:
STRB R0, [R4], #1 // store and go back
B poll_uart
input_complete:
MOV R0, #0
STRB R0, [R4] // put 0 in the end so i know where to stop looking
BL find_compl
MOV R9, #0
LDR R10,=new_sequence
LDR R12,=user_input_data
LDR R1, =rna_sequence // reload the pointer
MOV R2, R1 // I will increment R2 each time for the outer loop
search:
LDRB R0, [R2], #1 // load in R0 the value stored in the address that R2 points to (last
accessed from DNA seq)
CMP R0, #0 // compare with 0
BEQ finished // if yes then finish the loop
MOV R0, R2 // if no load to R0 the value of R2 (ie where to start looking)
SUB R0, R0, #1
MOV R3, R12 // move the start of user input here
loop2:
LDRB R4, [R3], #1 // load start of user input and increment
LDRB R5, [R0], #1 // load start of sequence and increment
CMP R4, #0 // if user input reached the end then match
BEQ match
CMP R4, R5 // if they are the same loop again
BEQ loop2
CMP R5, #0 // if i reached end of dna then we have a mismatch
BEQ mismatch
B mismatch
match:
ADD R9, R9, #1
SUB R11, R2, R1 // distance from rna_sequence start
SUB R11, R11, #1 // correct for post-increment
STR R11, [R10], #4
B search
mismatch:
B search
finished:
//send header message for total
LDR R0, =msg_count
BL uart2_send_string
//send the total match count stored in R9 register
MOV R0, R9
BL uart2_send_num
LDR R0, =msg_nl
BL uart2_send_string
// If zero matches were found skip
CMP R9, #0
BEQ output_done
// print
LDR R8, =new_sequence // R8 forarray
MOV R7, #0 // Loop counter (0 up to R9)
print_locations_loop:
CMP R7, R9
BEQ output_done // Exit loop when all matches are printed
LDR R0, =msg_pos
BL uart2_send_string
LDR R0, [R8], #4
BL uart2_send_num
LDR R0, =msg_nl
BL uart2_send_string
ADD R7, R7, #1 // Move to next saved result
B print_locations_loop
output_done:
B main_asm
.align 2
msg_count: .asciz "Matches found: "
msg_pos: .asciz "Found at position: "
msg_nl: .asciz "\r\n"
.align 2
uart2_send_num:
PUSH {R4, R5, LR}
MOV R4, R0 // R4 = working copy of number
CMP R4, #0
BNE num_not_zero
MOV R0, #'0'
BL uart2_send_char
B num_send_done
num_not_zero:
MOV R5, #0 // Digit stack counter
stack_push_loop:
CMP R4, #0
BEQ stack_pop_loop
MOV R1, #10
UDIV R2, R4, R1 // R2 = R4 / 10
MLS R3, R2, R1, R4 // R3 = R4 - (R2 * 10)
ADD R3, R3, #0x30 // Convert to ASCII representation character
PUSH {R3} // Store digit on stack
ADD R5, R5, #1
MOV R4, R2 // Move to R4 for next division
B stack_push_loop
stack_pop_loop:
CMP R5, #0
BEQ num_send_done
POP {R0} // Pull characters back in correct left-to-right order
BL uart2_send_char // Send via existing UART function
SUB R5, R5, #1
B stack_pop_loop
num_send_done:
POP {R4, R5, PC}
find_compl:
push {lr}
LDR R0, =user_input_data
compl_loop:
LDRB R3, [R0]
CMP R3, #0
BEQ end1
check_A:
CMP R3, #CHAR_A
BNE check_U
MOV R2, #CHAR_T
STRB R2, [r0]
B next_base
check_U:
CMP R3, #CHAR_U
BNE check_G
MOV R2, #CHAR_A
STRB R2, [r0]
B next_base
check_G:
CMP R3, #CHAR_G
BNE check_C
MOV R2, #CHAR_C
STRB R2, [r0]
B next_base
check_C:
CMP R3, #CHAR_C
BNE next_base
MOV R2, #CHAR_G
STRB R2, [r0]
next_base:
ADD R0, R0, #1
B compl_loop
end1:
POP {pc}
gpio_init:
push {lr}
/*
Enable GPIOA and GPIOD clocks.
RCC_AHB1ENR:
bit 0 = GPIOAEN
bit 3 = GPIODEN
*/
ldr r0, =RCC_AHB1ENR
ldr r1, [r0]
orr r1, r1, #GPIOAEN
orr r1, r1, #GPIODEN
str r1, [r0]
ldr r0, =GPIOD_MODER
ldr r1, [r0]
bic r1, r1, #(3 << 24)
orr r1, r1, #(1 << 24)
str r1, [r0]
ldr r0, =GPIOD_OTYPER
ldr r1, [r0]
bic r1, r1, #(1 << 12)
str r1, [r0]
/*
PD12 medium speed.
*/
ldr r0, =GPIOD_OSPEEDR
ldr r1, [r0]
bic r1, r1, #(3 << 24)
orr r1, r1, #(1 << 24)
str r1, [r0]
/*
PD12 no pull-up / pull-down.
*/
ldr r0, =GPIOD_PUPDR
ldr r1, [r0]
bic r1, r1, #(3 << 24)
str r1, [r0]
/*
Configure PA2 and PA3
PA2 MODER bits 5:4 = 10
PA3 MODER bits 7:6 = 10
*/
ldr r0, =GPIOA_MODER
ldr r1, [r0]
bic r1, r1, #(3 << 4)
bic r1, r1, #(3 << 6)
orr r1, r1, #(2 << 4)
orr r1, r1, #(2 << 6)
str r1, [r0]
/*
PA2 and PA3 high speed.
*/
ldr r0, =GPIOA_OSPEEDR
ldr r1, [r0]
bic r1, r1, #(3 << 4)
bic r1, r1, #(3 << 6)
orr r1, r1, #(3 << 4)
orr r1, r1, #(3 << 6)
str r1, [r0]
/*
PA2 no pull.
PA3 pull-up.
*/
ldr r0, =GPIOA_PUPDR
ldr r1, [r0]
bic r1, r1, #(3 << 4)
bic r1, r1, #(3 << 6)
orr r1, r1, #(1 << 6)
str r1, [r0]
/*
Select AF7 for PA2 and PA3.
GPIOA_AFRL:
PA2 uses bits 11:8
PA3 uses bits 15:12
AF7 = USART2
*/
ldr r0, =GPIOA_AFRL
ldr r1, [r0]
bic r1, r1, #(0xF << 8)
bic r1, r1, #(0xF << 12)
orr r1, r1, #(7 << 8)
orr r1, r1, #(7 << 12)
str r1, [r0]
pop {pc}
/* =====================================================
USART2 initialization
PA2 = TX
PA3 = RX
===================================================== */
uart2_init:
push {lr}
/*
Enable USART2 clock.
RCC_APB1ENR:
bit 17 = USART 2EN
*/
ldr r0, =RCC_APB1ENR
ldr r1, [r0]
orr r1, r1, #USART2EN
str r1, [r0]
/*
Disable USART2 before configuration.
*/
ldr r0, =USART2_CR1
mov r1, #0
str r1, [r0]
/*
*/
ldr r0, =USART2_BRR
ldr r1, =USART2_BRR_115200_16MHZ
str r1, [r0]
/*
*/
ldr r0, =USART2_CR2
mov r1, #0
str r1, [r0]
/*
*/
ldr r0, =USART2_CR3
mov r1, #0
str r1, [r0]
/*
*/
ldr r0, =USART2_CR1
ldr r1, =(USART_CR1_UE | USART_CR1_TE | USART_CR1_RE)
str r1, [r0]
pop {pc}
uart2_send_char:
push {r1, r2, lr}
mov r2, r0
wait_txe:
ldr r0, =USART2_SR
ldr r1, [r0]
tst r1, #USART_SR_TXE
beq wait_txe
ldr r0, =USART2_DR
str r2, [r0]
pop {r1, r2, pc}
uart2_send_string:
push {r1, r2, lr}
mov r2, r0
send_string_loop:
ldrb r1, [r2]
cmp r1, #0
beq send_string_done
mov r0, r1
bl uart2_send_char
add r2, r2, #1
b send_string_loop
send_string_done:
pop {r1, r2, pc}
uart2_char_available:
ldr r0, =USART2_SR
ldr r1, [r0]
tst r1, #USART_SR_RXNE
beq no_char
mov r0, #1
bx lr
no_char:
mov r0, #0
bx lr
uart2_read_char:
wait_rxne:
ldr r0, =USART2_SR
ldr r1, [r0]
tst r1, #USART_SR_RXNE
beq wait_rxne
ldr r0, =USART2_DR
ldr r0, [r0]
and r0, r0, #0xFF
bx lr
delay:
subs r0, r0, #1
bne delay
bx lr
uart2_flush:
PUSH {LR} // Save return address
flush_loop:
BL uart2_char_available
CMP R0, #0
BEQ flush_finished // if no characters are left, exit loop
BL uart2_read_char // read and discard the character
B flush_loop // check again
flush_finished:
POP {PC} // return to main_asm
.section .rodata
.align 2
msg_test: .asciz "READY\r\n"
.global rna_sequence
rna_sequence:
.incbin "../Core/Src/sequence.txt"
.byte 0
.section .bss
.align 2
.global new_sequence
new_sequence: .space 700
.section .bss
.align 2
.global user_input_data
user_input_data: .space 200
