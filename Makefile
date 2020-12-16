TARGET=main
C_FILES=$(shell ls *.c)
BASE=$(basename $(C_FILES))
OBJS=$(addprefix obj/, $(addsuffix .o , $(BASE)))

$(TARGET):$(OBJS)
	rm -f $(TARGET)
	gcc -o $(TARGET) $(OBJS)

obj/%.o:%.c
	@if test ! -e obj; then\
	    mkdir obj;\
	fi;
	gcc -c -o $@ $<

clean:
	rm -f obj/*.o
	rm -f $(TARGET)
