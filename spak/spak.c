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

            if (fclose(pak_file))
            {
                goto pak_load_file_error;
            }
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

void spak_build_pak(const char **paths, const char **filenames, uint32_t file_count, const char *output_name)
{
    pak_header_t header;
    FILE *output_file = NULL;
    FILE *input_file = NULL;
    uint32_t input_file_size = 12;
    void *input_file_buffer = NULL;
    spak_file_entry_t *file_entries = NULL;

    memcpy(header.signature, "PACK", 4);
    header.size = file_count * sizeof(spak_file_entry_t);
    header.offset = 0;

    output_file = fopen(output_name, "wb");
    if (output_file == NULL)
    {
        goto build_error;
    }

    if (fwrite(&header, sizeof(pak_header_t), 1, output_file) != 1)
    {
        goto build_error;
    }

    file_entries = malloc(file_count * sizeof(spak_file_entry_t));
    if (file_entries == NULL)
    {
        goto build_error;
    }

    for (uint32_t i = 0; i < file_count; ++i)
    {
        input_file = fopen(paths[i], "rb");
        if (input_file == NULL)
        {
            goto build_error;
        }

        fseek(input_file, 0, SEEK_END);
        input_file_size = ftell(input_file);
        fseek(input_file, 0, SEEK_SET);

        strncpy(file_entries[i].name, filenames[i], 56);
        file_entries[i].offset = header.offset;
        file_entries[i].size = input_file_size;

        header.offset += input_file_size;

        input_file_buffer = malloc(input_file_size);
        if (input_file_buffer == NULL)
        {
            goto build_error;
        }

        if (fread(input_file_buffer, input_file_size, 1, input_file) != 1)
        {
            goto build_error;
        }

        if (fwrite(input_file_buffer, input_file_size, 1, output_file) != 1)
        {
            goto build_error;
        }

        free(input_file_buffer);

        if (fclose(input_file) != 0)
        {
            goto build_error;
        }
    }

    header.offset = ftell(output_file);

    if (fwrite(file_entries, sizeof(spak_file_entry_t), file_count, output_file) != file_count)
    {
        goto build_error;
    }

    fseek(output_file, 0, SEEK_SET);

    if (fwrite(&header, sizeof(pak_header_t), 1, output_file) != 1)
    {
        goto build_error;
    }

    return;

build_error:
    if (output_file != NULL)
    {
        fclose(output_file);
    }
    if (input_file != NULL)
    {
        fclose(input_file);
    }
    if (file_entries != NULL)
    {
        free(file_entries);
    }

    return;
}