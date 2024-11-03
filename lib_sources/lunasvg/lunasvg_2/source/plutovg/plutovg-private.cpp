/*
 * These functions are used to replace the plutovg_array_ensure() macro in
 * plutovg-private.h.
 *
 * Note that reallocation is essentially the same algorithm as std::vector
 * which after the initial allocation, doubles the capacity each time the
 * current capacity is reached.
 */

#include "plutovg-private.h"

void pvg_path_ensure(plutovg_path_t* path, int cElements, int cPoints)
{
    if (path->elements.size + cElements > path->elements.capacity)
    {
        int capacity = path->elements.size + cElements;
        int newcapacity = path->elements.capacity == 0 ? 8 : path->elements.capacity;
        while (newcapacity < capacity)
        {
            newcapacity *= 2;
        }
        path->elements.data =
            (plutovg_path_element_t*) realloc(path->elements.data, newcapacity * sizeof(path->elements.data[0]));
        path->elements.capacity = newcapacity;
    }
    if (path->points.size + cPoints > path->points.capacity)
    {
        int capacity = path->points.size + cPoints;
        int newcapacity = path->points.capacity == 0 ? 8 : path->points.capacity;
        while (newcapacity < capacity)
        {
            newcapacity *= 2;
        }
        path->points.data = (plutovg_point_t*) realloc(path->points.data, newcapacity * sizeof(path->points.data[0]));
        path->points.capacity = newcapacity;
    }
}

void pvg_rle_ensure(plutovg_rle_t* path, int count)
{
    if (path->spans.size + count > path->spans.capacity)
    {
        int capacity = path->spans.size + count;
        int newcapacity = path->spans.capacity == 0 ? 8 : path->spans.capacity;
        while (newcapacity < capacity)
        {
            newcapacity *= 2;
        }
        path->spans.data = (plutovg_span_t*) realloc(path->spans.data, newcapacity * sizeof(path->spans.data[0]));
        path->spans.capacity = newcapacity;
    }
}

void pvg_gradient_ensure(plutovg_gradient_t* path, int count)
{
    if (path->stops.size + count > path->stops.capacity)
    {
        int capacity = path->stops.size + count;
        int newcapacity = path->stops.capacity == 0 ? 8 : path->stops.capacity;
        while (newcapacity < capacity)
        {
            newcapacity *= 2;
        }
        path->stops.data = (plutovg_gradient_stop_t*) realloc(path->stops.data, newcapacity * sizeof(path->stops.data[0]));
        path->stops.capacity = newcapacity;
    }
}
