;copyright iadix 2016

[BITS 32]



section .data

CRC32_Table:
dd 0x00000000,0x77073096,0xee0e612c,0x990951ba,0x076dc419,0x706af48f,0xe963a535,
dd 0x9e6495a3,0x0edb8832,0x79dcb8a4,0xe0d5e91e,0x97d2d988,0x09b64c2b,0x7eb17cbd,
dd 0xe7b82d07,0x90bf1d91,0x1db71064,0x6ab020f2,0xf3b97148,0x84be41de,0x1adad47d,
dd 0x6ddde4eb,0xf4d4b551,0x83d385c7,0x136c9856,0x646ba8c0,0xfd62f97a,0x8a65c9ec,
dd 0x14015c4f,0x63066cd9,0xfa0f3d63,0x8d080df5,0x3b6e20c8,0x4c69105e,0xd56041e4,
dd 0xa2677172,0x3c03e4d1,0x4b04d447,0xd20d85fd,0xa50ab56b,0x35b5a8fa,0x42b2986c,
dd 0xdbbbc9d6,0xacbcf940,0x32d86ce3,0x45df5c75,0xdcd60dcf,0xabd13d59,0x26d930ac,
dd 0x51de003a,0xc8d75180,0xbfd06116,0x21b4f4b5,0x56b3c423,0xcfba9599,0xb8bda50f,
dd 0x2802b89e,0x5f058808,0xc60cd9b2,0xb10be924,0x2f6f7c87,0x58684c11,0xc1611dab,
dd 0xb6662d3d,0x76dc4190,0x01db7106,0x98d220bc,0xefd5102a,0x71b18589,0x06b6b51f,
dd 0x9fbfe4a5,0xe8b8d433,0x7807c9a2,0x0f00f934,0x9609a88e,0xe10e9818,0x7f6a0dbb,
dd 0x086d3d2d,0x91646c97,0xe6635c01,0x6b6b51f4,0x1c6c6162,0x856530d8,0xf262004e,
dd 0x6c0695ed,0x1b01a57b,0x8208f4c1,0xf50fc457,0x65b0d9c6,0x12b7e950,0x8bbeb8ea,
dd 0xfcb9887c,0x62dd1ddf,0x15da2d49,0x8cd37cf3,0xfbd44c65,0x4db26158,0x3ab551ce,
dd 0xa3bc0074,0xd4bb30e2,0x4adfa541,0x3dd895d7,0xa4d1c46d,0xd3d6f4fb,0x4369e96a,
dd 0x346ed9fc,0xad678846,0xda60b8d0,0x44042d73,0x33031de5,0xaa0a4c5f,0xdd0d7cc9,
dd 0x5005713c,0x270241aa,0xbe0b1010,0xc90c2086,0x5768b525,0x206f85b3,0xb966d409,
dd 0xce61e49f,0x5edef90e,0x29d9c998,0xb0d09822,0xc7d7a8b4,0x59b33d17,0x2eb40d81,
dd 0xb7bd5c3b,0xc0ba6cad,0xedb88320,0x9abfb3b6,0x03b6e20c,0x74b1d29a,0xead54739,
dd 0x9dd277af,0x04db2615,0x73dc1683,0xe3630b12,0x94643b84,0x0d6d6a3e,0x7a6a5aa8,
dd 0xe40ecf0b,0x9309ff9d,0x0a00ae27,0x7d079eb1,0xf00f9344,0x8708a3d2,0x1e01f268,
dd 0x6906c2fe,0xf762575d,0x806567cb,0x196c3671,0x6e6b06e7,0xfed41b76,0x89d32be0,
dd 0x10da7a5a,0x67dd4acc,0xf9b9df6f,0x8ebeeff9,0x17b7be43,0x60b08ed5,0xd6d6a3e8,
dd 0xa1d1937e,0x38d8c2c4,0x4fdff252,0xd1bb67f1,0xa6bc5767,0x3fb506dd,0x48b2364b,
dd 0xd80d2bda,0xaf0a1b4c,0x36034af6,0x41047a60,0xdf60efc3,0xa867df55,0x316e8eef,
dd 0x4669be79,0xcb61b38c,0xbc66831a,0x256fd2a0,0x5268e236,0xcc0c7795,0xbb0b4703,
dd 0x220216b9,0x5505262f,0xc5ba3bbe,0xb2bd0b28,0x2bb45a92,0x5cb36a04,0xc2d7ffa7,
dd 0xb5d0cf31,0x2cd99e8b,0x5bdeae1d,0x9b64c2b0,0xec63f226,0x756aa39c,0x026d930a,
dd 0x9c0906a9,0xeb0e363f,0x72076785,0x05005713,0x95bf4a82,0xe2b87a14,0x7bb12bae,
dd 0x0cb61b38,0x92d28e9b,0xe5d5be0d,0x7cdcefb7,0x0bdbdf21,0x86d3d2d4,0xf1d4e242,
dd 0x68ddb3f8,0x1fda836e,0x81be16cd,0xf6b9265b,0x6fb077e1,0x18b74777,0x88085ae6,
dd 0xff0f6a70,0x66063bca,0x11010b5c,0x8f659eff,0xf862ae69,0x616bffd3,0x166ccf45,
dd 0xa00ae278,0xd70dd2ee,0x4e048354,0x3903b3c2,0xa7672661,0xd06016f7,0x4969474d,
dd 0x3e6e77db,0xaed16a4a,0xd9d65adc,0x40df0b66,0x37d83bf0,0xa9bcae53,0xdebb9ec5,
dd 0x47b2cf7f,0x30b5ffe9,0xbdbdf21c,0xcabac28a,0x53b39330,0x24b4a3a6,0xbad03605,
dd 0xcdd70693,0x54de5729,0x23d967bf,0xb3667a2e,0xc4614ab8,0x5d681b02,0x2a6f2b94,
dd 0xb40bbe37,0xc30c8ea1,0x5a05df1b,0x2d02ef8d

unable_to_resolve				:db 'unable to resolve hash ',0
unable_to_resolve_n				:db 'unable to resolve name ',0
imported						:db 'function imported ',0
text_new_tpo_hash				:db 'new tpo hash added ',10,0

sys_num_tpo_mod_loaded			:dd 0
sys_num_tpo_mod_func_loaded		:dd 0

ret_val:dd 0

align 16
sys_tpo_mod_loaded				:times 64 dd 0
sys_tpo_mod_exp_funcs			:times 64*256*16 db 00
sys_tpo_mod_exp_funcs_ptr		:dd sys_tpo_mod_exp_funcs
sys_tpo_mod_sections			:times (256*16) dq 0      ; enough room for 64 modules (64 * 16 sections/modules * 8 bytes per section)

