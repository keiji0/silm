/* Image */

Image image;
static void setup_image(void *memory, Cell size){
  assert(memory && size);
  clear(&image, sizeof(Image));
  image.body = memory;
  image.here = image.body;
  image.size = size/2;
  setup_space(memory+(size/2), size/2);
}

#define comp(x) _comp(cell(x))
static void _comp(Cell x){
  *image.here++ = x;
}

Port *port_stdin = nil;
Port *port_stdout = nil;
Port *port_stderr = nil;
Cell reader_line = 0;

/* Space */

Space space;
static void setup_space(void *memory, Cell size){
  assert(memory && size);
  space.size = size;
  space.free = space.from = memory;
  space.to = (void *)exp(/, memory, 2);
}

/* static Obj allot(Cell size){ */
/*   assert(size); */
/*   Obj p = space.free; */
/*   space.free += size; */
/*   return p; */
/* } */

static Obj allot(Cell size){
  /* return GC_MALLOC(size); */
  return malloc(size);
}

/* Error */

#define err(...) do { safe_err(__VA_ARGS__); exit(1); } while (0)
#define cerr(n) err("c error: %s\n", strerror(n))

static void safe_err(const char *m, ...){
  va_list args;
  va_start(args, m);
  vfprintf(stderr, m, args);
  va_end(args);
  fflush(stderr);
}

/* Type */

#define Constantdisplay Constantwrite
#define Dictdisplay Dictwrite
#define Blockdisplay Blockwrite
#define Macrodisplay Macrowrite
#define Valuedisplay Valuewrite
#define Vectordisplay Vectorwrite
#define Nildisplay Nilwrite
#define Numberdisplay Numberwrite
#define Portdisplay Portwrite
#define Symboldisplay Symbolwrite

struct TypeTable {
  Type Pair;
  Type Symbol;
  Type Dict;
  Type String;
  Type Block;
  Type Macro;
  Type Constant;
  Type Value;
  Type Vector;
  Type Port;
  Type InputPort;
  Type OutputPort;
  Type InputStringPort;
  Type OutputStringPort;
  Type Number;
  Type Char;
  Type Nil;
} typetable = {
  #define X(n) { sizeof(n), #n, n##write, n##display },
  #define XX(n) { 0, #n, n##write, n##display },
  X(Pair)
  X(Symbol)
  X(Dict)
  X(String)
  X(Block)
  X(Macro)
  X(Constant)
  X(Value)
  X(Vector)
  X(Port)
  { sizeof(Port), "Input Port", Portwrite, Portwrite },
  { sizeof(Port), "Output Port", Portwrite, Portwrite },
  { sizeof(Port), "Input String Port", Portwrite, Portwrite },
  { sizeof(Port), "Output String Port", Portwrite, Portwrite },
  XX(Number)
  XX(Char)
  XX(Nil)
  #undef X
  #undef XX
};

static Type *gettype(Obj x){
  if (isnil(x)) return typeobj(Nil);
  if (ispair(x)) return typeobj(Pair);
  if (ischar(x)) return typeobj(Char);
  if (isnum(x)) return typeobj(Number);
  if (isobj(x)) return (Type *)type(x);
  err("unkown type %x", x);
  return nil;
}

/* Nil */

static void Nilwrite(Obj p, Obj x){
  port_write_string(p, "()");
}

/* Constant */

#define constobj(x) ((&_constant_sruct.x))
struct ConstantStruct {
  Constant eof;
  Constant undefined;
  Constant t;
  Constant f;
} _constant_sruct = {
  { typeobj(Constant), "#eof" },
  { typeobj(Constant), "#undefined" },
  { typeobj(Constant), "#t" },
  { typeobj(Constant), "#f" },
};

static void Constantwrite(Obj p, Obj x){
  port_write_string(p, ((Constant *)x)->name);
}

/* Value */

static void Valuewrite(Obj p, Obj x){
  port_write_string(p, "#<value ");
  writesexp(p, ((Value *)x)->body);
  port_write_string(p, ">");
}

/* Vector */

static Vector *newvector(Cell l){
  Vector *v = allot(sizeof(Vector) + (cellsize*l));
  v->type = typeobj(Vector);
  v->size = l;
  while (l--) v->body[l] = constobj(undefined);
  return v;
}

static void Vectorwrite(Obj p, Obj v){
  port_write_format(p, "#<vector %d>", ((Vector *)v)->size);
}

/* VM */

VM *vm;
Cell *vmtable = nil;
const char *vmname_table[] = {
  #define X(_, name, __) name,
  #include "tmp/vmdata.h"
  #undef X
  nil
};

#define vmcgensym(n) __vmcode__##n
#define vmc(n) vmtable[vmcgensym(n)]
enum VMCode {
  #define X(c, _, n) vmcgensym(n) = c,
  #include "tmp/vmdata.h"
  #undef X
};

