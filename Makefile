all: rpn

.PHONY: rpn
rpn: rpn.c
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $<
