#include "os/util.h"

#ifdef __x86_64__
#include <stdio.h>
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

int strncpy(char *dest, const char *str, const int len) {
  int i;

  i = 0;
  while(i < len && str[i] != '\0' && (*(dest+i) = str[i]))
    ++i;

  return i;
}

int snprintf(char *dest, size_t len, const char *fmt, ...) {
    #ifdef __x86_64__
    va_list varg;
    va_start(varg, fmt);
    #else
    int *varg = (int *)(&fmt + 1);
    #endif
    char *format, *pos;
    --len; // leave one char for the trailing '\0'

	  for (format = (char*)fmt, pos = dest; *format != '\0' && pos < dest+len; ++format) {
        if (*format == '%') {
            ++format;
            if (*format == '\0') break; // error: unterminated replace directive
            if (*format == '%') goto out;
            if( *format == 's' ) 
                pos += strncpy(pos, arg(varg, const char *), len-(pos-dest));
            else if( *format == 'd' )
                pos += itoa(pos, len-(pos-dest), arg(varg, int));
            else {
              *(pos++) = '%';
              goto out;
            }
        } else {
  out:
            *(pos++) = *format;
        }
    }
    *(dest == pos ? pos : ++pos) = '\0';
    #ifdef __x86_64__
    va_end(varg);
    #endif
    return pos-dest;
}
