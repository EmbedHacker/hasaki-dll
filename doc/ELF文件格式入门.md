
# ELF文件格式入门

## 简介

可执行链接格式(Executable and Linking Format)最初是由 UNIX 系统实验室(UNIX System Laboratories，USL)开发并发布的，作为应用程序二进制接口(Application Binary Interface，ABI)的一部分。工具接口标准(Tool Interface Standards，TIS)委员会将还 在发展的 ELF 标准选作为一种可移植的目标文件格式，可以在 32 位 Intel 体系结构上的 很多操作系统中使用。

目标文件有三种类型:

可重定位文件(Relocatable File))包含适合于与其他目标文件链接来创建可执行文件或者共享目标文件的代码和数据，对应windows平台的.obj文件和Linux平台的.o文件。

可执行文件(Executable File)) 包含了程序创建进程映像所需要的信息。对应windows的exe文件和Lihux平台的动态/静态编译程序。

共享目标文件(Shared Object File)  包含可在两种上下文中链接的代码和数据。首先链接编辑器可以将它和其它可重定位文件和共享目标文件一起处理， 生成另外一个目标文件。其次动态链接器(Dynamic Linker)可能将它与某个可执行文件以及其它共享目标一起组合，创建进程映像。对应windows平台的。dll文件和Linux平台的.so文件。

目标文件全部是程序的二进制表示，目的是直接在某种处理器上直接执行。

## ELF文件格式

目标文件既要参与程序链接又要参与程序执行，必须要有一个良好的架构设计。
为了提高程序安全性和执行效率，目标程序通常分为多个段，如常见的.text段、.data段、.rodata段、.bss段等。

分段设计的程序，有以下好处:

- 防止程序程序的指令被改写。当程序被装载后，数据和指令分别被映射到两个虚存区域，数据区域对于进程可读写，而指令区域只读，所以可以把权限分别设置为可读写与只读。
- 有利于CPU的缓存命中率。现代的CPU缓存一般都被设计成数据缓存和指令缓存分离。
- 共享指令。在系统运行多个该程序的副本时，节省大量空间，写时拷贝。


目标文件既要参与程序链接又要参与程序执行。出于方便性和效率考虑，目标文件格式提供了两种并行视图，分别反映了这些活动的不同需求。

官方原文:

![](./../picture/ELF2.jpg)


中文翻译:
![](./../picture/ELF1.png)

文件开始处是一个 ELF 头部(ELF Header)，用来描述整个文件的组织。节区部分包含链接视图的大量信息:指令、数据、符号表、重定位信息等等。

程序头部表(Program Header Table)，如果存在的话，告诉系统如何创建进程映像。用来构造进程映像的目标文件必须具有程序头部表，可重定位文件不需要这个表。

节区头部表(Section Heade Table)包含了描述文件节区的信息，每个节区在表中 都有一项，每一项给出诸如节区名称、节区大小这类信息。用于链接的目标文件必须包含节区头部表，其他目标文件可以有，也可以没有这个表。

注意： 

- 尽管图中显示的各个组成部分是有顺序的，实际上除了 ELF 头部表以外， 其他节区和段都没有规定的顺序

- 我通常将section称为节，将program header称为segment（段）elf执行不需要section header table但必须要program header table，我尝试将所有的section header设置为0，程序依然可以执行

- elf链接不需要program header table，但必须要section header table，因为可重定位文件中没有program header table，但是有section header table，并且一个动态库so文件如果将section header table全部设置为0的话，同样链接不成功，因为链接过程中查找符号是依赖section‘查找的。

这也就是为什么上图中，链接视图中program header table是optional（可选的），而执行视图中section header table是optional（可选的）。
## ELF中的数据类型

为什么elf文件自己定义数据类型：这是因为这种文件格式支持8位到32位多种类型的处理器（现在有了64位），elf文件使用自己定义的数据类型以便于兼容这些各个平台。

