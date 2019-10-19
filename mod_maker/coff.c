//copyright antoine bentue-ferrer 2016
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "coff.h"

unsigned char *MapPEFile	(PEFile *PE)
{
	unsigned char	*mem_map;
	unsigned char	*mem_map_ptr;
	int				i;
	
	mem_map		=	calloc((PE->num_section+1),4096);
	
	//first page used for headers (?)
	mem_map_ptr	=	&mem_map[4096];
	
	for(i=0;i<PE->num_section;i++)
	{
		memcpy(mem_map_ptr,PE->Sections[i]->Data,PE->Sections[i]->SectionDataLen);

		mem_map_ptr+=4096;
	}

	return mem_map;


}

void init_section(Section *sec)
{
	int i;
	for(i=0;i<50;i++){sec->ExtrnSyms[i].index=-1;}

	sec->numreloc=0;
	sec->numsyms=0;
	sec->numexterns=0;
}
void addEmptySection(PEFile *PE,unsigned int size)
{
int n;
unsigned int last_addr;
unsigned int last_file_ptr;
Section		*new_section;

last_addr=0;
last_file_ptr=0;
n=0;
while(n<PE->num_section)
{
	unsigned int end_section;
	unsigned int end_file_ptr;

	end_section		=	PE->Sections[n]->SectionHeader.s_vaddr+PE->Sections[n]->SectionHeader.s_size;
	end_file_ptr	=	PE->Sections[n]->sections_file_data_ptr+PE->Sections[n]->SectionHeader.s_size;
	if(end_section>last_addr)
	{
		last_addr=end_section;
	}
	if(end_file_ptr>last_file_ptr)
	{
		last_file_ptr=end_file_ptr;
	}

	

	n++;
}
new_section								=	(Section *) calloc(sizeof(Section),1);

new_section->SectionHeader.s_vaddr		=	last_addr;
new_section->SectionHeader.s_size		=	size;
strcpy	(new_section->Name,".newsec");
strcpy	(new_section->SectionHeader.s_name,new_section->Name);
new_section->SectionHeader.s_nreloc		=	0;
new_section->SectionHeader.s_scnptr		=	NULL;

new_section->num_sec_imp_ord			=	0;
new_section->num_sec_imp_name			=	0;
new_section->num_remap					=	0;
new_section->index						=	PE->num_section;
new_section->sections_file_data_ptr		=	last_file_ptr;
new_section->numexterns					=	0;
new_section->numsyms					=	0;
new_section->numreloc					=	0;
new_section->Data						=	calloc(size,1);
new_section->SectionDataLen				=	size;
PE->Sections[PE->num_section]			=	new_section;
PE->num_section++;
}


unsigned int FindSectionMem(PEFile *PE,unsigned int addr)
{
	unsigned int i;
	for(i=0;i<PE->num_section;i++)
	{
		//if((addr>=PE->Sections[i]->SectionHeader.s_vaddr)&&(addr<(PE->Sections[i]->SectionHeader.s_vaddr+PE->Sections[i]->SectionHeader.s_size)))
		if((addr>=PE->Sections[i]->v_addr)&&(addr<(PE->Sections[i]->v_addr+PE->Sections[i]->SectionDataLen)))
		{
			return i;
		}
	}

	return 0xFFFFFFFF;
}

unsigned char *FindSectionMemPtr(PEFile *PE,unsigned int addr)
{
	unsigned int i;
	for(i=0;i<PE->num_section;i++)
	{
		Section *sec=PE->Sections[i];
		if((addr>=sec->SectionHeader.s_vaddr)&&(addr<(sec->SectionHeader.s_vaddr+sec->SectionHeader.s_size)))
		{	
			return &PE->Sections[i]->Data[addr-sec->SectionHeader.s_vaddr];
		}
	}

	return NULL;
}

void ReadExeFile(char *file,PEFile *PE)
{
	int				i,j,k;
	FILE			*in;


	memset(&PE->fhdr,0,sizeof(FILHDR));



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

	fread	(&PE->doshdr,sizeof(IMAGE_DOS_HEADER),1,in);


	fseek	(in,PE->doshdr.e_lfanew,SEEK_SET);
	fread	(PE->sig,4,1,in);
	fread	(&PE->fhdr,sizeof(FILHDR),1,in);

	printf("PE HEADER Symbols : %d , %x   flag : %x \n",PE->fhdr.f_nsyms,PE->fhdr.f_symptr,PE->fhdr.f_flags);

	//printf("sections : %d , symobls  : %d , opt hdr  : %d\n" ,PE->fhdr.f_nscns,PE->fhdr.f_nsyms,PE->fhdr.f_opthdr);

	if(PE->fhdr.f_opthdr>0)
	{
		fread	(&PE->OptionalHeader,sizeof(IMAGE_OPTIONAL_HEADER),1,in);
		PE->ImageBase=PE->OptionalHeader.ImageBase;

	}
	PE->num_section		=	PE->fhdr.f_nscns;

	for(i=0;i<PE->num_section;i++)
	{
		unsigned int	sec_memory_size;
		PE->Sections[i]	=	(Section *) calloc(sizeof(Section),1);

		fread	(&PE->Sections[i]->SectionHeader,sizeof(SCNHDR),1,in);
		strcpy	(PE->Sections[i]->Name,PE->Sections[i]->SectionHeader.s_name);

		PE->Sections[i]->v_addr			=	PE->Sections[i]->SectionHeader.s_vaddr;
		PE->Sections[i]->SectionDataLen	=	PE->Sections[i]->SectionHeader.s_size;
	}
	fclose(in);
}

unsigned int findAddrOfsetOff(PEFile *PE,unsigned int addr)
{
int n;
unsigned int last_addr;
unsigned int last_file_ptr;

last_addr=0;
last_file_ptr=0;
n=0;
while(n<PE->num_section)
{
	unsigned int end_section;
	unsigned int end_file_ptr;

	end_section		=	PE->Sections[n]->SectionHeader.s_vaddr+PE->Sections[n]->SectionHeader.s_size;
	end_file_ptr	=	PE->Sections[n]->sections_file_data_ptr+PE->Sections[n]->SectionHeader.s_size;
	if(end_section>last_addr)
	{
		last_addr=end_section;
	}
	if(end_file_ptr>last_file_ptr)
	{
		last_file_ptr=end_file_ptr;
	}
	n++;
}

if(addr<last_addr)
	return	0;
else
	return	(addr-last_addr);

}

