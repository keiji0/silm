#ifndef LOCAL_H
#define LOCAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>

typedef long Cell;
typedef unsigned long uCell;

#define Obj void *
#define cellsize sizeof(Cell)
#define nil ((Obj)0)
#define true ((Obj)1)
#define cell(x) ((Cell)x)
#define obj(x) ((Obj)x)
#define addr(x) ((Cell *)x)
#define fetch(x) (*addr(x))
#define store(x, y) (fetch(x) = cell(y))
#define eq(x, y) (((Cell)x) == ((Cell)y))
#define eqs(x, y) ((*(char *)(x) == *(char *)(y)) ? !strcmp((char *)(x), (char *)(y)) : 0)
#define exp(a, x, y) (((Cell)x) a ((Cell)y))
#define alignment(x) ((((Cell)x)+(sizeof(Cell)-1))&(-sizeof(Cell)))
#define each(x) for (; x; x = cdr(x))
#define P(...) do { printf(__VA_ARGS__); fflush(stdout); } while (0)
#define funtype(tr, ta) (tr(*)ta)
#define funcall(tr, f, ta, ...) (funtype(tr, ta)f)(__VA_ARGS__)
#define clear(t, s) memset((t), 0, (s))

/* Silm Object */

enum Tag {
  tagobj = 0,
  tagnum = 1,
  tagpair = 2,
  tagchar = 3,
};

#define tag(x) ((cell(x)) & (cellsize-1))
#define istag(x, t) (exp(==, tag(x), t))
#define settag(x, t) (exp(|, x, t))
#define isobj(x) (istag(x, tagobj))
#define istagobj(x) (tag(x))

#define tagsetpair(x) (settag(x, tagpair))
#define ispair(x) (istag(x, tagpair))
#define car(x) (obj(fetch(exp(-, (x), tagpair))))
#define cdr(x) (obj(fetch(exp(+, (x), cellsize-tagpair))))
#define setcar(x, y) (store(exp(-, (x), tagpair), (y)))
#define setcdr(x, y) (store(exp(+, (x), cellsize-tagpair), (y)))
#define isnum(x) (istag(x, tagnum))
#define newnum(x) (obj(settag(cell(x) << 2, tagnum)))
#define refnum(x) (cell(x) >> 2)
#define oexp(o, x, y) (newnum(exp(o, refnum(x), refnum(y))))
#define ischar(x) (istag(x, tagchar))
#define newchar(x) (obj(settag(cell(x) << 2, tagchar)))
#define refchar(x) (cell(x) >> 2)
#define trueobj constobj(t)
#define falseobj constobj(f)
#define newbool(x) ((x) ? trueobj : falseobj)
#define notbool(x) (newbool(eq(x, falseobj)))
#define isbool(x) (eq(x, trueobj) && eq(x, falseobj))
#define istrue(x) (eq(x, trueobj))
#define isfalse(x) (eq(x, falseobj))
#define refstr(x) (((String *)x)->body)
#define type(x) (fetch(x))
#define isnil(x) (eq(x, nil))
#define isatom(x) (!ispair(x))
#define istype(x, t) (!isnil(x) && isobj(x) && eq(type(x), typeobj(t)))
#define issym(x) (istype(x, Symbol))
#define symstr(x) (&((Symbol *)x)->string)
#define isblock(x) (istype(x, Block))
#define isprimitive(x) (isblock(x) && (((Block *)x)->body[1] == vmc(END)))
#define ismacro(x) (istype(x, Macro))
#define isstr(x) (istype(x, String))
#define isdict(x) (istype(x, Dict))
#define isport(x) (istype(x, Port))
#define isval(x) (istype(x, Value))
#define isvector(x) (istype(x, Vector))
#define isinport(x) (istype(x, InputStringPort) || istype(x, InputPort))
#define isoutport(x) (istype(x, OutputStringPort) || istype(x, OutputPort))
/* #define isstrinport(x) (istype(x, InputStringPort)) */
#define isstroutport(x) (istype(x, OutputStringPort))
#define iseof(x) (eq(constobj(eof), (x)))
#define isundefined(x) (eq(constobj(undefined), (x)))
#define islit(x) (!(ispair(x) || issym(x)))

typedef struct Type {
  Cell size;
  const char *name;
  void (*write)(Obj port, Obj obj);
  void (*display)(Obj port, Obj obj);
} Type;

typedef struct Constant {
  Type *type;
  const char *name;
} Constant;

typedef struct Pair {
  Obj car;
  Obj cdr;
} Pair;

typedef struct String {
  Type *type;
  char body[];
} String;

typedef struct Symbol {
  Type *type;
  String string;
} Symbol;

typedef struct Vector {
  Type *type;
  Cell size;
  Obj body[];
} Vector;

typedef struct HTNode {
  String *key;
  Obj val;
  struct HTNode *prev;
} HTNode;

typedef struct Dict {
  Type *type;
  Cell size;
  struct HTNode **nodes;
} Dict;

typedef struct Block {
  Type *type;
  Cell body[];
} Block;

typedef struct Macro {
  Type *type;
  Block *block;
} Macro;

typedef struct Value {
  Type *type;
  Obj body;
} Value;

/* Port */

typedef struct PortFile {
  FILE *fp;
  const char *path;
} PortFile;

typedef struct PortMem {
  void *start;
  void *end;
  void *here;
} PortMem;

typedef union PortX {
  PortFile file;
  PortMem mem;
} PortX;

typedef struct Port {
  Type *type;
  PortX x;
  Cell line;
} Port;

/* Type instance */

#define typeobj(x) (&typetable.x)

/* Image */

typedef struct Stack {
  Cell *base;
  Cell *top;
  Cell *over;
} Stack;

typedef struct Image {
  Cell size;
  Cell *here;
  Cell *body;
} Image;

typedef struct Space {
  Cell size;
  void *from;
  void *to;
  void *free;
} Space;

typedef struct VM {
  Stack *stack;
  Stack *rstack;
  Stack *lstack;
  void **ip;
  Port *stdin;
  Port *stdout;
  Port *stderr;
  Port *buf;
} VM;

/* Etc */

typedef struct ReadContext {
  Cell type;
  Cell line;
  const char *path;
} ReadContext;

typedef struct Env {
  struct Env *prev;
  Cell size;
  Obj body[];
} Env;

typedef struct LexicalData {
  unsigned short deep;
  unsigned short id;
} LexicalData;

typedef struct LexicalVar {
  Symbol *var;
  LexicalData data;
} LexicalVar;

typedef struct CompileContext {
  Block *block;
  Pair *src;
  Cell in_block_flag;
  struct {
    Cell here;
    LexicalVar vars[20];
    Cell level;
    Cell deep;
    Cell count;
    Cell block_flag;
    Cell use;
  } lexical;
  Cell if_block;
  Stack if_stack;
} CompileContext;

#endif
