[BITS 32]
section .text


%ifdef ASM_EXPORT
	export  _fetch_add_c
	export  _atomic_dec16_c
	export  _atomic_inc16_c
	export  _mfence_c
	export  _memset_asm
	export  _memset
	export  _memcpy_asm
	export  _memcpy
	export  _compare_z_exchange_c
	export  _compare_exchange_c
	export  _swap_ptr_c

	export  _get_stack_c
	export _libc_sind
	export _libc_cosd
	export _libc_sqrtd
	export _libc_atand
	export _libc_sinf
	export _libc_cosf
	export _libc_atanf
	export _libc_ftol
	export _libc_ftouc
	export _sqrtf_c
	export _libc_sqrtf

	export _asm_pow_f_c
	export _powd_c
	export _exp_c
	export _log2_c

%endif

%ifdef PREFIX
	global  _atomic_dec16_c
	global  _atomic_inc16_c
	global  _fetch_add_c
	global  _mfence_c
	global  _memset
	global  _memset_asm
	global  _memcpy
	global  _memcpy_asm
	
	global  _compare_z_exchange_c
	global  _compare_exchange_c
	global  _swap_ptr_c

	global  _scan_stack_c
	global  _get_stack_frame_c
	global  _get_stack_c

	global _libc_sind
	global _libc_cosd
	global _libc_sqrtd
	global _libc_atand
	global _libc_sinf
	global _libc_cosf
	global _libc_atanf
	global _sqrtf_c
	global _libc_sqrtf
	global _libc_ftol
	global _libc_ftouc
	global _asm_pow_f_c
	global _powd_c
	global _exp_c
	global _log2_c
	 
	

%else
	GLOBAL fetch_add_c:function 
	GLOBAL _atomic_dec16_c:function 
	GLOBAL _atomic_inc16_c:function 

	GLOBAL mfence_c:function 
	global memset_asm:function
	global memcpy_asm:function
	GLOBAL compare_z_exchange_c:function
	global compare_exchange_c:function
	global swap_ptr_c:function
	global libc_sind:function
	global libc_cosd:function
	global libc_sqrtd:function
	global libc_atand:function
	global libc_sinf:function
	global libc_cosf:function
	global libc_atanf:function
	global libc_ftol:function
	global libc_ftouc:function
	global libc_sqrtf:function
	global sqrtf_c:function
	
	
	global scan_stack_c:function
	global get_stack_frame_c:function
	global get_stack_c:function

	global asm_pow_f_c:function
	global powd_c:function
	global exp_c:function
	global log2_c:function
	
%endif



%ifdef PREFIX
_memcpy:
_memcpy_asm:
%else
memcpy_asm:
%endif
   push ebp
   mov  ebp, esp
   
   push esi
   push edi
   push ecx

   mov edi, [ebp+8]   ; edi = dest
   mov esi, [ebp+12]   ; esi = src
   mov ecx, [ebp+16]   ; ecx = count
   rep movsb   ; for(i = 0; i < ecx; i++){edi[i]=esi[i]}

   mov eax, [ebp+8]      ; eax = return value = dest
  
   pop ecx
   pop edi
   pop esi
   pop ebp
   
ret


%ifdef PREFIX
_memset:
_memset_asm:
%else
memset_asm:
%endif

 push ebp
    mov ebp, esp
    add ebp, 4 ; We pushed one register to stack, count it
    push ebx ; Save used registers
    push ecx
    mov eax, DWORD [ebp + 12]     ; size
    mov ecx, DWORD [ebp + 8]      ; tcx = val
    mov ebx, DWORD [ebp + 4]      ; tbx = destination

  .memset_loop:
    or eax, eax ; Fast compare to 0
    jz .aftermemset_loop
    mov [ebx], BYTE cl
    dec eax
    inc ebx
  jmp .memset_loop
  .aftermemset_loop:
    mov eax, DWORD [ebp + 4]            ; Return destionation
    pop ecx ; Restore used registers
    pop ebx
    pop ebp

ret

%ifdef PREFIX
_compare_exchange_c:
%else
compare_exchange_c:
%endif

	push edi
	push ebx
	
	mov edi	,	[esp+12]
	mov eax ,	[esp+16]
	mov ebx ,	[esp+20]

	;Compare EAX with r/m32. If equal, ZF is set and r32 is	 loaded into r/m32. Else, clear ZF and load r/m32 into AL
	lock CMPXCHG dword [edi],	ebx

	pop ebx
	pop edi
ret


%ifdef PREFIX
_compare_z_exchange_c:
%else
compare_z_exchange_c:
%endif

	push edi
	push ebx
	
	mov edi	,	[esp+12]
	mov ebx ,	[esp+16]
	
	;Compare EAX with r/m32. If equal, ZF is set and r32 is	 loaded into r/m32. Else, clear ZF and load r/m32 into AL

	xor	eax				,	eax
	lock CMPXCHG [edi]	,	ebx
	jnz _compare_z_exchange_c_not_changed
		mov eax,1
		jmp _compare_z_exchange_c_done
	_compare_z_exchange_c_not_changed:
		xor eax,eax
	 
	_compare_z_exchange_c_done:

	pop ebx
	pop edi