void ReadPEImpExp(PEFile *PE)
{
	FILE *in;
	unsigned int v_addr;
	unsigned int ofset_first;
	unsigned int i;
	in=fopen(PE->file_name,"rb");

	printf("import table : %x - %d \n",PE->OptionalHeader.DataDirectory[1].VirtualAddress,PE->OptionalHeader.DataDirectory[1].Size);

	for(i=0;i<PE->num_section;i++)
	{
		PE->Sections[i]->num_sec_imp_ord=0;
		PE->Sections[i]->num_sec_imp_name=0;
	}

	ofset_first=0xFFFFFFFF;

	
	

	if(PE->OptionalHeader.DataDirectory[1].Size>0)
	{
		char					name[128];
		IMAGE_THUNK_DATA		*thunk_ptr;
		IMAGE_THUNK_DATA		*thunk_vptr;
		unsigned int			thunk_bit;
		IMAGE_IMPORT_BY_NAME	*img_imp;
		IMAGE_IMPORT_DIRECTORY	*CurrentImport;
		unsigned int			ordinal_func;
		unsigned int			sec_idx;
		unsigned int			thunk_ofset;
		Section					*sec;

		
		PE->ImageImport	=	FindSectionMemPtr	(PE,PE->OptionalHeader.DataDirectory[1].VirtualAddress);
	


		CurrentImport	=	PE->ImageImport;

		while(CurrentImport->FirstThunk!=NULL)
		{
			int len_name;
			strcpy		(name,FindSectionMemPtr	(PE,CurrentImport->Name));

			len_name	=strlen(name);
			while(len_name--){if (name[len_name]=='.'){name[len_name]=0;break;}}


			sec_idx	=	FindSectionMem(PE,CurrentImport->FirstThunk);
			if(sec_idx!=0xFFFFFFFF)
			{
				unsigned int	thunk_start;
				sec				=	PE->Sections[sec_idx];

				//printf("mod name : %s - section name : %s (%x) - FirstThunk : %x \n",name,sec->Name,sec->SectionHeader.s_vaddr,CurrentImport->FirstThunk);

				thunk_vptr								=	(IMAGE_THUNK_DATA *)CurrentImport->FirstThunk;
				thunk_ptr								=	(IMAGE_THUNK_DATA *)FindSectionMemPtr(PE,thunk_vptr);
				/*
				thunk_start								=	((unsigned int)(thunk_ptr))-((unsigned int)(PE->Sections[sec_idx]->Data));
				PE->Sections[sec_idx]->SectionDataLen	=	thunk_start;
				*/

				while(thunk_ptr->u1.Function!=NULL)
				{
					thunk_bit		=	(thunk_ptr->u1.Ordinal) & 0x80000000;
					if(!thunk_bit)
					{

						img_imp			=	FindSectionMemPtr(PE,thunk_ptr->u1.AddressOfData);
						printf("function name : %s \n",img_imp->Name);	
					
						strcpy(sec->ImportsName[sec->num_sec_imp_name].dll_name		,name);
						strcpy(sec->ImportsName[sec->num_sec_imp_name].func_name	,img_imp->Name);
						sec->ImportsName[sec->num_sec_imp_name].addr_reloc		=	((unsigned int)(&thunk_vptr->u1.Function))-sec->SectionHeader.s_vaddr;
						printf("reloc addr : %8.8x@%8.8x => %x \n",sec->ImportsName[sec->num_sec_imp_name].addr_reloc,sec->SectionHeader.s_vaddr,thunk_ptr->u1.Function);
						sec->num_sec_imp_name++;
					}
					else
					{
						ordinal_func	=	(thunk_ptr->u1.Ordinal) & 0x7FFFFFFF;
						printf("function ordinal : %d \n",ordinal_func);

						strcpy(sec->ImportsOrd[sec->num_sec_imp_ord].dll_name	,name);
						sec->ImportsOrd[sec->num_sec_imp_ord].ordinal_imp	=	ordinal_func;
						sec->ImportsOrd[sec->num_sec_imp_ord].addr_reloc	=	(unsigned int)(thunk_vptr)-sec->SectionHeader.s_vaddr;

						printf("reloc addr : %d \n",sec->ImportsOrd[sec->num_sec_imp_ord].addr_reloc);
						sec->num_sec_imp_ord++;
					}
					thunk_ptr++;
					thunk_vptr++;
				}
			}
			CurrentImport++;
		}
	}

	printf("export table : %x - %d \n",PE->OptionalHeader.DataDirectory[0].VirtualAddress,PE->OptionalHeader.DataDirectory[0].Size);
	if(PE->OptionalHeader.DataDirectory[0].Size>0)
	{
		char					name[128];
		unsigned char			*v_addr_name;
		unsigned char			*v_addr_fn;
		unsigned char			*v_addr_ords;
		unsigned int			*addr_name_v_ptr;
		unsigned short			*addr_ord;
		unsigned int			*addr_fn;
		unsigned int			len_name;
		PE->ImageExport	=	FindSectionMemPtr	(PE,PE->OptionalHeader.DataDirectory[0].VirtualAddress);

		strcpy(name,FindSectionMemPtr	(PE,PE->ImageExport->Name));

		len_name	=strlen(name);
		while(len_name--){if (name[len_name]=='.'){name[len_name]=0;break;}}
		


		printf("export table name : %s - functions : %d - names : %d \n",name,PE->ImageExport->NumberOfFunctions,PE->ImageExport->NumberOfNames);
		
		v_addr_name		=PE->ImageExport->AddressOfNames;
		v_addr_fn		=PE->ImageExport->AddressOfFunctions;
		v_addr_ords		=PE->ImageExport->AddressOfNameOrdinals;

		addr_name_v_ptr	=FindSectionMemPtr(PE,v_addr_name);
		addr_fn			=FindSectionMemPtr(PE,v_addr_fn);
		addr_ord		=FindSectionMemPtr(PE,v_addr_ords);

		printf("name : 0x%8.8x - func : 0x%8.8x \n"	,v_addr_name,v_addr_fn);

		for(i=0;i<PE->ImageExport->NumberOfFunctions;i++)
		{	
			char			fn_name[128];
			unsigned int	ordinal;
			unsigned int	addr;
			unsigned int	sec_idx;
			Section			*sec;



			if(i<PE->ImageExport->NumberOfNames)
				strcpy(fn_name	,FindSectionMemPtr(PE,addr_name_v_ptr[i]));
			else
				strcpy(fn_name	,"no name");

			ordinal	=	addr_ord[i];
			addr	=	addr_fn[ordinal];


			sec_idx	=	FindSectionMem(PE,addr);
			if(sec_idx!=0xFFFFFFFF)
			{
				sec		=	PE->Sections[sec_idx];
	
				
				if(i<PE->ImageExport->NumberOfNames)
				{
					printf("name : %s@%s - func addr: 0x%8.8x 0x%8.8x - ordinal : %d \n"	,fn_name,name,addr,sec->SectionHeader.s_vaddr,ordinal);
					strcpy(sec->ExportsName[sec->num_sec_exp_name].dll_name		,name);
					strcpy(sec->ExportsName[sec->num_sec_exp_name].func_name	,fn_name);
					sec->ExportsName[sec->num_sec_exp_name].addr_reloc		=	addr-sec->SectionHeader.s_vaddr;
					sec->num_sec_exp_name++;
				}
				else
				{
					printf("ordinal : %d@%s - func addr: 0x%8.8x \n"	,ordinal,name,addr);
					strcpy(sec->ExportsOrd[sec->num_sec_exp_ord].dll_name		,name);
					
					sec->ExportsOrd[sec->num_sec_exp_ord].ordinal_exp		=	ordinal;
					sec->ExportsOrd[sec->num_sec_exp_ord].addr_reloc		=	addr-sec->SectionHeader.s_vaddr;
					sec->num_sec_exp_ord++;
				}
				
			}

		}
		
	}
	

	fclose(in);
}

