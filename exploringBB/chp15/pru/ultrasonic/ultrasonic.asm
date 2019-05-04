; Written for Exploring BeagleBone v2 by Derek Molloy
; This program uses the PRU as an ultrasonic controller based on the values in
; 0x00000000 (percentage) and 0x00000004 (delay)
  .cdecls "main.c"
  .clink
  .global START
  .asg  32, PRU0_R31_VEC_VALID  ; allows notification of program completion
  .asg  3,  PRU_EVTOUT_0        ; the event number that is sent back
  .asg  1000, TRIGGER_COUNT     ;
  .asg  100000, SAMPLE_DELAY_1MS

; Using register 0 for all temporary storage (reused multiple times)
START:
   ; Read number of samples to read and inter-sample delay
   LDI32  r0, 0x00000000        ; load the memory location, number of samples
   LBBO   &r1, r0, 0, 4         ; load the value into memory - keep r1
   ; Read the sample delay
   LDI32  r0, 0x00000004        ; the sample delay is in the second 32-bits
   LBBO   &r2, r0, 0, 4         ; the sample delay is stored in r2

MAINLOOP:
   LDI32    r0, TRIGGER_COUNT     ; store length of the trigger pulse delay
   SET    r30, r30.t5           ; set the trigger high

TRIGGERING:                     ; delay for 10us
   SUB    r0, r0, 1             ; decrement loop counter
   QBNE   TRIGGERING, r0, 0     ; repeat loop unless zero
   CLR    r30, r30.t5           ; 10us over, set the triger low - pulse sent
   ; clear the counter and wait until the echo goes high
   LDI32  r3, 0                 ; r3 will store the echo pulse width
   WBS    r31, 3                ; wait until the echo goes high

   ; start counting (measuring echo pulse width)  until the echo goes low
COUNTING:
   ADD    r3, r3, 1             ; increment the counter by 1
   QBBS   COUNTING, r31, 3      ; loop if the echo is still high
   ; at this point the echo is now low - write the value to shared memory
   LDI32  r0, 0x00000008        ; going to write the result to this address
   SBBO   &r3, r0, 0, 4         ; store the count at this address
   ; one more sample iteration has taken place
   SUB    r1, r1, 1             ; take 1 away from the number of iterations
   MOV    r0, r2                ; need a delay between samples

SAMPLEDELAY:                    ; do this loop r2 times (1ms delay each time)
   SUB    r0, r0, 1             ; decrement counter by 1
   LDI32  r4, SAMPLE_DELAY_1MS  ; load 1ms delay into r4

DELAY1MS:
   SUB    r4, r4, 1
   QBNE   DELAY1MS, r4, 0       ; keep going until 1ms has elapsed
   QBNE   SAMPLEDELAY, r0, 0    ; repeat loop unless zero
   ; generate an interrupt to update the display on the host computer
   ; LDI32  R31, (PRU0_R31_VEC_VALID|PRU_EVTOUT_1)
   QBNE   MAINLOOP, r1, 0       ; loop if the no of iterations has not passed

END:                               ; end of program, send back interrupt
   LDI32  R31, (PRU0_R31_VEC_VALID|PRU_EVTOUT_0)
   HALT                       ; halt the pru program