;general functions api

sys_tpo_mod_name_ptr				:dd 0
sys_tpo_mod_name    				:times 64 db 0
sys_tpo_mod_hash					:dd 0
sys_tpo_mod_idx						:dd 0
sys_tpo_fn_hash						:dd 0
sys_tpo_fn_idx						:dd 0
sys_tpo_fn_name_ptr					:dd 0
sys_tpo_fn_addr						:dd 0
sys_tpo_fn_type						:dd 0


tpo_module_name						:times 128 db 0
tpo_module_deco_type				:dd 0
tpo_module_src_deco_type			:dd 0
tpo_module_base_addr				:dd 0
num_sections_tpo					:dd 0
string_buffer_size_tpo				:dd 0
string_buffer_ptr_tpo				:dd 0
string_buffer_ptr_idx				:dd 0


text_max_func						:db 'too much export (',0

text_tpo_loaded						:db 'loading tpo lib  (',0
text_tpo_loaded_end					:db ')',10,0

text_tpo_sections_number			:db 'n sections : ',0
text_tpo_sections_number_end		:db 10,0

text_tpo_section_size				:db 'section size : ',0
text_tpo_section_size_end			:db ' ',10,0

text_tpo_section_num_export			:db 'n exports : ',0
text_tpo_section_num_export_end		:db ' ',10,0

text_tpo_section_num_import			:db 'n imports : ',0
text_tpo_section_num_import_end		:db ' ',10,0

text_tpo_section_name				:db 'section name : ',0
text_tpo_section_name_end			:db ' ',10,0

tpo_file_pointer					:dd 0

;used for parsing tpo file
tpo_section_cnt:dd 0
tpo_section_e_cnt:dd 0
tpo_section_i_cnt:dd 0
tpo_section_r_cnt:dd 0
tpo_section_size:dd 0
tpo_section_data_ptr:dd 0
tpo_section_name:times 2 dd 0
tpo_section_flags:dd 0

tpo_section_n_imp_name:dd 0
tpo_section_n_imp_ord:dd 0

tpo_section_n_exp_name:dd 0
tpo_section_n_exp_ord:dd 0

tpo_section_n_hard_reloc:dd 0
tpo_section_reloc_base_addr:dd 0
tpo_section_reloc_addr:dd 0

tpo_func_resolve_ret:dd 0

tpo_file_binary_data_ptr:dd 0


tpo_imp_exp_ret:dd 0
tpo_add_exp_ret:dd 0
tpo_lib_imp_ofs:dd 0

tpo_mod_entry_ptr:dd 0
tpo_fn_entry_ptr:dd 0


debug_id		:dd 0

section .text


%ifdef ASM_EXPORT
	export  _fetch_add_c
	export  _compare_z_exchange_c
	export  _sys_add_tpo_mod_func_name
	export  _calc_crc32_c
	export  _tpo_calc_imp_func_hash_name_c
	export  _tpo_calc_exp_func_hash_name_c
	export  _tpo_calc_exp_func_hash_c
	export  _tpo_mod_add_func_c
	export  _tpo_mod_add_section_c
	export  _tpo_add_mod_c
	export  _tpo_mod_imp_func_addr_c
	export  _tpo_mod_add_func_addr_c
	export  _tpo_get_mod_entry_hash_c
	export  _tpo_get_mod_sec_idx_c
	export  _tpo_get_mod_entry_idx_c
	export  _tpo_get_fn_entry_name_c
	export  _tpo_get_fn_entry_hash_c
	export  _tpo_get_fn_entry_idx_c
	export  _get_CRC_table
%endif

%ifdef PREFIX
	global  _tpo_mod_imp_func_addr_c
	global  _tpo_mod_add_func_addr_c
	global  _tpo_get_mod_entry_hash_c
	global  _tpo_get_mod_sec_idx_c
	global  _tpo_get_mod_entry_idx_c
	global  _tpo_get_fn_entry_name_c
	global  _tpo_get_fn_entry_hash_c
	global  _tpo_get_fn_entry_idx_c
	global  _tpo_add_mod_c
	global  _tpo_mod_add_section_c
	global  _tpo_mod_add_func_c
	global  _tpo_calc_exp_func_hash_c
	global  _tpo_calc_exp_func_hash_name_c
	global  _tpo_calc_imp_func_hash_name_c
	global  _calc_crc32_c
	global  _sys_add_tpo_mod_func_name
	global  _get_CRC_table

%else
	GLOBAL tpo_mod_imp_func_addr:function
	GLOBAL tpo_mod_imp_func_addr_c:function
	GLOBAL tpo_mod_add_func_addr_c:function
	GLOBAL tpo_get_mod_entry_hash_c:function
	GLOBAL tpo_get_mod_sec_idx_c:function
	GLOBAL tpo_get_mod_entry_idx_c:function
	GLOBAL tpo_get_fn_entry_name_c:function
	GLOBAL tpo_get_fn_entry_hash_c:function
	GLOBAL tpo_get_fn_entry_idx_c:function
	GLOBAL tpo_add_mod_c:function
	GLOBAL tpo_mod_add_section_c:function
	GLOBAL tpo_mod_add_func_c:function
	GLOBAL tpo_calc_exp_func_hash_c:function
	GLOBAL tpo_calc_exp_func_hash_name_c:function
	GLOBAL calc_crc32_c:function
	global sys_add_tpo_mod_func_name:function
%endif

_get_CRC_table:
	lea eax, [CRC32_Table]
ret

calc_crc32:
	; load arguments into registers
    
    lea ebx, [CRC32_Table]	; CRC-table
    
	; initialize
	mov eax, 0xFFFFFFFF
	xor edx, edx
	calcbyte:
		; process a single byte:
		; crc = table[(unsigned char)crc ^ byte] ^ (crc >> 8)
		mov dl, al
		xor dl, [esi]
		shr eax, 8
		xor eax, [ebx+edx*4]
		
		cmp byte [esi],0
		jz no_crc_incr
			inc esi
		no_crc_incr:
	
	dec ecx
	jnz calcbyte
	; clean up and return
ret


%ifdef PREFIX
_calc_crc32_c:
%else
calc_crc32_c:
%endif
  
  push ebp
  mov  ebp,esp

  pushad

  mov esi							,	[ebp+8]
  mov ecx							,	[ebp+12]
  
  call calc_crc32
  mov [esp+28]						,	eax
  
  popad

  mov esp,ebp
  pop ebp
  