struct PrimitiveCode {
  const char *name;
  Cell code;
} primitive[] = {
  #define X(id, name, _) { name, id },
  #include "tmp/vmdata.h"
  #undef X
};

/* static Cell vmcode(const char *name){ */
/*   assert(name); */
/*   int i; */
/*   for (i = 0; vmtable[i]; i++) */
/*     if (eqs(vmname_table[i], name)) */
/*       return vmtable[i]; */
/*   return 0; */
/* } */

static const char *vmcode_name(Cell code){
  assert(code);
  int i;
  for (i = 0; vmtable[i]; i++)
    if (eq(code, vmtable[i]))
      return vmname_table[i];
  return nil;
}

static void setup_vm(){
  vm = allot(sizeof(VM));
  clear(vm, sizeof(VM));
  vm->stack = newstack(1024);
  vm->rstack = newstack(1024);
  vm->lstack = newstack(1024);
  vm->buf = portout_memory(1024);
  vmtable = (Cell *)exec(nil);
  int i = 0, max = sizeof(primitive) / sizeof(struct PrimitiveCode);
  for (i = 0; i < max; i++) {
    Block *b = newblock();
    comp(vmtable[i]);
    comp(vmc(END));
    def(intern(primitive[i].name), b);
  }
}

/* Object */

#define newobj(type) _nobj(typeobj(type))
#define align() space.free = (void *)alignment(space.free)

static Obj _nobj(Type *type){
  assert(type);
  void **obj = allot(type->size);
  *obj = type;
  return obj;
}

/* Pair */

static Obj cons(Obj x, Obj y){
  Pair *p = allot(sizeof(Pair));
  p->car = x;
  p->cdr = y;
  return obj(tagsetpair(p));
}

static Obj equal(Obj x, Obj y){
  if (eq(x, y)) return obj(1);
  else if (ispair(x)) {
    if (!ispair(y)) return 0;
    do {
      if (!equal(car(x), car(y))) return 0;
      x = cdr(x);
      y = cdr(y);
    } while (ispair(x) && ispair(y));
    return equal(x, y);
  } else if (isnum(x)) {
    return obj(eq(x, y));
  } else if (ischar(x)) {
    return obj(eq(x, y));
  } else if (isstr(x) && isstr(y)) {
    return obj(eqs(((String *)x)->body, ((String *)y)->body));
  } else
    return 0;
}

static void Pairwrite(Obj p, Obj x){
  Pair *pair = x;
  port_write_char(p, '(');
  while (pair) {
    if (ispair(pair)) {
      writesexp(p, car(pair));
      if (cdr(pair)) putchar(' ');
      pair = (Pair *)cdr(pair);
    } else {
      port_write_string(p, ". ");
      writesexp(p, pair);
      goto end;
    }
  }
 end:
  port_write_char(p, ')');
}

static void Pairdisplay(Obj p, Obj x){
  Pair *pair = x;
  putchar('(');
  while (pair) {
    if (ispair(pair)) {
      displaysexp(p, car(pair));
      if (cdr(pair)) putchar(' ');
      pair = (Pair *)cdr(pair);
    } else {
      P(". ");
      displaysexp(p, pair);
      goto end;
    }
  }
 end:
  putchar(')');
}

/* Stack */

#define _tos(s) (*((s)->top))
#define _tosset(s, x) _tos(s) = ((Cell)x)
#define _push(s, x) *(++(s)->top) = ((Cell)x)
#define _pop(s) (*((s)->top--))
#define _drop(s) ((s)->top--)
#define _stack_index(s, x) ((s)->top[-(x)])
#define _stack_empty(s) (((s)->top+1) == (s)->base)
#define _stack_underflow(s) ((s)->top < (s)->base)
#define _stack_overflow(s) ((s)->top >= (s)->over)
#define _stack_topp(s) &(s)->base[-1];
#define _stack_size(s) (((s)->over - (s)->base)+1)
#define _stack_used(s) ((((s)->top - (s)->base)+1))
#define _stack_rest(s) (((s)->over - (s)->top))
#define _stack_clear(s) ((s)->top = (s)->base)

static Stack *stack_init(Stack *st, void *mem, Cell size){
  assert(st);
  st->base = mem;
  st->top = _stack_topp(st);
  st->over = addr(exp(+, st->base, size));
  return st;
}

static Stack *newstack(Cell size){
  assert(0 < size);
  return stack_init(allot(sizeof(Stack)), allot(size), size);
}

/* Node */

static Dict *make_dict(Cell size){
  assert(size);
  Dict *hash = newobj(Dict);
  hash->size = size;
  hash->nodes = allot(sizeof(HTNode *) * size);
  clear(hash->nodes, sizeof(HTNode *) * size);
  return hash;
}

static void dict_keys(Dict *d){
  assert(d);
  Cell i = d->size;
  HTNode *node;
  while (i--)
    if ((node = d->nodes[i]))
      for (; node; node = node->prev)
        _push(vm->stack, node->key);
}

