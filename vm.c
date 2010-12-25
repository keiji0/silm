#define typeerr(req, val) _typeerr(vm, __VMCODE_NAME__, #req, obj(val))

#define typecheck(pred, req, val)\
  do {\
    if (!pred(val))\
      _typeerr(vm, __VMCODE_NAME__, #req, obj(val));\
  } while (0)

: nop = NOP ( > )

: call# = CALL ( xt: x > )
  r1s(*ip++);
  _push(rp, ip);
  ipset(r1);

: jmp# = JMP ( xt: xt > )
  ipset(*ip);

: 0jmp# = 0JMP ( n > & xt: xt > )
  if (isfalse(_pop(sp))) {
    ipset(*ip);
  } else
    ip++;

: lit# = LIT ( xt: o > o )
  _push(sp, *ip++);

: val# = VAL ( xt: o > o )
  _push(sp, fetch(*ip));
  ip++;

: block# = BLOCK ( xt: goto_address, body ... )
  _push(sp, ip+cellsize);
  ipset(fetch(ip));

: end# = END ( > )
  ret;

: exit = EXIT ( > )
  _drop(lp);
  return (void *)_pop(sp);

: bye ( n > )
  typecheck(isnum, Number, _tos(sp));
  exit(refnum(_pop(sp)));

: ret = RET ( > )
  ret;

#define QRET()\
  if (istrue(_pop(sp)))\
    ret;

: ?ret ( b > )
  QRET();

: lp = LP ( xt: addr | > )
  ipset(*ip);

: ?lp = QLP ( xt: addr | b > )
  if (isnil(_pop(sp))) {
    ip++;
  } else
    ipset(*ip);

: i ( block > ... )
  r1s(_pop(sp));
  typecheck(isblock, Bloc, r1);
  _push(rp, ip);
  ipset(((Block *)r1)->body);

/* Type */

: eq? ( a b > b )
  r1s(_pop(sp));
  _tosset(sp, newbool(eq(r1, _tos(sp))));

: equal? ( a b > b )
  r1s(_pop(sp));
  _tosset(sp, newbool(equal(r1o, (Obj)_tos(sp))));

: atom? ( o > b )
  _tosset(sp, newbool(isatom(_tos(sp))));

: pair? ( o > b )
  _tosset(sp, newbool(ispair(_tos(sp))));

: symbol? ( o > b )
  _tosset(sp, newbool(issym(_tos(sp))));

: number? ( o > b )
  _tosset(sp, newbool(isnum(_tos(sp))));

: char? ( o > b )
  _tosset(sp, newbool(ischar(_tos(sp))));

: string? ( o > b )
  _tosset(sp, newbool(isstr(_tos(sp))));

: port? ( o > b )
  _tosset(sp, newbool(isport(_tos(sp))));

: block? ( o > b )
  _tosset(sp, newbool(isblock(_tos(sp))));

: undefined? ( o > b )
  _tosset(sp, newbool(isundefined(_tos(sp))));

/* Boolean */

: boolean? ( x > b )
  _tosset(sp, newbool(isbool(_tos(sp))));

: #t ( > b )
  _push(sp, trueobj);

: #f ( > b )
  _push(sp, falseobj);

/* Symbol */

: symbol->string ( sym > str )
  typecheck(issym, Symbol, _tos(sp));
  _tosset(sp, &(((Symbol *)_tos(sp))->string));

: string->symbol ( str > sym )
  typecheck(isstr, String, _tos(sp));
  _tosset(sp, intern(((String *)_tos(sp))->body));

/* String */

: string-append ( str str1 > str2 )
  r1s(_pop(sp));
  typecheck(isstr, String, r1);
  typecheck(isstr, String, _tos(sp));
  port_memory_init2(vm->buf);
  port_write_string(vm->buf, refstr(_tos(sp)));
  port_write_string(vm->buf, refstr(r1));
  _tosset(sp, port_to_string(vm->buf));
  port_memory_init2(vm->buf);

/* Number */

#define _exp(o)\
  r1s(_pop(sp));\
  typecheck(isnum, Number, r1);\
  typecheck(isnum, Number, _tos(sp));\
  _tosset(sp, oexp(o, _tos(sp), r1))

: + ( n1 n2 > n3 )
  _exp(+);

: - ( n1 n2 > n3 )
  _exp(-);

: / ( n1 n2 > n3 )
  _exp(-);

: * ( n1 n2 > n3 )
  _exp(*);

: / ( n1 n2 > n3 )
  _exp(/);

: mod ( n1 n2 > n3 )
  _exp(%);

: neg ( n > n1 )
  typecheck(isnum, Number, _tos(sp));
  _tosset(sp, newnum(-refnum(_tos(sp))));

: abs ( n > u )
  typecheck(isnum, Number, _tos(sp));
  r1s(refnum(_tos(sp)));
  _tosset(sp, newnum(r1 < 0 ? -r1 : r1));

: lshift ( n1 n2 > n3 )
  _exp(<<);

: rshift ( n1 n2 > n3 )
  _exp(>>);

: bitand ( n1 n2 > n3 )
  _exp(&);

: bitor ( n1 n2 > n3 )
  _exp(|);

: bitxor ( n1 n2 > n3 )
  _exp(^);

: bitinvert ( n1 n2 > n3 )
  _tosset(sp, ~(_tos(sp)));

#define _expbool(o)\
  r1s(_pop(sp));\
  typecheck(isnum, Number, r1);\
  typecheck(isnum, Number, _tos(sp));\
  _tosset(sp, newbool(_tos(sp) o r1))

: = ( x x1 > b )
  _expbool(==);

: <> ( x x1 > b )
  _expbool(!=);

: < ( a b > b )
  _expbool(<);

: > ( a b > b )
  _expbool(>);

: >= ( a b > b )
  _expbool(>=);

: <= ( a b > b )
  _expbool(<=);

#define _eq(x)\
  r1s(_tos(sp));\
  typecheck(isnum, Number, r1);\
  _tosset(sp, newbool(eq(refnum(r1), (x))))

: 0= ( n > b )
  _eq(0);

: 0< ( n > b )
  _tosset(sp, _tos(sp) < 0);

: t ( > b )
  _push(sp, true);

: 0 ( > n )
  _push(sp, newnum(0));

: 1 ( > n )
  _push(sp, newnum(1));

: -1 ( > n )
  _push(sp, newnum(-1));

: and ( n1 n2 > n3 )
  _exp(&&);

: or ( n1 n2 > n3 )
  _exp(||);

: not ( f > f1 )
  _tosset(sp, notbool(_tos(sp)));

#undef _exp
#undef _eq

/* Stack */

#define _dup(s) r1s(_tos(s)); _push(s, r1);

: dup ( x > x x )
  _dup(sp);

: 2dup ( x > x x )
  r1s(_stack_index(sp, 0));
  r2s(_stack_index(sp, 1));
  _push(sp, r2);
  _push(sp, r1);

: drop ( x > )
  _drop(sp);

: 2drop ( x > )
  _drop(sp);
  _drop(sp);

: swap ( x x1 > x1 x )
  r1s(_tos(sp));
  _tosset(sp, _si(1));
  _si(1) = r1;

: nip ( x x1 > x1 )
  r1s(_pop(sp));
  _tosset(sp, r1);

: over ( x x1 > x x1 x )
  r1s(_si(1));
  _push(sp, r1);

: tuck ( a b > b a b )
  r1s(_tos(sp));
  _push(sp, r1);
  _si(1) = _si(1);
  _si(2) = _tos(sp);

: pick ( a > b )
  r1s(_tos(sp));
  _tosset(sp, r1);

: rot ( a b c > b c a )
  r1s(_si(0));
  _si(0) = _si(2);
  _si(2) = _si(1);
  _si(1) = r1;

: -rot ( a b c > c a b )
  r1s(_si(0));
  _si(0) = _si(1);
  _si(1) = _si(2);
  _si(2) = r1;

: empty? ( > b )
  r1s(_stack_empty(sp));
  _push(sp, r1);

: ?dup ( x > x x | 0 )
  if (_tos(sp)) {
    _dup(sp);
  } else {
    _tosset(sp, 0);
  }

/* Test */

: test ( b > )
  if (isfalse(_pop(sp))) {
    P("test.sl:%d: error\n", (int)reader_line);
    exit(1);
  }

/* Library */

: load ( str > )
  typecheck(isstr, String, _tos(sp));
  si_load(((String *)_pop(sp))->body);

/* List */

: cons ( a b > c )
  r1s(_pop(sp));
  _tosset(sp, cons((Obj)_tos(sp), (Obj)r1));

: car ( p > x )
  typecheck(ispair, Pair, _tos(sp));
  _tosset(sp, car(_tos(sp)));

: caar ( p > x )
  r1s(_tos(sp));
  typecheck(ispair, Pair, r1);
  r1s(car(r1));
  typecheck(ispair, Pair, r1);
  _tosset(sp, car(r1));

: cadr ( p > x )
  r1s(_tos(sp));
  typecheck(ispair, Pair, r1);
  r1s(car(r1));
  typecheck(ispair, Pair, r1);
  _tosset(sp, cdr(r1));

: cdr ( p > x )
  typecheck(ispair, Pair, _tos(sp));
  _tosset(sp, cdr(_tos(sp)));

: cdar ( p > x )
  r1s(_tos(sp));
  typecheck(ispair, Pair, r1);
  r1s(cdr(r1));
  typecheck(ispair, Pair, r1);
  _tosset(sp, car(r1));

: cddr ( p > x )
  r1s(_tos(sp));
  typecheck(ispair, Pair, r1);
  r1s(cdr(r1));
  typecheck(ispair, Pair, r1);
  _tosset(sp, cdr(r1));

: car! ( x p > )
  r1s(_pop(sp));
  typecheck(ispair, Pair, r1);
  setcar(r1, _pop(sp));

: cdr! ( x p > )
  r1s(_pop(sp));
  typecheck(ispair, Pair, r1);
  setcdr(r1, _pop(sp));

: length ( p > n )
  r1s(_tos(sp));
  typecheck(ispair, Pair, _tos(sp));
  r2s(0);
  for (; r1; r1s(cdr(r1))) r2++;
  _tosset(sp, newnum(r2));

: list>stack ( p > )
  r1s(_pop(sp));
  typecheck(ispair, Pair, r1);
  r2s(0);
  for (; r1; r1s(cdr(r1))) _push(sp, car(r1));

: lcopy ( p > p2 )
  r1s(_tos(sp));
  typecheck(ispair, Pair, r1);
  r3s(cons(car(r1), nil));
  r1s(cdr(r1));
  r2s(r3);
  for (; r1; r1s(cdr(r1))) {
    setcdr(r2, cons(car(r1), nil));
    r2s(cdr(r2));
  }
  _tosset(sp, r3);

: memq ( p o > x )
  r1s(_pop(sp));
  r2s(_tos(sp));
  typecheck(ispair, Pair, r2);
  for (; r2; r2s(cdr(r2)))
    if (eq(car(r2), r1)) { _tosset(sp, cdr(r2)); next; }
  _tosset(sp, falseobj);

: null? ( a > b )
  _tosset(sp, newbool(isnil(_tos(sp))));

: reverse! ( l > l1 )
  r1s(_pop(sp));
  if (isnil(r1)) {
    _push(sp, nil);
    next;
  }
  typecheck(ispair, Pair, r1);
  r2s(nil);
  for (; r1; r1s(r3)) {
    r3s(cdr(r1));
    setcdr(r1, r2);
    r2s(r1);
  }
  _push(sp, r2);

/* Port */

: current-input-port ( > p )
  _push(sp, vm->stdin);

: current-output-port ( > p )
  _push(sp, vm->stdout);

: stdin! ( > p )
  typecheck(isinport, Port, _tos(sp));
  vm->stdin = (Obj)_pop(sp);

: stdout! ( > p )
  typecheck(isoutport, Port, _tos(sp));
  vm->stdout = (Obj)_pop(sp);

: port? ( o > b )
  _tosset(sp, newbool(isinport(_tos(sp)) || isoutport(_tos(sp))));  

: input-port? ( p > b )
  _tosset(sp, newbool(isinport(_tos(sp))));  

: output-port? ( p > b )
  _tosset(sp, newbool(isoutport(_tos(sp))));  

: open-input-file ( s > p )
  typecheck(isstr, String, _tos(sp));
  _tosset(sp, inport_open(((String *)_tos(sp))->body));

: open-output-file ( s > p )
  typecheck(isstr, String, _tos(sp));
  _tosset(sp, outport_open(((String *)_tos(sp))->body));

: open-output-string ( > p )
  _push(sp, portout_memory(1024));

: read* ( p > o )
  typecheck(isoutport, Port, _tos(sp));
  _tosset(sp, readsexp((Port *)_tos(sp), nil));

: read ( > o )
  _push(sp, readsexp(vm->stdin, nil));

char _c;
#define _rc(p) cell(port_read(((Port *)p), &_c, sizeof(char)) ? newchar(_c) : constobj(eof))

: read-char* ( p > c )
  typecheck(isoutport, Port, _tos(sp));
  _tosset(sp, _rc(_tos(sp)));

: read-char ( > c )
  _push(sp, _rc(vm->stdin));

: eof-object? ( o > b )
  _tosset(sp, newbool(iseof(_tos(sp))));

: write* ( o p > )
  r1s(_pop(sp));
  r2s(_pop(sp));
  typecheck(isoutport, Port, r1);
  writesexp((Port *)r1, (Obj)r2);

: write ( o > )
  writesexp(vm->stdout, (Obj)_pop(sp));

: write-char* ( c p > )
  r1s(_pop(sp));
  r2s(_pop(sp));
  typecheck(isoutport, Port, r1);
  typecheck(ischar, Char, r2);
  port_write_char((Port *)r1, refchar(r2));

: write-char ( c > )
  r1s(_pop(sp));
  typecheck(ischar, Char, r1);
  port_write_char(vm->stdout, refchar(r1));

: display* ( c p > )
  r1s(_pop(sp));
  typecheck(isoutport, Port, r1);
  displaysexp((Port *)r1, obj(_pop(sp)));

: display ( c > )
  displaysexp(vm->stdout, obj(_pop(sp)));

: newline* ( p > )
  r1s(_pop(sp));
  typecheck(isoutport, Port, r1);
  port_write_char((Port *)r1, '\n');

: newline ( > )
  port_write_char(vm->stdout, '\n');

: space* ( p > )
  r1s(_pop(sp));
  typecheck(isoutport, Port, r1);
  port_write_char((Port *)r1, ' ');

: space ( > )
  port_write_char(vm->stdout, ' ');

: flush ( p > )
  r1s(_pop(sp));
  typecheck(isoutport, Port, r1);
  port_flush((Port *)r1);

: flush ( p > )
  port_flush(vm->stdout);

/* String port */

: port->string ( p > )
  typecheck(isstroutport, OutputStringPort, _tos(sp));
  _tosset(sp, port_to_string((Port *)_tos(sp)));

/* Context */

: context ( > context )
  _push(sp, context);

: context! ( list > )
  typecheck(ispair, List, _tos(sp));
  context = (Pair *)_pop(sp);

: >context ( dict > )
  typecheck(isdict, Dict, _tos(sp));
  context = cons((Obj)_pop(sp), context);

: context> ( > dict )
  if (isnil(context)) err("error: context>は空です");
  _push(sp, car(context));
  context = cdr(context);

: def ( sym obj > )
  r1s(_pop(sp));
  r2s(_pop(sp));
  typecheck(issym, Symbol, r2);
  def((Obj)r2, (Obj)r1);

: lookup ( sym > obj )
  if (!(issym(_tos(sp)) || isstr(_tos(sp))))
    typeerr("Symbol or String", _tos(sp));
  r1s(safe_lookup(context, ss_string(obj(_tos(sp)))));
  if (isundefined(r1))
    _tosset(sp, falseobj);
  else {
    _tosset(sp, r1);
    _push(sp, trueobj);
  }

/* Stack */

: .stack ( > )
  stack_dump(sp);

: stack-deep ( > n )
  _push(sp, newnum(_stack_used(sp)-1));

void *stack_stack_block;
Cell *stack_stack_top;
Cell *stack_stack_end;
void *stack_stack_ip;
: stack-each ( b f > )
  r1s(_pop(sp));
  typecheck(isblock, Block, r1);
  stack_stack_block = ((Block *)r1)->body;
  r2s(_pop(sp));
  typecheck(isblock, Block, r2);
  {
    Cell *callback[] = { &&_stack_each_label };
    Cell *callback2[] = { &&_stack_each_while_label };
    stack_stack_ip = ip;
    stack_stack_end = sp->top;
    _push(rp, callback);
    ipset(((Block *)r2)->body);
    next;
  _stack_each_label:
    stack_stack_top = sp->top;
    while (stack_stack_end < stack_stack_top) {
      _push(rp, callback2);
      ipset(stack_stack_block);
      next;
    _stack_each_while_label:
      stack_stack_top--;
    }
    ipset(stack_stack_ip);
    sp->top = stack_stack_end;
  }

: -stack-each ( b f > )
  r1s(_pop(sp));
  typecheck(isblock, Block, r1);
  stack_stack_block = ((Block *)r1)->body;
  r2s(_pop(sp));
  typecheck(isblock, Block, r2);
  {
    Cell *callback[] = { &&__stack_each_label };
    Cell *callback2[] = { &&__stack_each_while_label };
    stack_stack_ip = ip;
    stack_stack_end = sp->top;
    _push(rp, sp->top);
    _push(rp, callback);
    ipset(((Block *)r2)->body);
    next;
  __stack_each_label:
    stack_stack_top = sp->top;
    while (stack_stack_end < stack_stack_top) {
      stack_stack_end++;
      _push(rp, callback2);
      _push(sp, *stack_stack_end);
      ipset(stack_stack_block);
      next;
    __stack_each_while_label:
      {};
    }
    ipset(stack_stack_ip);
    sp->top = (Cell *)_pop(rp);
  }

: dump ( block > )
  typecheck(isblock, Block, _tos(sp));
  dump(((Block *)_tos(sp))->body);

/* Dict */

: dict? ( dict > b )
  _tosset(sp, newbool(isdict(_tos(sp))));

: make-dict ( size > dict )
  typecheck(isnum, Number, _tos(sp));
  _tosset(sp, make_dict(_tos(sp)));

: dict-get ( symbol dict > o )
  r1s(_pop(sp));
  typecheck(isdict, Dict, r1);
  r2s(_pop(sp));
  if (issym(r2)) { _push(sp, refhash((Obj)r1, ((Symbol *)r2)->string.body)); }
  else if (isstr(r2)) { _push(sp, refhash((Obj)r1, ((String *)r2)->body)); }
  else { typeerr("Symbol or String", r2); }

: dict-put! ( obj symbol dict > )
  r1s(_pop(sp));
  typecheck(isdict, Dict, r1);
  r2s(_pop(sp));
  r3s(_pop(sp));
  if (issym(r2)) { puthash((Obj)r1, &((Symbol *)r2)->string, (Obj)r3); }
  else if (isstr(r2)) { puthash((Obj)r1, ((String *)r2), (Obj)r3); }
  else { typeerr("Symbol or String", r2); }

: dict-keys ( dict > )
  r1s(_pop(sp));
  typecheck(isdict, Dict, r1);
  dict_keys((Dict *)r1);

/* Compose */

: dodcompose# = DODCOMPOSE ( xt: block obj )
  r1s(*ip++);
  _push(sp, *ip);
  ipset(((Block *)r1)->body);

: data+block = DCOMPOSE ( data block > block1 )
  r1s(_pop(sp));
  typecheck(isblock, Block, r1);
  _tosset(sp, newdblock(obj(r1), obj(_tos(sp))));

/* Lexical */

: x>env# = SAVE_ENV ( xt: size > )
  r1s(*ip++);
  r2s(allot(r1*cellsize + sizeof(Env)));
  ((Env *)r2)->size = r1;
  ((Env *)r2)->prev = (Env *)_tos(lp);
  while (r1--)
    ((Env *)r2)->body[r1] = obj(_pop(sp));
  _push(lp, r2);

: lexblock# = LEXBLOCK ( xt:block_end > )
  r1s(allot(sizeof(Block)+(cellsize*3)));
  ((Block *)r1)->type = typeobj(Block);
  ((Block *)r1)->body[0] = vmc(CLOSURE);
  ((Block *)r1)->body[1] = _tos(lp);
  ((Block *)r1)->body[2] = exp(+, ip, cellsize);
  _push(sp, r1);
  ipset(*ip);

: lexblock-end# = LEXBLOCKEND ( > )
  _drop(lp);
  ret;

: closure-end# = CLOSUREEND ( > )
  _drop(lp);
  _drop(lp);
  ret;

: closure# = CLOSURE ( xt: estack block > )
  _push(lp, *ip++);
  ipset((*ip)+cellsize);

: lref# = LREF ( xt: data > o )
  r2s(_tos(lp));
  r1s(((LexicalData *)ip)->deep);
  while (r1--)
    r2s(((Env *)r2)->prev);
  _push(sp, ((Env *)r2)->body[((LexicalData *)ip)->id]);
  ip++;

: lset# = LSET ( xt: data > o )  
  r2s(_tos(lp));
  r1s(((LexicalData *)ip)->deep);
  while (r1--)
    r2s(((Env *)r2)->prev);
  ((Env *)r2)->body[((LexicalData *)ip)->id] = obj(_pop(sp));
  ip++;

/* VALUE */

: val ( x > val )
  r1s(newobj(Value));
  ((Value *)r1)->body = (Obj)_tos(sp);
  _tosset(sp, r1);

: val@ = VALREF ( val > x )
  typecheck(isval, Value, _tos(sp));
  _tosset(sp, ((Value *)_tos(sp))->body);

: val! = VALSET ( x val > )
  r1s(_pop(sp));
  typecheck(isval, Value, r1);
  ((Value *)r1)->body = (Obj)_pop(sp);

: eval ( sexp > o )
  _tosset(sp, eval(obj(_tos(sp))));

/* Vector */

: vector? ( v > b )
  _tosset(sp, newbool(isvector(_tos(sp))));

: make-vector ( l > v )
  typecheck(isnum, Vector, _tos(sp));
  _tosset(sp, newvector(refnum(_tos(sp))));

: vector-length ( v > l )
  typecheck(isvector, Vector, _tos(sp));
  _tosset(sp, newnum(((Vector *)_tos(sp))->size));

: vector-ref ( n v > o )
  r1s(_pop(sp));
  typecheck(isvector, Vector, r1);
  typecheck(isnum, Number, _tos(sp));
  r2s(refnum(_tos(sp)));
  if (((Vector *)r1)->size <= r2) err("vector-ref: 添字がベクターのサイズを越えています");
  _tosset(sp, ((Vector *)r1)->body[r2]);

: vector-set! ( o n v > )
  r1s(_pop(sp));
  typecheck(isvector, Vector, r1);
  typecheck(isnum, Number, _tos(sp));
  r2s(refnum(_pop(sp)));
  if (((Vector *)r1)->size <= r2) err("vector-ref: 添字がベクターのサイズを越えています");
  ((Vector *)r1)->body[r2] = obj(_pop(sp));

/* Macro */

: macroexpand ( pair > pair2 )
  typecheck(ispair, Pair, _tos(sp));
  _tosset(sp, macroexpand(obj(_tos(sp))));

: macro ( block > macro )
  typecheck(isblock, Block, _tos(sp));
  _tosset(sp, newmacro(obj(_tos(sp))));

: ' ( > symbol )
  _push(sp, readsexp(vm->stdin, nil));