ret

;-------------------------------------------------------
;in  :[sys_tpo_mod_hash] mod hash to find
;-------------------------------------------------------
;out :[tpo_mod_entry_ptr] mod entry if eax = 1
;-------------------------------------------------------
find_mod_entry_hash:
;-------------------------------------------------------
	xor ecx,ecx																										
	loop_find_mod_entry_hash:													
		
		cmp ecx, [sys_num_tpo_mod_loaded]				
		jge end_loop_find_mod_entry_hash
		
		mov edi, ecx
		shl edi, 4
		lea	edi, [sys_tpo_mod_loaded+edi]
		
		mov edx, [edi]						;	mod name hash 	
		cmp edx, [sys_tpo_mod_hash]
		jne next_find_find_mod_entry_hash
			mov [tpo_mod_entry_ptr]	,	edi
			mov	[sys_tpo_mod_idx]	,	ecx
			mov eax					,	1
			
			ret		
		next_find_find_mod_entry_hash:
		inc ecx
	jmp loop_find_mod_entry_hash
	end_loop_find_mod_entry_hash:
	
	;not found
	xor eax						,	eax
	mov	dword [sys_tpo_mod_idx]	,	0xFFFFFFFF
	mov dword[tpo_mod_entry_ptr],	0xFFFFFFFF
	

ret

transform_name_decoration:

	

ret



calc_import_hash:

	;find src module decoration type
	mov edi							,	[tpo_mod_entry_ptr]
	mov eax							,	[edi+8]
	mov [tpo_module_src_deco_type]	,	eax
					
	sub esp							,	256
	mov esi							,	[sys_tpo_fn_name_ptr]
	mov [sys_tpo_fn_name_ptr]		,	esp
	mov edi							,	esp
	mov al							,	0
	mov ecx							,	256
	rep stosb

	cmp dword [tpo_module_src_deco_type]	, 0
	je	tpo_module_src_no_deco
	
	cmp dword [tpo_module_src_deco_type]	, 1
	je	tpo_module_src_deco_msvc_stdcall_32
	
	cmp dword [tpo_module_src_deco_type]	, 3
	je	tpo_module_src_deco_gcc_stdcall_32
	
	;src decoration unknown
	jmp end_tpo_module_src_deco
	
	
	
	tpo_module_src_no_deco:
	

			cmp dword [tpo_module_deco_type]	,	0
			je tpo_module_src_no_deco_dst_no_deco
						
			cmp dword [tpo_module_deco_type]	,	1
			je tpo_module_src_no_deco_dst_msvc_stdcall_32
			
			cmp dword [tpo_module_deco_type]	,	3
			je tpo_module_src_no_deco_dst_gcc_stdcall_32
						
			jmp tpo_module_src_no_deco_end
	
			
			tpo_module_src_no_deco_dst_no_deco:
				mov edi			,	[sys_tpo_fn_name_ptr]
				mov ecx			,	256
				rep movsb
			jmp tpo_module_src_no_deco_end
	
			tpo_module_src_no_deco_dst_msvc_stdcall_32:
				
				cmp byte [esi],'_'
				jne src_no_deco_dst_msvc_stdcall_32_skip
					inc esi	
				src_no_deco_dst_msvc_stdcall_32_skip:

				xor ecx			,	ecx
				loop_src_no_deco_dst_msvc_stdcall_32:
					lodsb
					cmp al,0
					je end_loop_src_no_deco_dst_msvc_stdcall_32
					
					cmp al,'@'
					je end_loop_src_no_deco_dst_msvc_stdcall_32
					
					mov [esp+ecx],al
					inc ecx
				jmp loop_src_no_deco_dst_msvc_stdcall_32
				
				end_loop_src_no_deco_dst_msvc_stdcall_32:
				
				mov byte [esp+ecx],0				
						
				;mov edi			,	[sys_tpo_fn_name_ptr]
				;mov ecx			,	256
				;rep movsb				
			jmp tpo_module_src_no_deco_end

		
			tpo_module_src_no_deco_dst_gcc_stdcall_32:
				xor ecx			,	ecx
				loop_src_no_deco_dst_gcc_stdcall_32:
					lodsb
					cmp al,0
					je end_loop_src_no_deco_dst_gcc_stdcall_32
					
					cmp al,'@'
					je end_loop_src_no_deco_dst_gcc_stdcall_32
					
					mov [esp+ecx],al
					inc ecx
				jmp loop_src_no_deco_dst_gcc_stdcall_32
				
				end_loop_src_no_deco_dst_gcc_stdcall_32:
				
				mov byte [esp+ecx],0				

				;mov edi			,	[sys_tpo_fn_name_ptr]
				;mov ecx			,	256
				;rep movsb			
			jmp tpo_module_src_no_deco_end
		tpo_module_src_no_deco_end:
	
	jmp end_tpo_module_src_deco
	
	tpo_module_src_deco_msvc_stdcall_32:
			;----------------------------------------------
			;source module use msvc_stdcall_32 decoration

			cmp dword [tpo_module_deco_type]	,	0
			je tpo_module_src_deco_msvc_stdcall_32_dst_no_deco
						
			cmp dword [tpo_module_deco_type]	,	1
			je tpo_module_src_deco_msvc_stdcall_32_dst_msvc_stdcall_32
			
			cmp dword [tpo_module_deco_type]	,	3
			je tpo_module_src_deco_msvc_stdcall_32_dst_gcc_stdcall_32						
			
			
			jmp tpo_module_src_deco_msvc_stdcall_32_end
			tpo_module_src_deco_msvc_stdcall_32_dst_msvc_stdcall_32:
				;-------------------------------------------------------------
				;client module use msvc_stdcall_32 decoration
				;do nothing
				
				;mov edi			,	[sys_tpo_fn_name_ptr]
				;mov ecx			,	256
				;rep movsb
				
				xor ecx						,ecx
				loop_src_deco_msvc_stdcall_32_dst_msvc_stdcall_32:
					lodsb
					cmp al,0
					je end_loop_src_deco_msvc_stdcall_32_dst_msvc_stdcall_32
					
					cmp al,'@'
					je end_loop_src_deco_msvc_stdcall_32_dst_msvc_stdcall_32
					
					mov [esp+ecx],al
					inc ecx
				jmp loop_src_deco_msvc_stdcall_32_dst_msvc_stdcall_32
				
				end_loop_src_deco_msvc_stdcall_32_dst_msvc_stdcall_32:
				
				mov byte [esp+ecx],0				

				
			jmp tpo_module_src_deco_msvc_stdcall_32_end
				
			tpo_module_src_deco_msvc_stdcall_32_dst_gcc_stdcall_32:
				;-------------------------------------------------------------
				;client module use gcc_stdcall_32 decoration
				;add leading trail in imported function name ?
			
				mov edi			,	[sys_tpo_fn_name_ptr]
				mov al			,	'_'
				stosb
				mov ecx			,	255
				rep movsb
								
			jmp tpo_module_src_deco_msvc_stdcall_32_end
			
			tpo_module_src_deco_msvc_stdcall_32_dst_no_deco:
				;-------------------------------------------------------------
				;client module use no decoration
				;add leading trail in imported function name
				
				
				mov edi			,	[sys_tpo_fn_name_ptr]
				mov al			,	'_'
				stosb
				mov ecx			,	255
				rep movsb
					
			jmp tpo_module_src_deco_msvc_stdcall_32_end

			tpo_module_src_deco_msvc_stdcall_32_end:
		
	jmp end_tpo_module_src_deco
	
	tpo_module_src_deco_gcc_stdcall_32:
			;----------------------------------------------
			;source module use gcc_stdcall_32 decoration
			
			cmp dword [tpo_module_deco_type]	,	0
			je tpo_module_src_deco_gcc_stdcall_32_dst_no_deco
						
			cmp dword [tpo_module_deco_type]	,	1
			je tpo_module_src_deco_gcc_stdcall_32_dst_msvc_stdcall_32
			
			cmp dword [tpo_module_deco_type]	,	3
			je tpo_module_src_deco_gcc_stdcall_32_dst_gcc_stdcall_32						
			
			
			jmp tpo_module_src_deco_gcc_stdcall_32_end
			tpo_module_src_deco_gcc_stdcall_32_dst_msvc_stdcall_32:
				;-------------------------------------------------------------
				;client module use msvc_stdcall_32 decoration
				;remove leading trail ? 
				
				cmp byte [esi],'_'
				jne src_deco_gcc_stdcall_32_dst_msvc_stdcall_32_skip
					inc esi	
				src_deco_gcc_stdcall_32_dst_msvc_stdcall_32_skip:

				xor ecx			,	ecx
				loop_src_deco_gcc_stdcall_32_dst_msvc_stdcall_32:
					lodsb
					cmp al,0
					je end_loop_src_deco_gcc_stdcall_32_dst_msvc_stdcall_32
					
					cmp al,'@'
					je end_loop_src_deco_gcc_stdcall_32_dst_msvc_stdcall_32
					
					mov [esp+ecx],al
					inc ecx
				jmp loop_src_deco_gcc_stdcall_32_dst_msvc_stdcall_32
				
				end_loop_src_deco_gcc_stdcall_32_dst_msvc_stdcall_32:
				mov byte [esp+ecx],0	

				;mov edi			,	[sys_tpo_fn_name_ptr]
				;add esi			,	1
				;mov ecx			,	255
				;rep movsb				
				
			jmp tpo_module_src_deco_gcc_stdcall_32_end
				
			tpo_module_src_deco_gcc_stdcall_32_dst_gcc_stdcall_32:
				;-------------------------------------------------------------
				;client module use gcc_stdcall_32 decoration
				;do nothing
				
				xor ecx			,	ecx
				loop_src_deco_gcc_stdcall_32_dst_gcc_stdcall_32:
					lodsb
					cmp al,0
					je end_loop_src_deco_gcc_stdcall_32_dst_gcc_stdcall_32
					
					cmp al,'@'
					je end_loop_src_deco_gcc_stdcall_32_dst_gcc_stdcall_32
					
					mov [esp+ecx],al
					inc ecx
				jmp loop_src_deco_gcc_stdcall_32_dst_gcc_stdcall_32
				
				end_loop_src_deco_gcc_stdcall_32_dst_gcc_stdcall_32:
				mov byte [esp+ecx],0

				;mov edi			,	[sys_tpo_fn_name_ptr]
				;mov ecx			,	256
				;rep movsb								
				
			jmp tpo_module_src_deco_gcc_stdcall_32_end
			
			tpo_module_src_deco_gcc_stdcall_32_dst_no_deco:
				;-------------------------------------------------------------
				;client module use no decoration
				;do nothing ?

						
				mov edi			,	[sys_tpo_fn_name_ptr]
				mov ecx			,	256
				rep movsb							
								
			jmp tpo_module_src_deco_gcc_stdcall_32_end

			tpo_module_src_deco_gcc_stdcall_32_end:						
	
	end_tpo_module_src_deco:
	
	test dword [debug_id],1
	jz no_debug_1

	
	no_debug_1:
	mov esi							,[sys_tpo_fn_name_ptr]
	mov ecx							,256
	call calc_crc32
	mov [sys_tpo_fn_hash]			,eax
		
	add		esp						,256
	
	mov		eax						,1
