# ---- Ρυθμίσεις ----
CC      = gcc
CFLAGS  = -Wall -Wextra -g
LDLIBS  = -pthread -lrt

# Όλα τα εκτελέσιμα που θέλουμε να φτιάχνονται.
# Όταν φτιάξεις enc.c / chan.c / proc.c, πρόσθεσέ τα εδώ (και σβήσε το child).
PROGS   = start chan enc proc

# Κοινά αρχεία που χρειάζονται ΟΛΑ τα προγράμματα
COMMON_SRC = ipc.c
COMMON_HDR = common.h

# ---- Στόχοι ----

# Ο πρώτος στόχος εκτελείται με σκέτο "make"
all: $(PROGS)

# Κανόνας-μοτίβο: κάθε πρόγραμμα X φτιάχνεται από το X.c + ipc.c
# και ξαναφτιάχνεται αν αλλάξει οποιοδήποτε από αυτά ή το common.h.
#   $@ = ο στόχος (π.χ. start)
#   $^ = όλες οι εξαρτήσεις
#   $(filter %.c,$^) = μόνο τα .c από τις εξαρτήσεις (όχι το common.h)
%: %.c $(COMMON_SRC) $(COMMON_HDR)
	$(CC) $(CFLAGS) -o $@ $(filter %.c,$^) $(LDLIBS)

# Σβήνει ό,τι φτιάχτηκε
clean:
	rm -f $(PROGS) *.o

# Λέει στο make ότι αυτοί οι στόχοι δεν είναι αρχεία
.PHONY: all clean
