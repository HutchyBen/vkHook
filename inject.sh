sudo gdb -n -q \
  -ex "attach $(pidof $1)"\
  -ex "set \$dlopen = (void* (*)(char*, int))dlopen"\
  -ex "set \$dlerror = (char* (*)(void))dlerror"\
  -ex "call \$dlopen( \"$(realpath ./build/libvkHook.so)\", 2)"\
  -ex "call \$dlerror()"\
  -ex "layout src"\
  -ex "continue"\