static uCell strhash(Dict *d, const char *p){
  assert(d && p);
  const char *key = p;
  uCell c;
  uCell hv = 0;
  while ((c = *key++)) hv = (hv << 5) - hv + c;
  return hv % d->size;
}

static HTNode *newhashnode(String *key, Obj val, HTNode *prev){
  assert(key);
  HTNode *node = allot(sizeof(HTNode));
  node->key = key;
  node->val = val;
  node->prev = prev;
  return node;
}

static HTNode *findnode(HTNode *node, const char *key){
  assert(node && key);
  while (node)
    if (eqs(key, node->key->body))
      return node;
    else
      node = node->prev;
  return nil;
}

static void puthash(Dict *hash, String *key, Obj val){
  assert(hash && key);
  uCell index = strhash(hash, key->body);
  HTNode *node, *nn;
  if ((node = hash->nodes[index])) {
    if ((nn = findnode(node, key->body)))
      nn->val = val;
    else
      hash->nodes[index] = newhashnode(key, val, node);
  } else
    hash->nodes[index] = newhashnode(key, val, nil);
}

static Obj refhash(Dict *hash, const char *key){
  assert(hash);
  uCell index = strhash(hash, key);
  HTNode *node;
  if ((node = hash->nodes[index])) {
    if ((node = findnode(node, key)))
      return node->val;
    return constobj(undefined);
  } else
    return constobj(undefined);
}

static void Dictwrite(Obj p, Obj x){
  port_write_format(p, "<dict %x>", (int)x);
}

/* Symbol */

Dict *symbol_hash = nil;
#define SYMBOL_HASH_SIZE 97

static void setup_symbol(){
  symbol_hash = make_dict(SYMBOL_HASH_SIZE);
}

static Symbol *intern(const char *str){
  assert(str);
  Symbol *symbol = (Symbol *)refhash(symbol_hash, str);
  if (!isundefined(symbol)) return symbol;
  int len = strlen(str)+1;
  symbol = allot(sizeof(Symbol)+len);
  symbol->type = typeobj(Symbol);
  symbol->string.type = typeobj(String);
  memcpy(symbol->string.body, str, len);
  puthash(symbol_hash, &symbol->string, symbol);
  return symbol;
}

static void Symbolwrite(Obj p, Obj x){
  port_write_string(p, (((Symbol *)x)->string.body));
}

/* Number */

static void Numberwrite(Obj p, Obj x){
  port_write_format(p, "%d", (int)refnum(x));
}

/* Char */

static void Charwrite(Obj p, Obj x){
  port_write_format(p, "#\\%c", (int)refchar(x));
}

static void Chardisplay(Obj p, Obj x){
  port_write_char(p, (int)refchar(x));
}

/* String */

static String *newstring(const char *str){
  assert(str);
  Cell len = strlen(str)+1;
  String *s = allot(sizeof(String)+len);
  align();
  s->type = typeobj(String);
  memcpy(s->body, str, len);
  return s;
}

static String *ss_string(Obj x){
  assert(x);
  if (isstr(x)) return x;
  if (issym(x)) return &((Symbol *)x)->string;
  err("String or Symbol");
}

static void Stringwrite(Obj p, Obj x){
  port_write_char(p, '"');
  char *s = ((String *)x)->body;
  do {
    if (*s == '"') port_write_char(p, '\\');
    port_write_char(p, *s);
  } while (*++s);
  port_write_char(p, '"');
}

static void Stringdisplay(Obj p, Obj x){
  port_write_string(p, ((String *)x)->body);
}

/* Block & Macro & Closure */

static Block *newblock(){
  Block *b = (Block *)image.here;
  comp(typeobj(Block));
  return b;
}

static Macro *newmacro(Block *block){
  Macro *m = (Macro *)image.here;
  comp(typeobj(Macro));
  comp(block);
  return m;
}

static Block *newdblock(Block *block, Obj d){
  assert(block);
  Block *b = allot(sizeof(Block) + cellsize*3);
  b->type = typeobj(Block);
  b->body[0] = vmc(DODCOMPOSE);
  b->body[1] = (Cell)block;
  b->body[2] = (Cell)d;
  return b;
}

static void Blockwrite(Obj p, Obj o){
  port_write_format(p, "#<%s %x>", (((Block *)o)->body[0] == vmc(CLOSURE) ? "closure" : "block"), (int)o);
}

static void Macrowrite(Obj p, Obj o){
  port_write_format(p, "#<macro %x>", (int)o);
}

/* Dict */

Pair *context = nil;

void setup_dict(){
  context = cons(make_dict(19), nil);
}

static void def(Symbol *sym, Obj obj){
  assert(sym);
  puthash((Dict *)car(context), &sym->string, obj);
}

static Obj safe_lookup(Pair *context, String *str){
  Obj r;
  Pair *pair = context;
  each (pair)
    if (!isundefined(r = refhash((Dict *)car(pair), str->body)))
      return r;
  return constobj(undefined);
}

