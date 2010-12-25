#define MEMORY_SIZE (1024*1024)
void *memory[MEMORY_SIZE];

int main(){
  si_setup(memory, MEMORY_SIZE);
  si_interpreter();
  return 0;
}
