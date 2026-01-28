## Tiny Quine: 54-bytes (32-bit ELF, 140e 2025) 

Thomason Zhao  
Tianle Yu

### Overview

This project is a *tiny* self-reproducing ELF executable (a quine) for Linux **x86 (32-bit)** --- 54 bytes.

Our size goal forced us to stop treating the executable as “something the toolchain outputs” and instead treat it as a **binary file format problem**: what is the *minimum* an ELF loader needs in order to map bytes into memory and jump to code?

The end result is a handcrafted ELF that is only slightly larger than a bare ELF header.

### Why not `gcc`

A normal `gcc` build is optimized for convenience and compatibility, not byte-counting. Even with `-Os` and `strip`, `gcc` typically pulls in extra structure such as:

- **C runtime startup** (e.g., `_start` glue, prologue/epilogue expectations)
- libc / dynamic linker conventions (depending on flags), plus metadata for linking/loading
- standard ELF **sections** and bookkeeping that aren’t required for execution (useful for linkers/debuggers)

For extreme size constraints, the right move is to avoid the default runtime model entirely and emit exactly what the Linux loader needs—no more.

### What we hand-crafted

Linux executes an ELF by reading the **ELF header** and then the **program header table** (segments). By contrast, the **section header table** is mainly for linkers/debuggers and is not required for the kernel to run a static “flat” executable.

To be clear, the *full* headers are larger than what we actively “care about”:

**ELF32 header fields (complete list)**  
`e_ident, e_type, e_machine, e_version, e_entry, e_phoff, e_shoff, e_flags, e_ehsize, e_phentsize, e_phnum, e_shentsize, e_shnum, e_shstrndx`

**ELF32 program header fields (complete list)**  
`p_type, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_flags, p_align`

For our tiny binary, we only need a minimal subset to make the loader map bytes and jump to code:
- ELF: `e_ident`, `e_type/e_machine`, `e_entry`, and enough of `e_phoff/e_phentsize/e_phnum` to locate the program header.
- Program header: `p_type=PT_LOAD`, `p_offset`, `p_vaddr`, `p_filesz/p_memsz`.

Why we don’t “check” (or spend extra bytes on) the rest:
- Section-related fields (`e_shoff`, `e_sh*`) exist for tooling; we don’t rely on sections at all.
- Some fields can be safe constants (e.g., sizes/versions) or simply not consulted in our layout.
- Some bytes can be *reused* (overlapped) because Linux is permissive about certain values and because the ELF + PHDR structures don’t have to be stored in separate file regions.
- THE MAIN GOAL IS MINIMIZING SIZE. We just make it small enough to let loader do its job.

This is the core size trick: treat the headers as *data the loader consumes*, then pack/overlap them so one set of bytes satisfies multiple “interpretations.”

### The main trick: overlapping headers

The key size-saving idea is that the ELF header and the first program header don’t *have* to be two separate regions in the file.

Some bytes in the ELF header are:
- unused padding, or
- fields that Linux is permissive about, or
- values that can safely “double as” program-header values (and vice versa)

So we **overlay** the ELF header and the program header into a single packed structure, carefully choosing values so the loader reads consistent information from the same bytes.

When a value can’t be shared cleanly, we prefer:
- generating it through very short code, or
- choosing constants that satisfy both interpretations.

### Quine behavior (high level)

At runtime, the program writes out its own bytes to standard output and then exits. We keep the runtime logic minimal (no libc): just direct Linux syscalls, and only what’s needed to reproduce the file exactly.

### References

- [The Smallest Hello World Program](https://blog.lohr.dev/smol-hello-world)
- [Hello World Optimized](https://gist.github.com/michidk/d71f994aab9e778f5542a19cd7c1a152)
- [A Whirlwind Tutorial on Creating Really Teensy ELF Executables for Linux](https://www.muppetlabs.com/~breadbox/software/tiny/teensy.html)