static Obj lookup(Symbol *sym){
  Obj o = safe_lookup(context, symstr(sym));
  if (isundefined(o))
    err("unbound variable: '%s'", sym->string.body);
  return o;
}

/* Port */

static void Portwrite(Obj p, Obj o){
  port_write_format(p, "#<%s>", ((Port *)o)->type->name);
}

#define mem_extend_size 2
#define mem_default_size 1024
#define mem_size(m) ((m)->x.mem.end - (m)->x.mem.start)
#define mem_used(m) ((m)->x.mem.here - (m)->x.mem.start)
#define mem_rest(m) ((m)->x.mem.end - (m)->x.mem.here)
#define mem_overp(m) ((m)->x.mem.here >= (m)->x.mem.end)
#define mem_clear(m) ((m)->x.mem.here = (m)->x.mem.start)
#define port_new(ty) port_init(malloc(sizeof(Port)), ty)
#define port_istype(p, t) ((p)->type == typeobj(t))

static void setup_port(){
  vm->stdin = port_stdin = port_new(typeobj(InputPort));
  port_stdin->x.file.fp = stdin;
  vm->stdout = port_stdout = port_new(typeobj(OutputPort));
  port_stdout->x.file.fp = stdout;
  vm->stderr = port_stderr = port_new(typeobj(OutputPort));
  port_stderr->x.file.fp = stderr;
}

static Port *port_init(Port *port, Type *type){
  assert(port && type);
  clear(port, sizeof(Port));
  port->type = type;
  return port;
}

#define port_memory_init2(port)\
  port_memory_init(port, port->x.mem.start, mem_size(port))

static Port *port_memory_init(Port *port, void *mem, Cell size){
  assert(port && mem && size);
  port->x.mem.start = mem;
  port->x.mem.here = mem;
  port->x.mem.end = mem + size;
  return port;
}

static Cell port_memory_extend(Port *port){
  assert(port);
  Cell i = mem_used(port);
  Cell ns = mem_size(port) * mem_extend_size;
  port->x.mem.start = realloc(port->x.mem.start, ns);
  port->x.mem.here = port->x.mem.start + i;
  port->x.mem.end = port->x.mem.start + ns;
  return ns;
}

/* static Port *portin_memory(void *mem, Cell size){ */
/*   assert(mem && size); */
/*   return port_memory_init(port_new(typeobj(InputPort)), mem, size); */
/* } */

static Port *portout_memory(Cell size){
  assert(size);
  return port_memory_init(port_new(typeobj(OutputStringPort)), allot(size), size);
}

static Port *inport_open(const char *path){
  assert(path);
  Port *port = port_new(typeobj(InputPort));
  port->x.file.path = strdup(path);
  if ((port->x.file.fp = fopen(path, "r")) == NULL)
    cerr(errno);
  return port;
}

static Port *outport_open(const char *path){
  assert(path);
  Port *port = port_new(typeobj(OutputPort));
  port->x.file.path = strdup(path);
  if ((port->x.file.fp = fopen(path, "w")) == NULL)
    cerr(errno);
  return port;
}

static void port_close(Port *port){
  assert(port);
  if (port_istype(port, InputStringPort)) {
    ;
  } else if (port_istype(port, OutputStringPort))
    free(port->x.mem.start);
  else if (port_istype(port, InputPort) ||
           port_istype(port, OutputPort)) {
    if (fclose(port->x.file.fp) == EOF)
      cerr(errno);
  } else
    err("port_close: undefined port");
  free(port);
}

static Cell port_seek(Port *port, Cell whence, Cell offset){
  assert(port);
  if (port_istype(port, InputStringPort) ||
      port_istype(port, OutputStringPort)) {
    switch (whence) {
    case SEEK_SET: port->x.mem.here = port->x.mem.start + offset; break;
    case SEEK_CUR: port->x.mem.here += offset; break;
    case SEEK_END: port->x.mem.here = port->x.mem.end + offset; break;
    }
    return port->x.mem.here - port->x.mem.start;
  } else if (port_istype(port, InputPort) ||
             port_istype(port, OutputPort)) {
    Cell i = fseek(port->x.file.fp, offset, whence);
    if (i < 0) cerr(errno);
    return i;
  } else
    err("port_seek: undefined port");
  return 0;
}

static Cell port_read(Port *port, void *ptr, Cell size){
  assert(port && ptr && size);
  if (port_istype(port, InputStringPort)) {
    if (size > mem_rest(port)) return 0;
    memcpy(ptr, port->x.mem.here, size);
    port->x.mem.here += size;
    return size;
  } else if (port_istype(port, InputPort)) {
    Cell i = fread(ptr, size, 1, port->x.file.fp);
    if (ferror(port->x.file.fp)) cerr(errno);
    Cell c;
    for (c = 0; c < i; c++) {
      if (((char *)ptr)[c] == '\n') port->line++;
    }
    return i;
  } else
    err("port_read: undefined port");
  return 0;
}

