//copyright antoine bentue-ferrer 2016
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "coff.h"


char *FindElfString(PEFile *PE,int string_ofs)
{
	int n;

	for(n = 0; n<PE->num_strings; n++)
	{
		if( (string_ofs >= PE->Strings[n].start_ofs)&&
			(string_ofs < PE->Strings[n].end_ofs))
		{
			int ofs = string_ofs - PE->Strings[n].start_ofs;
			return &PE->Strings[n].string[ofs];
		}
	}

	return NULL;
}


char *FindElfeString(PEFile *PE,int section_id,int string_ofs)
{
	int n;

	for(n = 0; n<PE->num_estrings;n++)
	{
		if ((PE->eStrings[n].section_id != section_id))continue;
		
		if( (string_ofs >= PE->eStrings[n].start_ofs)&&
			(string_ofs <  PE->eStrings[n].end_ofs))
		{
			int ofs = string_ofs - PE->eStrings[n].start_ofs;
			return &PE->eStrings[n].string[ofs];
		}
	}

	return NULL;
}


section_imp_name *FindeImpSym(PEFile *PE,int sym_id)
{
	int n;

	for(n = 0; n<PE->num_esection; n++)
	{
		int nn;
		for(nn = 0; nn<PE->eSections[n]->num_sec_imp_name; nn++)
		{
			if(PE->eSections[n]->ImportsName[nn].sym_idx == sym_id)
				return &PE->eSections[n]->ImportsName[nn];
		}
	}
	return NULL;
}


Elf32_Sym *FindSym(PEFile *PE,int sym_id,char *o_sym_name)
{
	int			i;
	Section		*sec;


	for(i = 0; i<PE->num_section; i++)
	{
		int n;

		sec = PE->Sections[i];

		if (sec->phdr.p_type != PT_DYNAMIC)continue;
					
		for(n =0; n < sec->SectionDataLen, ((Elf32_Dyn *)&sec->Data[n])->d_tag != DT_NULL; n += sizeof(Elf32_Dyn))
		{
			Elf32_Dyn *dyn = (Elf32_Dyn *)&sec->Data[n];
			char *sym_table;
			unsigned int mem_ofs, sym_table_size;
			int nn;
			int s_id;
			int	sec_id;

			if (dyn->d_tag != DT_SYMTAB)continue;

			sec_id = FindSectionMem(PE,dyn->d_un.d_ptr);
			if (sec_id < 0)
			{
				printf("section not found 'FindSym' \n");
				exit(0);
			}

			mem_ofs	= dyn->d_un.d_ptr-PE->Sections[sec_id]->phdr.p_vaddr;
			sym_table =	&PE->Sections[sec_id]->Data[mem_ofs];
			sym_table_size = PE->Sections[sec_id]->SectionDataLen-mem_ofs;

			for(s_id = 0, nn = 0; nn < sym_table_size; s_id++, nn += sizeof(Elf32_Sym))
			{
				if (s_id == sym_id)
				{
					Elf32_Sym	*SymTbl;
					char		*sym_name;

					SymTbl = (Elf32_Sym *)&sym_table[nn];
					sym_name = FindElfString(PE, SymTbl->st_name);

					if (sym_name)
						strcpy(o_sym_name, sym_name);
					else
						strcpy(o_sym_name, "");

					return SymTbl;
				}
			}
		}
	}

	return NULL;
}


Elf32_Sym	*FindeSym(PEFile *PE,int sym_id,char *o_sym_name)
{
	int			n;
	Section		*sec;

	for(n = 0; n<PE->num_esection; n++)
	{
		int nn;
		int s_id;

		sec=PE->eSections[n];

		//if(sec->shdr.sh_type==SHT_SYMTAB)
		if (sec->shdr.sh_type != SHT_DYNSYM)continue;

		for(nn = 0, s_id = 0; nn<sec->SectionDataLen; s_id++ , nn += sizeof(Elf32_Sym))
		{
			Elf32_Sym	*SymTbl;
			char		*sym_name;
			if (s_id != sym_id)continue;
			
			SymTbl		=	(Elf32_Sym *)&sec->Data[nn];
			sym_name	=	FindElfeString(PE,sec->shdr.sh_link,SymTbl->st_name);

			if(sym_name)
				strcpy(o_sym_name,sym_name);
			else
				strcpy(o_sym_name,"");

			return SymTbl;
			
		}
	}

	return NULL;
}

