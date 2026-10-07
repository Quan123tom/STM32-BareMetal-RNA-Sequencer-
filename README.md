# STM32-BareMetal-RNA-Sequencer-
# Bare-Metal STM32 ARM Assembly Sequencer

An academic embedded systems exercise implementing a streaming string-search algorithm on an STM32F407VGT6 microcontroller. This project was written entirely in ARM Cortex-M4 assembly without the use of Hardware Abstraction Libraries (HAL).
## Overview
The application receives strings of RNA nucleotides via a serial terminal, translates them into their DNA complements, and performs an exact string match against the SARS-CoV-2 genome stored in read-only memory. It features direct register manipulation for peripheral configuration and custom string-processing routines.

## Key Implementation Details
*   **Direct Peripheral Control:** Configures clock trees (RCC), GPIO pins, and USART2 communication by directly bit-masking memory-mapped registers.
*   **In-Place Data Translation:** Implements an algorithm (`find_compl`) to substitute incoming RNA bases ('A', 'C', 'G', 'U') with their complementary pairs in-place, while safely ignoring invalid noise characters.
*   **Memory-Efficient Search:** Utilizes a sliding-window string matching algorithm heavily relying on ARM's post-indexed addressing (`LDRB R4, [R3], #1`) to minimize loop overhead and stack usage.
*   **Hardware Polling:** Per the academic constraints of the exercise, asynchronous button inputs (`GPIOD4`) for search-resets are handled via software polling rather than hardware interrupts (`EXTI0`). 
*   **Custom ASCII Conversion:** Includes a stack-based integer-to-ASCII routine using hardware division (`UDIV`) and multiply-subtract (`MLS`) to transmit numerical match counts and memory offsets over UART.

## Hardware Pinout
Developed for the **STM32F407VGT6**.
*   **USART2 TX / RX:** `PA2` / `PA3`
*   **Match Indicator LED:** `PA12` (Illuminates for 2 seconds on match)
*   **Reset Button (Input):** `PD4`
*   **Reset Indicator LED:** `PA13` (Illuminates for 3 seconds on reset request)

