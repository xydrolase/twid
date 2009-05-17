#include "glib-object.h"
#include "json-glib.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "const.h"

PLIST twid_plist_init(int elements, ...);
JsonNode *twid_json_get_root(const gchar *data, JsonParser *parser);
int twid_json_get_value_by_name(JsonNode *root, const char *name, GValue *value);
int twid_json_get_node_by_path(JsonNode *root, PLIST path, GValue *value);
void twid_plist_free(PLIST list);