void ReadStringSection(unsigned int sec_id,unsigned char *str_sec, unsigned int size,string_el *Strings,unsigned int *num_str)
{
	unsigned int n;
	unsigned int start_str,str_id;

	
	start_str			=	0;
	str_id				=	1;

	for(n = 0; n<size; n++)
	{
		if((str_sec[n]==0)&&(str_sec[n+1]==0))break;
		if(str_sec[n]==0)
		{
			int str_len;

			str_len = n - start_str;

			Strings[*num_str].string = calloc(str_len+1,1);

			if(str_len>0)
				memcpy(Strings[*num_str].string,&str_sec[start_str],str_len);

			Strings[*num_str].string[str_len] = 0;
			Strings[*num_str].section_id = sec_id;
			Strings[*num_str].start_ofs = start_str;
			Strings[*num_str].end_ofs = Strings[*num_str].start_ofs+str_len;

			str_id++;
			start_str = n+1;
			(*num_str)++;
		}
	}

}
void ReadElfFile(char *file,PEFile *PE)
{
	int				i;
	FILE			*in;

	in=fopen(file,"rb");
	if(!in)
	{
		printf("could not open file '%s' \n" ,file);
		return;
	}

	strcpy(PE->file_name,file);

	for(i=0;i<strlen(PE->file_name);i++)
	{
		if(PE->file_name[i]=='\\')PE->file_name[i]='/';
	}

	fread	(&PE->ehdr,sizeof(Elf32_Ehdr),1,in);

	PE->num_libs			=	0;
	PE->num_esection		=	PE->ehdr.e_shnum;
	PE->num_estrings		=	0;

	for(i=0;i<PE->num_esection;i++)
	{

		PE->eSections[i] = (Section *) calloc(sizeof(Section),1);
		fseek	(in,PE->ehdr.e_shoff+i*PE->ehdr.e_shentsize,SEEK_SET);
		fread	(&PE->eSections[i]->shdr,sizeof(Elf32_Shdr),1,in);

		PE->eSections[i]->SectionDataLen = PE->eSections[i]->shdr.sh_size;

		if(PE->eSections[i]->shdr.sh_type==SHT_STRTAB)
		{
			char *str_sec;
			
			
			str_sec		=	calloc(PE->eSections[i]->shdr.sh_size,1);
			fseek	(in,PE->eSections[i]->shdr.sh_offset+1		,SEEK_SET);
			fread	(str_sec,PE->eSections[i]->shdr.sh_size,1,in);

			ReadStringSection(i,str_sec,PE->eSections[i]->SectionDataLen,PE->eStrings,&PE->num_estrings);
			
			/*
			while(n<PE->eSections[i]->shdr.sh_size)
			{
				if(str_sec[n]==0)
				{
					int str_len;

					str_len		=	n-start_str;
					PE->eStrings[PE->num_estrings].string=calloc(str_len+1,1);
					memcpy(PE->eStrings[PE->num_estrings].string,&str_sec[start_str],str_len+1);

					PE->eStrings[PE->num_estrings].section_id	=i;
					PE->eStrings[PE->num_estrings].start_ofs	=start_str+1;
					PE->eStrings[PE->num_estrings].end_ofs	=PE->eStrings[PE->num_estrings].start_ofs+str_len;
					str_id++;
					start_str	=	n+1;
					PE->num_estrings++;
				}
				n++;
			}
			*/

			free(str_sec);

		}
	}
	
	for(i=0;i<PE->num_esection;i++)
	{
		char *sec_name;
		sec_name=FindElfeString(PE,PE->ehdr.e_shstrndx,PE->eSections[i]->shdr.sh_name);

		if(sec_name!=NULL)
			strcpy(PE->eSections[i]->Name,sec_name);

		PE->eSections[i]->v_addr			=	PE->eSections[i]->shdr.sh_addr;

	}

	PE->ImageBase		=	0xFFFFFFFF;
	PE->num_section		=	PE->ehdr.e_phnum;
	PE->num_strings		=	0;
	for(i=0;i<PE->num_section;i++)
	{
		unsigned int	sec_memory_size;
		unsigned int	sec_hdr_ofset;
		unsigned int	sec_dat_ofset;
		
		PE->Sections[i]	= (Section *) calloc(sizeof(Section),1);
		
		sec_hdr_ofset = PE->ehdr.e_phoff+i*PE->ehdr.e_phentsize;
		
		fseek	(in,sec_hdr_ofset,SEEK_SET);
		fread	(&PE->Sections[i]->phdr,sizeof(Elf32_Phdr),1,in);

		PE->Sections[i]->SectionDataLen = PE->Sections[i]->phdr.p_memsz;
		
		sec_dat_ofset = PE->Sections[i]->phdr.p_offset;
		
		if(PE->Sections[i]->phdr.p_type==PT_LOAD)
		{
			PE->Sections[i]->v_addr				=PE->Sections[i]->phdr.p_vaddr;
			
			if(PE->Sections[i]->v_addr<PE->ImageBase)
				PE->ImageBase=PE->Sections[i]->v_addr;

			printf("section %d , vaddr : %x , ofset file %d, size %d \n",i,PE->Sections[i]->v_addr,sec_dat_ofset,PE->Sections[i]->SectionDataLen);
		}
	}

	for(i=0;i<PE->num_section;i++)
	{
		
		if(PE->Sections[i]->phdr.p_type==PT_DYNAMIC)
		{
			unsigned char *dyn_sec;
			int	 n;
			
			dyn_sec	=	(unsigned char *)calloc(PE->Sections[i]->phdr.p_filesz,1);

			fseek	(in,PE->Sections[i]->phdr.p_offset		,SEEK_SET);
			fread	(dyn_sec,PE->Sections[i]->phdr.p_filesz,1,in);

			for (n = 0;n < PE->Sections[i]->phdr.p_filesz;	n += sizeof(Elf32_Dyn))
			{
				Elf32_Dyn *dyn = (Elf32_Dyn *)&dyn_sec[n];

				if(dyn->d_tag==DT_STRTAB)
				{
					int				sec_id;
										int				mem_ofs;
					char			*str_table;
					unsigned char	*sec_data;
					Section			*sec;

					sec_id = FindSectionMem(PE,dyn->d_un.d_ptr);
					if(sec_id>=0)
					{
						sec	= PE->Sections[sec_id];
						sec_data = (unsigned char *)calloc(sec->phdr.p_filesz,1);

						fseek		(in,sec->phdr.p_offset,SEEK_SET);
						fread		(sec_data,sec->phdr.p_filesz,1,in);

						mem_ofs	= dyn->d_un.d_ptr-sec->phdr.p_vaddr;
						str_table =	&sec_data[mem_ofs];

						ReadStringSection	(sec_id,str_table,sec->phdr.p_filesz-mem_ofs,PE->Strings,&PE->num_strings);
						free				(sec_data);
					}
				}
				if(dyn->d_tag==DT_NULL)break;
			}
			free(dyn_sec);
		}
	}

	fclose(in);
}




