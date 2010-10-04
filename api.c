#include "local.c"

extern void si_setup(void *memory, Cell size){
  assert(memory && size);
  setup_image(memory, size);
  setup_symbol();
  setup_dict();
  setup_vm();
  setup_port();
}

extern void si_interpreter(){
  Obj sexp;
  ReadContext context;
  clear(&context, sizeof(ReadContext));
  while (!iseof(sexp = readsexp(vm->stdin, &context))) {
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

extern void si_load(const char *path){
  assert(path);
  Port *def = vm->stdin;
  vm->stdin = inport_open(path);
  si_interpreter();
  port_close(vm->stdin);
  vm->stdin = def;
}
