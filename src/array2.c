#include "array2.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <assert.h>

/********************************************************
 Audio Tools, a module and set of tools for manipulating audio data
 Copyright (C) 2007-2011  Brian Langenberger

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *******************************************************/

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

array_i* array_i_new(unsigned count)
{
    int* data = malloc(sizeof(int) * count);

    return array_i_wrap(data, 0, count);
}

array_i* array_i_wrap(int* data, unsigned size, unsigned total_size)
{
    array_i* a = malloc(sizeof(array_i));
    a->data = data;
    a->size = size;
    a->total_size = total_size;

    a->del = array_i_del;
    a->resize = array_i_resize;
    a->reset = array_i_reset;
    a->append = array_i_append;
    a->vappend = array_i_vappend;
    a->extend = array_i_extend;
    a->equals = array_i_equals;
    a->min = array_i_min;
    a->max = array_i_max;
    a->sum = array_i_sum;
    a->copy = array_i_copy;
    a->head = array_i_head;
    a->tail = array_i_tail;
    a->split = array_i_split;
    a->slice = array_i_slice;
    a->reverse = array_i_reverse;
    a->sort = array_i_sort;
    a->print = array_i_print;

    return a;
}

#define FUNC_ARRAY_DEL(FUNC_NAME, ARRAY_TYPE) \
    void                                      \
    FUNC_NAME(ARRAY_TYPE *array)              \
    {                                         \
        free(array->data);                    \
        free(array);                          \
    }
FUNC_ARRAY_DEL(array_i_del, array_i)
FUNC_ARRAY_DEL(array_f_del, array_f)

#define FUNC_ARRAY_RESIZE(FUNC_NAME, ARRAY_TYPE, ARRAY_DATA_TYPE) \
    void                                                          \
    FUNC_NAME(ARRAY_TYPE *array, unsigned minimum)                \
    {                                                             \
        if (minimum > array->total_size) {                         \
            array->total_size = minimum;                                \
            array->data = realloc(array->data,                          \
                                  sizeof(ARRAY_DATA_TYPE) * minimum);   \
        }                                                               \
    }
FUNC_ARRAY_RESIZE(array_i_resize, array_i, int)
FUNC_ARRAY_RESIZE(array_f_resize, array_f, double)

void array_i_reset(array_i *array)
{
    array->size = 0;
}

#define FUNC_ARRAY_APPEND(FUNC_NAME, ARRAY_TYPE, ARRAY_DATA_TYPE) \
    void                                                          \
    FUNC_NAME(ARRAY_TYPE *array, ARRAY_DATA_TYPE value)           \
    {                                                             \
        if (array->size == array->total_size)                     \
            array->resize(array, array->total_size * 2);          \
                                                                  \
        array->data[array->size++] = value;                       \
    }
FUNC_ARRAY_APPEND(array_i_append, array_i, int)
FUNC_ARRAY_APPEND(array_f_append, array_f, double)

#define FUNC_ARRAY_VAPPEND(FUNC_NAME, ARRAY_TYPE, ARRAY_DATA_TYPE) \
    void                                                           \
    FUNC_NAME(ARRAY_TYPE *array, unsigned count, ...)              \
    {                                                              \
    ARRAY_DATA_TYPE i;                                             \
    va_list ap;                                                    \
                                                                   \
    array->resize(array, array->size + count);                     \
    va_start(ap, count);                                           \
    for (; count > 0; count--) {                                   \
        i = va_arg(ap, ARRAY_DATA_TYPE);                           \
        array->data[array->size++] = i;                            \
    }                                                              \
    va_end(ap);                                                    \
    }
FUNC_ARRAY_VAPPEND(array_i_vappend, array_i, int)
FUNC_ARRAY_VAPPEND(array_f_vappend, array_f, double)

#define FUNC_ARRAY_EXTEND(FUNC_NAME, ARRAY_TYPE, ARRAY_DATA_TYPE) \
    void                                                          \
    FUNC_NAME(ARRAY_TYPE *array, ARRAY_TYPE *to_add)              \
    {                                                             \
        array->resize(array, array->size + to_add->size);         \
        memcpy(array->data + array->size,                         \
               to_add->data,                                      \
               sizeof(ARRAY_DATA_TYPE) * to_add->size);           \
        array->size += to_add->size;                              \
    }
FUNC_ARRAY_EXTEND(array_i_extend, array_i, int)
FUNC_ARRAY_EXTEND(array_f_extend, array_f, double)