static Cell port_read_char(Port *port){
  assert(port);
  char c;
  if (port_read(port, &c, sizeof(char)) == 0)
    return -1;
  return c;
}

/* static Cell port_read_line(Port *port, Cell size, char *dst){ */
/*   assert(port && dst && size); */
/*   switch (port->type) { */
/*   case inmem_t: { */
/*     Cell n = size; */
/*     char c; */
/*     lp: */
/*     if (port_read(port, &c, sizeof(char))) { */
/*       *dst++ = c; */
/*       if (n-- < 1) goto end; */
/*       if (c == '\n') goto end; */
/*       goto lp; */
/*     } */
/*     end: */
/*     *dst = '\0'; */
/*     return size - n; */
/*   } */
/*   case infile_t: { */
/*     if (fgets(dst, size, port->x.file.fp)) */
/*       return strlen(dst); */
/*     cerror(errno); */
/*   } */
/*   default: */
/*     err("port_read_line: undefined port"); */
/*   } */
/*   return 0; */
/* } */

static Cell port_peek_char(Port *port){
  assert(port);
  Cell n = port_read_char(port);
  port_seek(port, SEEK_CUR, -sizeof(char));
  return n;
}

static void port_flush(Port *port){
  assert(port);
  if (port_istype(port, OutputStringPort)) {
    ;
  } else if (port_istype(port, OutputPort)) {
    fflush(port->x.file.fp);
  } else
    err("port_flush: undefined port");
}

static Cell port_write(Port *port, const void *ptr, Cell size){
  assert(port && ptr && size);
  if (port_istype(port, OutputStringPort)) {
    if (mem_rest(port) < size)
      port_memory_extend(port);
    memcpy(port->x.mem.here, ptr, size);
    port->x.mem.here += size;
    return size;
  } else if (port_istype(port, OutputPort)) {
    Cell n = fwrite(ptr, 1, size, port->x.file.fp);
    if (ferror(port->x.file.fp)) cerr(errno);
    return n;
  } else
    err("port_write: undefined port");
  return 0;
}

static void port_write_char(Port *port, Cell c){
  assert(port);
  port_write(port, &c, 1);
}

static void port_write_string(Port *port, const char *str){
  assert(port && str);
  port_write(port, str, strlen(str));
}

static void port_write_format(Port *port, const char *format, ...){
  assert(port && format);
  va_list args;
  va_start(args, format);
  char buf[1024];
  Cell l = vsnprintf(buf, 1024, format, args);
  va_end(args);
  port_write(port, buf, l);
}

static String *port_to_string(Port *port){
  assert(port);
  *(char *)port->x.mem.here = '\0';
  return newstring(port->x.mem.start);
}

/* Reader */

const char *syntax_ws = " \n\t\0";
const char *syntax_delimiter = "()[]'`,|";
#define SYNTAX_WSP(x) strchr(syntax_ws, x)
#define SYNTAX_DELIMITERP(x) strchr(syntax_delimiter, x)

static Cell skipws(Port *port, ReadContext *context){
  assert(port);
  Cell c;
  while ((c = port_peek_char(port))) {
    if (c == -1) return -1;
    else if (!SYNTAX_WSP(c)) return 1;
    else port_read_char(port);
  }
  return 1;
}

static Cell parse(Port *port, char *buf, Cell size, ReadContext *context){
  assert(buf && size);
  char *p = buf;
  Cell c = port_read_char(port);
  if (c == -1) return -1;
  if (SYNTAX_DELIMITERP(c)) {
    *buf++ = c;
    *buf = '\0';
    return 1;
  }
  do {
    if (SYNTAX_WSP(c)) {
      *p = '\0';
      return 1;
    } else if (SYNTAX_DELIMITERP(c)) {
      ungetc(c, port->x.file.fp);
      *p = '\0';
      return 1;
    }
    *p++ = c;
  } while ((c = port_read_char(port)) || size--);
  return 0;
}

static Cell isstrx(int (*pred)(int), const char *s){
  int c;
  while ((c = *s++)) if (!pred(c)) return 0;
  return 1;
}

enum ReadContextType {
  context_type_neutral,
  context_type_dot,
  context_type_list,
  context_type_end_list,
  context_type_nil,
};

static Obj readsexp(Port *port, ReadContext *context){
  assert(port);
  if (context) {
    context->type = context_type_neutral;
    return readsexp_inner(port, context);
  } else {
    ReadContext c;
    clear(&c, sizeof(ReadContext));
    c.type = context_type_neutral;
    return readsexp_inner(port, &c);
  }
}

#define reader_err(port, message, ...)\
  err("%s:%d: reader error: " message, port->x.file.path, port->line, __VA_ARGS__)