```
/* Type for a 16-bit quantity.  */
typedef uint16_t Elf32_Half;
typedef uint16_t Elf64_Half;

/* Types for signed and unsigned 32-bit quantities.  */
typedef uint32_t Elf32_Word;
typedef int32_t  Elf32_Sword;
typedef uint32_t Elf64_Word;
typedef int32_t  Elf64_Sword;

/* Types for signed and unsigned 64-bit quantities.  */
typedef uint64_t Elf32_Xword;
typedef int64_t  Elf32_Sxword;
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;

/* Type of addresses.  */
typedef uint32_t Elf32_Addr;
typedef uint64_t Elf64_Addr;
```

这些数据类型都可以在Linux系统中的/usr/include/elf.h中找到定义

## ELF Hearder部分

文件的最开始几个字节给出如何解释文件的提示信息。这些信息独立于处理器，也
独立于文件中的其余内容。ELF Header 部分可以用以下的数据结构表示:

```
/* ELF Header */
typedef struct elfhdr {
	unsigned char	e_ident[EI_NIDENT]; /* ELF Identification */
	Elf32_Half	e_type;		/* object file type */
	Elf32_Half	e_machine;	/* machine */
	Elf32_Word	e_version;	/* object file version */
	Elf32_Addr	e_entry;	/* virtual entry point */
	Elf32_Off	e_phoff;	/* program header table offset */
	Elf32_Off	e_shoff;	/* section header table offset */
	Elf32_Word	e_flags;	/* processor-specific flags */
	Elf32_Half	e_ehsize;	/* ELF header size */
	Elf32_Half	e_phentsize;	/* program header entry size */
	Elf32_Half	e_phnum;	/* number of program header entries */
	Elf32_Half	e_shentsize;	/* section header entry size */
	Elf32_Half	e_shnum;	/* number of section header entries */
	Elf32_Half	e_shstrndx;	/* section header table's "section 
					   header string table" entry offset */
} Elf32_Ehdr;
typedef struct {
	unsigned char	e_ident[EI_NIDENT];	/* Id bytes */
	Elf64_Quarter	e_type;			/* file type */
	Elf64_Quarter	e_machine;		/* machine type */
	Elf64_Half	e_version;		/* version number */
	Elf64_Addr	e_entry;		/* entry point */
	Elf64_Off	e_phoff;		/* Program hdr offset */
	Elf64_Off	e_shoff;		/* Section hdr offset */
	Elf64_Half	e_flags;		/* Processor flags */
	Elf64_Quarter	e_ehsize;		/* sizeof ehdr */
	Elf64_Quarter	e_phentsize;		/* Program header entry size */
	Elf64_Quarter	e_phnum;		/* Number of program headers */
	Elf64_Quarter	e_shentsize;		/* Section header entry size */
	Elf64_Quarter	e_shnum;		/* Number of section headers */
	Elf64_Quarter	e_shstrndx;		/* String table index */
} Elf64_Ehdr;
```
- e_ident：介绍为Magic number and other info，总共为16个字节。
    - Magic number就是文件最开始的4个字节0x7F，’E‘，’L‘，’F‘，用于判断是否是ELF文件可以直接将文件缓冲区指针强转为unsigned与宏定义#define ELFMAG "\177ELF"判断是否相等。

    - #define EI_CLASS 4 /* File class byte index /，从宏定义中可看出第5个字节为ei_class，从后边的宏定义可以看出，这个字段是用来标识文件类型的，有ELFCLASS32和ELFCLASS64代表32位文件和64位文件。

    - #define EI_DATA 5 /* Data encoding byte index */，第6个字节是用来编码格式的，可根据此字节判断大端还是小端。
    - #define EI_VERSION 6 /* File version byte index /，/ Value must be EV_CURRENT */，第7个字节....它说必须是EV_CURRENT就EV_CURRENT吧。。。

    - #define EI_OSABI 7 /* OS ABI identification */，第8个字节，这个。。。看了后边的宏定义，貌似是用来标记系统平台的，有FREEBSD、LINUX、NETBSD、FREEBSD、ARM等。

    - #define EI_ABIVERSION 8 /* ABI version */，第9个字节，介绍ABI的版本，好像都是0，x86、x64上没见过不是0的，其他平台看

    - #define EI_PAD 9 /* Byte index of padding bytes */，到这里就没了，还剩7个字节，应该都是用于padding（填充）的