#define FUNC_ARRAY_EQUALS(FUNC_NAME, ARRAY_TYPE, ARRAY_DATA_TYPE) \
    int                                                            \
    FUNC_NAME(ARRAY_TYPE *array, ARRAY_TYPE *compare)              \
    {                                                              \
        if (array->size == compare->size) {                        \
            return (memcmp(array->data, compare->data,                  \
                           sizeof(ARRAY_DATA_TYPE) * array->size) == 0); \
        } else                                                          \
            return 0;                                                   \
    }
FUNC_ARRAY_EQUALS(array_i_equals, array_i, int)
FUNC_ARRAY_EQUALS(array_f_equals, array_f, double)

int array_i_min(array_i *array)
{
    int min = INT_MAX;
    unsigned i;

    for (i = 0; i < array->size; i++)
        if (array->data[i] < min)
            min = array->data[i];

    return min;
}

int array_i_max(array_i *array)
{
    int max = INT_MIN;
    unsigned i;

    for (i = 0; i < array->size; i++)
        if (array->data[i] > max)
            max = array->data[i];

    return max;
}

int array_i_sum(array_i *array)
{
    int accumulator = 0;
    int *data = array->data;
    unsigned size = array->size;
    unsigned i;

    for (i = 0; i < size; i++)
        accumulator += data[i];

    return accumulator;
}

#define FUNC_ARRAY_COPY(FUNC_NAME, ARRAY_TYPE, ARRAY_DATA_TYPE) \
    void                                                        \
    FUNC_NAME(ARRAY_TYPE *array, ARRAY_TYPE *copy)              \
    {                                                           \
        if (array != copy) {                                        \
            array->resize(copy, array->size);                       \
            memcpy(copy->data, array->data,                         \
                   array->size * sizeof(ARRAY_DATA_TYPE));          \
            copy->size = array->size;                               \
        }                                                           \
    }
FUNC_ARRAY_COPY(array_i_copy, array_i, int)
FUNC_ARRAY_COPY(array_f_copy, array_f, double)

#define FUNC_ARRAY_HEAD(FUNC_NAME, ARRAY_TYPE, ARRAY_DATA_TYPE)     \
    void                                                            \
    FUNC_NAME(ARRAY_TYPE *array, unsigned count, ARRAY_TYPE *head)  \
    {                                                               \
        unsigned to_copy = MIN(count, array->size);                 \
                                                                    \
        if (head != array) {                                        \
            array->resize(head, to_copy);                           \
            memcpy(head->data, array->data,                         \
                   sizeof(ARRAY_DATA_TYPE) * to_copy);              \
            head->size = to_copy;                                   \
        } else {                                                    \
            array->size = to_copy;                                  \
        }                                                           \
    }
FUNC_ARRAY_HEAD(array_i_head, array_i, int)
FUNC_ARRAY_HEAD(array_f_head, array_f, double)

#define FUNC_ARRAY_TAIL(FUNC_NAME, ARRAY_TYPE, ARRAY_DATA_TYPE)     \
    void                                                            \
    FUNC_NAME(ARRAY_TYPE *array, unsigned count, ARRAY_TYPE *tail)  \
    {                                                               \
        unsigned to_copy = MIN(count, array->size);                 \
                                                                    \
        if (tail != array) {                                        \
            array->resize(tail, to_copy);                           \
            memcpy(tail->data, array->data + (array->size - to_copy),   \
                   sizeof(ARRAY_DATA_TYPE) * to_copy);                  \
            tail->size = to_copy;                                       \
        } else {                                                        \
            memmove(array->data, array->data + (array->size - to_copy), \
                    sizeof(ARRAY_DATA_TYPE) * to_copy);                 \
            array->size = to_copy;                                      \
        }                                                               \
    }
FUNC_ARRAY_TAIL(array_i_tail, array_i, int)
FUNC_ARRAY_TAIL(array_f_tail, array_f, double)