static Obj readsexp_inner(Port *port, ReadContext *context){
  if (-1 == skipws(port, context)) return constobj(eof);
  Cell c = port_read_char(port);
  switch ((c)) {
  case -1:
    return constobj(eof);
  case '(': { /* List */
    ReadContext context_save = *context;
    skipws(port, context);
    if (')' == port_peek_char(port)) {
      port_read_char(port);
      return nil;
    }
    context->type = context_type_list;
    Obj obj = readsexp_inner(port, context);
    if (iseof(obj)) reader_err(port, "リストの始めでEOFが発生しました", "");
    Pair *p = cons(obj, nil);
    Pair *last = p;
    lp: {
      context->type = context_type_list;
      obj = readsexp_inner(port, context);
      if (iseof(obj)) reader_err(port, "リストの途中でEOFが発生しました", "");
      setcdr(last, obj);
      if (context->type == context_type_dot) { /* Dot Pair */
        context->type = context_type_neutral;
        return p;
      }
      if (context->type != context_type_end_list) {
        setcdr(last, cons(cdr(last), nil));
        last = cdr(last);
        goto lp;
      }
    }
    *context = context_save;
    return p;
  }

  case ')': /* End List */
    context->type = context_type_end_list;
    return nil;

  case '.': { /* Dot Pair */
    if (!SYNTAX_WSP(port_peek_char(port))) goto symbol;
    Obj obj = readsexp_inner(port, context);
    context->type = context_type_dot;
    if (iseof(obj)) reader_err(port, "「.」の後にS式が存在しません", "");
    Obj obj2 = readsexp_inner(port, context);
    if (!isnil(obj2)) reader_err(port, "「.」はS式の最後しか使用出来ません", "");
    return obj;
  }

  case '"': {
    char buf[2024], *p = buf;
    while ((c = port_read_char(port)))
      if (c == '"') {
        *p = '\0';
        return newstring(buf);
      } else if (c == '\\') {
        c = port_read_char(port);
        if (c == '"')
          *p++ = '"';
        else {
          *p++ = '\\';
          *p++ = c;
        }
      } else
        *p++ = c;
  }

  case '|':
    return intern("|");

  case '\'':
    return intern("'");

  case '`':
    return intern("`");

  case '[': {
    Cell n = 1;
    while ((c = port_read_char(port))) {
      if (c == -1) return constobj(eof);
      else if (c == '[') n++;
      else if (c == ']') n--;
      if (n == 0) return readsexp(port, context);
    }
  }

  case ';': {
    while (port_read_char(port) != '\n');
    return readsexp(port, context);
  }

  case '#':
    c = port_read_char(port);
    switch (c) {
    case '\\': {
      char buf[20];
      if (parse(port, buf, 20, context) == -1) reader_err(port, "文字定数が指定されていません", "");
      if (eqs("space", buf)) return newchar(' ');
      else if (eqs("cr", buf)) return newchar('\n');
      else if (eqs("tab", buf)) return newchar('\t');
      else if (eqs("\\", buf)) return newchar('\\');
      else if (eqs("null", buf)) return newchar('\0');
      else return newchar(buf[0]);
    }
    default:
      if (c == 't') return trueobj;
      if (c == 'f') return falseobj;
      reader_err(port, "未定義のスペシャルキャラクタが指定されています", "");
    }

  symbol:
  default: {
    char buf[255];
    buf[0] = c;
    char cc = port_peek_char(port);
    if ((SYNTAX_WSP(cc) || SYNTAX_DELIMITERP(cc)))
      buf[1] = '\0';
    else
      parse(port, buf+1, 254, context);
    if (isstrx(isdigit, buf) || (buf[0] == '-' && isdigit(buf[1])))
      return newnum(strtol(buf, nil, 10));
    else {
      return intern(buf);
    }
  }
  }
}

static void writesexp(Obj p, Obj o){
  Type *t = gettype(o);
  t->write(p, o);
}

static void displaysexp(Obj p, Obj o){
  Type *t = gettype(o);
  t->display(p, o);
}

/* Eval */

static Obj eval(Obj sexp){
  if (isnil(sexp))
    return sexp;
  else if (ispair(sexp))
    return compile(sexp);
  else if (issym(sexp)) {
    if ((sexp = lookup(sexp))) {
      if (isval(sexp))
        return ((Value *)sexp)->body;
      else
        return sexp;
    }
  } else if (islit(sexp))
    return sexp;
  else
    err("compile error: unknown value %x", sexp);
  return nil;
}

/* Complie */

static Pair *macroexpand(Pair *pair){
  Pair *def = pair;
  Cell first = 1;
  Pair *prev = nil;
  for (; pair; pair = cdr(pair)) {
    if (issym(car(pair))) {
      Macro *macro = safe_lookup(context, symstr(car(pair)));
      if (macro && ismacro(macro)) {
        _push(vm->stack, cdr(pair));
        Obj o = exec(macro->block->body);
        if (first)
          return macroexpand(o);
        else {
          setcdr(prev, o);
          pair = o;
        }
      }
    } else if (ispair(car(pair)))
      setcar(pair, macroexpand(car(pair)));
    prev = pair;
    first = 0;
  }
  return def;
}