ret


calc_export_hash:


	sub esp								,	256
	mov esi								,	[sys_tpo_fn_name_ptr]
	mov [sys_tpo_fn_name_ptr]			,	esp
		
	cmp dword [tpo_module_deco_type]	,	0
	je mod_name_no_deco
			
	cmp dword [tpo_module_deco_type]	,	1
	je mod_name_deco_msvc_stdcall_32
	
	cmp dword [tpo_module_deco_type]	,	3
	je mod_name_deco_gcc_stdcall_32
		
	jmp tpo_module_end_deco
					
	mod_name_deco_msvc_stdcall_32:
		;decoration type = msvc_stdcall_32, need to remove size of argument and the arobase
		;to make the name compatible with hash search on non decorated name
		
		
		xor ecx						,ecx
		loop_msvc_stdcall_32_find_arobase:
			lodsb
			cmp al,0
			je end_loop_msvc_stdcall_32_find_arobase
			
			cmp al,'@'
			je end_loop_msvc_stdcall_32_find_arobase
			
			mov [esp+ecx],al
			inc ecx
		jmp loop_msvc_stdcall_32_find_arobase
		
		end_loop_msvc_stdcall_32_find_arobase:
		
		mov byte [esp+ecx],0
					
	jmp tpo_module_end_deco
	
	mod_name_deco_gcc_stdcall_32:
		;decoration type =gcc_stdcall_32, need to remove size of argument and the arobase
		;to make the name compatible with hash search on non decorated name
	
		xor ecx						,ecx
		loop_gcc_stdcall_32_find_arobase:
			lodsb
			cmp al,0
			je end_loop_gcc_stdcall_32_find_arobase
			
			cmp al,'@'
			je end_loop_gcc_stdcall_32_find_arobase
			
			mov [esp+ecx],al
			inc ecx
		jmp loop_gcc_stdcall_32_find_arobase
		
		end_loop_gcc_stdcall_32_find_arobase:
		
		mov byte [esp+ecx],0
	
	jmp tpo_module_end_deco
	
	mod_name_no_deco:
		;decoration type =no decoration, just copy the function name
	
		xor ecx						,ecx
		loop_no_deco:
			lodsb
			cmp al,0
			je end_loop_no_deco
			
			mov [esp+ecx],al
			inc ecx
		jmp loop_no_deco
		
		end_loop_no_deco:
		
		mov byte [esp+ecx],0	
	jmp tpo_module_end_deco
	tpo_module_end_deco:
	

	mov esi							,[sys_tpo_fn_name_ptr]
	mov ecx							,256
	call calc_crc32
	mov [sys_tpo_fn_hash]			,eax
	
	test dword [debug_id],3
	jz no_debug
	