#define FUNC_ARRAY_SPLIT(FUNC_NAME, ARRAY_TYPE, ARRAY_DATA_TYPE) \
    void                                                               \
    FUNC_NAME(ARRAY_TYPE *array, unsigned count,                       \
              ARRAY_TYPE *head, ARRAY_TYPE *tail)                      \
    {                                                                  \
        /*ensure we don't try to move too many items*/                 \
        unsigned to_head = MIN(count, array->size);                    \
        unsigned to_tail = array->size - to_head;                      \
                                                                       \
        if ((head == array) && (tail == array)) {                      \
            /*do nothing*/                                             \
            return;                                                    \
        } else if ((head != array) && (tail == array)) {               \
            /*move "count" values to head and shift the rest down*/    \
            array->resize(head, to_head);                               \
            memcpy(head->data, array->data,                             \
                   sizeof(ARRAY_DATA_TYPE) * to_head);                  \
            head->size = to_head;                                       \
                                                                        \
            memmove(array->data, array->data + to_head,                 \
                    sizeof(ARRAY_DATA_TYPE) * to_tail);                 \
            array->size = to_tail;                                      \
        } else if ((head == array) && (tail != array)) {                \
            /*move "count" values from our end to tail and reduce our size*/ \
            array->resize(tail, to_tail);                               \
            memcpy(tail->data, array->data + to_head,                   \
                   sizeof(ARRAY_DATA_TYPE) * to_tail);                  \
            tail->size = to_tail;                                       \
                                                                        \
            array->size = to_head;                                      \
        } else {                                                        \
            /*copy "count" values to "head" and the remainder to "tail"*/ \
            array->resize(head, to_head);                               \
            memcpy(head->data, array->data, sizeof(ARRAY_DATA_TYPE) * to_head); \
            head->size = to_head;                                       \
                                                                        \
            array->resize(tail, to_tail);                               \
            memcpy(tail->data, array->data + to_head,                   \
                   sizeof(ARRAY_DATA_TYPE) * to_tail);                  \
            tail->size = to_tail;                                       \
        }                                                               \
    }
FUNC_ARRAY_SPLIT(array_i_split, array_i, int)
FUNC_ARRAY_SPLIT(array_f_split, array_f, double)

#define FUNC_ARRAY_SLICE(FUNC_NAME, ARRAY_TYPE, ARRAY_DATA_TYPE, ARRAY_NEW) \
    void                                                            \
    FUNC_NAME(ARRAY_TYPE *array,                                    \
              unsigned start, unsigned end, unsigned jump,          \
              ARRAY_TYPE *slice)                                    \
    {                                                               \
        assert(start <= end);                                       \
        assert(jump > 0);                                           \
                                                                    \
        start = MIN(start, array->size);                            \
        end = MIN(end, array->size);                                \
                                                                    \
        if (array != slice) {                                       \
            if (jump == 1) {                                        \
                slice->resize(slice, end - start);                  \
                memcpy(slice->data, array->data + start,            \
                       sizeof(ARRAY_DATA_TYPE) * (end - start));    \
                slice->size = end - start;                          \
            } else {                                                \
                slice->reset(slice);                                \
                for (; start < end; start += jump)                  \
                    slice->append(slice, array->data[start]);       \
            }                                                       \
        } else {                                                    \
            if (jump == 1) {                                        \
                memmove(array->data, array->data + start,           \
                        sizeof(ARRAY_DATA_TYPE) * (end - start));   \
                array->size = end - start;                          \
            } else {                                                \
                slice = ARRAY_NEW(1);                               \
                for (; start < end; start += jump)                  \
                    slice->append(slice, array->data[start]);       \
                slice->copy(slice, array);                          \
                slice->del(slice);                                  \
            }                                                       \
        }                                                           \
    }
FUNC_ARRAY_SLICE(array_i_slice, array_i, int, array_i_new)
FUNC_ARRAY_SLICE(array_f_slice, array_f, double, array_f_new)

#define FUNC_ARRAY_REVERSE(FUNC_NAME, ARRAY_TYPE, ARRAY_DATA_TYPE)  \
    void                                                            \
    FUNC_NAME(ARRAY_TYPE *array)                                    \
    {                                                               \
        unsigned i;                                                 \
        unsigned j;                                                 \
        int x;                                                      \
        ARRAY_DATA_TYPE *data = array->data;                        \
                                                                    \
        if (array->size > 0) {                                      \
            for (i = 0, j = array->size - 1; i < j; i++, j--) {     \
                x = data[i];                                        \
                data[i] = data[j];                                  \
                data[j] = x;                                        \
            }                                                       \
        }                                                           \
    }
FUNC_ARRAY_REVERSE(array_i_reverse, array_i, int)
FUNC_ARRAY_REVERSE(array_f_reverse, array_f, double)

int array_int_cmp(const void *x, const void *y)
{
    return *(int*)x - *(int*)y;
}

void array_i_sort(array_i *array)
{
    qsort(array->data, (size_t)(array->size), sizeof(int), array_int_cmp);
}

