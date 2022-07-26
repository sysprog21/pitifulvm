CC ?= gcc
CFLAGS = -std=c99 -Os -Wall -Wextra

BIN = jvm
OBJS = jvm.o stack.o constant-pool.o classfile.o

deps := $(OBJS:%.o=.%.o.d)

include mk/common.mk
include mk/jdk.mk

# Build PitifulVM
all: $(BIN)
$(BIN): $(OBJS)
	$(VECHO) "  CC+LD\t$@\n"
	$(Q)$(CC) -o $@ $^

%.o: %.c
	$(VECHO) "  CC\t$@\n"
	$(Q)$(CC) $(CFLAGS) -c -MMD -MF .$@.d $<

TESTS = \
	Factorial \
	Return \
	Constants \
	MoreLocals \
	PrintLargeNumbers \
	Collatz \
	PythagoreanTriplet \
	Arithmetic \
	CoinSums \
	DigitPermutations \
	FunctionCall \
	Goldbach \
	IntegerTypes \
	Jumps \
	PalindromeProduct \
	Primes \
	Recursion \
	Long

check: $(addprefix tests/,$(TESTS:=-result.out))

ifneq (, $(shell which valgrind))
leak: $(addprefix tests/,$(TESTS:=-leak.out))
endif

tests/%.class: tests/%.java
	$(Q)$(JAVAC) $^

tests/%-expected.out: tests/%.class
	$(Q)$(JAVA) -cp tests $(*F) > $@

tests/%-actual.out: tests/%.class jvm
	$(Q)./jvm $< > $@

tests/%-result.out: tests/%-expected.out tests/%-actual.out
	$(Q)diff -u $^ | tee $@; \
	name='test $(@F:-result.out=)'; \
	$(PRINTF) "Running $$name..."; \
	if [ -s $@ ]; then $(PRINTF) FAILED $$name. Aborting.; false; \
	else $(call pass); fi

tests/%-leak.out: tests/%.class jvm
	$(Q)valgrind ./jvm $< > $@ 2>&1; \
	name='test $(@F:-leak.out=)'; \
	$(PRINTF) "Running $$name..."; \
	if grep -q 'All heap blocks were freed' $@; \
	then $(call pass); \
	else $(PRINTF) FAILED $$name. Aborting.; false; fi

clean:
	$(Q)$(RM) $(OBJS) $(deps) *~ jvm tests/*.out tests/*.class $(REDIR)

.PRECIOUS: %.o tests/%.class tests/%-expected.out tests/%-actual.out tests/%-result.out tests/%-leak.out

indent:
	clang-format -i *.c *.h 
	cloc jvm.c

-include $(deps)
