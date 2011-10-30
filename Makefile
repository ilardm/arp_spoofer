PNAME		=	spoofer

SRC			=	main.c\
				pf.c

OBJCPP		=	$(subst .cpp,.o,$(SRC))
OBJ			=	$(subst .c,.o,$(OBJCPP))

INCPATH		=	-I/usr/include/ -I./inc/
LIBS		=	-L/usr/lib -lc

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

HGBRANCH=$(shell hg branch 2>/dev/null )
HGREVISION=$(shell hg identify -i )
override CFLAGS:=$(CFLAGS) -D_VERSION=\"$(HGREVISION)-$(HGBRANCH)\"

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
