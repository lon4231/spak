#include "spak.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
    char signature[4];
    uint32_t offset;
    uint32_t size;
} pak_header_t;

spak_handle_t *current_handle;

void spak_set_current_handle(spak_handle_t *handle)
{
    current_handle = handle;
}

spak_handle_t *spak_load_pak(const char *filename)
{
    pak_header_t header;
    spak_handle_t *handle = NULL;
    FILE *pak_file = NULL;

    pak_file = fopen(filename, "rb");
    if (pak_file == NULL)
    {
        return NULL;
    }

    if (fread(&header, sizeof(pak_header_t), 1, pak_file) != 1)
    {
        goto pak_error;
    }

    handle = malloc(sizeof(spak_handle_t));
    if (handle == NULL)
    {
        goto pak_error;
    }
    memset(handle, 0, sizeof(spak_handle_t));

    if (fseek(pak_file, header.offset, SEEK_SET) != 0)
    {
        goto pak_error;
    }

    handle->file_count = header.size / sizeof(spak_file_entry_t);
    handle->entries = malloc(handle->file_count * sizeof(spak_file_entry_t));
    if (handle->entries == NULL)
    {
        goto pak_error;
    }
    memset(handle->entries, 0, handle->file_count * sizeof(spak_file_entry_t));

    if (fread(handle->entries, sizeof(spak_file_entry_t), handle->file_count, pak_file) != handle->file_count)
    {
        goto pak_error;
    }

    handle->pak_file_name = malloc(strlen(filename) + 1);
    if (handle->pak_file_name == NULL)
    {
        goto pak_error;
    }
    strcpy(handle->pak_file_name, filename);

    fclose(pak_file);
    return handle;

pak_error:

    if (pak_file != NULL)
    {
        fclose(pak_file);
    }

    if (handle != NULL)
    {
        if (handle->entries != NULL)
        {
            free(handle->entries);
        }

        if (handle->pak_file_name != NULL)
        {
            free(handle->pak_file_name);
        }

        free(handle);
    }

    return NULL;
}

void *spak_load_file(const char *filename, uint32_t *file_size)
{
    void *file_data = NULL;
    FILE *pak_file = NULL;

    if (current_handle == NULL)
    {
        goto pak_load_file_error;
    }

    if (filename == NULL)
    {
        goto pak_load_file_error;
    }

    for (uint32_t i = 0; i < current_handle->file_count; ++i)
    {
        if (strncmp(current_handle->entries[i].name, filename, 56) == 0)
        {
            pak_file = fopen(current_handle->pak_file_name, "rb");
            if (pak_file == NULL)
            {
                goto pak_load_file_error;
            }

            if (fseek(pak_file, current_handle->entries[i].offset, SEEK_SET) != 0)
            {
                goto pak_load_file_error;
            }

            file_data = malloc(current_handle->entries[i].size);
            if (file_data == NULL)
            {
                goto pak_load_file_error;
            }

            if (fread(file_data, current_handle->entries[i].size, 1, pak_file) != 1)
            {
                goto pak_load_file_error;
            }

            if (file_size != NULL)
            {
                *file_size = current_handle->entries[i].size;
            }

            fclose(pak_file);
            return file_data;
        }
    }

pak_load_file_error:

    if (pak_file != NULL)
    {
        fclose(pak_file);
    }

    if (file_data != NULL)
    {
        free(file_data);
    }

    return NULL;
}