- e_type：用于指定文件类型，主要有ET_REL 、ET_EXEC、ET_DYN 三种类型，其他类型在PC上没见过
- e_machine：描述此elf文件支持的体系结构，PC上通常是EM_386和EM_X86_64
- e_version：有一个版本相关的，目前在elf.h中只有一个EV_CURRENT。
- e_entry：程序入口点的虚拟地址，也就是OEP。（此处需要注意，当elf文件的e_type为ET_EXEC时，这里的e_entry存放的是虚拟地址VA，如果- e_type是ET_DYN时（不仅仅动态库，可执行文件也会是ET_DYN，即开启了随机基址的可执行程序），那这里存放的就是RVA，加载到内存中后还  要加上imagebase）
- e_phoff：program header table在文件中的位置（文件偏移）。
- e_shoff：section header table在文件中的位置（文件偏移）
- e_flags：该成员保存与文件关联的特定于处理器的标志，标志名称形式EF_machine_flag，有关标志定义，请参阅“Machine Information”。（官方介绍）。intel上遇到的都是0
- e_ehsize：elf header的大小
- e_phentsize：program header table的每个表项的大小
- e_phnum：program header table表项的个数
- e_shentsize：section header table的每个表项的大小
- e_shnum：section header table表项的个数
- e_shstrndx：Section header string table index，表示字符串表这个节在section header table的索引（注意：这个字符串表存放的是每个scetion的名称的字符串表，还有其他的字符串表）

一个实际可执行文件的头文件头部形式如下：
```
$greadelf -h hello.so
ELF 头：
  Magic：  7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00
  类别:                             ELF32
  数据:                             2 补码，小端序 (little endian)
  版本:                             1 (current)
  OS/ABI:                           UNIX - System V
  ABI 版本:                         0
  类型:                             DYN (共享目标文件)
  系统架构:                          ARM
  版本:                             0x1
  入口点地址：                       0x0
  程序头起点：                       52 (bytes into file)
  Start of section headers:        61816 (bytes into file)
  标志：                            0x5000000, Version5 EABI
  本头的大小：                       52 (字节)
  程序头大小：                       32 (字节)
  Number of program headers:       9
  节头大小：                        40 (字节)
  节头数量：                        24
  字符串表索引节头：                 23
```
## 程序段表头（Program Header）

可执行文件或者共享目标文件的程序段表头部是一个结构数组，每个结构描述了一个段 或者系统准备程序执行所必需的其它信息。目标文件的“段”包含一个或者多个“节区”， 也就是“段内容(Segment Contents)”。程序头部仅对于可执行文件和共享目标文件有意义。

可执行目标文件在 ELF 头部的 e_phentsize 和 e_phnum 成员中给出其自身程序头部的大小。程序头部的数据结构:

```
/* Program Header */
typedef struct {
	Elf32_Word	p_type;		/* segment type */
	Elf32_Off	p_offset;	/* segment offset */
	Elf32_Addr	p_vaddr;	/* virtual address of segment */
	Elf32_Addr	p_paddr;	/* physical address - ignored? */
	Elf32_Word	p_filesz;	/* number of bytes in file for seg. */
	Elf32_Word	p_memsz;	/* number of bytes in mem. for seg. */
	Elf32_Word	p_flags;	/* flags */
	Elf32_Word	p_align;	/* memory alignment */
} Elf32_Phdr;
typedef struct {
	Elf64_Half	p_type;		/* entry type */
	Elf64_Half	p_flags;	/* flags */
	Elf64_Off	p_offset;	/* offset */
	Elf64_Addr	p_vaddr;	/* virtual address */
	Elf64_Addr	p_paddr;	/* physical address */
	Elf64_Xword	p_filesz;	/* file size */
	Elf64_Xword	p_memsz;	/* memory size */
	Elf64_Xword	p_align;	/* memory & file alignment */
} Elf64_Phdr;
```
- p_type 此数组元素描述的段的类型，或者如何解释此数组元素的信息。具体如下图。
    - PT_LOAD:一个可执行文件至少有一个PT_LOAD类型的段。这类程序头描述的是可装载的段，也就是说，这种类型的段将被装载或映射到内中
    - PT_DYNAMIC:动态段是动态链接可执行文件所特有的，包含了动态链接器所必需的一些信息
    - PT_PHDR:PT_HDR段保存了程序头表本身的位置和大小。Phdr表保存了所有的Phdr对文件（以及内存镜像）中段的描述信息
