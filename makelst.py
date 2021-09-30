from os import system

SPMPC = "C:/Users/Daniel/spm-practice-codes"
BASE_PATH = SPMPC + "/include/spm.eu0.lst"
MOD_PATH = SPMPC + "/spm-practice-codes.eu0.elf"
MOD_REL_PATH = SPMPC + "/spm-practice-codes.eu0.rel"
TEMP_PATH = "spmpc.lst"
OUT_PATH = "include/spm.eu0.lst"

system(f"%ELF2LST% {MOD_PATH} {TEMP_PATH}")

system(f"cat {MOD_REL_PATH} > host.rel")

with open(BASE_PATH) as f:
    baseLst = f.read()

with open(TEMP_PATH) as f:
    modLst = f.read()

with open(OUT_PATH, 'w') as f:
    f.write(baseLst + '\n\n' + modLst)

