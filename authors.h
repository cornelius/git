#ifndef AUTHORS_H
#define AUTHORS_H

#include "string-list.h"

extern int co_authors_enabled(void);
extern void append_co_authors(struct strbuf *);

extern void read_authors_map_file(struct string_list *);

extern char *lookup_author(struct string_list *, const char *);
extern const char *expand_co_authors(struct string_list *, const char *, const char *);

#endif /* AUTHORS_H */
