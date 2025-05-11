CC = gcc
CFLAGS = -Wall -Wextra -g
LEX = flex
YACC = bison
YFLAGS = -d

# Target binary
TARGET = json2relcsv

# Source files
SRCS = main.c ast.c schema.c csv_gen.c
OBJS = $(SRCS:.c=.o) lex.yy.o parser.tab.o

# Build rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Generate lexer and parser
lex.yy.c: scanner.l parser.tab.h
	$(LEX) $<

parser.tab.c parser.tab.h: parser.y
	$(YACC) $(YFLAGS) $<

# Object files
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Dependencies
main.o: main.c ast.h schema.h csv_gen.h
ast.o: ast.c ast.h
schema.o: schema.c schema.h ast.h
csv_gen.o: csv_gen.c csv_gen.h schema.h ast.h
lex.yy.o: lex.yy.c parser.tab.h ast.h
parser.tab.o: parser.tab.c parser.tab.h ast.h

# Clean
clean:
	rm -f $(TARGET) $(OBJS) lex.yy.c parser.tab.c parser.tab.h

.PHONY: all clean