void BuildPE_RVA(PEFile *PE)
{
	int				n,i;
	int				size_relocs;
	unsigned short *blk_ptr;
	unsigned int	PE_BaseAddr;
	unsigned int	last_far_off;
	Section			*sec;

	
	sec				=	GetPESectionPtr(PE,".reloc");
	if(sec==NULL)return;
	
	size_relocs		=	PE->OptionalHeader.DataDirectory[5].Size;
	PE_BaseAddr		=   PE->OptionalHeader.DataDirectory[5].VirtualAddress;
	sec->num_remap	=	0;
	n				=	0;
	last_far_off	=	0;

	while(n<size_relocs)
	{
		
		unsigned int	page_rva;
		unsigned int	block_size;
		unsigned int	n_entries;
		unsigned int	section_idx;
			
		sec			=	GetPESectionPtr(PE,".reloc");

		i			=	0;
		page_rva	=	*((unsigned int *)(&sec->Data[n]));
		section_idx	=	FindSectionMem(PE,page_rva);

		block_size	=	*((unsigned int *)(&sec->Data[n+4]));
		blk_ptr		=	&sec->Data[n+8];
		n_entries	=	(block_size-8)/2;
		
		while(i<n_entries)
		{
			unsigned short s_flags;
			unsigned int type;
			unsigned int offset;
			unsigned int rva_addr;
			
			unsigned int value;
			Section		*sec_dat;

			s_flags	=	blk_ptr[i];	

			type	=	(s_flags & 0xF000)>>12;
			offset	=	(s_flags & 0x0FFF);

			if(section_idx<PE->num_section)
			{
				sec_dat		=	PE->Sections[section_idx];
				if(type==3)
				{
					unsigned int target_sec_idx;
					unsigned int section_ofset;

					rva_addr		=		page_rva-sec_dat->SectionHeader.s_vaddr;
					section_ofset	=		rva_addr+offset;
					value			=		*((unsigned int *)(&sec_dat->Data[section_ofset]));
					value			=		value-PE->OptionalHeader.ImageBase;
					
					target_sec_idx	=		FindSectionMem(PE,value);

					printf("addr : %x \n",value);
					if(target_sec_idx==0xFFFFFFFF)
					{
						unsigned int far_off;
						far_off=findAddrOfsetOff(PE,value);
						if((far_off>0)&&(far_off<100000))
						{
							if(far_off>last_far_off)
							{
								last_far_off=far_off;
							}
						}
					}
					
					sec_dat->RemapList[sec_dat->num_remap].base_addr	=	page_rva;
					sec_dat->RemapList[sec_dat->num_remap].offset		=	offset;
					sec_dat->RemapList[sec_dat->num_remap].type			=	0;
					
					sec_dat->num_remap++;
				}
			}

			i++;
		}
			
		n+=block_size;
	}


	if(last_far_off>0)
		addEmptySection	(PE,last_far_off+8);
	
}



Section *GetPESectionPtr(PEFile *PE,char *name)
{
	int i;
	for(i=0;i<PE->num_section;i++)
	{
		if(!strcmp(PE->Sections[i]->Name,name))
		{
			return PE->Sections[i];
		}
	}
	return NULL;
}


void ReadPeSections(PEFile *PE)
{	
	
	int i;

	for(i=0;i<PE->num_section;i++)
	{
		ReadPeSection(PE,PE->Sections[i]->Name);
	}
}

void ReadPeSection(PEFile *PE,char *name)
{
	int				i,j,k;
	int				secid;
	FILE			*in,*t;
	SYMENT			sym;
	Section			*Sec;

	in=fopen(PE->file_name,"rb");
	if(!in)
	{
		printf("could not open file '%s' \n" ,PE->file_name);
		return;

	}

	Sec		=	NULL;
	secid	=	-1;

	for(i=0;i<PE->num_section;i++)
	{
		if(!strcmp(PE->Sections[i]->Name,name))
		{
			Sec		=	PE->Sections[i];
			secid	=	i+1;
		}
	}
	if(Sec==NULL)return;

	Sec->numreloc		=	Sec->SectionHeader.s_nreloc;
	//Sec->SectionDataLen	=	Sec->SectionHeader.s_size; //changed
	Sec->Data			=	calloc(Sec->SectionDataLen,1);
	

	fseek	(in,Sec->SectionHeader.s_scnptr,SEEK_SET);
	fread	(Sec->Data,Sec->SectionDataLen,1,in);


	fseek	(in,Sec->SectionHeader.s_relptr,SEEK_SET);
	fread	(Sec->RelocList,sizeof(RELOC),Sec->numreloc,in);

	Sec->numexterns	=0;
	Sec->numsyms	=0;


	fseek	(in,PE->fhdr.f_symptr,SEEK_SET);
	for(i=0;i<PE->fhdr.f_nsyms;i++)
	{
		fread(&sym,sizeof(SYMENT),1,in);

		if(!strcmp(sym.e.e_name,".text"))continue;
	//	if(!strcmp(sym.e.e_name,".libc"))continue;
		if(!strcmp(sym.e.e_name,".bss"))continue;
		if(!strcmp(sym.e.e_name,".file"))continue;

		
		if((sym.e_scnum==secid)&&(strcmp(sym.e.e_name,".libc")))
		{
			memcpy(&Sec->SymList[Sec->numsyms],&sym,sizeof(SYMENT));
			Sec->numsyms++;
		}
		
		if(!strcmp(sym.e.e_name,"_Scene"))
		{
			for(j=0;j<Sec->numreloc;j++)
			{
				if(Sec->RelocList[j].r_symndx==i)
				{
					Sec->RelocList[j].r_symndx=6666666;
				}
			}
			continue;
		}

		for(j=0;j<Sec->numreloc;j++)
		{
			if(Sec->RelocList[j].r_symndx==i)
			{
				for(k=0;k<Sec->numexterns;k++)
				{	
					if(Sec->ExtrnSyms[k].index==i)
						goto alreadyseen;
				}
				
				memcpy(&Sec->ExtrnSyms[Sec->numexterns].Sym,&sym,sizeof(SYMENT));
				Sec->ExtrnSyms[Sec->numexterns].index=i;
				Sec->numexterns++;
				if(!strcmp(sym.e.e_name,".libc"))
				{
					fread(&Sec->ExtrnSyms[Sec->numexterns].Sym,sizeof(SYMENT),1,in);
					Sec->numexterns++;
					i++;
				}


			}
			alreadyseen:;
		}

	}
}


