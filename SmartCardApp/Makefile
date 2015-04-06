OPENSSL_DIR	= /usr/usc/openssl/0.9.7c
OPENSSL_DIR	= /home/scf-22/csci551b/openssl
OPENSSL_LIB_DIR	= $(OPENSSL_DIR)/lib
OPENSSL_INC_DIR	= $(OPENSSL_DIR)/include

SHLIBLDFLAGS	= -G

EXTRA_LDFLAGS	= -L$(OPENSSL_LIB_DIR) -lcrypto
EXTRA_INCLUDES	= -I$(OPENSSL_INC_DIR)

# NOTE: these are for linux
#
# OPENSSL_LIB_DIR	= /usr/lib
# OPENSSL_INC_DIR	= /usr/include
# SHLIBLDFLAGS		= -export-dynamic -shared
# EXTRA_LDFLAGS		= -rdynamic -L$(OPENSSL_LIB_DIR) -lcrypto
# EXTRA_INCLUDES	= -I$(OPENSSL_INC_DIR) -fPIC

all: libhw4sc.so hw4

hw4: main.o secret1.o sign.o verify.o enc.o dec.o edit.o 
	gcc main.o secret1.o sign.o verify.o enc.o dec.o edit.o -L/home/scf-22/csci551b/openssl/lib -I/home/scf-22/csci551b/openssl/include -lsocket -lcrypto -ldl -o hw4
	
main.o: main.c header.h
	gcc -g -c -Wall main.c -I/home/scf-22/csci551b/openssl/include

secret1.o: secret1.c header.h
	gcc -g -c -Wall secret1.c -I/home/scf-22/csci551b/openssl/include

sign.o: sign.c header.h
	gcc -g -c -Wall sign.c -I/home/scf-22/csci551b/openssl/include

verify.o: verify.c header.h
	gcc -g -c -Wall verify.c -I/home/scf-22/csci551b/openssl/include

enc.o: enc.c header.h
	gcc -c -Wall enc.c -I/home/scf-22/csci551b/openssl/include

dec.o: dec.c header.h
	gcc -g -c -Wall dec.c -I/home/scf-22/csci551b/openssl/include

edit.o: edit.c header.h
	gcc -g -c -Wall edit.c -I/home/scf-22/csci551b/openssl/include

smartcard: smartcard.o
	gcc -g -o libhw4sc.so $(SHLIBLDFLAGS) smartcard.o -lc -L/home/scf-22/csci551b/openssl/lib -lsocket -lcrypto -ldl -I/home/scf-22/csci551b/openssl/include

libhw4sc.so: smartcard.o
	gcc -g -o libhw4sc.so $(SHLIBLDFLAGS) smartcard.o -lc -L/home/scf-22/csci551b/openssl/lib -lsocket -lcrypto -ldl -I/home/scf-22/csci551b/openssl/include

smartcard.o: defs.h scintrf.h secret.h secret.c smartcard.c
	gcc -g -Wall $(EXTRA_INCLUDES) -c -o smartcard.o smartcard.c


clean:
	rm -f *.o *.so hw4