no_debug:

	add		esp						,256
ret




;-------------------------------------------------------
;in  :[sys_tpo_fn_hash]		func hash to find
;-------------------------------------------------------
;in :[tpo_mod_entry_ptr]	mod entry ptr
;-------------------------------------------------------
;out :[tpo_fn_entry_ptr]		fn entry ptr
;-------------------------------------------------------
find_fn_entry_hash:
;-------------------------------------------------------

	mov		edi, [tpo_mod_entry_ptr]
	movzx	eax, word [edi+6]
	shl		eax, 12
	
	
	;lea		esi, [sys_tpo_mod_exp_funcs+eax]
	
	mov			esi, [sys_tpo_mod_exp_funcs_ptr]
	lea			esi, [esi+eax]
	
	xor		ecx, ecx
		
	loop_find_fn_entry_hash:
		
	
		movzx eax,byte [edi+4]
		cmp	  ecx,eax
		jge end_loop_find_fn_entry_hash
		
		
		mov edx			,[sys_tpo_fn_hash]		; fn hash	
		cmp dword[esi]	,edx
		jne next_find_fn_entry_hash_found
			
			mov [tpo_fn_entry_ptr]	,esi
		
				
			
			mov eax					,1
			ret
		next_find_fn_entry_hash_found:
		add esi			,16
		inc ecx
		

	jmp loop_find_fn_entry_hash
	end_loop_find_fn_entry_hash:



	
	;not found
	xor eax					,eax
	mov [tpo_fn_entry_ptr]	,eax
ret

;-------------------------------------------------------
;in  :[sys_tpo_fn_hash]		func hash to add
;-------------------------------------------------------
;in :[tpo_mod_entry_ptr]	mod entry ptr
;-------------------------------------------------------
;out :[tpo_fn_entry_ptr]	fn entry ptr
;-------------------------------------------------------
add_fn_entry_hash:
;-------------------------------------------------------
	mov edi	,[tpo_mod_entry_ptr]								; mod ptr
	
	mov al,[edi+4]
	
	cmp al, 254
	jb add_fn_entry_hash_n_funcs_ok
		xor eax,eax
		ret		
	add_fn_entry_hash_n_funcs_ok:
	
	movzx eax		,	word [edi+6]				
	shl   eax		,	12

	mov			esi	, [sys_tpo_mod_exp_funcs_ptr]
	lea			esi	, [esi+eax]	
	
	xor			ecx ,  ecx
		
	loop_add_fn_entry_hash_func:
	
		movzx eax		,byte [edi+4]
		cmp ecx			,eax				; compare number of functions in the module
		jge end_loop_add_fn_entry_hash_func		
		
		mov ebx			,[sys_tpo_fn_hash]		
		cmp dword[esi]	,ebx						; compare function hash	
		je found_add_tpo_mod_hash_func
			
		add esi			,16							; next function
		inc ecx
	
	jmp loop_add_fn_entry_hash_func	
	end_loop_add_fn_entry_hash_func:
	
	;function not found, add one at the tail of the buffer
	
	
	inc byte [edi+4]										;	inc n funcs in module
	
	mov ebx				,dword [sys_tpo_fn_hash]			;	fn hash
	mov dword [esi]		,ebx
	
	found_add_tpo_mod_hash_func:
	
	mov ebx				,dword [sys_tpo_fn_addr]			;	fn ptr
	mov dword [esi+4]	,ebx
	
	mov ebx				,dword [tpo_section_data_ptr]		;	section ptr
	mov dword [esi+8]	,ebx
	
	mov ebx				,dword [string_buffer_ptr_idx]		;	string table ptr
	mov dword [esi+12]	,ebx
	
	mov eax,1
ret

;-------------------------------------------------------
;in [sys_tpo_mod_hash] dll hash to find
;-------------------------------------------------------
sys_add_tpo_mod_hash_ptr:
;-------------------------------------------------------

	call find_mod_entry_hash
	test eax,eax
	jnz end_add_tpo_mod_hash
		
	mov edi					, dword [sys_num_tpo_mod_loaded]
	shl edi					, 4
	lea	edi					, [sys_tpo_mod_loaded+edi]
	
	;dll hash
	mov eax					, [sys_tpo_mod_hash]
	mov dword[edi+0]		, eax
	
	;num funcs in the mod
	mov byte[edi+4]			,0
	
	;num sections in the mod
	mov byte[edi+5]			,0
	
	;ofset in the function table ( 256_fn*16_bytes_per_fn) )
	movzx	eax					,word [sys_num_tpo_mod_loaded]
	mov		[sys_tpo_mod_idx]	,eax
	mov		word[edi+6]			,ax
	
	;module base addr
	mov		eax					,dword [tpo_module_deco_type]
	mov		dword[edi+8]		,eax
	
	;module string table
	mov		eax					,dword [string_buffer_ptr_tpo]
	mov		dword[edi+12]		,eax
	
	mov		[tpo_mod_entry_ptr]	,edi
	
	inc dword [sys_num_tpo_mod_loaded]
		
	end_add_tpo_mod_hash:
	mov eax					,[tpo_mod_entry_ptr]	
ret




;-------------------------------------------------------
;in [sys_tpo_mod_hash] dll hash to add
;in [sys_tpo_fn_hash] function hash to add
;in [sys_tpo_fn_addr] function hash to add
;-------------------------------------------------------
sys_add_tpo_mod_func:
;-------------------------------------------------------
	
	call	sys_add_tpo_mod_hash_ptr
	
	mov	[tpo_mod_entry_ptr]	,eax
	
	call add_fn_entry_hash
	
	
	
