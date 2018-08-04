public NtUserSetWindowsHookExGate

.CODE _TEXT


NtUserSetWindowsHookExGate PROC PUBLIC
	;int 3
	mov     r10,rcx
	mov     eax,108Ch
	syscall
	ret
NtUserSetWindowsHookExGate ENDP

END