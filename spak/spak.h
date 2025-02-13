#ifndef _SPAK_DEFINED_
#define _SPAK_DEFINED_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        char name[56];
        uint32_t offset;
        uint32_t size;
    } spak_file_entry_t;

    typedef struct
    {
        char *pak_file_name;
        uint32_t file_count;
        spak_file_entry_t *entries;
    } spak_handle_t;

    spak_handle_t *spak_load_pak(const char *filename);
    void spak_set_current_handle(spak_handle_t *handle);
    void *spak_load_file(const char *filename, uint32_t *file_size);

    void spak_build_pak(const char **paths, const char **filenames, uint32_t file_count, const char *output_name);

#ifdef __cplusplus
}
#endif

#endif