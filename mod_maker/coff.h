
#ifdef __GNUC__
	typedef unsigned int size_t;
	#define __cdecl		__attribute__((__cdecl__))
    #define __stdcall	__attribute__((__stdcall__))	
	#define DLL_EXPORT 
	#define DLL_IMPORT

	#define stricmp strcasecmp
#endif


#define	MAX_IMPORT 4096
#define	MAX_REMAP  16*1024
#define	MAX_EXPORT 256
#define EI_NIDENT  16

typedef unsigned int Elf32_Addr;
typedef unsigned short Elf32_Half;
typedef unsigned int Elf32_Off;
typedef int Elf32_Sword;
typedef unsigned int Elf32_Word;


typedef struct {  // DOS .EXE header
    unsigned short e_magic;         // Magic number
    unsigned short e_cblp;          // Bytes on last page of file
    unsigned short e_cp;            // Pages in file
    unsigned short e_crlc;          // Relocations
    unsigned short e_cparhdr;       // Size of header in paragraphs
    unsigned short e_minalloc;      // Minimum extra paragraphs needed
    unsigned short e_maxalloc;      // Maximum extra paragraphs needed
    unsigned short e_ss;            // Initial (relative) SS value
    unsigned short e_sp;            // Initial SP value
    unsigned short e_csum;          // Checksum
    unsigned short e_ip;            // Initial IP value
    unsigned short e_cs;            // Initial (relative) CS value
    unsigned short e_lfarlc;        // File address of relocation table
    unsigned short e_ovno;          // Overlay number
    unsigned short e_res[4];        // Reserved unsigned shorts
    unsigned short e_oemid;         // OEM identifier (for e_oeminfo)
    unsigned short e_oeminfo;       // OEM information; e_oemid specific
    unsigned short e_res2[10];      // Reserved unsigned shorts
    unsigned long   e_lfanew;        // File address of new exe header
  } IMAGE_DOS_HEADER;



typedef struct {
  unsigned short f_magic;         /* magic number             */
  unsigned short f_nscns;         /* number of sections       */
  unsigned int  f_timdat;        /* time & date stamp        */
  unsigned int  f_symptr;        /* file pointer to symtab   */
  unsigned int  f_nsyms;         /* number of symtab entries */
  unsigned short f_opthdr;        /* sizeof(optional hdr)     */
  unsigned short f_flags;         /* flags                    */
} FILHDR;


typedef struct  {
    unsigned long	VirtualAddress;
    unsigned long   Size;
} IMAGE_DATA_DIRECTORY;

typedef struct {
	unsigned short  Hint;		// Possible ordinal number to use
	unsigned char Name[1];	// Name of function, null terminated
} IMAGE_IMPORT_BY_NAME;

typedef struct{
union {
	unsigned int			*Function;				// address of imported function
	unsigned int			Ordinal;				// ordinal value of function
	IMAGE_IMPORT_BY_NAME	*AddressOfData;			// RVA of imported name
	unsigned int			ForwarderStringl;		// RVA to forwarder string
} u1;
} IMAGE_THUNK_DATA;

typedef struct  {
unsigned int		Characteristics; //  At one time, this may have been a set of flags. However, Microsoft changed its meaning and never bothered to update WINNT.H. This field is really an offset (an RVA) to an array of pointers. Each of these pointers points to an IMAGE_IMPORT_BY_NAME structure. 
unsigned int		TimeDateStamp ; //  The time/date stamp indicating when the file was built. 
unsigned int		ForwarderChain; //  This field relates to forwarding. Forwarding involves one DLL sending on references to one of its functions to another DLL. For example, in Windows NT, NTDLL.DLL appears to forward some of its exported functions to KERNEL32.DLL. An application may think it's calling a function in NTDLL.DLL, but it actually ends up calling into KERNEL32.DLL. This field contains an index into FirstThunk array (described momentarily). The function indexed by this field will be forwarded to another DLL. Unfortunately, the format of how a function is forwarded isn't documented, and examples of forwarded functions are hard to find.
unsigned int		Name; //  This is an RVA to a NULL-terminated ASCII string containing the imported DLL's name. Common examples are "KERNEL32.DLL" and "USER32.DLL".

IMAGE_THUNK_DATA	*FirstThunk ;
}IMAGE_IMPORT_DIRECTORY;

