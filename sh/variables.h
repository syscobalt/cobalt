/* Copyright (c) 2019, 2020, 2021, 2022 Dennis Wölfing
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* sh/variables.h
 * Shell variables.
 */

#ifndef VARIABLES_H
#define VARIABLES_H

#include <stdbool.h>

struct ShellVar {
    char* name;
    char* value;
};

extern char** arguments;
extern int numArguments;
extern struct ShellVar* variables;
extern size_t variablesAllocated;

const char* getVariable(const char* name);
void initializeVariables(void);
bool isRegularVariableName(const char* s);
void popVariables(void);
void printVariables(bool exported);
void pushVariable(const char* name, const char* value);
void setVariable(const char* name, const char* value, bool export);
void unsetVariable(const char* name);

#endif
