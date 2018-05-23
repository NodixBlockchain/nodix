
section .code

global _intersect_ray_boxf_sse
export _intersect_ray_boxf_sse

global _intersect_ray_boxesf_sse
export _intersect_ray_boxesf_sse
 
 align 32
 _intersect_ray_boxesf_sse :
 
	
	
	mov eax			,[esp+4]
	MOVAPS xmm6		,[eax]
	MOVAPS xmm4		,[eax+16]					;ray origin
	
	mov eax			,[esp+8]
	MOVAPS xmm7		,[eax]
	MOVAPS xmm5		,[eax+16]					;ray dir
	
;	mov eax			,[esp+12]					;ray sign mask
;	mov [esp-4]		,eax
	
;	mov eax			,[esp+16]					;box ptr
;	mov [esp-8]		,eax
	
;	mov eax			,[esp+20]					;num box 
;	mov [esp-12]	,eax
	
;	mov eax			,[esp+24]					;cast
;	mov [esp-16]	,eax

;	mov eax			,[esp+28]					;bound ids
;	mov [esp-20]	,eax

;	mov eax			,[esp+32]					;min
;	mov [esp-24]	,eax

;	mov eax			,[esp+36]					;max
;	mov [esp-28]	,eax


	mov eax,ecx
	mov ecx,esp

	push eax
	push ebx
	push edi


	mov eax			,[ecx+28]					;bound ids
	mov [esp-28]	,eax	
	
	
	mov dword [esp-4] ,0						;counter
	mov dword [esp-16],0						;intersect
	
	mov edi			,[ecx+16]					;box list ptr

	mov ebx			,[ecx+12]				;ray sign mask
	shl ebx			,2

	align 32
	start_loop:
		
		mov eax			,[edi+0]				;box id
		
		cmp eax			,[ecx+24]				;box id == cast ?
		je next_loop
		
		mov eax			,ebx
		add eax			,[edi+4]				;box parameters
			
		MOVAPS xmm0		,[eax]				
		MOVAPS xmm3		,[eax+16]

		;--------------------------------------
		SUBPS   xmm0,xmm6						;xmm0 min[0] ,min[1] ,max[0],max[1]
		mulps	xmm0,xmm7
		;--------------------------------------
		
		PSHUFD   xmm1,xmm0,11101011b			;xmm1 max[1], max[0] ,max[0],max[1]
		
		CMPPS 		xmm1,xmm0,1
		PMOVMSKB	eax,xmm1
		test		eax,eax
		jnz			next_loop
		
			
		PSHUFD xmm1,xmm0,00110001b		;xmm1	=	tymin,tmin,tymax,tmin
		PSHUFD xmm2,xmm0,00100000b		;xmm2	=	tmin ,tmin,tmax,tmin
		  
		;if (tymin > tmin)  tmin = tymin;
		maxss   xmm1,xmm2
		movss   [esp-8],xmm1						;tmin

		;if (tymax < tmax)  tmax = tymax;	
		minps   xmm2,xmm1
		movhlps xmm2,xmm2
		movss   [esp-12],xmm2					;tmax
		
		;-------------------------------------
		SUBPS   xmm3,xmm4
		mulps	xmm3,xmm5           	;xmm3 = v2*inv_dir[3],v2*inv_dir[3]	
		;--------------------------
			
		;tmin,tzmin
		;tzmax,tmax
		
		;if ( (tmin > tzmax) || (tzmin > tmax) )    return 0;
		
		MOVLPS [esp-24]	,xmm3
		
		fld dword [esp-8]					;tmin
		fld	dword [esp-20]					;tzmax
		fcomip								;tzmax < tmin
		fstp   					
		JB  next_loop						;(!tzmax < tmin) ?

		fld dword [esp-24]					;tzmin
		fld dword [esp-12]					;tmax
		fcomip								;(tmax < tzmin ) ?
		fstp								;
		JB next_loop						;(!tmax <  tzmin) ?

		movd xmm0		,[esp-24]
		maxss xmm0		,[esp-8]
		movss [esp-8]	,xmm0
		
		movd xmm0		,[esp-20]
		minss xmm0		,[esp-12]
		movss [esp-12]	,xmm0

		
		;--interesect---
		
		
		mov eax			,[ecx+32]			;final min
		
		movss xmm0		,[esp-8]
		minss xmm0		,[eax]
		movss [eax]		,xmm0
		
		movss xmm0		,[esp-12]
		maxss xmm0		,[eax+4]
		movss [eax+4]	,xmm0
			
		mov dword [esp-16],1
		
		mov  eax			,[edi]			;box id
		mov  edx			,[esp-28]		;bound ids
		mov  [edx]			,eax
		add dword [esp-28]	,4
		
		next_loop:
		
		add		edi,8
		inc		dword [esp-4]		;inc box counter
				
		mov eax	   ,[ecx+20]		;num_boxes
		cmp [esp-4],eax				;counter < num_boxes
		jl start_loop

	end_loop:

	xor  eax	,eax
	mov  edx	,[esp-28]		;bound ids
	mov  [edx]	,eax
	
	pop edi
	pop ebx
	pop ecx
	
	
	mov eax,[esp-16-12]
  ret
 
 align 32
 _intersect_ray_boxf_sse :
 
	mov ecx,esp
	
	mov eax,[esp+4]
	MOVAPS xmm6,[eax]
	MOVAPS xmm4,[eax+16]
	
	mov eax,[esp+8]
	MOVAPS xmm7,[eax]
	MOVAPS xmm5,[eax+16]

	mov eax,[esp+12]
	MOVAPS xmm0,[eax]
	MOVAPS xmm3,[eax+16]

	;--------------------------------------
	SUBPS   xmm0,xmm6						;xmm0 min[0] ,min[1] ,max[0],max[1]
	mulps	xmm0,xmm7
	;--------------------------------------
	
	PSHUFD   xmm1,xmm0,11101011b			;xmm1 max[1], max[0] ,max[0],max[1]
	
	CMPPS 		xmm1,xmm0,1
	PMOVMSKB	eax,xmm1
	test		eax,eax
	jz test_ok
		xor eax,eax
		ret
	test_ok:
	
	
	mov eax,[esp+16]
		
	PSHUFD xmm1,xmm0,00110001b		;xmm1	=	tymin,tmin,tymax,tmin
	PSHUFD xmm2,xmm0,00100000b		;xmm2	=	tmin ,tmin,tmax,tmin
	  
    ;if (tymin > tmin)  tmin = tymin;
	maxss   xmm1,xmm2
	movss   [eax],xmm1						;tmin

	;if (tymax < tmax)  tmax = tymax;	
	minps   xmm2,xmm1
	movhlps xmm2,xmm2
	movss   [eax+4],xmm2					;tmax
	
	;-------------------------------------
	SUBPS   xmm3,xmm4
	mulps	xmm3,xmm5           	;xmm3 = v2*inv_dir[3],v2*inv_dir[3]	
	;--------------------------
		
		
	;tmin,tzmin
	;tzmax,tmax
	
	;if ( (tmin > tzmax) || (tzmin > tmax) )    return 0;
	
	MOVLPS [esp-24]	,xmm3
	
			
	
	fld	dword [eax]
	fld	dword [esp-20]
	fcomip
	fstp   					
	JnB  test1_ok
		xor eax,eax
		ret
	test1_ok:
		
	fld dword [esp-24]
	fld dword [eax+4]
	fcomip
	fstp
	JnB  test2_ok
		xor eax,eax
		ret		
	test2_ok:
	
	movd xmm0		,[esp-20]
	maxss xmm0		,[eax]
	movss [eax]		,xmm0
		
	movd xmm0		,[esp-24]
	minss xmm0		,[eax+4]
	movss [eax+4]	,xmm0
	
	mov eax,1

  ret
 


