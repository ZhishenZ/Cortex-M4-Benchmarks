# STM32CUBE IDE configurations:

## System Core

### NVIC: 

Timer Global Interrupt

### HSE
Cristal Resonator enable 

## Timers 

### TIM2
clock source internal clock
modifie Counter Period
autoreload enable

Prescaler = (72-1);
 

# About over flow 

if we set the overflow as 1kHz(1ms a for a overflow)

the bubble sort takes

without overflow counter: 434120 us
with overflow conter :    432344 us
400 overflows take about 1.776 ms, so we try to minimize the effect of the overflow. and set the overflow every second.


# in the main code 

TIM2->DIER |= TIM




# ABOUT the flash latency 

 




## disassemble in the disassembly.txt

```
# Questions: in the processing of measuring time, if we define somwthing in the measureing body and the executiontime will dcrease
```  


|       |**pointer array on SRAM**|**pointer array on FLASH**|
| :-----| ----: | ----: |
| **100 pointers:**|0.019ms  |  0.022ms |
| **500 pointers:** | 0.076ms | 0.111ms |
| **1000 pointers:** | 0.153ms | 0.222ms |
| **2000 pointers:** | 0.667ms | 0.444ms |
| **5000 pointers:** | 1.737ms | 1.112ms |
| **10000 pointers:** | overflow(40KB )| 2.224ms  |


## 100 pointers: 
RAM:     0.019ms 
FLASH:   0.022ms 


## 500 pointers: 
RAM:     0.076ms
FLASH:   0.111ms 


## 1000 pointers: 
RAM:     0.153ms 
FLASH:   0.222ms 


## 2000 pointers: 
RAM:     0.667ms
FLASH:   0.444ms 

## 5000 pointers: 
RAM:     1.737ms
FLASH:   1.112ms 

## 10000 pointers: 
RAM      overflow(40KB )
FLASH:   2.224ms 

```
tasks:
1. why it takes longer that we we declare int a = 10 before we start the timer 
2. pointer chase inside the main function. 
3. read somthing about the allignment  
4. use macro to assign things in the FLASH
5. read chapter 1 in Zhu's book
6. Piplining is very important.

``` 