main:
	pushq	%rbp
	movq	%rsp, %rbp
	movl	$main.size, %eax
	subq	%rax, %rsp
	movl	x, %eax
	addl	y, %eax
	movl	x, %eax
	addl	y, %eax
	addl	$1, %eax
	movl	x, %eax
	addl	$1, %eax
	movl	y, %edi
	addl	$1, %edi
	addl	%edi, %eax
	movq	m, %rax
	addq	n, %rax
	movq	m, %rax
	addq	n, %rax
	addq	k, %rax
	movq	m, %rax
	addq	k, %rax
	movq	n, %rdi
	addq	k, %rdi
	addq	%rdi, %rax

main.exit:
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.set	main.size, 0
	.globl	main

	.comm	x, 4
	.comm	y, 4
	.comm	k, 8
	.comm	m, 8
	.comm	n, 8
