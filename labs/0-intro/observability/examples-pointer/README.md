### Compiler optimization examples

These are snippets organized in increasing fancy-ness showing how the
compiler optimizes when it decides a transformation cannot be observed.

Each file:
  1. gets compiled to machine code (the .o)
  2. disassembled to assembly so you can see what was done (`o.dis`)
  3. also compiled to an annoted assemblyf that can sometimes be
     easier to understand (`.o.dis.annot`).   

You should compare the disassembly to the C code --- its ok if you 
don't understand everything but it should be possible to pick out 
generally when variables are getting removed, reordered, etc.