static Obj item_read(CompileContext *c){
  if (isnil(c->src)) return obj(-1);
  if (!ispair(c->src)) err("compile error: Dot Pairはコンパイル出来ません");
  Obj o = car(c->src);
  c->src = cdr(c->src);
  return o;
}

static Cell syntax_lexical_count(CompileContext *c){
  Pair *pair = c->src;
  if (!ispair(pair)) return 0;
  Obj cdel = intern("|");
  Obj lqt = intern("'");
  Cell i = 0;
  each (pair)
    if (eq(car(pair), lqt))
      pair = cdr(pair);
    else if (eq(car(pair), cdel)) {
      Cell ii;
      c->lexical.level = c->lexical.here;
      for (ii = 0; ii < i; ii++) {
        c->lexical.vars[c->lexical.here].var = item_read(c);
        c->lexical.vars[c->lexical.here].data.deep = c->lexical.deep;
        c->lexical.vars[c->lexical.here].data.id = ii;
        c->lexical.here++;
      }
      c->lexical.deep++;
      item_read(c); /* drop | */
      return i;
    }
    else if (issym(car(pair))) {
      i++;
    }
    else return 0;
  return 0;
 }

static LexicalVar *findlexical(CompileContext *c, Symbol *var){
  Cell i;
  for (i = c->lexical.here-1; i >= 0; i--)
    if (c->lexical.vars[i].var == var)
      return &c->lexical.vars[i];
   return nil;
}

static Cell syntax_lexical_use(CompileContext *c, Pair *pair){
  Cell f = 0;
  each (pair)
    if (issym(car(pair)) && findlexical(c, car(pair)))
      return 1;
    else if (ispair(car(pair)))
      f = syntax_lexical_use(c, car(pair));
  return f;
}

static void compile_block_start(CompileContext *context){
  Cell c;
  context->lexical.block_flag = 0;
  if ((c = syntax_lexical_count(context))) {
    context->lexical.block_flag = 1;
    comp(vmc(SAVE_ENV));
    comp(c);
  }
}

static void compile_block_end(CompileContext *c){
  if (c->lexical.block_flag) {
    if (c->lexical.use)
      comp(vmc(CLOSUREEND));
    else
      comp(vmc(LEXBLOCKEND));
  } else
    comp(vmc(END));
}

static void compile_block(CompileContext *context, Obj o){
  if (ispair(o)) {
    CompileContext context_save = *context;
    context->src = o;
    context->lexical.use = syntax_lexical_use(context, o);
    if (context->lexical.use)
      comp(vmc(LEXBLOCK));
    else
      comp(vmc(BLOCK));
    Cell *p = image.here;
    comp(0);
    comp(typeobj(Block));
    compile_block_start(context);
    while ((o = item_read(context)) != obj(-1))
      compile_inner(context, o);
    compile_block_end(context);
    store(p, image.here);
    *context = context_save;
  } else 
    compile_inner(context, o);
}

#define compile_err(message, ...)\
  err("%s:%d: syntax error: " message "\n", vm->stdin->x.file.path, vm->stdin->line, __VA_ARGS__)

static void compile_inner(CompileContext *contx, Obj o){
   if (ispair(o)) {
     CompileContext context_save = *contx;
     {
       contx->src = o;
       if (contx->in_block_flag) {
         compile_block(contx, o);
       } else {
         contx->in_block_flag = 1;
         compile_block_start(contx);
         while ((o = item_read(contx)) != obj(-1))
           compile_block(contx, o);
         compile_block_end(contx);
       }
     }
     *contx = context_save;
   } else if (issym(o)) {
     LexicalVar *lvar;
     Obj o2;
     if ((lvar = findlexical(contx, o))) {
       comp(vmc(LREF));
       LexicalData xx = { contx->lexical.deep - lvar->data.deep - 1, lvar->data.id };
       comp(fetch(&xx));
     } else if (o == intern("'")) {
       comp(vmc(LIT));
       o = item_read(contx);
       if (o == obj(-1)) compile_err("「'」される式がありません", "");
       comp(o);
     } else if (o == intern("=>")) {
       o = item_read(contx);
       if (o == obj(-1) || !issym(o))
         compile_err("「=>」は割当てる変数を指定して下さい", "");
       else {
         if ((lvar = findlexical(contx, o))) {
           comp(vmc(LSET));
           LexicalData xx = { contx->lexical.deep - lvar->data.deep - 1, lvar->data.id };
           comp(fetch(&xx));
         } else {
           Obj v;
           if (!isundefined(v = safe_lookup(context, symstr(o)))) {
             if (isval(v)) {
               comp(vmc(LIT));
               comp(v);
               comp(vmc(VALSET));
             } else
               goto _err;
           } else {
           _err:
             compile_err("「%s」はレキシカル変数か変数を指定して下さい", ((Symbol *)o)->string.body);
           }
         }
       }
     } else if (o == intern("if")) {
       comp(vmc(0JMP));
       _push(&(contx->if_stack), image.here);
       contx->if_block = cell(image.here);
       comp(0);
     } else if (o == intern("else")) {
       comp(vmc(JMP));
       void *x = image.here;
       comp(0);
       store(_pop(&(contx->if_stack)), image.here);
       _push(&(contx->if_stack), x);
     } else if (o == intern("then")) {
       store(_pop(&(contx->if_stack)), image.here);
     } else if (o == intern("lp")) {
       comp(vmc(LP));
       goto lp_finish;
     } else if (o == intern("?lp")) {
       comp(vmc(QLP));
     lp_finish:
       comp(contx->lexical.block_flag ?
              cell(&contx->block->body[2]) :
              cell(contx->block->body));
     } else if (!isundefined(o2 = safe_lookup(context, symstr(o)))) {
       if (isprimitive(o2)) {
         comp(((Block *)o2)->body[0]);
       } else if (isblock(o2)) {
         comp(vmc(CALL));
         comp(((Block *)o2)->body);
       } else if (isval(o2)) {
         comp(vmc(LIT));
         comp(o2);
         comp(vmc(VALREF));
       } else {
         o = o2;
         goto lit;
       }
     } else
       compile_err("「%s」はバインドされていません", ((Symbol *)o)->string.body);
   } else {
   lit:
     comp(vmc(LIT));
     comp(o);
   }
}

