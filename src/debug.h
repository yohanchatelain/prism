#ifndef __PRISM_DEBUG_H__
#define __PRISM_DEBUG_H__

#include <cassert>
#include <cstdlib>

extern int is_debug();
extern void prism_debug_printf(const char *fmt, ...);
extern void prism_debug_header_start(const char *func);
extern void prism_debug_header_end(const char *func);

#ifdef PRISM_DEBUG
#ifndef PRISM_DEBUG_FUNCTIONS_DECLARED
#include <cstdarg>
#include <cstdio>
#define sr_buffer_size 1024
static char prism_end = '\0';
static char prism_indent = '\t';
static int prism_debug_level = 0;
static char prism_debug_buffer[sr_buffer_size] = {prism_indent};

#define sr_buffer_size_str 1048576
static char prism_debug_buffer_str[sr_buffer_size_str] = {prism_end};
static int prism_debug_str_pos = 0;
#endif // PRISM_DEBUG_FUNCTIONS_DECLARED

inline int is_debug() {
  static int is_debug = -1;
  if (is_debug == -1) {
    const char *debug = getenv("PRISM_DEBUG");
    is_debug = (debug != NULL) and (debug[0] == '1');
  }
  return is_debug;
}

inline void prism_debug_printf(const char *fmt, ...) {
  if (!is_debug()) {
    return;
  }
  assert(prism_debug_level >= 0);
  assert(prism_debug_level < sr_buffer_size);
  prism_debug_buffer[prism_debug_level] = prism_end;
  fprintf(stderr, "[debug] %s", prism_debug_buffer);
  prism_debug_buffer[prism_debug_level] = prism_indent;
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
}

inline void prism_debug_header_start(const char *func) {
  if (!is_debug()) {
    return;
  }
  assert(prism_debug_level >= 0);
  assert(prism_debug_level < sr_buffer_size);
  prism_debug_buffer[prism_debug_level] = prism_end;
  fprintf(stderr, "[debug] %s===%s===\n", prism_debug_buffer, func);
  prism_debug_buffer[prism_debug_level] = prism_indent;
  prism_debug_level++;
}

inline void prism_debug_header_end(const char *func) {
  if (!is_debug()) {
    return;
  }
  prism_debug_level--;
  assert(prism_debug_level >= 0);
  assert(prism_debug_level < sr_buffer_size);
  prism_debug_buffer[prism_debug_level] = prism_end;
  fprintf(stderr, "[debug] %s===%s===\n\n", prism_debug_buffer, func);
  prism_debug_buffer[prism_debug_level] = prism_indent;
}

inline void prism_debug_reset() {
  if (!is_debug()) {
    return;
  }
  prism_debug_str_pos = 0;
  prism_debug_buffer_str[prism_debug_str_pos] = prism_end;
}

inline void prism_debug_flush() {
  if (!is_debug()) {
    return;
  }
  fprintf(stderr, "%s", prism_debug_buffer_str);
  prism_debug_reset();
}
#endif // PRISM_DEBUG

#define DEBUG_FUNCTIONS_DECLARED

#ifdef PRISM_DEBUG
#define debug_print(fmt, ...) prism_debug_printf(fmt, __VA_ARGS__)
#define debug_start() prism_debug_header_start(__func__)
#define debug_end() prism_debug_header_end(__func__)
#define debug_flush() fprintf(stderr, "%s", prism_debug_buffer_str)
#define debug_reset() prism_debug_reset()
#else
#define debug_print(fmt, ...)
#define debug_start()
#define debug_end()
#define debug_flush()
#define debug_reset()
#endif // PRISM_DEBUG

#endif // __PRISM_DEBUG_H__