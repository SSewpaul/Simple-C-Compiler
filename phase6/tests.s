main:
	pushq	%rbp
	movq	%rsp, %rbp
	movl	$main.size, %eax
	subq	%rax, %rsp
	movl	$4, -4(%rbp)
	movl	-4(%rbp), %eax
	movslq	%eax, %rax
	imulq	$8, %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	malloc
	movq	%rax, %rdi
	movq	%rdi, -16(%rbp)
# for
.L0:
	movl	-8(%rbp), %eax
	cmpl	-4(%rbp), %eax
	setl	%al
	movzbl	%al, %eax
	cmpl	$0, %eax
	je	.L1
	movl	-4(%rbp), %eax
	movslq	%eax, %rax
	imulq	$4, %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	malloc
	movl	-8(%rbp), %edi
	movslq	%edi, %rdi
	imulq	$8, %rdi
	movq	-16(%rbp), %rsi
	addq	%rdi, %rsi
	movq	%rax, (%rsi)
	movl	-8(%rbp), %eax
	movslq	%eax, %rax
	imulq	$8, %rax
	movq	-16(%rbp), %rdi
	addq	%rax, %rdi
	movq	(%rdi), %rdi
	leaq	.L2, %rax
	movq	%rdi, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	movl	-8(%rbp), %eax
	addl	$1, %eax
	movl	%eax, %edi
	movl	%edi, -8(%rbp)
	jmp	.L0
.L1:
	movq	-16(%rbp), %rax
	jmp	main.exit

main.exit:
	movq	%rbp, %rsp
	popq	%rbp
	ret

	.set	main.size, 16
	.globl	main

	.data
.L2:	.asciz	"%d\012"