void array_i_print(array_i *array, FILE* output)
{
    unsigned i;

    putc('[', output);
    if (array->size == 1) {
        printf("%d", array->data[0]);
    } else if (array->size > 1) {
        for (i = 0; i < array->size - 1; i++)
            printf("%d, ", array->data[i]);
        printf("%d", array->data[i]);
    }
    putc(']', output);
}


array_f* array_f_new(unsigned count)
{
    double* data = malloc(sizeof(double) * count);

    return array_f_wrap(data, 0, count);
}

void array_f_reset(array_f *array)
{
    array->size = 0;
}

array_f* array_f_wrap(double* data, unsigned size, unsigned total_size)
{
    array_f* a = malloc(sizeof(array_f));
    a->data = data;
    a->size = size;
    a->total_size = total_size;

    a->del = array_f_del;
    a->resize = array_f_resize;
    a->reset = array_f_reset;
    a->append = array_f_append;
    a->vappend = array_f_vappend;
    a->extend = array_f_extend;
    a->equals = array_f_equals;
    a->min = array_f_min;
    a->max = array_f_max;
    a->sum = array_f_sum;
    a->copy = array_f_copy;
    a->head = array_f_head;
    a->tail = array_f_tail;
    a->split = array_f_split;
    a->slice = array_f_slice;
    a->reverse = array_f_reverse;
    a->sort = array_f_sort;
    a->print = array_f_print;

    return a;
}

double array_f_min(array_f *array)
{
    double min = DBL_MAX;
    unsigned i;

    for (i = 0; i < array->size; i++)
        if (array->data[i] < min)
            min = array->data[i];

    return min;
}

double array_f_max(array_f *array)
{
    double max = DBL_MIN;
    unsigned i;

    for (i = 0; i < array->size; i++)
        if (array->data[i] > max)
            max = array->data[i];

    return max;
}

double array_f_sum(array_f *array)
{
    double accumulator = 0.0;
    double *data = array->data;
    unsigned size = array->size;
    unsigned i;

    for (i = 0; i < size; i++)
        accumulator += data[i];

    return accumulator;
}

int array_float_cmp(const void *x, const void *y)
{
    return *(double*)x - *(double*)y;
}

void array_f_sort(array_f *array)
{
    qsort(array->data, (size_t)(array->size), sizeof(double), array_float_cmp);
}

void array_f_print(array_f *array, FILE* output)
{
    unsigned i;

    putc('[', output);
    if (array->size == 1) {
        printf("%f", array->data[0]);
    } else if (array->size > 1) {
        for (i = 0; i < array->size - 1; i++)
            printf("%f, ", array->data[i]);
        printf("%f", array->data[i]);
    }
    putc(']', output);
}


array_ia*
array_ia_new(unsigned count)
{
    array_ia* a = malloc(sizeof(array_ia));
    unsigned i;

    a->data = malloc(sizeof(array_i*) * count);
    a->size = 0;
    a->total_size = count;

    for (i = 0; i < count; i++) {
        a->data[i] = array_i_new(1);
    }

    a->del = array_ia_del;
    a->resize = array_ia_resize;
    a->reset = array_ia_reset;
    a->append = array_ia_append;
    a->vappend = array_ia_vappend;
    a->extend = array_ia_extend;
    a->equals = array_ia_equals;
    a->print = array_ia_print;

    return a;
}

#define ARRAY_A_DEL(FUNC_NAME, ARRAY_TYPE) \
    void                                   \
    FUNC_NAME(ARRAY_TYPE *array)           \
    {                                      \
        unsigned i;                        \
                                            \
        for (i = 0; i < array->total_size; i++) \
            array->data[i]->del(array->data[i]);    \
                                                    \
        free(array->data);                          \
        free(array);                                \
    }
ARRAY_A_DEL(array_ia_del, array_ia)
ARRAY_A_DEL(array_fa_del, array_fa)

#define ARRAY_A_RESIZE(FUNC_NAME, ARRAY_TYPE, NEW_FUNC)     \
    void                                                    \
    FUNC_NAME(ARRAY_TYPE *array, unsigned minimum)          \
    {                                                                   \
        if (minimum > array->total_size) {                              \
            array->data = realloc(array->data, sizeof(array_i*) * minimum); \
            while (array->total_size < minimum) {                       \
                array->data[array->total_size++] = NEW_FUNC(1);         \
            }                                                           \
        }                                                               \
    }
ARRAY_A_RESIZE(array_ia_resize, array_ia, array_i_new)
ARRAY_A_RESIZE(array_fa_resize, array_fa, array_f_new)