void ReadSection(char *file,char *name,Section *Sec)
{
int				i,j,k;
int				secid;
FILE			*in,*t;
FILHDR			fhdr;
SCNHDR			shdr;
SYMENT			sym;
IMAGE_DOS_HEADER doshdr;

char sig[4];

memset(&Sec->SectionHeader,0,sizeof(SCNHDR));
secid=-1;


in=fopen(file,"rb");
if(!in)
{
	printf("could not open file '%s' \n" ,file);
	return;

}
fread	(&doshdr,sizeof(IMAGE_DOS_HEADER),1,in);


fseek	(in,doshdr.e_lfanew,SEEK_SET);
fread	(sig,4,1,in);
fread	(&fhdr,sizeof(FILHDR),1,in);

printf("sections : %d , symobls  : %d , opt hdr  : %d\n" ,fhdr.f_nscns,fhdr.f_nsyms,fhdr.f_opthdr);


for(i=0;i<fhdr.f_nscns;i++)
{
	fread(&shdr,sizeof(SCNHDR),1,in);

	if(!strcmp(shdr.s_name,name))
	{
		memcpy(&Sec->SectionHeader,&shdr,sizeof(SCNHDR));
		secid=i+1;
	}
}
strcpy(Sec->Name,Sec->SectionHeader.s_name);

Sec->numreloc		=	Sec->SectionHeader.s_nreloc;
Sec->SectionDataLen	=	Sec->SectionHeader.s_size;
Sec->Data			=	calloc(Sec->SectionDataLen,1);

fseek	(in,Sec->SectionHeader.s_scnptr,SEEK_SET);
fread	(Sec->Data,Sec->SectionDataLen,1,in);


fseek	(in,Sec->SectionHeader.s_relptr,SEEK_SET);
fread	(Sec->RelocList,sizeof(RELOC),Sec->numreloc,in);

Sec->numexterns	=0;
Sec->numsyms	=0;


fseek	(in,fhdr.f_symptr,SEEK_SET);
for(i=0;i<fhdr.f_nsyms;i++)
{
	fread(&sym,sizeof(SYMENT),1,in);

	if(!strcmp(sym.e.e_name,".text"))continue;
//	if(!strcmp(sym.e.e_name,".libc"))continue;
	if(!strcmp(sym.e.e_name,".bss"))continue;
	if(!strcmp(sym.e.e_name,".file"))continue;

	
	if((sym.e_scnum==secid)&&(strcmp(sym.e.e_name,".libc")))
	{
		memcpy(&Sec->SymList[Sec->numsyms],&sym,sizeof(SYMENT));
		Sec->numsyms++;
	}
	
	if(!strcmp(sym.e.e_name,"_Scene"))
	{
		for(j=0;j<Sec->numreloc;j++)
		{
			if(Sec->RelocList[j].r_symndx==i)
			{
				Sec->RelocList[j].r_symndx=6666666;
			}
		}
		continue;
	}

	for(j=0;j<Sec->numreloc;j++)
	{
		if(Sec->RelocList[j].r_symndx==i)
		{
			for(k=0;k<Sec->numexterns;k++)
			{	
				if(Sec->ExtrnSyms[k].index==i)
					goto alreadyseen;
			}
			
			memcpy(&Sec->ExtrnSyms[Sec->numexterns].Sym,&sym,sizeof(SYMENT));
			Sec->ExtrnSyms[Sec->numexterns].index=i;
			Sec->numexterns++;
			if(!strcmp(sym.e.e_name,".libc"))
			{
				fread(&Sec->ExtrnSyms[Sec->numexterns].Sym,sizeof(SYMENT),1,in);
				Sec->numexterns++;
				i++;
			}


		}
		alreadyseen:;
	}

}


}

void InitSection(Section *Sec,char *name,int index,int hasdata)
{
memset(Sec->SymList,0,sizeof(SymEntry)*1000);
memset(Sec->RelocList,0,sizeof(RELOC)*1000);
Sec->numsyms		=0;
Sec->numreloc		=0;
Sec->numexterns		=0;

if(hasdata)	
	Sec->Data			=calloc(1,1);
else
	Sec->Data			=NULL;

Sec->SectionDataLen	=0;

strcpy(Sec->Name,name);
Sec->index=index;
}

SymEntry *GetSym(Section *Sec,char *symname)
{
	int i,symindex;
	int	len=strlen(symname);
	
	for(i=0;i<Sec->numsyms;i++)
	{
		char sname[10];
		memset(sname,0,10);
		memcpy(sname,Sec->SymList[i].Sym.e.e_name,8);

		if(!strcmp(sname,symname))
		{
			return &Sec->SymList[i];
		}
	}

	return NULL;
}


void AddSym(Section *Sec,char *name,int type,unsigned char *Data,int len)
{
SYMENT *Sym=&Sec->SymList[Sec->numsyms].Sym;

memset(Sym->e.e_name,0,8);
memcpy(Sym->e.e_name,name,strlen(name));
Sym->e_value						=Sec->SectionDataLen;
Sym->e_scnum						=Sec->index;
Sym->e_type							=type;
Sym->e_sclass						=SYM_CLASS_STATIC;
Sym->e_numaux						=0;

if(Sec->Data!=NULL)
{
	Sec->Data							=realloc(Sec->Data,Sec->SectionDataLen+len);
	memcpy(&Sec->Data[Sec->SectionDataLen],Data,len);
}

Sec->SectionDataLen					=Sec->SectionDataLen+len;	
Sec->SymList[Sec->numsyms].index	=Sec->numsyms;
Sec->numsyms++;

}

void AddExtSym(Section *Sec,char *name,int type,unsigned char *Data,int len)
{
SYMENT *Sym=&Sec->SymList[Sec->numsyms].Sym;

memset(Sym->e.e_name,0,8);
memcpy(Sym->e.e_name,name,strlen(name));
Sym->e_value						=Sec->SectionDataLen;
Sym->e_scnum						=Sec->index;
Sym->e_type							=type;
Sym->e_sclass						=SYM_CLASS_EXTERNAL;
Sym->e_numaux						=0;

if(Sec->Data!=NULL)
{
	Sec->Data							=realloc(Sec->Data,Sec->SectionDataLen+len);
	memcpy(&Sec->Data[Sec->SectionDataLen],Data,len);
}

Sec->SectionDataLen					=Sec->SectionDataLen+len;	
Sec->SymList[Sec->numsyms].index	=Sec->numsyms;
Sec->numsyms++;
}

void AddExtSym2(Section *Sec,unsigned int StringOfset,int type,int len)
{
SYMENT *Sym=&Sec->SymList[Sec->numsyms].Sym;

Sym->e.e.e_zeroes					=0;
Sym->e.e.e_offset					=StringOfset;

Sym->e_value						=Sec->SectionDataLen;
Sym->e_scnum						=Sec->index;
Sym->e_type							=type;
Sym->e_sclass						=SYM_CLASS_EXTERNAL;
Sym->e_numaux						=0;

Sec->SectionDataLen					=Sec->SectionDataLen+len;
Sec->SymList[Sec->numsyms].index	=Sec->numsyms;
Sec->numsyms++;
}


void AddReloc(Section *Sec,int addr,char *symname)
{
	SymEntry *Sym=GetSym(Sec,symname);
	Sec->RelocList[Sec->numreloc].r_vaddr	=addr;
	Sec->RelocList[Sec->numreloc].r_symndx	=Sym->index;
	Sec->RelocList[Sec->numreloc].r_type	=6;

	if(Sec->Data!=NULL) memset(&Sec->Data[addr],0,4);
	Sec->numreloc++;
}

void AddRelocIndex(Section *Sec,int addr,int index)
{
	Sec->RelocList[Sec->numreloc].r_vaddr	=addr;
	Sec->RelocList[Sec->numreloc].r_symndx	=index;
	Sec->RelocList[Sec->numreloc].r_type	=6;

	if(Sec->Data!=NULL) memset(&Sec->Data[addr],0,4);
	Sec->numreloc++;
}



void CreateFileHeader(COFFObj *Coff,int alosfet)
{
	int	i;
	int	SecDataEnd;

	Coff->Header.f_magic			=0x014c;
	Coff->Header.f_nscns			=Coff->num_section;
	Coff->Header.f_timdat			=time(0);
	
	SecDataEnd=sizeof(FILHDR)+sizeof(SCNHDR)*Coff->num_section+alosfet;

	Coff->Header.f_nsyms=0;
	for(i=0;i<Coff->num_section;i++)
	{
		SecDataEnd				+=Coff->Sections[i]->TotalLen;
		Coff->Header.f_nsyms	+=Coff->Sections[i]->numsyms+Coff->Sections[i]->numexterns;
	}

	Coff->Header.f_symptr		=SecDataEnd;
	/*
	if(Coff->CodeSection==NULL)
	{
		Coff->Header.f_symptr			=sizeof(FILHDR)+sizeof(SCNHDR)*2+Coff->DataSection->TotalLen+Coff->UDataSection->TotalLen;
		Coff->Header.f_nsyms			=Coff->DataSection->numsyms+Coff->UDataSection->numsyms;
	}
	else
	{
		Coff->Header.f_symptr			=sizeof(FILHDR)+sizeof(SCNHDR)*3+Coff->DataSection->TotalLen+Coff->UDataSection->TotalLen+Coff->CodeSection->TotalLen;
		Coff->Header.f_nsyms			=Coff->DataSection->numsyms+Coff->UDataSection->numsyms+Coff->CodeSection->numsyms+Coff->CodeSection->numexterns;
	}
	*/
	Coff->Header.f_opthdr			=0;
	Coff->Header.f_flags			=0;
}

