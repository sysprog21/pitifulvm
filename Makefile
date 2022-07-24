CC ?= gcc
CFLAGS = -std=c99 -Os -Wall -Wextra

BIN = jvm
OBJ = jvm.o stack.o

include mk/common.mk
include mk/jdk.mk

# Build PitifulVM
all: $(BIN)
$(BIN): $(OBJ)
	$(VECHO) "  CC+LD\t\t$@\n"
	$(Q)$(CC) -o $@ $^

%.o: %.c
	$(Q)$(CC) $(CFLAGS) -c $<

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
	Recursion

check: $(addprefix tests/,$(TESTS:=-result.out))

leak: $(addprefix tests/,$(TESTS:=-leak.out))

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
	$(Q)$(RM) *.o jvm tests/*.out tests/*.class $(REDIR)

.PRECIOUS: %.o tests/%.class tests/%-expected.out tests/%-actual.out tests/%-result.out tests/%-leak.out

indent:
	clang-format -i *.c *.h 
	cloc jvm.c