ret

%ifdef PREFIX
_fetch_add_c:
%else
fetch_add_c:
%endif
	push edi
	mov  edi				, [esp+8]
	mov  ax					, [esp+12]
	lock xadd word [edi]	, ax
	pop edi
ret


%ifdef PREFIX
_atomic_inc16_c:
%else
atomic_inc16_c:
%endif
	mov  eax, [esp+4]
	lock inc word [eax]
ret

%ifdef PREFIX
_atomic_dec16_c:
%else
atomic_dec16_c:
%endif
	mov  eax, [esp+4]
	lock dec word [eax]
ret

%ifdef PREFIX
_mfence_c:
%else
mfence_c:
%endif
	mfence
ret

%ifdef PREFIX
_swap_ptr_c:
%else
swap_ptr_c:
%endif

	push edi

	mov eax, [esp+8]
	mov eax, [eax]
	mov edi, [esp+12]
	xchg eax, [edi]

	pop edi
	

ret


;----------------
;float
;----------------
%ifdef PREFIX
_sqrtf_c:
%else
sqrtf_c:
%endif
	fld dword [esp+4]
	fsqrt
	mov eax,dword [esp+8]
	fstp dword [eax]
ret

;----------------
;float
;----------------
%ifdef PREFIX
_libc_sqrtf:
%else
libc_sqrtf:
%endif
	fld dword [esp+4]
	fsqrt
ret

;----------------
;double
;----------------
%ifdef PREFIX
_libc_sqrtd:
%else
libc_sqrtd:
%endif
	fld qword [esp+4]
	fsqrt
	mov eax,dword [esp+12]
	fstp qword [eax]
ret


%ifdef PREFIX
_libc_sind:
%else
libc_sind:
%endif
	fld qword [esp+4]
	fsin
	mov eax,dword [esp+12]
	fstp qword[eax]
ret

%ifdef PREFIX
_libc_cosd:
%else
libc_cosd:
%endif

	fld qword [esp+4]
	fcos
	mov eax,dword [esp+12]
	fstp qword[eax]
ret

%ifdef PREFIX
_libc_atand:
%else
libc_atand:
%endif
	fld qword [esp+4]
    fld1 	
	fpatan
	mov eax,dword [esp+12]
	fstp qword [eax]
ret




;----------------
;float
;----------------
%ifdef PREFIX
_libc_sinf:
%else
libc_sinf:
%endif
	fld dword [esp+4]
	fsin
ret

%ifdef PREFIX
_libc_cosf:
%else
libc_cosf:
%endif
	fld dword [esp+4]
	fcos
ret

%ifdef PREFIX
_libc_atanf:
%else
libc_atanf:
%endif
	fld dword [esp+4]
    fld1 	
	fpatan
ret

%ifdef PREFIX
_libc_ftol:
%else
libc_ftol:
%endif
	fld dword [esp+4]
	mov eax,dword [esp+8]
	fistp dword [eax]
ret

%ifdef PREFIX
_libc_ftouc:
%else
libc_ftouc:
%endif
	fld   dword [esp+4]
	fistp dword [esp-4]
	cmp dword [esp-4],255
	jl inf_255
		mov   al,255
	ret
	inf_255:
		mov   al,byte [esp-4]
ret


%define a               DWORD [ebp+8]
%define b               DWORD [ebp+12]
%define result          DWORD [ebp+16]
%define ctrlWord            WORD [ebp-2]
%define tmp             DWORD [ebp-6]

%ifdef PREFIX
	_asm_pow_f_c:
%else
	asm_pow_f_c:
%endif
    
    push ebp
    mov ebp, esp

    sub esp, 6
    push ebx

    fstcw ctrlWord
    or ctrlWord, 110000000000b
    fldcw ctrlWord

    fld b
    fld a
    fyl2x

    fist tmp

    fild tmp
    fsub
    f2xm1
    fld1
    fadd
    fild tmp
    fxch
    fscale

    mov ebx, result
    fstp DWORD [ebx]
	fstp tmp

    pop ebx
    mov esp, ebp
    pop ebp

 ret




%undef a
%undef b
%undef result
%undef ctrlWord
%undef tmp


%ifdef PREFIX
	_exp_c:
%else
	exp_c:
%endif

	mov eax,dword [esp+4]

	finit
	fld qword [esp+8]
	fldl2e
	fmulp st1,st0       ;st0 = x*log2(e) = tmp1
	fld1
	fscale              ;st0 = 2^int(tmp1), st1=tmp1
	fxch
	fld1
	fxch                ;st0 = tmp1, st1=1, st2=2^int(tmp1)

	fprem               ;st0 = fract(tmp1) = tmp2
	f2xm1               ;st0 = 2^(tmp2) - 1 = tmp3
	faddp st1,st0       ;st0 = tmp3+1, st1 = 2^int(tmp1)
	fmulp st1,st0       ;st0 = 2^int(tmp1) + 2^fract(tmp1) = 2^(x*log2(e))
	fstp qword [eax]     