typedef struct  {
    unsigned int   Characteristics;
    unsigned int   TimeDateStamp;
    unsigned short    MajorVersion;
    unsigned short    MinorVersion;
    unsigned int   Name;
    unsigned int   Base;
    unsigned int   NumberOfFunctions;
    unsigned int   NumberOfNames;
    unsigned int   AddressOfFunctions;     // RVA from base of image
    unsigned int   AddressOfNames;         // RVA from base of image
    unsigned int   AddressOfNameOrdinals;  // RVA from base of image
} IMAGE_EXPORT_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16

typedef struct {
    //
    // Standard fields.
    //
    unsigned short  Magic;
    unsigned char   MajorLinkerVersion;
    unsigned char   MinorLinkerVersion;
    unsigned long   SizeOfCode;
    unsigned long   SizeOfInitializedData;
    unsigned long   SizeOfUninitializedData;
    unsigned long   AddressOfEntryPoint;
    unsigned long   BaseOfCode;
    unsigned long   BaseOfData;
    //
    // NT additional fields.
    //
    unsigned long   ImageBase;
    unsigned long   SectionAlignment;
    unsigned long   FileAlignment;
    unsigned short  MajorOperatingSystemVersion;
    unsigned short  MinorOperatingSystemVersion;
    unsigned short  MajorImageVersion;
    unsigned short  MinorImageVersion;
    unsigned short  MajorSubsystemVersion;
    unsigned short  MinorSubsystemVersion;
    unsigned long   Reserved1;
    unsigned long   SizeOfImage;
    unsigned long   SizeOfHeaders;
    unsigned long   CheckSum;
    unsigned short  Subsystem;
    unsigned short  DllCharacteristics;
    unsigned long   SizeOfStackReserve;
    unsigned long   SizeOfStackCommit;
    unsigned long   SizeOfHeapReserve;
    unsigned long   SizeOfHeapCommit;
    unsigned long   LoaderFl2ags;
    unsigned long   NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER;

#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL 9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11
#define SHT_LOPROC 0x70000000
#define SHT_HIPROC 0x7fffffff
#define SHT_LOUSER 0x80000000
#define SHT_HIUSER 0xffffffff

#define STB_LOCAL 0
#define STB_GLOBAL 1
#define STB_WEAK 2
#define STB_LOPROC 13
#define STB_HIPROC 15

#define STT_NOTYPE 0
#define STT_OBJECT 1
#define STT_FUNC 2
#define STT_SECTION 3
#define STT_FILE 4
#define STT_LOPROC 13
#define STT_HIPROC 15


#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0x7fffffff

#define DT_NULL 0			//ignored mandatory mandatory
#define DT_NEEDED 1			//d_val optional optional
#define DT_PLTRELSZ 2		//d_val optional optional
#define DT_PLTGOT 3			//d_ptr optional optional
#define DT_HASH 4			//d_ptr mandatory mandatory
#define DT_STRTAB 5			//d_ptr mandatory mandatory
#define DT_SYMTAB 6			//d_ptr mandatory mandatory
#define DT_RELA 7			//d_ptr mandatory optional
#define DT_RELASZ 8			//d_val mandatory optional
#define DT_RELAENT 9		//d_val mandatory optional
#define DT_STRSZ 10			//d_val mandatory mandatory
#define DT_SYMENT 11		//d_val mandatory mandatory
#define DT_INIT 12			//d_ptr optional optional
#define DT_FINI 13			//d_ptr optional optional
#define DT_SONAME 14		//d_val ignored optional
#define DT_RPATH 15			//d_val optional ignored
#define DT_SYMBOLIC 16		//ignored ignored optional
#define DT_REL 17			//d_ptr		mandatory optional
#define DT_RELSZ 18			//d_val mandatory optional
#define DT_RELENT 19		//d_val mandatory optional
#define DT_PLTREL 20		//d_val optional optional
#define DT_DEBUG 21			//d_ptr optional ignored
#define DT_TEXTREL 22		//ignored optional optional
#define DT_JMPREL 23		//d_ptr optional optional
#define DT_LOPROC 0x70000000 //unspeci?ed unspeci?ed unspeci?ed
#define DT_HIPROC 0x7fffffff //unspeci?ed unspeci?ed unspeci?ed


#define ELF32_ST_BIND(i) ((i) >> 4 )
#define ELF32_ST_TYPE(i) ((i) & 0xf )
#define ELF32_ST_INFO(b,t) (((b) << 4 ) + ((t)&0xf))

#define ELF32_R_SYM(i) ((i)>> 8)
#define ELF32_R_TYPE(i) ((unsigned char) (i))
#define ELF32_R_INFO(s,t) (((s) << 8) + ( unsigned char ) (t))

typedef struct {
  char           s_name[8];  /* section name                     */
  unsigned int  s_paddr;    /* physical address, aliased s_nlib */
  unsigned int  s_vaddr;    /* virtual address                  */
  unsigned int  s_size;     /* section size                     */
  unsigned int  s_scnptr;   /* file ptr to raw data for section */
  unsigned int  s_relptr;   /* file ptr to relocation           */
  unsigned int  s_lnnoptr;  /* file ptr to line numbers         */
  unsigned short s_nreloc;   /* number of relocation entries     */
  unsigned short s_nlnno;    /* number of line number entries    */
  unsigned int  s_flags;    /* flags                            */
} SCNHDR;

typedef struct {
Elf32_Word sh_name;
Elf32_Word sh_type;
Elf32_Word sh_flags ;
Elf32_Addr sh_addr;
Elf32_Off  sh_offset ;
Elf32_Word sh_size;
Elf32_Word sh_link;
Elf32_Word sh_info;
Elf32_Word sh_addralign;
Elf32_Word sh_entsize ;
}Elf32_Shdr ;

typedef struct 
{
Elf32_Word p_type ;
Elf32_Off  p_offset ;
Elf32_Addr p_vaddr ;
Elf32_Addr p_paddr ;
Elf32_Word p_filesz ;
Elf32_Word p_memsz ;
Elf32_Word p_flags ;
Elf32_Word p_align ;
}Elf32_Phdr;


typedef struct {
  union {
    char e_name[8];
    struct {
      unsigned int e_zeroes;
      unsigned int e_offset;
    } e;
  } e;
  unsigned int e_value;
  short e_scnum;
  unsigned short e_type;
  unsigned char e_sclass;
  unsigned char e_numaux;
} SYMENT;

typedef struct {
Elf32_Word st_name ;
Elf32_Addr st_value ;
Elf32_Word st_size ;
unsigned char st_info ;
unsigned char st_other ;
Elf32_Half st_shndx ;
}Elf32_Sym ;

typedef struct  {
Elf32_Addr r_offset;
Elf32_Word r_info ;
}Elf32_Rel;

typedef struct 
{
Elf32_Addr r_offset;
Elf32_Word r_info ;
Elf32_Sword r_addend ;
}Elf32_Rela;


typedef struct  {
  unsigned long r_vaddr ;
  unsigned long r_symndx ;
  unsigned short r_type;
}RELOC;

typedef struct
{
SYMENT		Sym;
Elf32_Sym	eSym;
int			index;
}SymEntry;

typedef struct
{
	unsigned int base_addr;
	unsigned int offset;
	unsigned int type;
}dyn_remap;

typedef struct
{
char			dll_name[64];
unsigned int	ordinal_imp;
unsigned char	*addr_reloc;
}section_imp_ord;

typedef struct
{
char			dll_name[64];
char			func_name[256];
unsigned int	addr_reloc;
unsigned int	sym_idx;
}section_imp_name;


typedef struct
{
char			dll_name[64];
unsigned int	ordinal_exp;
unsigned char	*addr_reloc;
}section_exp_ord;

typedef struct
{
char			dll_name[64];
char			func_name[256];
unsigned int    addr_reloc;
unsigned int	sym_idx;
}section_exp_name;

typedef struct
{
SCNHDR					SectionHeader;
Elf32_Shdr				shdr;
Elf32_Phdr				phdr;

char					Name[10];
int						index;

unsigned int			sections_file_data_ptr;

section_imp_ord			ImportsOrd[1000];
section_imp_name		ImportsName[MAX_IMPORT];
section_exp_ord			ExportsOrd[1000];
section_exp_name		ExportsName[MAX_EXPORT];

SymEntry				SymList[1000];
RELOC					RelocList[1000];
dyn_remap				RemapList[MAX_REMAP];
SymEntry				ExtrnSyms[50];
int						num_sec_imp_ord;
int						num_sec_imp_name;
int						num_sec_exp_ord;
int						num_sec_exp_name;

int						numexterns;
int						numsyms;
int						numreloc;
int						num_remap;
unsigned char			*Data;
unsigned int			SectionDataLen;

unsigned int			PEVirtualPageAddrEnd;
unsigned int			TotalLen;
unsigned int			v_addr;
}Section;


typedef struct
{
FILHDR		Header;
Section		*Sections[10];
int			num_section;

int			strings_len;
int			num_strings;
char		*Strings[10];
}COFFObj;



typedef struct
{
unsigned char e_ident[EI_NIDENT] ;
Elf32_Half e_type ;
Elf32_Half e_machine;
Elf32_Word e_version;
Elf32_Addr e_entry;
Elf32_Off  e_phoff;
Elf32_Off  e_shoff;
Elf32_Word e_flags;
Elf32_Half e_ehsize;
Elf32_Half e_phentsize;
Elf32_Half e_phnum;
Elf32_Half e_shentsize ;
Elf32_Half e_shnum;
Elf32_Half e_shstrndx;
}Elf32_Ehdr;

typedef struct {
Elf32_Sword d_tag ;

union {
	Elf32_Word d_val ;
	Elf32_Addr d_ptr ;
}d_un ;

}Elf32_Dyn ;

typedef struct
{
char					*string;
int						section_id;
int						start_ofs;
int						end_ofs;
}string_el;

typedef struct
{
char					sig[4];
char					file_name[512];
unsigned int			ImageBase;


Elf32_Ehdr				ehdr;




FILHDR					fhdr;
IMAGE_DOS_HEADER		doshdr;
IMAGE_OPTIONAL_HEADER	OptionalHeader;
IMAGE_EXPORT_DIRECTORY	*ImageExport;
IMAGE_IMPORT_DIRECTORY	*ImageImport;


Section					*Sections[32];
int						num_section;


Section					*eSections[32];
int						num_esection;


int						strings_len;

int						num_strings;
string_el				Strings[4096];

int						num_estrings;
string_el				eStrings[4096];

int						num_libs;
char					*lib_names[64];


}PEFile;


#define SCN_WRITE			0x80000000
#define SCN_READ			0x40000000
#define SCN_EXE				0x20000000
#define SCN_ALIGN1			0x00100000
#define SCN_ALIGN4			0x00300000
#define SCN_ALIGN8			0x00400000
#define SCN_ALIGN16			0x00500000




#define SCN_DATA			0x00000040
#define SCN_UDATA			0x00000080
#define SCN_CODE			0x00000020

#define SYM_CLASS_EXTERNAL  0x02
#define SYM_CLASS_STATIC	0x03

#define SYM_DTYPE_PTR		0x01
#define SYM_DTYPE_ARY		0x03

#define SYM_TYPE_SHORT		0x03
#define SYM_TYPE_FLOAT		0x06
#define	SYM_TYPE_UINT		0x0E
#define	SYM_TYPE_CHAR		0x02
#define	SYM_TYPE_BYTE		0x0C
char	zero[4096];

void			ReadExeFile			(char *file,PEFile *PE);
void			ReadPeSection		(PEFile *PE,char *name);
void			ReadPeSections		(PEFile *PE);
Section			*GetPESectionPtr	(PEFile *PE,char *name);
unsigned char	*MapPEFile			(PEFile *PE);
void			BuildPE_RVA			(PEFile *PE);
unsigned int	FindSectionMem		(PEFile *PE,unsigned int addr);
unsigned char  *FindSectionMemPtr	(PEFile *PE,unsigned int addr);


void init_section(Section *sec);
void ReadSection(char *file,char *name,Section *Sec);
void InitSection(Section *Sec,char *name,int index,int hasdata);
SymEntry *GetSym(Section *Sec,char *symname);
void AddSym(Section *Sec,char *name,int type,unsigned char *Data,int len);
void AddExtSym(Section *Sec,char *name,int type,unsigned char *Data,int len);
void AddExtSym2(Section *Sec,unsigned int StringOfset,int type,int len);
void AddReloc(Section *Sec,int addr,char *symname);
void AddRelocIndex(Section *Sec,int addr,int index);
void CreateFileHeader(COFFObj *Coff,int alosfet);
void CreateSectionHeader(Section *Sec,int startpos);
void WriteSection(Section *Sec,FILE *s);
void WriteCOFF(COFFObj *Coff,char *file);
void DeleteSection(Section *Sec);
void InitCoff(COFFObj *Coff);
void AddCoffString(COFFObj *Coff,char *String);
void AddCoffSection(COFFObj *Coff,Section *Sec);
void addEmptySection(PEFile *PE,unsigned int size);


void ReadElfFile		(char *file,PEFile *PE);
void ReadElfSections	(PEFile *PE);
void ReadElfSegments	(PEFile *PE);
void ReadElfeImpExp		(PEFile *PE);
void BuildElf_RVA		(PEFile *PE);