## Dynamic loading shared library principle analysis

Loading the dynamic library is done in the dlopen function in the dlmodule.c file, we can pay attention to the first parameter filename in this function,
The flags parameter is not strictly implemented here, so don't pay attention.

First look at the code:
```
1: void* dlopen(const char *filename, int flags)
2:  {
3: size_t pagesize = 0x1000;
4: int fd = open(filename, O_RDONLY);
5: ElfW(Ehdr) ehdr;
6: pread(fd, &ehdr, sizeof(ehdr), 0);
7:
8: if (ehdr.e_ident[EI_MAG0] != ELFMAG0 ||
9: ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
10: ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
11: ehdr.e_ident[EI_MAG3] != ELFMAG3 ||
12: ehdr.e_version != EV_CURRENT || ehdr.e_ehsize != sizeof(ehdr) ||
13: ehdr.e_phentsize != sizeof(ElfW(Phdr)))
14: fail(filename, "File has no valid ELF header!", NULL, 0);
15:
16: switch (ehdr.e_machine) {
17: case EM_X86_64:
18: case EM_ARM:
19: case EM_AARCH64:
20: break;
21: default:
22: fail(filename, "ELF file has wrong architecture! ",
23: "e_machine", ehdr.e_machine);
24: break;
25:}
26:
27: ElfW(Phdr) phdr[MAX_PHNUM];
28: if (ehdr.e_phnum> sizeof(phdr) / sizeof(phdr[0]) || ehdr.e_phnum <1)
29: fail(filename, "ELF file has unreasonable ", "e_phnum", ehdr.e_phnum);
30:
31: if (ehdr.e_type != ET_DYN)
32: fail(filename, "ELF file not ET_DYN! ", "e_type", ehdr.e_type);
33:
34: pread(fd, phdr, sizeof(phdr[0]) * ehdr.e_phnum, ehdr.e_phoff);
35:
36: size_t i = 0;
37: while (i <ehdr.e_phnum && phdr[i].p_type != PT_LOAD)
38: ++i;
39: if (i == ehdr.e_phnum)
40: fail(filename, "ELF file has no PT_LOAD header!", NULL, 0);
41:
42: /*
43: * ELF requires that PT_LOAD segments be in ascending order of p_vaddr.
44: * Find the last one to calculate the whole address span of the image.
45: */
46: const ElfW(Phdr) *first_load = &phdr[i];
47: const ElfW(Phdr) *last_load = &phdr[ehdr.e_phnum-1];
48: while (last_load> first_load && last_load->p_type != PT_LOAD)
49: --last_load;
50:
51: /*
52: * Total memory size of phdr between first and last PT_LOAD.
53: */
54: size_t span = last_load->p_vaddr + last_load->p_memsz-first_load->p_vaddr;
55:
56: /*
57: * Map the first segment and reserve the space used for the rest and
58: * for holes between segments.
59: */
60: const uintptr_t mapping =
61: (uintptr_t) mmap((void *) round_down(first_load->p_vaddr, pagesize),
62: span, prot_from_phdr(first_load), MAP_PRIVATE, fd,
63: round_down(first_load->p_offset, pagesize));
64:
65: /*
66: * Mapping will not always equal to round_down(first_load->p_vaddr, pagesize).
67: */
68: const ElfW(Addr) load_bias =
69: mapping-round_down(first_load->p_vaddr, pagesize);
70:
71: if (first_load->p_offset> ehdr.e_phoff ||
72: first_load->p_filesz <
73: ehdr.e_phoff + (ehdr.e_phnum * sizeof(ElfW(Phdr))))
74: fail(filename, "First load segment of ELF does not contain phdrs!",
75: NULL, 0);
76:
77: const ElfW(Phdr) *ro_load = NULL;
78: if (!(first_load->p_flags & PF_W))
79: ro_load = first_load;
80:
81: handle_bss(first_load, load_bias, pagesize);
82:
83: ElfW(Addr) last_end = first_load->p_vaddr + load_bias +
84: first_load->p_memsz;
85:
86: /* Map the remaining segments, and protect any holes between them. */
87: for (const ElfW(Phdr) *ph = first_load + 1; ph <= last_load; ++ph) {
88: if (ph->p_type == PT_LOAD) {
89: ElfW(Addr) last_page_end = round_up(last_end, pagesize);
90:
91: last_end = ph->p_vaddr + load_bias + ph->p_memsz;
92: ElfW(Addr) start = round_down(ph->p_vaddr + load_bias, pagesize);
93: ElfW(Addr) end = round_up(last_end, pagesize);
94:
95: if (start> last_page_end)
96: mprotect((void *) last_page_end,
97: start-last_page_end, PROT_NONE);
98:
99: mmap((void *) start, end-start,
100: prot_from_phdr(ph), MAP_PRIVATE | MAP_FIXED, fd,
101: round_down(ph->p_offset, pagesize));
102:
103: handle_bss(ph, load_bias, pagesize);
104: if (!(ph->p_flags & PF_W) && !ro_load)
105: ro_load = ph;
106:}
107:}
108:
109: /* Find PT_DYNAMIC header. */
110: ElfW(Dyn) *dynamic = NULL;
111: for (i = 0; i <ehdr.e_phnum; ++i) {
112: if (phdr[i].p_type == PT_DYNAMIC) {
113: assert(dynamic == NULL);
114: dynamic = (ElfW(Dyn) *) (load_bias + phdr[i].p_vaddr);
115:}
116:}
117: assert(dynamic != NULL);
118:
119: ElfW(Addr) ro_start = ro_load->p_offset + load_bias;
120: ElfW(Addr) ro_end = ro_start + ro_load->p_memsz;
121: ElfW_Reloc *rel
122:          (ElfW_Reloc *)(load_bias + get_dynamic_entry(dynamic, ELFW_DT_RELW));
123:      size_t relocs_size = get_dynamic_entry(dynamic, ELFW_DT_RELWSZ);
124:      for (i = 0; i < relocs_size / sizeof(ElfW_Reloc); i++) {
125:          ElfW_Reloc *reloc = &relocs[i];
126:          int reloc_type = ELFW_R_TYPE(reloc->r_info);
127:          switch (reloc_type) {
128:          case R_X86_64_RELATIVE:
129:          case R_ARM_RELATIVE:
130:          case R_AARCH64_RELATIVE:
131:          {
132:              ElfW(Addr) *addr = (ElfW(Addr) *)(load_bias + reloc->r_offset);
133:              /*
134:               * If addr loactes in read-only PT_LOAD section, i.e., .text, then
135:               * we give the memory fragment WRITE permission during relocating
136:               * its address. Reset its access permission after relocation to
137:               * avoid some secure issue.
138:               */
139:              if ((intptr_t) addr < ro_end && (intptr_t) addr >= ro_start) {
140:                  mprotect((void*) round_down((intptr_t) addr, pagesize),
141:                           pagesize, PROT_WRITE);
142:                  *addr += load_bias;
143:                  mprotect((void*) round_down((intptr_t) addr, pagesize),
144:                           pagesize, prot_from_phdr(ro_load));
145:              }
146:              else
147:                  *addr += load_bias;
148:              break;
149:          }
150:          default:
151:              assert(0);
152:          }
153:      }
154:  
155:      dloader_p o = malloc(sizeof(struct __DLoader_Internal));
156:      assert(o != NULL);
157:  
158:      o->load_bias = load_bias;
159:      o->entry = (void *)(ehdr.e_entry + load_bias);
160:      o->functab = o->entry;
161:  
162:      close(fd);
163:      return o;
164:  }
```

