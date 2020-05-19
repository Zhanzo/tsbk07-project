#pragma once
#if !defined(STRING_H)
#define STRING_H

#include "type.h"

struct String {
    char* data;
    s64 length;
};

static bool file_read(const char* path, String& data) {
    auto file = fopen(path, "rb");
    if (!file) return false;

    fseek(file, 0, SEEK_END);
    auto length = ftell(file);
    rewind(file);

    auto memory = (char*)malloc(length + 1);
    fread(memory, 1, length, file);
    memory[length] = 0;

    data.data   = memory;
    data.length = length;
    fclose(file);
    return true;
}

#endif
