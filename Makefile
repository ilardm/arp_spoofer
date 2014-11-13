PNAME		=	spoofer

SRC			=	main.c\
				utils.c\
				pf.c\
				spoofer.c

OBJCPP		=	$(subst .cpp,.o,$(SRC))
OBJ			=	$(subst .c,.o,$(OBJCPP))

INCPATH		=	-I/usr/include/ -I./inc/
LIBS		=	-L/usr/lib -lc -lpthread

vpath %.c src/
vpath %.cpp src/

# CC			=	clang
# CXX			=	clang++
# LNK			=	clang++
CC			=	gcc
CXX			=	g++
LNK			=	g++

CFLAGS		=	-c -Wall
DFLAGS		=	-g -O0 -D_DEBUG
RFLAGS		=	-O3 -D_RELEASE

VCBRANCH=$(shell git rev-parse --abbrev-ref HEAD)
VCREVISION=$(shell git describe --always HEAD)
override CFLAGS:=$(CFLAGS) -D_VERSION=\"$(VCREVISION)-$(VCBRANCH)\"

all: release

.PHONY: mtest
mtest:
	@echo "src '$(SRC)'"
	@echo "obj '$(OBJ)'"
	@echo "cflags '$(CFLAGS)'"

debug: CFLAGS += $(DFLAGS)
debug: $(OBJ)
	$(LNK) $(LIBS) $(OBJ) -o $(PNAME)

release: CFLAGS += $(RFLAGS)
release: $(OBJ)
	$(LNK) $(LIBS) $(OBJ) -o $(PNAME)

%.o: %.c
	$(CC) $(CFLAGS) $(INCPATH) $< -o $@

%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCPATH) $< -o $@


.PHONY: clean
clean:
	rm -f $(OBJ)

.PHONY: distclean
distclean: clean
	rm -f $(PNAME)
