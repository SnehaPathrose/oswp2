.file "contextswitch.s"
.section    .rodata
.globl  switch_to
.type   switch_to, @function
switch_to:
mov (%rsp),%rax
mov %rax,3192(%rsi)
push %rdi
mov %rsp,3208(%rdi)
mov 3208(%rsi),%rsp
ret