cmd_block/partitions/built-in.o :=  arm-hisiv300-linux-ld -EL    -r -o block/partitions/built-in.o block/partitions/check.o block/partitions/msdos.o block/partitions/efi.o block/partitions/cmdline.o 