void CreateSectionHeader(Section *Sec,int startpos)
{
	int lenal;

	strcpy(Sec->SectionHeader.s_name,Sec->Name);

	Sec->SectionHeader.s_paddr	=0;
	Sec->SectionHeader.s_vaddr	=0;
	Sec->SectionHeader.s_size	=Sec->SectionDataLen;
	if(Sec->Data!=NULL)
	{
		Sec->SectionHeader.s_scnptr	=startpos;
		
		if(Sec->numreloc>0)
			Sec->SectionHeader.s_relptr	=startpos+Sec->SectionDataLen;
		else
			Sec->SectionHeader.s_relptr	=0;
	}
	else
	{
		Sec->SectionHeader.s_scnptr	=0;

		if(Sec->numreloc>0)
			Sec->SectionHeader.s_relptr	=startpos;
		else
			Sec->SectionHeader.s_relptr	=0;

	}
	Sec->SectionHeader.s_lnnoptr=0;
	Sec->SectionHeader.s_nreloc	=Sec->numreloc;
	Sec->SectionHeader.s_nlnno	=0;
	Sec->SectionHeader.s_flags	=0;


	if(Sec->Data!=NULL)
	{
		if((strcmp(Sec->Name,".text"))&&(strcmp(Sec->Name,".libc")))
		{
			Sec->SectionHeader.s_flags	=SCN_DATA|SCN_WRITE|SCN_READ|SCN_ALIGN4;
		}
		else
		{
			
			Sec->SectionHeader.s_flags	=SCN_CODE|SCN_EXE|SCN_READ|SCN_ALIGN16;
			
			if(!strcmp(Sec->Name,".libc"))
				Sec->SectionHeader.s_flags	|=SCN_WRITE;
		}
		Sec->TotalLen				=Sec->SectionDataLen+Sec->numreloc*sizeof(RELOC);

	}
	else
	{
		Sec->SectionHeader.s_flags	=SCN_UDATA|SCN_WRITE|SCN_READ|SCN_ALIGN4;
		Sec->TotalLen				=Sec->numreloc*sizeof(RELOC);
	}
}

void WriteSection(Section *Sec,FILE *s)
{
	int i;

	if(Sec->Data!=NULL)
		fwrite(Sec->Data				,Sec->SectionDataLen,1,s);

	for(i=0;i<Sec->numreloc;i++)
	{
		fwrite(&Sec->RelocList[i],sizeof(RELOC),1,s);
	}
}


void WriteCOFF(COFFObj *Coff,char *file)
{

int		i,j,sec_data_ofs,p1,p2,p3,p4,p5,p6,p7,total,alofset;
FILE	*out;


sec_data_ofs	=sizeof(FILHDR)+sizeof(SCNHDR)*Coff->num_section;

alofset			=sec_data_ofs&0x000000F;
alofset			=16-alofset;

sec_data_ofs	=sec_data_ofs+alofset;

for(i=0;i<Coff->num_section;i++)
{
	CreateSectionHeader	(Coff->Sections[i],sec_data_ofs);
	sec_data_ofs		+=Coff->Sections[i]->TotalLen;
}
CreateFileHeader	(Coff,alofset);

/*
if(Coff->CodeSection!=NULL)
{
	sec_data_ofs		=sizeof(FILHDR)+sizeof(SCNHDR)*3;

	CreateSectionHeader	(Coff->CodeSection ,sec_data_ofs);
	sec_data_ofs		+=Coff->CodeSection->TotalLen;
}
else
	sec_data_ofs		=sizeof(FILHDR)+sizeof(SCNHDR)*2;

CreateSectionHeader	(Coff->DataSection ,sec_data_ofs);
sec_data_ofs		+=Coff->DataSection->TotalLen;
CreateSectionHeader	(Coff->UDataSection,sec_data_ofs);
CreateFileHeader	(Coff);
*/

out=fopen(file,"wb");
fwrite(&Coff->Header,sizeof(FILHDR),1,out);

for(i=0;i<Coff->num_section;i++)
{
	fwrite(&Coff->Sections[i]->SectionHeader,sizeof(SCNHDR),1,out);
}
fwrite(zero,alofset,1,out);
for(i=0;i<Coff->num_section;i++)
{
	WriteSection(Coff->Sections[i],out);
}



if(!strcmp(Coff->Sections[0]->Name,".text"))
{
	for(i=1;i<Coff->num_section;i++)
	{
		for(j=0;j<Coff->Sections[i]->numsyms;j++)
		{
			fwrite(&Coff->Sections[i]->SymList[j].Sym,sizeof(SYMENT),1,out);
		}
	}
	for(i=0;i<Coff->Sections[0]->numsyms;i++)
	{
		fwrite(&Coff->Sections[0]->SymList[i].Sym,sizeof(SYMENT),1,out);
	}
}
else
{
	for(i=0;i<Coff->num_section;i++)
	{
		for(j=0;j<Coff->Sections[i]->numsyms;j++)
		{
			fwrite(&Coff->Sections[i]->SymList[j].Sym,sizeof(SYMENT),1,out);
		}
	}
}



for(i=0;i<Coff->num_section;i++)
{
	for(j=0;j<Coff->Sections[i]->numexterns;j++)
	{
		fwrite(&Coff->Sections[i]->ExtrnSyms[j].Sym,sizeof(SYMENT),1,out);
	}
}


p1		=strlen("__imp__ExitProcess@4")+1;
p2		=strlen("__imp__VirtualAlloc@16")+1;
p3		=strlen("__imp__VirtualFree@12")+1;
p4		=strlen("__fltused")+1;
p5		=strlen("__except_list")+1;
p6		=strlen("___CxxFrameHandler")+1;
p7		=strlen("__alloca_probe")+1;

total	=4+p1+p2+p3+p4+p5+p6+p7;
fwrite	(&total,4,1,out);
fwrite	("__imp__ExitProcess@4"		,p1,1,out);
fwrite	("__imp__VirtualAlloc@16"	,p2,1,out);
fwrite	("__imp__VirtualFree@12"	,p3,1,out);
fwrite	("__fltused"				,p4,1,out);
fwrite	("__except_list"			,p5,1,out);
fwrite	("___CxxFrameHandler"       ,p6,1,out);
fwrite	("__alloca_probe"           ,p7,1,out);



fwrite(zero			,32,1,out);
fclose(out);


}


void DeleteSection(Section *Sec)
{
	free(Sec->Data);
	Sec->Data=NULL;
}

void InitCoff(COFFObj *Coff)
{
	int i;
	memset(&Coff->Header,0,sizeof(FILHDR));
	Coff->num_section=0;

	Coff->strings_len=4;
	Coff->num_strings=0;
	for(i=0;i<10;i++)
	{
		Coff->Strings[i]=NULL;
		Coff->Sections[i]=NULL;
	}
}

