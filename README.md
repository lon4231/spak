# spak
the simple .pak implementation for your projects

### usage

```c
#include<spak/spak.h>

int main()
{
spak_handle*handle=spak_load_pak("path/to/pak.pak");
spak_set_current_handle(handle);


// load file

uint32_t file_size=0; // not necesary
void*data=spak_load_file("filename",&file_size);

// build .pak

spak_build_pak(input_file_array,output_name_array,count,output_file_name);


}


```
