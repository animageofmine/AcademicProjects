#include <stdio.h>
#include <ctype.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <openssl/bn.h>
#include <openssl/bio.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <openssl/des.h>
int extern secret();
int extern sign(char *,char *);
int extern verify(char *,char *);
int extern enc(char *,char *);
int extern dec(char *,char *);
int extern edit(char *,char *);
