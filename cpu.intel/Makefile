###############################################################
#   Students' Makefile for Operating Systems Project          #
#                     Use As Is                               #
###############################################################


# The assignment directory 
ASG_DIR = /home/osp/cpu2.intel/

# The default C compiler 
C_COMP = cc 


#make OSP:  

OSP : $(ASG_DIR)osp.o dialog.o cpu.o 
	$(C_COMP) $(ASG_DIR)osp.o dialog.o cpu.o  -lm -g -o OSP

dialog.o : dialog.c
	$(C_COMP) -c -g dialog.c

cpu.o : cpu.c 
	$(C_COMP) -c -g cpu.c 

