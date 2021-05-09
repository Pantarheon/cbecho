cbecho.exe: cbecho.c
	cl /Ox /GF /Gz /MD cbecho.c
	rm cbecho.obj