static Block *compile(Pair *pair){
  assert(pair);
  CompileContext context;
  clear(&context, sizeof(CompileContext));
  Cell if_stack_body[10];
  stack_init(&context.if_stack, if_stack_body, cellsize*10);
  context.block = newblock();
  context.src = macroexpand(pair);
  compile_inner(&context, context.src);
  return context.block;
}

/* Interpreter */

void interpreter(){
  Obj sexp;
  ReadContext context;
  clear(&context, sizeof(ReadContext));
  while (!iseof(sexp = readsexp(vm->stdin, &context))) {
    //P("["); writesexp(vm->stdout, sexp); P("]\n");
    Obj obj = eval(sexp);
    if (!ispair(sexp) && isblock(obj)) {
      obj = exec(((Block *)obj)->body);
      _push(vm->stack, obj);
    } else {
      _push(vm->stack, obj);
    }
    port_flush(vm->stdout);
  }
}

/* Debug */

static void dump(Cell *a){
  Cell n = 32;
  int i, ii, iii;
  Cell *a2;
  for (i = 0; i < n; i++) {
    P("%08x: ", (int)a);
    a2 = a;
    for (ii = 0; ii < 4; ii++, i++) {
      if (a && *a && vmcode_name(*a)) {
        P("%12s ", vmcode_name(*a));
      } else {
        P("%12x ", (int)*a);
      }
      a++;
    }
    P(" ");
    for (iii = 0; iii < 16 && (n - iii) > 0; iii++) {
      const char *s = (const char *)(a2) + iii;
      P("%c", *s < 32 ? '.' : *s);
    }
    putchar('\n');
  }
  putchar('\n');
}

static void stack_dump(Stack *sp){
  assert(sp);
  printf("<%d> ", _stack_used(sp));
  int i;
  for (i = _stack_used(sp)-1; i > -1; i--) {
    writesexp(port_stdout, (Obj)sp->top[-i]);
    P(" ");
  }
  P("\n");
  fflush(stdout);
}

/* VM engine */

static void _typeerr(VM *vm, const char *name, const char *req, Obj val){
  safe_err("%s:%d: %s is %s required, but got ", vm->stdin->x.file.path, vm->stdin->line, name, req);
  writesexp(vm->stdout, val);
  putchar('\n');
  exit(1);
}

static void *exec(register Cell *ip){
  #define label_name(name) _##name
  #define label(name) _##name:
  static Cell *table[] = {
    #define X(code, _, __) &&label_name(code),
    #include "tmp/vmdata.h"
    #undef X
    nil
  };
  if (ip == nil) return (Cell *)table;
  #define r1s(x) r1 = ((Cell)(x))
  #define r2s(x) r2 = ((Cell)(x))
  #define r3s(x) r3 = ((Cell)(x))
  #define jump(x) goto *(void *)(x)
  #define ipset(x) ip = (Cell *)(x)
  #define _si(n) _stack_index(sp, n)
  #define _ri(n) _stack_index(rp, n)
  #define ret ipset(_pop(rp));
  #define dispatch goto **ip++;
  #define next goto **ip++
  #define r1o obj(r1)
  #define r2o obj(r2)
  #define r3o obj(r3)
  register Cell r1, r2; Cell r3;
  register Stack *sp = vm->stack;
  register Stack *rp = vm->rstack;
  register Stack *lp = vm->lstack;
  Cell return_code[] = { vmc(EXIT) };

  _push(rp, return_code);
  _push(lp, nil);
  
  dispatch {
    #include "tmp/vm.c"
  }
  return 0;
}