ret

;-------------------------------------------------------
;in [sys_tpo_mod_hash] dll hash to get
;in [sys_tpo_fn_hash] function hash to get
;-------------------------------------------------------
sys_get_tpo_mod_func_addr:
;-------------------------------------------------------
	
	xor		eax,eax
	call	find_mod_entry_hash				;find module entry
	test	eax,eax
	jz		error_get_tpo_mod_func
	
	
	call	find_fn_entry_hash				;find function entry
	test	eax,eax
	jz		error_get_tpo_mod_func
	
	mov		esi,[tpo_fn_entry_ptr]
	mov		eax,[esi+4]						;return function address in eax
	
	ret
	
error_get_tpo_mod_func:
	mov eax	,0xFFFFFFFF
ret	


%ifdef PREFIX
_sys_add_tpo_mod_func_name:
%else
sys_add_tpo_mod_func_name:
%endif
  
  push ebp
  mov  ebp, esp
  pusha

  mov	eax						,	[ebp+8]
  mov	[sys_tpo_mod_name_ptr]	,	eax
  
  mov	eax						,	[ebp+12]
  mov	[sys_tpo_fn_name_ptr]	,	eax

  mov	eax						,	[ebp+16]
  mov	[sys_tpo_fn_addr]		,	eax
  
  mov	eax						,	[ebp+20]
  mov   [tpo_module_deco_type]	,	eax
 
  mov esi						,	[sys_tpo_mod_name_ptr]
  mov ecx						,	64
  call calc_crc32
  mov [sys_tpo_mod_hash]		,	eax
			
  call	calc_export_hash	
  call	sys_add_tpo_mod_func

  mov dword [ret_val],esi
  popa
  mov esp,ebp
  pop ebp
  
  mov eax,[ret_val]   
  
ret


;-------------------------------------------------------
;in (char *) dll name to get
;in (char *) function name to get
;-------------------------------------------------------
sys_get_tpo_mod_func_name:
  
  mov	eax							,	[esp+4]
  mov	[sys_tpo_mod_name_ptr]		,	eax
  
  mov	eax							,	[esp+8]
  mov	[sys_tpo_fn_name_ptr]		,	eax

  mov	eax							,	[esp+12]
  mov	[tpo_module_deco_type]		,	eax

  mov esi							,	[sys_tpo_mod_name_ptr]
  mov ecx							,	64
  call calc_crc32
  mov [sys_tpo_mod_hash]			,	eax
  
  
  
  call	find_mod_entry_hash
  test	eax							,	eax
  jnz	get_mod_func_name_import_hash
  mov	eax,0xFFFFFFFF
  ret

  get_mod_func_name_import_hash:  
		
    
  call	calc_import_hash
  call	find_fn_entry_hash				;find function entry
  test	eax,eax
  jnz	get_mod_func_name_ok
  mov	eax,0xFFFFFFFF
  ret	
	
  get_mod_func_name_ok:
  
 
    
  mov		esi,[tpo_fn_entry_ptr]
  mov		eax,[esi+4]						;return function address in eax
ret



;------------------------------------------------------------------------------------------------------------------------------------------------
;			unsigned int 	tpo_mod_imp_func_addr_c	(unsigned int mod_hash,unsigned int crc_func);
;------------------------------------------------------------------------------------------------------------------------------------------------

%ifdef PREFIX 
_tpo_mod_imp_func_addr_c: 
%else 
tpo_mod_imp_func_addr_c: 
%endif
;------------------------------------------------------------------------------------------------------------------------------------------------

	mov eax						,[esp+4]				; import dll hash
	mov [sys_tpo_mod_hash]		,eax
	
	
	mov eax						,[esp+8]				; import fn hash
	mov [sys_tpo_fn_hash]		,eax
	
	
	
	pusha
		call sys_get_tpo_mod_func_addr
		mov [tpo_imp_exp_ret],eax
	popa
	
	mov eax,[tpo_imp_exp_ret]
ret

;------------------------------------------------------------------------------------------------------------------------------------------------
;			unsigned int 	tpo_mod_add_func_addr_c			(unsigned int mod_hash,unsigned int crc_func,unsigned int func_addr);
;------------------------------------------------------------------------------------------------------------------------------------------------
%ifdef PREFIX 
_tpo_mod_add_func_addr_c: 
%else 
tpo_mod_add_func_addr_c: 
%endif
;------------------------------------------------------------------------------------------------------------------------------------------------

	mov eax,	[esp+4]
	mov [sys_tpo_mod_hash]		,eax
	
	mov eax,	[esp+8]
	mov [sys_tpo_fn_hash]		,eax
	
	mov eax,	[esp+12]
	mov [sys_tpo_fn_addr]		,eax
	
	pusha	
		call sys_add_tpo_mod_func
		mov [tpo_add_exp_ret],eax
	popa
	
	mov eax,[tpo_add_exp_ret]

ret

;------------------------------------------------------------------------------------------------------------------------------------------------
;			struct kern_mod_t	*KERN_API_FUNC 	tpo_get_mod_entry_hash_c		(unsigned int mod_hash);
;------------------------------------------------------------------------------------------------------------------------------------------------
%ifdef PREFIX 
_tpo_get_mod_entry_hash_c: 
%else 
tpo_get_mod_entry_hash_c:  
%endif
;------------------------------------------------------------------------------------------------------------------------------------------------

	mov eax							,[esp+4]				; import dll hash
	mov [sys_tpo_mod_hash]		,eax
	
	pusha 
		call find_mod_entry_hash
	popa
	
	mov eax	,[tpo_mod_entry_ptr]

ret




;------------------------------------------------------------------------------------------------------------------------------------------------
;			struct kern_mod_sec_t	*KERN_API_FUNC 	tpo_get_mod_sec_idx_c		(unsigned int mod_idx,unsigned int sec_idx);
;------------------------------------------------------------------------------------------------------------------------------------------------
%ifdef PREFIX 
_tpo_get_mod_sec_idx_c: 
%else 
tpo_get_mod_sec_idx_c: 
%endif
;-------------------------------------------------------
	push ebx
	mov eax		,	[esp+8]				; import dll idx
	mov ebx		,	[esp+12]
	
	
	shl eax		,	4
	lea	eax		,	[sys_tpo_mod_loaded+eax]	

	cmp bl						,	[eax+5]
	jl _tpo_get_mod_sec_idx_c_ok_2
		xor eax,eax
		pop ebx
	ret
	_tpo_get_mod_sec_idx_c_ok_2:
	
	mov eax		,	[esp+8]				; import dll idx

	cmp eax, [sys_num_tpo_mod_loaded]				
	jl _tpo_get_mod_sec_idx_c_ok
		xor eax,eax
		pop ebx
	ret

	_tpo_get_mod_sec_idx_c_ok:
	
	shl eax						,	7
	lea	eax						,	[sys_tpo_mod_sections+eax]
	lea eax						,	[eax+ebx*8]
	
	pop ebx