void array_ia_reset(array_ia *array)
{
    array->size = 0;
}

#define ARRAY_A_APPEND(FUNC_NAME, ARRAY_TYPE, ARRAY_DATA_TYPE) \
    void                                                       \
    FUNC_NAME(ARRAY_TYPE *array, ARRAY_DATA_TYPE *value)       \
    {                                                          \
        if (array->size == array->total_size)                  \
            array->resize(array, array->total_size * 2);       \
                                                               \
        value->copy(value, array->data[array->size++]);        \
    }
ARRAY_A_APPEND(array_ia_append, array_ia, array_i)
ARRAY_A_APPEND(array_fa_append, array_fa, array_f)

#define ARRAY_A_VAPPEND(FUNC_NAME, ARRAY_TYPE, ARRAY_DATA_TYPE) \
    void                                                        \
    FUNC_NAME(ARRAY_TYPE *array, unsigned count, ...)           \
    {                                                           \
        ARRAY_DATA_TYPE *a;                                     \
        va_list ap;                                             \
                                                                \
        array->resize(array, array->size + count);              \
        va_start(ap, count);                                    \
        for (; count > 0; count--) {                            \
            a = va_arg(ap, ARRAY_DATA_TYPE*);                   \
            a->copy(a, array->data[array->size++]);             \
        }                                                       \
        va_end(ap);                                             \
    }
ARRAY_A_VAPPEND(array_ia_vappend, array_ia, array_i)
ARRAY_A_VAPPEND(array_fa_vappend, array_fa, array_f)

#define ARRAY_A_EXTEND(FUNC_NAME, ARRAY_TYPE)      \
    void                                                \
    FUNC_NAME(ARRAY_TYPE *array, ARRAY_TYPE *to_add)    \
    {                                                   \
        unsigned i;                                     \
                                                        \
        array->resize(array, array->size + to_add->size);   \
        for (i = 0; i < to_add->size; i++) {                            \
            to_add->data[i]->copy(to_add->data[i],                      \
                                  array->data[array->size++]);          \
        }                                                               \
    }
ARRAY_A_EXTEND(array_ia_extend, array_ia)
ARRAY_A_EXTEND(array_fa_extend, array_fa)

#define ARRAY_A_EQUALS(FUNC_NAME, ARRAY_TYPE) \
    int                                               \
    FUNC_NAME(ARRAY_TYPE *array, ARRAY_TYPE *compare) \
    {                                                 \
        unsigned i;                                   \
                                                      \
        if (array->size == compare->size) {           \
            for (i = 0; i < array->size; i++)                           \
                if (!array->data[i]->equals(array->data[i], \
                                            compare->data[i]))  \
                    return 0;                                   \
                                                                \
            return 1;                                           \
        } else                                                  \
            return 0;                                           \
    }
ARRAY_A_EQUALS(array_ia_equals, array_ia)
ARRAY_A_EQUALS(array_fa_equals, array_fa)

#define ARRAY_A_PRINT(FUNC_NAME, ARRAY_TYPE) \
    void                                      \
    FUNC_NAME(ARRAY_TYPE *array, FILE* output)  \
    {                                           \
        unsigned i;                             \
                                                \
        putc('[', output);                      \
        if (array->size == 1) {                        \
            array->data[0]->print(array->data[0], output);  \
        } else if (array->size > 1) {                       \
            for (i = 0; i < array->size - 1; i++) {         \
                array->data[i]->print(array->data[i], output);  \
                printf(", ");                                   \
            }                                                   \
            array->data[i]->print(array->data[i], output);      \
        }                                                       \
        putc(']', output);                                      \
    }
ARRAY_A_PRINT(array_ia_print, array_ia)
ARRAY_A_PRINT(array_fa_print, array_fa)


array_fa*
array_fa_new(unsigned count)
{
    array_fa* a = malloc(sizeof(array_f));
    unsigned i;

    a->data = malloc(sizeof(array_f*) * count);
    a->size = 0;
    a->total_size = count;

    for (i = 0; i < count; i++) {
        a->data[i] = array_f_new(1);
    }

    a->del = array_fa_del;
    a->resize = array_fa_resize;
    a->reset = array_fa_reset;
    a->append = array_fa_append;
    a->vappend = array_fa_vappend;
    a->extend = array_fa_extend;
    a->equals = array_fa_equals;
    a->print = array_fa_print;

    return a;
}

void array_fa_reset(array_fa *array)
{
    array->size = 0;
}
