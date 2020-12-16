#ifndef _CMD_H
#define _CMD_H
    
#include "myMemory.h"
#include "init.h"
#include "super.h"
#include "fs.h"
#include "namei.h"
#include "open.h"
#include "inode.h"
#include "namei.h"
#include "read_write.h"

void list();

void mkdir();

void touch();

void shell_write();

void shell_read();

void ln();

void cd();

void lookup(char* dir, int*  res);

void pwd(int i);

void rm();

void rmdir();

void help();
#endif 