ret	



;------------------------------------------------------------------------------------------------------------------------------------------------
;			struct kern_mod_t	*KERN_API_FUNC 	tpo_get_mod_entry_idx_c		(unsigned int idx);
;------------------------------------------------------------------------------------------------------------------------------------------------
%ifdef PREFIX 
_tpo_get_mod_entry_idx_c: 
%else 
tpo_get_mod_entry_idx_c: 
%endif
;------------------------------------------------------------------------------------------------------------------------------------------------

	mov eax	,[esp+4]				; import dll idx
	
	cmp eax, [sys_num_tpo_mod_loaded]				
	jl tpo_get_mod_idx_c_ok
	xor eax,eax
	ret
		
	tpo_get_mod_idx_c_ok:
	
	shl eax, 4
	lea	eax, [sys_tpo_mod_loaded+eax]

ret

;------------------------------------------------------------------------------------------------------------------------------------------------
;			kern_mod_fn_t *_tpo_get_fn_entry_name_c	(unsigned int mod_idx,unsigned int mod_hash,unsigned int str_idx,unsigned int deco_type);
;------------------------------------------------------------------------------------------------------------------------------------------------
%ifdef PREFIX 
_tpo_get_fn_entry_name_c: 
%else 
tpo_get_fn_entry_name_c: 
%endif
;------------------------------------------------------------------------------------------------------------------------------------------------

  mov	eax							,	[esp+4]
  mov	[sys_tpo_mod_idx]			,	eax
  
  mov	eax							,	[esp+8]
  mov	[sys_tpo_mod_hash]			,	eax
  
  mov	eax							,	[esp+12]
  mov	[string_buffer_ptr_idx]		,	eax

  mov	eax							,	[esp+16]
  mov	[tpo_module_deco_type]		,	eax
  
  pusha
  
  mov	eax							,	[sys_tpo_mod_idx]
  shl   eax							,	4
  lea	edi							,	[sys_tpo_mod_loaded+eax]
	
  mov eax							,	[edi+12]
  mov [string_buffer_ptr_tpo]		,	eax
  add eax							,   [string_buffer_ptr_idx]
  mov  [sys_tpo_fn_name_ptr]		,	eax



  call	find_mod_entry_hash
  test	eax							,	eax
  jnz	tpo_get_fn_entry_name_c_hash_ok
	popa
	mov		eax,0xFFFFFFFF
  ret
	
  tpo_get_fn_entry_name_c_hash_ok:
  
 
    
  call	calc_import_hash
  call	find_fn_entry_hash			;find function entry
  
  
  test	eax,eax
  jnz	tpo_get_fn_entry_name_c_ok
	popa
	mov	eax,0xFFFFFFFF
  ret	
	
  tpo_get_fn_entry_name_c_ok:
  


  
	popa
    mov		eax,[tpo_fn_entry_ptr]
 
ret



;------------------------------------------------------------------------------------------------------------------------------------------------
;			kern_mod_fn_t *tpo_get_fn_entry_hash_c	(unsigned int mod_hash,unsigned int crc_func);
;------------------------------------------------------------------------------------------------------------------------------------------------
%ifdef PREFIX 
_tpo_get_fn_entry_hash_c: 
%else 
tpo_get_fn_entry_hash_c: 
%endif
;------------------------------------------------------------------------------------------------------------------------------------------------

	mov eax						,[esp+4]				; import dll hash
	mov [sys_tpo_mod_hash]		,eax
	
	
	mov eax						,[esp+8]				; import fn hash
	mov [sys_tpo_fn_hash]		,eax
	
	pusha 
		call find_mod_entry_hash
		test eax,eax
		jz error_get_fn_entry_hash_c
		call find_fn_entry_hash
		error_get_fn_entry_hash_c:
	popa
	
	mov eax,[tpo_fn_entry_ptr]
ret

;------------------------------------------------------------------------------------------------------------------------------------------------
;		kern_mod_fn_t *tpo_get_fn_entry_idx_c	(unsigned int mod_hash,unsigned int idx_func); 
;------------------------------------------------------------------------------------------------------------------------------------------------
%ifdef PREFIX 
_tpo_get_fn_entry_idx_c: 
%else 
_tpo_get_fn_entry_idx_c:  
%endif
;------------------------------------------------------------------------------------------------------------------------------------------------

	mov eax						,[esp+4]				; import dll hash
	mov [sys_tpo_mod_hash]		,eax
	
	
	mov eax						,[esp+8]				; import fn idx
	mov [sys_tpo_fn_idx]		,eax
	
	pusha 
		call find_mod_entry_hash
		test eax,eax
		jz error_get_fn_entry_idx_c
		
			mov		edi ,[tpo_mod_entry_ptr]
			
			;num functions in the module
			
			movzx	eax		,	byte	[edi+4]
			
			cmp		[sys_tpo_fn_idx], eax		
			jge		error_get_fn_entry_idx_c
			
				mov		ebx					,	[sys_tpo_fn_idx]
				shl		ebx					,	4

				movzx	eax					,	word [edi+6]
				shl		eax					,	12
				add		eax					,	ebx
				
				mov			esi, [sys_tpo_mod_exp_funcs_ptr]
				lea			esi, [esi+eax]
					
				;lea		esi					,	[sys_tpo_mod_exp_funcs+eax]
				
				mov		[tpo_fn_entry_ptr]	,	esi 
			jmp end_get_fn_entry_idx_c
		
		    error_get_fn_entry_idx_c:
				mov		dword [tpo_fn_entry_ptr]	,	0xFFFFFFFF
		
			end_get_fn_entry_idx_c:
	popa
	
	mov eax,[tpo_fn_entry_ptr]
ret


;------------------------------------------------------------------------------------------------------------------------------------------------
;		unsigned int 		tpo_add_mod_c			(unsigned int mod_hash,unsigned int deco_type	,unsigned int string_table_addr);
;------------------------------------------------------------------------------------------------------------------------------------------------
%ifdef PREFIX 
_tpo_add_mod_c: 
%else 
tpo_add_mod_c:  
%endif
;------------------------------------------------------------------------------------------------------------------------------------------------
	mov eax							,[esp+4]
	mov [sys_tpo_mod_hash]			,eax
	
	mov eax							,[esp+8]
	mov [tpo_module_deco_type]		,eax
	
	mov eax							,[esp+12]
	mov [string_buffer_ptr_tpo]		,eax

	pusha 
		call sys_add_tpo_mod_hash_ptr
	popa
	
	mov eax,[sys_tpo_mod_idx]
