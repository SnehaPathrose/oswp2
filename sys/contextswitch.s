.file "contextswitch.s"
.section    .rodata
.globl  switch_to
.type   switch_to, @function
switch_to:
push %rdi
mov %rsp,3208(%rdi)
mov 3208(%rsi),%rsp
pop %rdi
ret