1. Line 4, use the open function to open the specified file in read-only mode (in fact, it is the dll_lib.so shared library file)
2. In line 5, the ElfW macro is actually designed to be compatible with many different chip architectures. The principle is as follows:

    These three macros are defined in the code:
    -#define ELFW(type) _ELFW(__ELF_NATIVE_CLASS, type)
    -#define _ELFW(bits, type) __ELFW(bits, type)
    -#define __ELFW(bits, type) ELF##bits##_##type
    
    "##" means a connector in C language, so ELFW(type) can be regarded as "ELF+__ELF_NATIVE_CLASS+type".
    __ELF_NATIVE_CLASS is also a macro. This macro is deeply hidden and needs to be traced. Taking the x86-64 platform as an example, the trace path is as follows:
    ```
    /usr/include/link.h->
    /usr/include/x86_64-linux-gnu/bits/elfclass.h->
    /usr/include/x86_64-linux-gnu/bits/wordsize.h->
    ```
    Tracing results:
    ```
    #define __ELF_NATIVE_CLASS __WORDSIZE
    # define __WORDSIZE 64
    ```
    Therefore, ELFW (Ehdr) is actually ELF64_Ehdr, which is the structure of the elf file used to represent the elf file header in the 64-bit system. It is defined in the /usr/lib/elf.h header file. For its specific functions, please refer to The previous "ELF file format entry document"

3. Line 6, use the preread function to read the elf file header and save it in the ehdr structure variable

4. Line 8~14, through checking and fixing the "ELF" character string, judge whether the ehdr file is legal

5. Line 16~25, judge whether it is x86-64/arm/AARCH64 platform. Not supported on other platforms

6. Line 27~29, define the segment header array to determine whether the number of segment headers is normal

7. Line 31, determine whether the elf file type is a shared library type

8. Line 34, according to the position of the segment header in the elf file header, read all the segment headers from the shared library file.

9. Lines 38 to 39, iteratively judges all segment headers, if the segment header is not a PT_LOAD type (need to be loaded into memory) segment,
Ignore it no longer.

10. Lines 46 to 49, find the first and last segment tables that need to be loaded into memory

11. Line 54 calculates the memory address range required if all segment tables are loaded into memory. The calculation method is:
The starting position of the virtual address required for the last segment + the memory size required for the last segment-the starting position of the virtual address required for the first segment

12. Lines 60 to 69, use the mmap function to load the first segment into memory. The use of the mmap function is more complicated (page alignment, anonymous mapping, permission settings, etc.), it is recommended to look at this code after viewing the man manual, here As a reminder: when the flags parameter of mmap does not specify MAP_FIXED, it is not guaranteed to map to the given address, so there may be a deviation in the mapping position, and the deviation value needs to be calculated.

13. Lines 70 to 76, determine whether the file offset position and size of the first segment header are legal.

14. Line 77~79, set a variable ro_load that records the header of the read-only segment, which will be used to record the read-only segment in the future.

15. Line 81, the .BSS segment in the processing program, the processing method: BSS segment length = memory size-file size, the memory of the BSS segment is cleared, and at the same time, because of the mamap mapping, page alignment access leads to excessive mapping. The mapped memory should also be cleared.

16. Lines 83 to 107, cyclically map the remaining segments, for the empty memory between segments, we need to use mprotect to protect. note,
When continuing to use the mmap function here, the flags parameter must specify MAP_FIXED, because at this time we can no longer tolerate the deviation of the mapped address.

17. Lines 119~153, deal with the characters that need to be relocated in the dynamic library (mainly some relative positioning instructions), because it may involve .text, .rodate
For segment modification, temporarily set the segment to be writable, and then protect it again after modification.

18. Lines 155~160, define the dynamic linker object, record the starting position of the shared library in the virtual memory in this object, and get the symbol table and function table at the starting address of the dynamic library (this is the shared library For details, see "Design Principles of Shared Libraries").