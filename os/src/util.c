#include "os/util.h"

#ifdef __x86_64__
#include <stdarg.h>
#define arg va_arg
#else
#define arg(varg, type) (type) *((type *)varg++)
#endif

int itoa(char *str, size_t len, int i) {
  int c = i, l = 1;
  // take a digit to display the sign
  if(i < 0) {
    str[0] = '-';
    ++l;
  }
  while((c /= 10) != 0)
    ++l;
  if(l > len-1)
    return 0;

  str[l] = '\0';
  c = l; // number of char wrote
  if(i < 0)
    i = -i;
  while(l-- && i != 0) {
    str[l] = '0' + i % 10;
    i /= 10;
  }
  return c;
}

size_t __printf(void *target, size_t (* writer)(void *, const char *), const char *fmt, 
#ifdef __x86_64__
    va_list varg
#else
    int *varg
#endif
){
    size_t wr, last_wrote;

	  for (wr = 0; *fmt != '\0'; ++fmt, wr += last_wrote) {
        if (*fmt == '%') {
            ++fmt;
            if( *fmt == 's' ) 
                last_wrote = writer(target, arg(varg, char *));
            else if( *fmt == 'd' ) {
                // TODO: itoa digit by digit to remove the fixed string limit
                char str[10];
                itoa(str, 10, arg(varg, int));
                last_wrote = writer(target, str);
            } else if(*fmt == '%') {
              char str[2] = "%";
              last_wrote = writer(target, str);
            } else {
              char str[3] = {*(fmt-1), *fmt, '\0'};
              last_wrote = writer(target, str);
            }
        } else {
            char str[2] = {*fmt, '\0'};
            last_wrote = writer(target, str);
        }
        if(!last_wrote)
          break;
    }
    // if we stpped printing because of a writer error, make sure to print the ending null character
    if(*fmt != '\0') {
      char end = '\0';
      wr += writer(target, &end);
    }
    return wr;
}

typedef struct str_writer {
  char *str;
  size_t size, wrote;
} str_writer_t;

size_t str_writer(void *dest, const char *data) {
  str_writer_t *d;
  int i;

  d = (str_writer_t *) dest;
  i = 0;
  // Make sure we always write the NULL terminator
  if(*data == '\0')
    *(d->str + (d->wrote >= d->size-1 ? d->wrote : ++d->wrote)) = '\0';
  else
    while(d->wrote+i < d->size-1 && (*(d->str+d->wrote+i) = data[i]))
      ++i;

  d->wrote += i;
  return i;
}

int snprintf(char *dest, size_t len, const char *fmt, ...) {
    str_writer_t w = {dest, len, 0};
    #ifdef __x86_64__
    va_list varg;
    va_start(varg, fmt);
    #else
    int *varg;
    varg = (int *)(&fmt + 1);
    #endif
    size_t res = __printf((void *) &w, str_writer, fmt, varg);
    #ifdef __x86_64__
    va_end(varg);
    #endif
    return res;
}