ret

%ifdef PREFIX
	_log2_c:
%else
	log2_c:
%endif
	mov eax	,	dword [esp+4]

	finit
	fld1						    ;; st0 = 1.0
	fld qword [esp+8]               ;; st0 = number1 , st1 = 1.0
	FYL2X                           ;; st0 = log2(number1)
	
	fstp qword [eax]
	fstp
	fstp
ret

%ifdef PREFIX
	_log_c:
%else
	log_c:
%endif
	mov eax	,	dword [esp+4]

	finit
	fld1						    ;; st0 = 1.0
	fld dword [esp+8]               ;; st0 = number1 , st1 = 1.0
	FYL2X                           ;; st0 = log2(number1)
	
	fld1							;; st0 = 1.0  , st1 = log2(number1)
	fld dword [esp+16]				;; st0 = number2 , st1 = 1.0 , st2 = log2(number1)
	FYL2X                           ;; st0 = log2(number2) , st1 = log2(number1)
	
	fdiv st1					    ;; st0 contains log2 of number2 , st1 contains log2 of number1  . so we can calculate st0/st1.

	fstp qword [eax] 
ret


%define a               QWORD [ebp+8]
%define b               QWORD [ebp+16]
%define result          DWORD [ebp+24]
%define ctrlWord        WORD [ebp-2]
%define tmp             DWORD [ebp-6]

%ifdef PREFIX
	_powd_c:
%else
	powd_c:
%endif
    push ebp
    mov ebp, esp

    sub esp, 6
    push ebx

    fstcw ctrlWord
    or ctrlWord, 110000000000b
    fldcw ctrlWord

    fld b
    fld a
    fyl2x

    fist tmp

    fild tmp
    fsub
    f2xm1
    fld1
    fadd
    fild tmp
    fxch
    fscale

    mov ebx, result
    fstp QWORD [ebx]
	fstp tmp

    pop ebx
    mov esp, ebp
    pop ebp
 ret

 %undef a
 %undef b
 %undef result
 %undef ctrlWord
 %undef tmp


%ifdef PREFIX
	_get_stack_frame_c:
%else
	get_stack_frame_c:
%endif
	mov eax,ebp
ret

%ifdef PREFIX
	_get_stack_c:
%else
	get_stack_c:
%endif
	mov eax,esp
ret

 %ifdef PREFIX
	extern _mark_zone@8
	_scan_stack_c:
 %else
	extern mark_zone
	scan_stack_c:
 %endif
	push ebp
	mov ebp,esp

	pusha

    mov eax	, [ebp + 8 ]	; lower bound of zone buffer
	mov ebx	, [ebp + 12 ]	; upper bound of zone buffer
	mov edi , [ebp + 16 ]	; upper bound of zone buffer

	mov ecx , edi			; get last stack frame pointer
	sub ecx , [ebp + 20 ]	; get last strack frame size
	 
	global_stack_loop:

		stack_frame_loop:

			lea esi,[edi]
			sub esi,ecx

			cmp [esi], eax
			jl  no_mark

			cmp [esi], ebx
			jg  no_mark

				mov edx				,	[esi]

				pusha

				push 1
				push edx
 
 %ifdef PREFIX
				call _mark_zone@8
%else
				call mark_zone
%endif

				popa

			no_mark:

		sub ecx, 4
		jnz stack_frame_loop

		mov ebp, [edi] ;next stack frame
		mov ecx, ebp
		sub ecx, edi
		jz scan_stack_done

		mov edi, ebp

		cmp edi,0
	jnz global_stack_loop

	scan_stack_done:
	
	popa
	mov esp,ebp
	pop ebp

 ret


 %if 0

 my_func:

 ret


 %define StackSeg		 0x20
 %define OrigStackSeg	 0x40
 
 %define CodeSeg		 0x50
 %define OrigCodeSeg	 0x60

 %define DataSeg		 0x70
 %define OrigDataSeg	 0x80
 
 
 %define StackOffset	 4

 export_stub:
	
	push ebp
	mov  ebp	, esp

	pusha 

	mov ax		, StackSeg
	mov es		, ax
	xor edi		, edi
	
	lea esi		, [ebp+4]
	mov ecx		, StackOffset
	shr ecx		, 2
		
	export_stub_cpy_stack:
		mov eax		,	[esi]
		mov [edi]	,	eax

		sub esi		,	4
		sub edi		,	4

		dec ecx
	jnz export_stub_cpy_stack

	mov ax		,	StackSeg
	mov ss		,	ax
	xor esp		,	esp

	mov ax		,	DataSeg
	mov es		,	ax
	mov ds		,	ax
	
	call CodeSeg:my_func
	
	jmp OrigCodeSeg:export_stub_back

	export_stub_back:

	mov ax		,	OrigDataSeg
	mov es		,	ax
	mov ds		,	ax

	mov ax		,	OrigStackSeg
	mov ss		,	ax

	popa

	mov esp		, ebp
	pop ebp

 ret StackOffset

 %endif