void ReadElfeImpExp(PEFile *PE)
{
	FILE *in;
	unsigned int i;
	Section		*sec;
	in=fopen(PE->file_name,"rb");

	for(i=0;i<PE->num_esection;i++)
	{
		sec			= PE->eSections[i];
		Elf32_Sym	*SymTbl;
		int			n, s_id;

		//if(sec->shdr.sh_type==SHT_SYMTAB)
		if (sec->shdr.sh_type != SHT_DYNSYM)continue;
			
		for(s_id = 0, n = 0; n < sec->SectionDataLen; s_id++ , n += sizeof(Elf32_Sym))
		{
			int		bind,type;
			char	*sym_name, *sec_name;
						
			SymTbl	=(Elf32_Sym *)&sec->Data[n];
			bind	=ELF32_ST_BIND(SymTbl->st_info);
			type	=ELF32_ST_TYPE(SymTbl->st_info);

			if(type!=3)
			{
				sym_name = FindElfeString(PE,sec->shdr.sh_link,SymTbl->st_name);

				if((SymTbl->st_shndx>0)&&(SymTbl->st_shndx<PE->num_esection))
					sec_name = PE->eSections[SymTbl->st_shndx]->Name;
				else
					sec_name = "";

				printf("section [%d]=>[%d,%s]: (%d,%d) ",i,SymTbl->st_shndx,sec_name,bind,type);
				
				 if((type==STT_FILE)&&(sym_name!=NULL))
					printf("source file %s",sym_name);
				 else if((type==STT_FUNC)&&(sym_name!=NULL))
					printf("func %s ",sym_name);
				 else if((type==STT_OBJECT)&&(sym_name!=NULL))
					printf("object %s ",sym_name);
				 else if(sym_name!=NULL)
					 printf("no type %s  ",sym_name);

				 printf("size : %d, value %x",SymTbl->st_size,SymTbl->st_value);
				 printf("\r\n");

				 if (sec->num_sec_imp_name < MAX_IMPORT)
				 {
					 if ((bind == 1) && (type == 0) && (SymTbl->st_shndx == 0))
					 {
						 strcpy(sec->ImportsName[sec->num_sec_imp_name].func_name, sym_name);
						 sec->ImportsName[sec->num_sec_imp_name].addr_reloc = SymTbl->st_value;
						 sec->ImportsName[sec->num_sec_imp_name].sym_idx = s_id;
						 sec->num_sec_imp_name++;
					 }
				 }
				 else
				 {
					 printf("too much imports ");
					 exit(0);
				 }

				 if((bind==1)&&((SymTbl->st_shndx>0)&&(SymTbl->st_shndx<PE->num_esection)))
				 {
					Section	 *t_sec;

					t_sec = PE->eSections[SymTbl->st_shndx];

					if (t_sec->num_sec_exp_name < MAX_EXPORT)
					{
						strcpy(t_sec->ExportsName[t_sec->num_sec_exp_name].func_name, sym_name);

						t_sec->ExportsName[t_sec->num_sec_exp_name].addr_reloc = SymTbl->st_value;
						t_sec->ExportsName[t_sec->num_sec_exp_name].sym_idx	= s_id;

						t_sec->num_sec_exp_name++;
					}
					else
					{
						printf	("too much exports ");
						exit	(0);
					}
				 }
			}
		}
	}


	for(i=0;i<PE->num_esection;i++)
	{
		Section					*sec = PE->eSections[i];

		if(sec->shdr.sh_type==SHT_REL)
		{
			Elf32_Rel *rel;
			int n;

			for(n = 0;n < sec->SectionDataLen; n += sizeof(Elf32_Rel))
			{
				unsigned int		 sym_id,type;
				section_imp_name	*sec_imp;
				char				sym_name[256];
				Elf32_Sym			*Sym_Tbl;
				

				rel		=	(Elf32_Rel *)&PE->eSections[i]->Data[n];

				sym_id	=	ELF32_R_SYM(rel->r_info);
				type	=	ELF32_R_TYPE(rel->r_info);
				sec_imp	=	FindeImpSym(PE,sym_id);

				printf("reloc : %x - %d,%d ",rel->r_offset,sym_id,type);
				
				if(sec_imp!=NULL)
					printf(" (%s) ",sec_imp->func_name);
				else
				{
					Sym_Tbl		=	FindeSym(PE,sym_id,sym_name);
					if(Sym_Tbl)
						printf(" (** %s) ",sym_name);

				}
				printf("\r\n");
			}
		}

		if(sec->shdr.sh_type==SHT_RELA)
		{
			Elf32_Rela *rela;
			int			n;

			n=0;

			for(n=0; n<sec->SectionDataLen; n += sizeof(Elf32_Rela))
			{
				unsigned int		sym_id,type;
				section_imp_name	*sec_imp;

				rela	=	(Elf32_Rela *)&PE->eSections[i]->Data[n];

				sym_id	=	ELF32_R_SYM(rela->r_info);
				type	=	ELF32_R_TYPE(rela->r_info);
				sec_imp	=	FindeImpSym(PE,sym_id);

				printf("relocA : %x - %d,%d - %x \r\n",rela->r_offset,sym_id,type,rela->r_addend);
			}			
		}
	}
	fclose(in);
}

