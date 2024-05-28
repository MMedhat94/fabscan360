BINARY=bin
CODEDIRS=. ./gui ./backend ./sock ./motor ./logging lib
INCDIRS=. ./gui ./backend ./sock ./motor ./logging

CC=gcc
OPT=-O0
# generate files that encode make rules for the .h dependencies
DEPFLAGS=-MP -MD
# automatically add the -I onto each include directory
CFLAGS=-Wall -Wextra -Werror -g $(foreach D,$(INCDIRS),-I$(D)) $(OPT) $(DEPFLAGS)

# for-style iteration (foreach) and regular expression completions (wildcard)
CFILES=$(foreach D,$(CODEDIRS),$(wildcard $(D)/*.c))
# regular expression replacement
OBJECTS=$(patsubst %.c,%.o,$(CFILES))
DEPFILES=$(patsubst %.c,%.d,$(CFILES))

# Debugging output
$(info CFILES: $(CFILES))
$(info OBJECTS: $(OBJECTS))
$(info DEPFILES: $(DEPFILES))
$(info INCDIRS: $(INCDIRS))

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) -o $@ $^ $(shell pkg-config --cflags --libs gtk+-3.0) -lcurl -lwiringPi

# only want the .c file dependency here, thus $< instead of $^.
#
%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $< $(shell pkg-config --cflags --libs gtk+-3.0) -lcurl -lwiringPi

clean:
	rm -rf $(BINARY) $(OBJECTS) $(DEPFILES)

# shell commands are a set of keystrokes away
distribute: clean
	tar zcvf dist.tgz *

# @ silences the printing of the command
# $(info ...) prints output
diff:
	$(info The status of the repository, and the volume of per-file changes:)
	@git status
	@git diff --stat

# include the dependencies
-include $(DEPFILES)

# add .PHONY so that the non-targetfile - rules work even if a file with the same name exists.
.PHONY: all clean distribute diff
