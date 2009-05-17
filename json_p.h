#include "glib-object.h"
#include "json-glib.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>


typedef struct listNode {
	char *name;
	struct listNode *next;
} *PLIST;

PLIST twid_plist_init(int elements, ...);
JsonNode *twid_json_get_root(const gchar *data);
JsonNode *twid_json_get_node_by_name(JsonNode *root, const char *name)