void ReadElfSection(PEFile *PE,char *name)
{
	int				i;
	int				secid;
	FILE			*in;
	Section			*Sec;

	in=fopen(PE->file_name,"rb");
	if(!in)
	{
		printf("could not open file '%s' \n" ,PE->file_name);
		return;

	}

	Sec		=	NULL;
	secid	=	-1;

	for(i=0;i<PE->num_esection;i++)
	{
		if(!strcmp(PE->eSections[i]->Name,name))
		{
			Sec		=	PE->eSections[i];
			secid	=	i;
		}
	}

	if(Sec==NULL)return;
		
	
	Sec->Data			=	calloc(Sec->SectionDataLen,1);

	fseek	(in,Sec->shdr.sh_offset,SEEK_SET);
	fread	(Sec->Data,Sec->SectionDataLen,1,in);
}


void ReadElfSections(PEFile *PE)
{	
	
	int i;

	for(i=0;i<PE->num_esection;i++)
	{
		ReadElfSection(PE,PE->eSections[i]->Name);
	}
}



void BuildElf_RVA(PEFile *PE)
{
	Section *sec;
	unsigned int rel_sz;
	int i;
	
	rel_sz = 0;

	for(i=0; i < PE->num_section; i++)
	{
		sec = PE->Sections[i];
		if(sec->phdr.p_type==PT_DYNAMIC)
		{
			int n;
			for(n=0; n<sec->SectionDataLen, ((Elf32_Dyn *)&sec->Data[n])->d_tag != DT_NULL; n += sizeof(Elf32_Dyn))
			{
				Elf32_Dyn *dyn = (Elf32_Dyn *)&sec->Data[n];
				if (dyn->d_tag != DT_RELSZ)continue;
				rel_sz = dyn->d_un.d_val;
			}
		}
	}
	

	for(i=0;i<PE->num_section;i++)
	{
		int			n, s_id;
		
		sec = PE->Sections[i];

		if (sec->phdr.p_type != PT_DYNAMIC)continue;
		
		for(s_id = 0, n = 0; n < sec->SectionDataLen, ((Elf32_Dyn *)&sec->Data[n])->d_tag != DT_NULL; s_id++, n += sizeof(Elf32_Dyn))
		{
			Elf32_Dyn *dyn = (Elf32_Dyn *)&sec->Data[n];
			Elf32_Rel *rel;
			unsigned char *rel_table;
			unsigned int sec_id, mem_ofs;
			int	nn;

			if (dyn->d_tag != DT_REL) continue;

			sec_id = FindSectionMem(PE,dyn->d_un.d_ptr);
			if (sec_id < 0)
			{
				printf("error: section not found for sym : %d - %d \n", dyn->d_tag, dyn->d_un.d_ptr);
				exit(0);
			}
				
			mem_ofs	= dyn->d_un.d_ptr - PE->Sections[sec_id]->phdr.p_vaddr;
			rel_table =	&PE->Sections[sec_id]->Data[mem_ofs];
			
			for(nn = 0; nn < rel_sz; nn += sizeof(Elf32_Rel))
			{
				char sym_name[256];
				Elf32_Sym *Sym_Tbl;
				Section	*rel_sec;
				unsigned int bind, sym_id, type, rel_type;
				unsigned int sec_ofs;
				unsigned int value;
				int rel_sec_id;

				rel = (Elf32_Rel *)&rel_table[nn];
				sym_id = ELF32_R_SYM(rel->r_info);
				rel_type = ELF32_R_TYPE(rel->r_info);

				printf("reloc : %x - %d,%d ",rel->r_offset,sym_id,rel_type);
				
				Sym_Tbl	= FindSym(PE,sym_id,sym_name);

				if(Sym_Tbl)
					printf(" (** %s) ",sym_name);
												
				printf("\n");
				
				rel_sec_id = FindSectionMem(PE,rel->r_offset);

				if (rel_sec_id < 0)
				{
					printf("error: relocation section not found for sym : %d - %d \n", dyn->d_tag, dyn->d_un.d_ptr);
					exit(0);
				}

				rel_sec	= PE->Sections[rel_sec_id];
				sec_ofs	= rel->r_offset - rel_sec->v_addr;
				value	= *((unsigned int *)(&rel_sec->Data[sec_ofs]));

				printf("data @ relocation = [%x] \n", value);

				if(rel_type == 8)
				{
					printf("relative reloc = [%d] \n", value);

					if (rel_sec->num_remap >= MAX_REMAP)
					{
						printf("too much remaps \n");
						exit(0);
					}

					rel_sec->RemapList[rel_sec->num_remap].base_addr = 0;
					rel_sec->RemapList[rel_sec->num_remap].offset = sec_ofs;
					rel_sec->RemapList[rel_sec->num_remap].type = 1;
					rel_sec->num_remap++;
				}
				else if(Sym_Tbl)
				{
					bind = ELF32_ST_BIND(Sym_Tbl->st_info);
					type = ELF32_ST_TYPE(Sym_Tbl->st_info);

					printf("segment [%d]=>[%d]: (%d,%d) ",i,Sym_Tbl->st_shndx,bind,type);
					
					if((type==STT_FILE)&&(sym_name!=NULL))
						printf("source file %s",sym_name);
					else if((type==STT_FUNC)&&(sym_name!=NULL))
						printf("func %s ",sym_name);
					else if((type==STT_OBJECT)&&(sym_name!=NULL))
						printf("object %s ",sym_name);
					else if(sym_name!=NULL)
						printf("no type %s  ",sym_name);

					printf("size : %d, value %x \n",Sym_Tbl->st_size,Sym_Tbl->st_value);

					if (bind != 1)continue;
					
					if(Sym_Tbl->st_value>0)
					{
						if (rel_sec->num_remap >= MAX_REMAP)
						{
							printf("too much remaps \n");
							exit(0);
						}

						rel_sec->RemapList[rel_sec->num_remap].offset = sec_ofs;

						if (rel_type == 2)
						{
							printf("relative relocation sym: '%s' \n", sym_name);
							rel_sec->RemapList[rel_sec->num_remap].type = 2;
							rel_sec->RemapList[rel_sec->num_remap].base_addr = Sym_Tbl->st_value;
							*((unsigned int *)(&rel_sec->Data[sec_ofs])) = 0xFFFFFFFC;
						}
						else
						{
							rel_sec->RemapList[rel_sec->num_remap].type = 0;
							rel_sec->RemapList[rel_sec->num_remap].base_addr = 0;
							*((unsigned int *)(&rel_sec->Data[sec_ofs])) = Sym_Tbl->st_value;
						}
						rel_sec->num_remap++;
					}
					else
					{
						int nnn;
						if (rel_sec->num_sec_imp_name >= MAX_IMPORT)
						{
							printf("too much imports \n");
							exit(0);
						}
							
						rel_sec->ImportsName[rel_sec->num_sec_imp_name].dll_name[0] = 0;

						for(nnn = 0;nnn < PE->num_libs; nnn++)
						{
							strcat(rel_sec->ImportsName[rel_sec->num_sec_imp_name].dll_name, PE->lib_names[nnn]);
							strcat(rel_sec->ImportsName[rel_sec->num_sec_imp_name].dll_name, ";");
						}

						strcpy	(rel_sec->ImportsName[rel_sec->num_sec_imp_name].func_name, sym_name);

						rel_sec->ImportsName[rel_sec->num_sec_imp_name].addr_reloc = sec_ofs;
						rel_sec->ImportsName[rel_sec->num_sec_imp_name].sym_idx	= s_id;

						rel_sec->num_sec_imp_name++;
					}
				}
				printf("\n");
			}
		}
	}
}

