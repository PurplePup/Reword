@echo off
@echo off
@rem Batch file to make reword for the GP2X on Windows dev kit

del *.o
C:/devkitGP2X/minsys/bin/make -f MakeFile_gp2x all
del /Q reword.gpe.fat
ren reword.gpe reword.gpe.fat
.\gpecomp reword.gpe.fat reword
mkdir reword_gp2x
move /Y reword .\reword_gp2x\reword
del /Q reword.gpe.fat 