void AddCoffString(COFFObj *Coff,char *String)
{
	Coff->Strings[Coff->num_strings]=String;
	Coff->num_strings++;
	Coff->strings_len+=strlen(String)+1;

}
void AddCoffSection(COFFObj *Coff,Section *Sec)
{
	Coff->Sections[Coff->num_section]=Sec;
	Coff->num_section++;
}

Section DataSection;
Section UDataSection;
Section CodeSection;
Section Libc1Section;

COFFObj	TheObj;
/*
void ObjExport(char *file,MainRendering *Render,int mode,char *SceneSym,char *Entry)
{
SymEntry		*Sym;
int				i,j;
MainRendering	*RenderPtr;
FILE			*ImageFile;
FILE			*SoundFile;
unsigned char	*Data=NULL;
unsigned int	DataLen,curtotalsyms;
unsigned int	NewRelocs[50];

chdir					(Render->Maindir);

for(i=0;i<50;i++){CodeSection.ExtrnSyms[i].index=-1;NewRelocs[i]=-1;}

Libc1Section.numsyms=0;
Libc1Section.numexterns=0;
Libc1Section.numreloc=0;

CodeSection.numreloc=0;
CodeSection.numsyms=0;
CodeSection.numexterns=0;

if(mode&2)
{
	ReadSection		("player/main.obj",".text",&CodeSection);
	InitSection		(&DataSection,".data",2,1);
	InitSection		(&UDataSection,".bss",3,0);
}
else
{
	InitSection		(&DataSection ,".data",1,1);
	InitSection		(&UDataSection,".bss",2,0);
}

if(mode&1)
	ReadSection		("player/main.obj",".libc",&Libc1Section);


AddExtSym		(&DataSection,SceneSym,SYM_TYPE_BYTE|((SYM_DTYPE_PTR)<<8)	,Render	,sizeof(MainRendering));

if(Render->hastext)
{
	int txtlen;

	ImageFile=fopen(Render->Text->FontFile,"rb");
	fseek	(ImageFile,0,SEEK_END);
	DataLen	=ftell(ImageFile);
	Data	=malloc(DataLen);
	rewind	(ImageFile);
	fread	(Data,DataLen,1,ImageFile);
	fclose	(ImageFile);

	AddSym		(&DataSection,"_FntImg"	,SYM_TYPE_BYTE|((SYM_DTYPE_PTR)<<8)	,Data				,DataLen);
	AddSym		(&DataSection,"_FImgLen",SYM_TYPE_UINT						,&DataLen			,4);
	free		(Data);

	AddSym		(&DataSection,"_Text",SYM_TYPE_BYTE|((SYM_DTYPE_PTR)<<8)	,Render->Text				,sizeof(text_t));

	Sym			=GetSym(&DataSection,"_Text");
	
	AddReloc	(&DataSection,Sym->Sym.e_value  ,"_FntImg");
	AddReloc	(&DataSection,Sym->Sym.e_value+4,"_FImgLen");

	memset		( ((text_t *)(&DataSection.Data[Sym->Sym.e_value]))->FontFile,0,255);

	Sym			=GetSym(&DataSection,SceneSym);
	AddReloc	(&DataSection,Sym->Sym.e_value+44  ,"_Text");


}

if(Render->hastexttyper)
{
	int txtlen;

	ImageFile=fopen(Render->TextTyper->FontFile,"rb");
	fseek	(ImageFile,0,SEEK_END);
	DataLen	=ftell(ImageFile);
	Data	=malloc(DataLen);
	rewind	(ImageFile);
	fread	(Data,DataLen,1,ImageFile);
	fclose	(ImageFile);

	AddSym		(&DataSection,"_TTImg"	,SYM_TYPE_BYTE|((SYM_DTYPE_PTR)<<8)	,Data				,DataLen);
	AddSym		(&DataSection,"_TTLen",SYM_TYPE_UINT						,&DataLen			,4);
	free		(Data);

	AddSym		(&DataSection,"_TxtTpr",SYM_TYPE_BYTE|((SYM_DTYPE_PTR)<<8)	,Render->TextTyper				,sizeof(texttyper_t));

	Sym			=GetSym(&DataSection,"_TxtTpr");
	
	AddReloc	(&DataSection,Sym->Sym.e_value  ,"_TTImg");
	AddReloc	(&DataSection,Sym->Sym.e_value+4,"_TTLen");

	memset		( ((texttyper_t *)(&DataSection.Data[Sym->Sym.e_value]))->FontFile,0,255);

	Sym			=GetSym(&DataSection,SceneSym);
	AddReloc	(&DataSection,Sym->Sym.e_value+48  ,"_TxtTpr");
}

if(Render->haslogo)
{
	int env_val_size;

	ImageFile=fopen(Render->Logo->file,"rb");
	
	fseek	(ImageFile,0,SEEK_END);
	
	DataLen		=ftell(ImageFile);
	Data		=malloc(DataLen);
	rewind		(ImageFile);
	fread		(Data,DataLen,1,ImageFile);
	fclose		(ImageFile);
	
	AddSym		(&DataSection,"_LogoImg",SYM_TYPE_BYTE|((SYM_DTYPE_PTR)<<8)	,Data				,DataLen);
	AddSym		(&DataSection,"_LImgLen",SYM_TYPE_UINT						,&DataLen			,4);

	free		(Data);

	AddSym		(&DataSection,"_Logo",SYM_TYPE_BYTE|((SYM_DTYPE_PTR)<<8)	,Render->Logo				,sizeof(logo_t));

	Sym			=GetSym(&DataSection,"_Logo");
	
	AddReloc	(&DataSection,Sym->Sym.e_value  ,"_LImgLen");
	AddReloc	(&DataSection,Sym->Sym.e_value+4,"_LogoImg");

	memset		( ((logo_t *)(&DataSection.Data[Sym->Sym.e_value]))->file,0,255);
	

	if(Render->Logo->PosXEnv!=NULL)
	{
		AddSym		(&DataSection,"_LXEnv" ,SYM_TYPE_UINT,Render->Logo->PosXEnv->vals	,Render->Logo->PosXEnv->numvals*sizeof(envval_t));
		AddSym		(&DataSection,"_LXEnv_",SYM_TYPE_UINT,Render->Logo->PosXEnv	,sizeof(enveloppe_t));

		Sym			=GetSym(&DataSection,"_LXEnv_");
		AddReloc	(&DataSection,Sym->Sym.e_value+4,"_LXEnv");

	
		Sym			=GetSym(&DataSection,"_Logo");
		AddReloc	(&DataSection,Sym->Sym.e_value+8,"_LXEnv_");
	
	}

	if(Render->Logo->PosYEnv!=NULL)
	{

		AddSym		(&DataSection,"_LYEnv" ,SYM_TYPE_UINT|((SYM_DTYPE_PTR)<<8),Render->Logo->PosYEnv->vals	,Render->Logo->PosYEnv->numvals*sizeof(envval_t));
		AddSym		(&DataSection,"_LYEnv_",SYM_TYPE_UINT|((SYM_DTYPE_PTR)<<8),Render->Logo->PosYEnv	,sizeof(enveloppe_t));

		Sym			=GetSym(&DataSection,"_LYEnv_");
		AddReloc	(&DataSection,Sym->Sym.e_value+4,"_LYEnv");

		Sym			=GetSym(&DataSection,"_Logo");
		AddReloc	(&DataSection,Sym->Sym.e_value+12,"_LYEnv_");

	}
	if(Render->Logo->OpacEnv!=NULL)
	{

		AddSym		(&DataSection,"_LOpEnv" ,SYM_TYPE_UINT|((SYM_DTYPE_PTR)<<8),Render->Logo->OpacEnv->vals	,Render->Logo->OpacEnv->numvals*sizeof(envval_t));
		AddSym		(&DataSection,"_LOpEnv_",SYM_TYPE_UINT|((SYM_DTYPE_PTR)<<8),Render->Logo->OpacEnv	,sizeof(enveloppe_t));
		
		Sym			=GetSym(&DataSection,"_LOpEnv_");
		AddReloc	(&DataSection,Sym->Sym.e_value+4,"_LOpEnv");

		Sym			=GetSym(&DataSection,"_Logo");
		AddReloc	(&DataSection,Sym->Sym.e_value+16,"_LOpEnv_");

	}
	if(Render->Logo->AngleEnv!=NULL)
	{

		AddSym		(&DataSection,"_LAngEnv" ,SYM_TYPE_UINT|((SYM_DTYPE_PTR)<<8),Render->Logo->AngleEnv->vals	,Render->Logo->AngleEnv->numvals*sizeof(envval_t));
		AddSym		(&DataSection,"_LAEnv_" ,SYM_TYPE_UINT|((SYM_DTYPE_PTR)<<8),Render->Logo->AngleEnv	,sizeof(enveloppe_t));
		Sym			=GetSym(&DataSection,"_LAEnv_");
		AddReloc	(&DataSection,Sym->Sym.e_value+4,"_LAngEnv");

		Sym			=GetSym(&DataSection,"_Logo");
		AddReloc	(&DataSection,Sym->Sym.e_value+20,"_LAEnv_");

	}
	if(Render->Logo->ZoomEnv!=NULL)
	{
		AddSym		(&DataSection,"_LZomEnv" ,SYM_TYPE_UINT|((SYM_DTYPE_PTR)<<8),Render->Logo->ZoomEnv->vals	,Render->Logo->ZoomEnv->numvals*sizeof(envval_t));
		AddSym		(&DataSection,"_LZEnv_" ,SYM_TYPE_UINT|((SYM_DTYPE_PTR)<<8),Render->Logo->ZoomEnv	,sizeof(enveloppe_t));
		Sym			=GetSym(&DataSection,"_LZEnv_");
		AddReloc	(&DataSection,Sym->Sym.e_value+4,"_LZomEnv");

		Sym			=GetSym(&DataSection,"_Logo");
		AddReloc	(&DataSection,Sym->Sym.e_value+24,"_LZEnv_");

	}
	Sym			=GetSym(&DataSection,SceneSym);
	AddReloc	(&DataSection,Sym->Sym.e_value+36  ,"_Logo");

}
if(Render->has3d)
{
	int vertex_size,fnorm_size,vnorm_size,color_size,index_size;
	obj3d_t		*Obj=Render->Rdr3d->object;


	AddSym		(&DataSection,"_ObjVert" ,SYM_TYPE_FLOAT|((SYM_DTYPE_PTR)<<8),Obj->vertex,Obj->num_vertex*sizeof(vertex_t));
	AddSym		(&DataSection,"_ObjFnrm" ,SYM_TYPE_FLOAT|((SYM_DTYPE_PTR)<<8),Obj->face_norm,Obj->num_triangle*sizeof(vertex_t));
	AddSym		(&DataSection,"_ObjVnrm" ,SYM_TYPE_FLOAT|((SYM_DTYPE_PTR)<<8),Obj->v_norm,Obj->num_vertex*sizeof(vertex_t));

	AddSym		(&DataSection,"_ObjCol" ,SYM_TYPE_UINT|((SYM_DTYPE_PTR)<<8),Obj->colors,Obj->num_triangle*sizeof(unsigned int));
	AddSym		(&DataSection,"_ObjInd" ,SYM_TYPE_UINT|((SYM_DTYPE_PTR)<<8),Obj->v_inds,Obj->num_triangle*3*sizeof(unsigned int));
	AddSym		(&DataSection,"_Obj3d" ,SYM_TYPE_BYTE|((SYM_DTYPE_PTR)<<8),Obj,sizeof(obj3d_t));
	
	Sym			=GetSym(&DataSection,"_Obj3d");


	
	if(((Obj->RendType>=5)&(Obj->RendType<8))|(Obj->RendType==1))
	{
		int uv_size;

		ImageFile=fopen(Render->Rdr3d->TexFile,"rb");
		fseek(ImageFile,0,SEEK_END);
		DataLen=ftell(ImageFile);
		Data=calloc(DataLen,1);
		rewind(ImageFile);
		fread(Data,DataLen,1,ImageFile);
		fclose(ImageFile);

		AddSym		(&DataSection,"_ObjImg" ,SYM_TYPE_BYTE|((SYM_DTYPE_PTR)<<8)	,Data				,DataLen);
		AddSym		(&DataSection,"_OImgLen",SYM_TYPE_UINT						,&DataLen			,4);
		free		(Data);

		AddReloc	(&DataSection,Sym->Sym.e_value  ,"_ObjImg");
		AddReloc	(&DataSection,Sym->Sym.e_value+4,"_OImgLen");

		
		
		
		if((Obj->RendType==5)|(Obj->RendType==6))
		{
			uv_size	=Obj->num_triangle*3*sizeof(uv_t);
			AddSym		(&DataSection,"_ObjUv" ,SYM_TYPE_SHORT|((SYM_DTYPE_PTR)<<8),Obj->uv,uv_size);
			AddReloc	(&DataSection,Sym->Sym.e_value+36,"_ObjUv");
		}
	}
	
	AddReloc	(&DataSection,Sym->Sym.e_value+20,"_ObjVert");
	AddReloc	(&DataSection,Sym->Sym.e_value+28,"_ObjFnrm");
	AddReloc	(&DataSection,Sym->Sym.e_value+32,"_ObjVnrm");
	AddReloc	(&DataSection,Sym->Sym.e_value+40,"_ObjCol");
	AddReloc	(&DataSection,Sym->Sym.e_value+44,"_ObjInd");

	AddSym		(&DataSection,"_Eng3d" ,SYM_TYPE_BYTE|((SYM_DTYPE_PTR)<<8),Render->Rdr3d,sizeof(eng3d_t));
	Sym			=GetSym(&DataSection,"_Eng3d");
	AddReloc	(&DataSection,Sym->Sym.e_value,"_Obj3d");

	memset		( ((eng3d_t *)(&DataSection.Data[Sym->Sym.e_value]))->TexFile,0,255);

	Sym			=GetSym(&DataSection,SceneSym);
	AddReloc	(&DataSection,Sym->Sym.e_value+32  ,"_Eng3d");




}

if(Render->hasstar)
{
	AddSym		(&DataSection,"_Stars",SYM_TYPE_BYTE|((SYM_DTYPE_PTR)<<8)	,Render->Stars				,sizeof(stars_t));
	
	Sym			=GetSym(&DataSection,SceneSym);
	AddReloc	(&DataSection,Sym->Sym.e_value+40  ,"_Stars");
}



if(Render->has3d)
{
	SymEntry *Sym2;
	obj3d_t		*Obj=Render->Rdr3d->object;

	AddSym	(&UDataSection,"_pverts" ,SYM_TYPE_SHORT|((SYM_DTYPE_PTR)<<8),NULL,Obj->num_vertex*sizeof(ivertex_t));
	AddSym	(&UDataSection,"_sverts" ,SYM_TYPE_UINT|((SYM_DTYPE_PTR)<<8),NULL,Obj->num_vertex*sizeof(unsigned int));
	AddSym	(&UDataSection,"_sinds"  ,SYM_TYPE_UINT|((SYM_DTYPE_PTR)<<8),NULL,Obj->num_vertex*sizeof(ivertex_t));
	
	Sym			=GetSym(&DataSection,"_Obj3d");

	AddRelocIndex(&DataSection,Sym->Sym.e_value+24,DataSection.numsyms);
	AddRelocIndex(&DataSection,Sym->Sym.e_value+48,DataSection.numsyms+1);
	AddRelocIndex(&DataSection,Sym->Sym.e_value+52,DataSection.numsyms+2);
}

if(Render->hassound)
{

	SoundFile	=fopen(Render->Sound->SndFile,"rb");
	fseek	    (SoundFile,0,SEEK_END);
	DataLen		=ftell(SoundFile);
	Data		=malloc(DataLen);
	rewind		(SoundFile);
	fread		(Data,DataLen,1,ImageFile);
	fclose		(SoundFile);
	
	AddSym		(&DataSection,"_SndFile",SYM_TYPE_BYTE|((SYM_DTYPE_PTR)<<8)	,Data				,DataLen);
	AddSym		(&DataSection,"_SndLenB",SYM_TYPE_UINT						,&DataLen			,4);
	
	AddSym		(&DataSection,"_Snd",SYM_TYPE_BYTE|((SYM_DTYPE_PTR)<<8)	    ,Render->Sound				,sizeof(snd_t));

	Sym			=GetSym(&DataSection,"_Snd");
	
	memset		( ((snd_t *)(&DataSection.Data[Sym->Sym.e_value]))->SndFile,0,255);
    ((snd_t *)(&DataSection.Data[Sym->Sym.e_value]))->Handle=NULL;
	

	AddReloc	(&DataSection,Sym->Sym.e_value  ,"_SndFile");
	AddReloc	(&DataSection,Sym->Sym.e_value+4,"_SndLenB");

	Sym			=GetSym(&DataSection,SceneSym);
	AddReloc	(&DataSection,Sym->Sym.e_value+52  ,"_Snd");
}

curtotalsyms=DataSection.numsyms+UDataSection.numsyms+CodeSection.numsyms+Libc1Section.numsyms;
if(mode&2)
{

	//change the relocation index to match new symbol index in the created obj
	
	for(j=0;j<CodeSection.numexterns;j++)
	{
		SymEntry *CurExtrn=&CodeSection.ExtrnSyms[j];
		
		if(CurExtrn->Sym.e.e.e_zeroes==0)
			CurExtrn->Sym.e.e.e_offset=4;

		for(i=0;i<CodeSection.numreloc;i++)
		{
			if(CodeSection.RelocList[i].r_symndx==CurExtrn->index)
			{
				NewRelocs[i]=curtotalsyms+j;
			}
		}
	}

	//set the new symindx in the relaction entry
	for(i=0;i<CodeSection.numreloc;i++)
	{
		if(CodeSection.RelocList[i].r_symndx!=6666666)
			CodeSection.RelocList[i].r_symndx=NewRelocs[i];
	}

	//set the new symindx in the scene symbol name
	for(i=0;i<CodeSection.numreloc;i++)
	{
		if(CodeSection.RelocList[i].r_symndx==6666666)
		{
			Sym=GetSym(&DataSection,SceneSym);
			CodeSection.RelocList[i].r_symndx=Sym->index;
		}
	}

	//change entry point symbol name
	for(i=0;i<CodeSection.numsyms;i++)
	{
		if(!strcmp(CodeSection.SymList[i].Sym.e.e_name,"_Start"))
		{
			strcpy(CodeSection.SymList[i].Sym.e.e_name,Entry);
		}
	}

}

if(mode&1)
{
	curtotalsyms+=CodeSection.numexterns;
	{
		int p1,p2,p3,p4,p5,p6,c;
		
		c   =strlen("__imp__ExitProcess@4")+1;

		
		
		p1=4 +strlen("__imp__VirtualAlloc@16")+1;
		p2=p1+strlen("__imp__VirtualFree@12")+1;
		p3=p2+strlen("__fltused")+1;
		p4=p3+strlen("_except_list")+1;
		p5=p4+strlen("___CxxFrameHandler")+1;
		p6=p5+strlen("__alloca_probe")+1;

	
		Libc1Section.ExtrnSyms[2].Sym.e.e.e_zeroes=0;
		Libc1Section.ExtrnSyms[2].Sym.e.e.e_offset=p1+c;

		Libc1Section.ExtrnSyms[3].Sym.e.e.e_zeroes=0;
		Libc1Section.ExtrnSyms[3].Sym.e.e.e_offset=p2+c;
		
		Libc1Section.ExtrnSyms[4].Sym.e.e.e_zeroes=0;
		Libc1Section.ExtrnSyms[4].Sym.e.e.e_offset=p3+c;

		Libc1Section.ExtrnSyms[5].Sym.e.e.e_zeroes=0;
		Libc1Section.ExtrnSyms[5].Sym.e.e.e_offset=p4+c;
		
		Libc1Section.ExtrnSyms[6].Sym.e.e.e_zeroes=0;
		Libc1Section.ExtrnSyms[6].Sym.e.e.e_offset=p5+c;

		Libc1Section.ExtrnSyms[7].Sym.e.e.e_zeroes=0;
		Libc1Section.ExtrnSyms[7].Sym.e.e.e_offset=p6+c;

		Libc1Section.RelocList[0].r_symndx =curtotalsyms+2;
		Libc1Section.RelocList[1].r_symndx =curtotalsyms+3;
		Libc1Section.RelocList[2].r_symndx =curtotalsyms+4;
		Libc1Section.RelocList[3].r_symndx =curtotalsyms+5;
		Libc1Section.RelocList[4].r_symndx =curtotalsyms+6;
		Libc1Section.RelocList[5].r_symndx =curtotalsyms+7;
	
		for(i=6;i<Libc1Section.numreloc;i++)
		{
			Libc1Section.RelocList[i].r_symndx=curtotalsyms;
		}
	}
}

InitCoff		(&TheObj);
if(mode&2)
{
	AddCoffSection	(&TheObj,&CodeSection);
}

AddCoffSection	(&TheObj,&DataSection);

if(UDataSection.SectionDataLen>0)
	AddCoffSection	(&TheObj,&UDataSection);

if(mode&1)
{
	Libc1Section.index	=TheObj.num_section+1;
	for(i=0;i<Libc1Section.numsyms;i++)
	{
		Libc1Section.SymList[i].Sym.e_scnum=Libc1Section.index;
	}

	for(i=0;i<Libc1Section.numexterns;i++)
	{
		if(Libc1Section.ExtrnSyms[i].Sym.e_scnum!=0)
		{
			Libc1Section.ExtrnSyms[i].Sym.e_scnum=Libc1Section.index;
		}
	}
	AddCoffSection	(&TheObj,&Libc1Section);
}



WriteCOFF		(&TheObj,file);

DeleteSection	(&DataSection);
DeleteSection	(&UDataSection);

DeleteSection	(&CodeSection);
}

*/

