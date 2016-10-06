[BITS 32]

segment .text

global _sendval

_sendval:
	push ebp             
	mov ebp, esp
	mov eax,[ebp+8]
	mov ecx,eax
	
	repeat:
		mov dx,03fdh
		in al,dx
		test al,20h
	jz repeat
	
	mov al,cl
	mov dx,03f8h
	out dx,al
	pop ebp
ret


 