void ReadElfImpExp(PEFile *PE)
{
	int			i;
	Section		*sec;

	for(i = 0; i<PE->num_section; i++)
	{
		int	n;
		sec=PE->Sections[i];
		if (sec->phdr.p_type != PT_DYNAMIC)continue;
		
		for(n = 0; n<sec->SectionDataLen, ((Elf32_Dyn *)&sec->Data[n])->d_tag != DT_NULL; n += sizeof(Elf32_Dyn))
		{
			Elf32_Dyn *dyn = (Elf32_Dyn *)&sec->Data[n];
			char *string;
			int	nn;

			if (dyn->d_tag != DT_NEEDED)continue;

			string = FindElfString(PE,dyn->d_un.d_val);
			nn = strlen(string);
			while(nn--)
			{
				if(string[nn]=='.')
				{
					string[nn]=0;
					break;
				}
			}
			PE->lib_names[PE->num_libs]	= calloc(strlen(string)+1,1);
			strcpy(PE->lib_names[PE->num_libs],string);
			PE->num_libs++;
		}
	}

	
	for (i = 0; i < PE->num_section;	i++)
	{
		int n;

		sec = PE->Sections[i];
		if (sec->phdr.p_type != PT_DYNAMIC)continue;
		
		for(n = 0; n < sec->SectionDataLen, ((Elf32_Dyn *)&sec->Data[n])->d_tag != DT_NULL; n += sizeof(Elf32_Dyn))
		{
			Elf32_Dyn		*dyn = (Elf32_Dyn *)&sec->Data[n];
			int				sec_id;
			unsigned int	mem_ofs, sym_table_size;
			char			*sym_table;
			int				nn;
			int				s_id;

			if (dyn->d_tag != DT_SYMTAB)continue;

			sec_id = FindSectionMem(PE,dyn->d_un.d_ptr);
			if (sec_id < 0)
			{
				printf("section not found in 'ReadElfImpExp' \n");
				exit(0);
			}

			mem_ofs	= dyn->d_un.d_ptr-PE->Sections[sec_id]->phdr.p_vaddr;
			sym_table = &PE->Sections[sec_id]->Data[mem_ofs];

			for(nn = 0; nn<PE->num_esection;nn++)
			{
				if (PE->eSections[nn]->shdr.sh_type != SHT_DYNSYM)continue;//SHT_SYMTAB
				sym_table_size = PE->eSections[nn]->shdr.sh_size;
			}

			for (s_id = 0, nn = 0; nn < sym_table_size; s_id++, nn += sizeof(Elf32_Sym))
			{
				char name[128];
				Section *exp_sec;
				Elf32_Sym *SymTbl;
				char *sym_name;
				unsigned int sec_ofs;
				int	nnn;
				int	exp_sec_id;
				int	bind, type;

				SymTbl = (Elf32_Sym *)&sym_table[nn];
				bind = ELF32_ST_BIND(SymTbl->st_info);
				type = ELF32_ST_TYPE(SymTbl->st_info);
				sym_name = FindElfString(PE,SymTbl->st_name);

				printf("read symbole export : bind[%d] type[%d] name[%d] scn_idx[%d] size[%d] value [%d] ",bind,type,SymTbl->st_name,SymTbl->st_shndx,SymTbl->st_size,SymTbl->st_value);

				if(sym_name)
					printf(" name '%s' ",sym_name);

				printf("\n");

				fflush(stdout);

				if ((bind != 1) || (sym_name == NULL))continue;
				if (SymTbl->st_value <= 0)continue;

				if((strncmp(sym_name,"FT_",3))&&
				   (strncmp(sym_name,"ft_",3))&&
				   (strncmp(sym_name,"tt_",3))&&
				   (strncmp(sym_name,"TT_",3))&&
				   (strncmp(sym_name,"af_",3))&&
				   (strncmp(sym_name,"t1_",3))&&
				   (strncmp(sym_name,"ps_",3))&&
				   (strncmp(sym_name,"t42_",4))&&
				   (strncmp(sym_name,"pcf_",4))&&
				   (strncmp(sym_name,"pfr_",4))&&
				   (strncmp(sym_name,"cff_",4))&&
				   (strncmp(sym_name,"afm_",4))&&
				   (strncmp(sym_name,"afm_",4))&&
				   (strncmp(sym_name,"psaux_",6))&&
				   (strncmp(sym_name,"t1cid_",6))&&
				   (strncmp(sym_name,"psnames_",8))&&
				   (strncmp(sym_name,"pshinter_",9))&&
				   (strncmp(sym_name,"autofit_",8))&&
				   (strncmp(sym_name,"winfnt_",7))&&
				   (strncmp(sym_name,"sfnt_",5))&&
				   (strncmp(sym_name,"png_",4))&&
				   (strncmp(sym_name,"bdf_",4))

				   
				   )
				{	
				exp_sec_id		=	FindSectionMem(PE,SymTbl->st_value);
				if (exp_sec_id < 0)
				{
					printf("export symbol section not found in 'ReadElfImpExp' \n");
					continue;
				}

				strcpy (name, basename(PE->file_name));
				nnn = strlen(name);
				while(nnn--)
				{
					 if(name[nnn]=='.')
					 {
						 name[nnn] = 0;
						 break;
					 }
				}

				exp_sec = PE->Sections[exp_sec_id];
				sec_ofs = SymTbl->st_value - exp_sec->v_addr;

				if (exp_sec->num_sec_exp_name >= MAX_EXPORT)
				{
					printf("too much exports ");
					exit(0);
				}

				if (sym_name != NULL)
				{
					 strcpy(exp_sec->ExportsName[exp_sec->num_sec_exp_name].dll_name, name);
					 strcpy(exp_sec->ExportsName[exp_sec->num_sec_exp_name].func_name, sym_name);
				}
				exp_sec->ExportsName[exp_sec->num_sec_exp_name].addr_reloc = sec_ofs;
				exp_sec->ExportsName[exp_sec->num_sec_exp_name].sym_idx = s_id;

				exp_sec->num_sec_exp_name++;
			   }
			}
		}
	}
}


void ReadElfSegment(PEFile *PE,unsigned int sec_idx)
{
	int				secid;
	FILE			*in;
	Section			*Sec;

	in=fopen(PE->file_name,"rb");
	if(!in)
	{
		printf("could not open file '%s' \n" ,PE->file_name);
		return;

	}

	if(sec_idx >= PE->num_section)
		return;

	secid = sec_idx;
	Sec = PE->Sections[secid];
			
	Sec->Data = calloc(Sec->phdr.p_memsz,1);

	fseek		(in,Sec->phdr.p_offset,SEEK_SET);
	fread		(Sec->Data,Sec->phdr.p_filesz,1,in);
}


void ReadElfSegments(PEFile *PE)
{	
	int i;
	for(i=0;i<PE->num_section;i++)
	{
		ReadElfSegment(PE,i);
	}
}