ret

_tpo_mod_add_section:
	
	mov	eax				,	[sys_tpo_mod_idx]
	shl eax				,	4
	lea	edi				,	[sys_tpo_mod_loaded+eax]
	
	mov	eax				,	[sys_tpo_mod_idx]
	shl eax				,	7
	lea	esi				,	[sys_tpo_mod_sections+eax]
		
	movzx eax			,	byte [edi+5]
	lea	  esi			,	[esi+eax*8]
	
	mov	  eax			,	[tpo_section_data_ptr]
	mov	  [esi]			,	eax
	
	mov	  eax			,	[tpo_section_size]
	mov	  [esi+4]		,	eax
	
	inc	byte [edi+5]

ret

;------------------------------------------------------------------------------------------------------------------------------------------------
;		unsigned int 		tpo_mod_add_section_c			(unsigned int mod_idx,unsigned int section_addr,unsigned int section_size);
;------------------------------------------------------------------------------------------------------------------------------------------------
%ifdef PREFIX 
_tpo_mod_add_section_c: 
%else 
tpo_mod_add_section_c:  
%endif
;------------------------------------------------------------------------------------------------------------------------------------------------
	mov eax							,[esp+4]
	mov [sys_tpo_mod_idx]			,eax
	
	mov eax							,[esp+8]
	mov [tpo_section_data_ptr]		,eax

	mov eax							,[esp+12]
	mov [tpo_section_size]			,eax
	
	pusha
		
		call _tpo_mod_add_section
		
	popa

	
ret

;------------------------------------------------------------------------------------------------------------------------------------------------
;		unsigned int 		tpo_mod_add_func_c				(unsigned int mod_idx ,unsigned int func_addr	,unsigned int func_type,unsigned int string_id);
;------------------------------------------------------------------------------------------------------------------------------------------------
%ifdef PREFIX 
_tpo_mod_add_func_c: 
%else 
tpo_mod_add_func_c: 
%endif
;------------------------------------------------------------------------------------------------------------------------------------------------	
	mov eax							,[esp+4]
	mov [sys_tpo_mod_idx]			,eax
	
	mov eax							,[esp+8]
	mov [sys_tpo_fn_addr]			,eax

	mov eax							,[esp+12]
	mov [sys_tpo_fn_type]			,eax

	mov eax							,[esp+16]
	mov [string_buffer_ptr_idx]		,eax

	pusha
		
		mov	eax						,	[sys_tpo_mod_idx]
		shl eax						,	4
		lea	edi						,	[sys_tpo_mod_loaded+eax]
		mov [tpo_mod_entry_ptr]		,	edi
		
		mov eax						,	[edi+0]
		mov [sys_tpo_mod_hash]		,	eax
		
		mov eax						,	[edi+8]
		mov [tpo_module_deco_type]	,	eax		
		
		mov esi						,	[string_buffer_ptr_idx]
		add esi						,	[edi+12]
		mov [sys_tpo_fn_name_ptr]	,	esi
		
		call calc_export_hash
		call add_fn_entry_hash
		
		
	popa
	
	
	
	mov eax,[tpo_add_exp_ret]
ret

;------------------------------------------------------------------------------------------------------------------------------------------------
;unsigned int 		tpo_calc_exp_func_hash_c	(unsigned int mod_idx ,unsigned int string_id);
;------------------------------------------------------------------------------------------------------------------------------------------------
%ifdef PREFIX 
_tpo_calc_exp_func_hash_c: 
%else 
tpo_calc_exp_func_hash_c: 
%endif
;------------------------------------------------------------------------------------------------------------------------------------------------

  mov	eax						,	[esp+4]
  mov	[sys_tpo_mod_idx]		,	eax
  
  mov	eax						,	[esp+8]
  mov	[string_buffer_ptr_idx]	,	eax


  pusha 
	  mov	eax							,	[sys_tpo_mod_idx]
	  shl   eax							,	4
	  lea	edi							,	[sys_tpo_mod_loaded+eax]
		
	  mov eax							,	[edi+12]
	  mov [string_buffer_ptr_tpo]		,	eax
	  add eax							,   [string_buffer_ptr_idx]
	  mov  [sys_tpo_fn_name_ptr]		,	eax
	  
	  mov eax							,	[edi+8]
	  mov [tpo_module_deco_type]		,	eax
	  
	  call	calc_export_hash	
 popa
 
	mov eax, [sys_tpo_fn_hash]

ret


;------------------------------------------------------------------------------------------------------------------------------------------------
;unsigned int 	tpo_calc_exp_func_hash_name_c	(char *func_name ,unsigned int deco_type);
;------------------------------------------------------------------------------------------------------------------------------------------------
%ifdef PREFIX 
_tpo_calc_exp_func_hash_name_c: 
%else 
_tpo_calc_exp_func_hash_name_c: 
%endif
;------------------------------------------------------------------------------------------------------------------------------------------------

  mov	eax							,	[esp+4]
  mov	[sys_tpo_fn_name_ptr]		,	eax
  
  mov	eax							,	[esp+8]
  mov	[tpo_module_deco_type]		,	eax


  pusha 
	  call	calc_export_hash	
  popa
 
	mov eax, [sys_tpo_fn_hash]

ret



;------------------------------------------------------------------------------------------------------------------------------------------------
;unsigned int 	tpo_calc_imp_func_hash_name_c	(char *func_name ,unsigned int src_deco_type,unsigned int deco_type);
;------------------------------------------------------------------------------------------------------------------------------------------------
%ifdef PREFIX 
_tpo_calc_imp_func_hash_name_c: 
%else 
tpo_calc_imp_func_hash_name_c:  
%endif
;------------------------------------------------------------------------------------------------------------------------------------------------

  mov	eax								,	[esp+4]
  mov	[sys_tpo_fn_name_ptr]			,	eax
  

  mov	eax								,	[esp+8]
  mov	[tpo_module_deco_type]			,	eax

  
  mov	eax								,	[esp+12]
  mov	[tpo_module_src_deco_type]		,	eax


  pusha 
	  call	calc_import_hash	
  popa
 
	mov eax, [sys_tpo_fn_hash]

ret

