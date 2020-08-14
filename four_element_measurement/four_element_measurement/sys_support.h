/*
 * sys_support.h
 *
 * Created: 4/13/2020 2:50:53 PM
 *  Author: kshort
 */ 

#ifndef SYS_SUPPORT_H_
#define SYS_SUPPORT_H_

#include "saml21j18b.h"
#include <stdio.h>

extern int _write(FILE *f, char *buf, int n);
extern int _read(FILE *f, char *buf, int n);
extern int _close(FILE *f);
extern int _fstat(FILE *f, void *p);
extern int _isatty(FILE *f);
extern int _lseek(FILE *f, int o, int w);
extern void* _sbrk(int i);

#endif /* SYS_SUPPORT_H_ */