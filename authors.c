#include "cache.h"
#include "config.h"
#include "authors.h"
#include "string.h"
#include "strbuf.h"

int co_authors_enabled(void)
{
	return 1;
}

void append_co_authors(struct strbuf *sb)
{
	const char *authors_config = NULL;
	struct string_list authors_map = STRING_LIST_INIT_NODUP;

	if (git_config_get_string_const("authors.current", &authors_config))
		return;

	read_authors_map_file(&authors_map);

	strbuf_addstr(sb, "\n\n");
	strbuf_addstr(sb, expand_co_authors(&authors_map, authors_config, "Co-authored-by: "));
}

void read_authors_map_line(struct string_list *map, char *buffer)
{
	int len = strlen(buffer);

	if (len && buffer[len - 1] == '\n')
		buffer[--len] = 0;

	string_list_insert(map, xstrdup(buffer));
}

void read_authors_map_file(struct string_list *map)
{
	char buffer[1024];
	FILE *f;
	const char *filename;
	const char *home;

	home = getenv("HOME");
	if (!home)
		die("HOME not set");

	filename = mkpathdup("%s/.git_authors_map", home);

	f = fopen(filename, "r");
	if (!f) {
		if (errno == ENOENT) {
			warning("~/.git_authors_map does not exist");
			return;
		}
		die_errno("unable to open authors map at %s", filename);
	}

	while (fgets(buffer, sizeof(buffer), f) != NULL)
		read_authors_map_line(map, buffer);
	fclose(f);
}

char *lookup_author(struct string_list *map, const char *author_abbr)
{
	struct string_list_item *author_item = NULL;
	struct string_list_item *item;

	for_each_string_list_item(item, map) {
		if (strncmp(item->string, author_abbr, strlen(author_abbr)) == 0 &&
		    strlen(item->string) > strlen(author_abbr) &&
		    *(item->string + strlen(author_abbr)) == ' ') {
			author_item = item;
			break;
		}
	}

	if (!author_item)
		return NULL;

	return xstrdup(author_item->string + strlen(author_abbr) + 1);
}

const char *expand_co_authors(struct string_list *map, const char *author_shorts, const char *prefix)
{
	int i;
	const char *author_start = author_shorts;
	const char *author_end;
	char *author_short, *expanded_author;
	static struct strbuf expanded_authors = STRBUF_INIT;

	strbuf_reset(&expanded_authors);

	for (i = 0; i <= strlen(author_shorts); i++) {
		author_end = author_shorts + i;
		if (*author_end == ' ' || *author_end == '\0') {
			author_short = xstrndup(author_start, author_end - author_start);
			expanded_author = lookup_author(map, author_short);
			if (!expanded_author)
				die("Could not expand author '%s'. Add it to the file ~/.git_authors_map.", author_short);
			else {
				strbuf_addstr(&expanded_authors, prefix);
				strbuf_addstr(&expanded_authors, expanded_author);
				strbuf_addch(&expanded_authors, '\n');

				free(expanded_author);
			}
			free(author_short);

			author_start = author_end + 1;
		}
	}

	return expanded_authors.buf;
}
