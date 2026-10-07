# STM32-BareMetal-RNA-Sequencer-
# Bare-Metal STM32 ARM Assembly Sequencer
## Overview
This is an academic embedded systems exercise implementing a string-search algorithm on an STM32F407VGT6 microcontroller. This project was written in ARM Cortex-M4 assembly without the use of Hardware Abstraction Libraries (HAL) or Real Time Operating System (RTOS) and in C, utilizing the potential of HAL. 
## Application Functionality
* The application receives strings of RNA nucleotides serially via the USART2 terminal, translates them into their DNA complements, and performs the string matching against the SARS-CoV-2 genome stored in ROM.
* It is designed to output how many times this specific sequence was found in the genome as well as the positions where it was found (the positions of the first nucleotide for each instance). If a match is found, it is designed to turn on a LED connected to GPIOA12 for 2 seconds.
## Implementation Differences
* In the assembly implementation, all configuration was done manually via direct register manipulation (clock trees via RCC, GPIO pins, and USART2), and the button inputs for the reset were handled via software polling. 
* In C, the reset was implemented using hardware interrupts (specifically EXTI0). Triggering this reset in either implementation causes the exact same behavior: halting the search program, turning on an LED connected to GPIOA13 for 3 seconds, and restarting the search from the beginning.

