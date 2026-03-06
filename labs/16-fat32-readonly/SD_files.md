```
total 7.7M
drwx------ 1 magi-nerv staff    0 Mar  6 10:27 .
-rwx------ 1 magi-nerv staff 4.0K Mar  5 19:04 ._bootcode.bin
-rwx------ 1 magi-nerv staff 4.0K Mar  5 19:04 ._bootloader.bin
-rwx------ 1 magi-nerv staff 4.0K Mar  5 19:04 ._config.txt
-rwx------ 1 magi-nerv staff 4.0K Mar  5 19:04 ._fixup_cd.dat
-rwx------ 1 magi-nerv staff 4.0K Mar  5 19:04 ._start_cd.elf
drwxr-xr-x 4 root      wheel  128 Mar  6 10:27 ..
drwx------ 1 magi-nerv staff  16K Mar  6 10:27 .fseventsd
drwx------ 1 magi-nerv staff  16K Mar  5 19:03 .Spotlight-V100
-rwx------ 1 magi-nerv staff  50K Feb 12 17:19 bootcode.bin
-rwx------ 1 magi-nerv staff 2.0M Feb 12 16:42 bootloader.bin
-rwx------ 1 magi-nerv staff  394 Feb 12 17:44 config.txt
drwx------ 1 magi-nerv staff  16K Mar  6 10:26 dir1
-rwx------ 1 magi-nerv staff 3.1K Feb 12 17:19 fixup_cd.dat
-rwx------ 1 magi-nerv staff 2.0M Feb 10 17:19 kernel.img
-rwx------ 1 magi-nerv staff 782K Feb 12 17:19 start_cd.elf
-rwx------ 1 magi-nerv staff 2.7M Feb 10 17:22 start.elf
```


```
Got 15 files.
        F: SPOTLI~1.    (cluster 3; 0 bytes)
        F: FSEVEN~4.    (cluster 5; 0 bytes)
        F: BOOTCODE.BIN (cluster 160; 50248 bytes)
        F: _BOOT~10.BIN (cluster 164; 4096 bytes)
        F: BOOTLO~3.BIN (cluster 165; 2067364 bytes)
        F: _BOOT~11.BIN (cluster 292; 4096 bytes)
        F: CONFIG.TXT (cluster 293; 394 bytes)
        F: _CONFI~5.TXT (cluster 154; 4096 bytes)
        F: FIXUP_CD.DAT (cluster 294; 3149 bytes)
        F: _FIXU~12.DAT (cluster 295; 4096 bytes)
        F: KERNEL.IMG (cluster 296; 2070060 bytes)
        D: DIR1.    (cluster 112)
        F: START_CD.ELF (cluster 423; 800380 bytes)
        F: _STAR~13.ELF (cluster 472; 4096 bytes)
        F: START.ELF (cluster 473; 2818884 bytes)
```

## dir1

```
total 48K
drwx------ 1 magi-nerv staff 16K Mar  6 10:26 .
drwx------ 1 magi-nerv staff   0 Mar  6 10:27 ..
-rwx------ 1 magi-nerv staff   0 Mar  6 10:26 dir1_file.txt
drwx------ 1 magi-nerv staff 16K Mar  6 10:26 dir2
```

```
TRACE:cluster_to_lba:cluster 112 to lba: 36022
        F: ..    (cluster 112; 0 bytes)
        F: ...    (cluster 0; 0 bytes)
        F: DIR1_F~2.TXT (cluster 0; 0 bytes)
        D: DIR2.    (cluster 113)
```


## dir2

```
total 32K
drwx------ 1 magi-nerv staff 16K Mar  6 10:26 .
drwx------ 1 magi-nerv staff 16K Mar  6 10:26 ..
-rwx------ 1 magi-nerv staff   0 Mar  6 10:26 a
-rwx------ 1 magi-nerv staff   0 Mar  6 10:26 b
-rwx------ 1 magi-nerv staff   0 Mar  6 10:26 c
-rwx------ 1 magi-nerv staff   0 Mar  6 10:26 dir2_file.txt
```

```
TRACE:cluster_to_lba:cluster 113 to lba: 36054
        F: ..    (cluster 113; 0 bytes)
        F: ...    (cluster 112; 0 bytes)
        F: DIR2_F~2.TXT (cluster 0; 0 bytes)
        F: A.    (cluster 0; 0 bytes)
        F: B.    (cluster 0; 0 bytes)
        F: C.    (cluster 0; 0 bytes)
```