- p_offset 此成员给出从文件头到该段第一个字节的偏移
- p_vaddr 加载到内存中的虚拟地址
- p_paddr 加载到内存中的物理地址
- p_filesz 文件映像中段的大小/字节数。可以为 0
- p_memsz 内存映像中段的大小/字节数。可以为 0
- p_flags 此成员给出与改段的权限标志。
    - #define PF_R            0x4 //可读
    - #define PF_W            0x2 //可写
    - #define PF_X            0x1 //可执行
- p_align 段对齐

## 程序节区头

节区中包含目标文件中的所有信息，属性相同的节区会组合构成一个段。ELF 头部中，e_shoff 成员给出从文件头到节区头部表格的偏移字节数;e_shnum 给出表格中条目数目;e_shentsize 给出每个项目的字节数。从这些信息中可以确切地定位节区的具体位置、长度。

每个节区头部数据结构描述:

```
/* Section Header */
typedef struct {
	Elf32_Word	sh_name;	/* name - index into section header
					   string table section */
	Elf32_Word	sh_type;	/* type */
	Elf32_Word	sh_flags;	/* flags */
	Elf32_Addr	sh_addr;	/* address */
	Elf32_Off	sh_offset;	/* file offset */
	Elf32_Word	sh_size;	/* section size */
	Elf32_Word	sh_link;	/* section header table index link */
	Elf32_Word	sh_info;	/* extra information */
	Elf32_Word	sh_addralign;	/* address alignment */
	Elf32_Word	sh_entsize;	/* section entry size */
} Elf32_Shdr;
typedef struct {
	Elf64_Half	sh_name;	/* section name */
	Elf64_Half	sh_type;	/* section type */
	Elf64_Xword	sh_flags;	/* section flags */
	Elf64_Addr	sh_addr;	/* virtual address */
	Elf64_Off	sh_offset;	/* file offset */
	Elf64_Xword	sh_size;	/* section size */
	Elf64_Half	sh_link;	/* link to another */
	Elf64_Half	sh_info;	/* misc info */
	Elf64_Xword	sh_addralign;	/* memory alignment */
	Elf64_Xword	sh_entsize;	/* table entry size */
} Elf64_Shdr;
```

- sh_name : 这个字段确定节的名字。它的值不是一个字符串，而是节名字符串表的索引。

- sh_type : 节的类型
    - SHT_SYMTAB:符号表
    - SHT_STRTAB:字符串表
    - SHT_DYNAMIC:动态链接信息
    - SHT_REAL:重定位表
- sh_flags : 支持一位的标志，描述各种各样的属性
    - SHF_WRITE: 节区包含进程执行过程中将可写的数据。
    - SHF_ALLOC: 此节区在进程执行过程中占用内存。某些控制节区并不出现于目标
    文件的内存映像中，对于那些节区，此位应设置为 0。
    - SHF_EXECINSTR: 节区包含可执行的机器指令。
    - SHF_MASKPROC: 所有包含于此掩码中的四位都用于处理器专用的语义。

sh_addr : 节区加载后所在的虚拟地址。

sh_offset : 节区在文件中的偏移。

sh_size : 节的字节数。

sh_link : 这个成员持有一个节头表索引链接，它的解释器根据节的类型决定。


sh_addralign : 对齐限制。

sh_entsize : 有些节持有/保存一个固定尺寸项的表，比如说符号表(symbol table)。对这样一个节，这个成员给出每一项的字节大小。