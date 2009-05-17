#include "json.h"

/* json.c, for parsing twitter response in JSON */

JsonNode *
twid_json_get_root(const gchar *data, JsonParser *parser){ 
	gboolean ret_val;
	ret_val = json_parser_load_from_data(parser,
    			data,
				strlen(data),
				NULL);
	
	if (!ret_val){
		return NULL;
	}
	
	JsonNode *root = json_parser_get_root(parser);
	
	return root;
}

int
twid_json_get_value_by_name(JsonNode *root, const char *name, GValue *value){
	PLIST path = twid_plist_init(1, name);	/* wrapper here */
	
	int ret_val = twid_json_get_node_by_path(root,
		path,
		value	
	);
	
	twid_plist_free(path);
	
	return ret_val;
}

int
twid_json_get_node_by_path(JsonNode *root, PLIST path, GValue *value){
	if (JSON_NODE_TYPE(root) != JSON_NODE_OBJECT){
		return 0;
	}
	
	JsonObject *obj = json_node_get_object(root);
	JsonNode *node = NULL;
	
	if(path){
		while(path){
			if (json_object_has_member(obj, path->name)){
				
				if (node){
					/* free previous memory allocated for JsonNode */
					json_node_free(node); 
				}
				
				node = json_object_get_member(obj, path->name);
				
				if (path->next == NULL){
					/* the last element in the PLIST
					where the node should be one containing a GValue */
					if (JSON_NODE_TYPE(node) == JSON_NODE_VALUE){
						json_node_get_value(node, value);
						return 1;
					}
					else{
						return 0;
					}
				}
				else{
					/* not the last element, we should expect a JsonObject */
					if (JSON_NODE_TYPE(node) == JSON_NODE_OBJECT){
						json_object_unref(obj);	/* free the previous object */
						obj = json_node_get_object(node);
					}
					else{
						return 0;
					}
				}
			}
			else{
				return 0;
			}
		
			path = path->next; /* move to the next node */
		}
	}
}

/* free the memory occpuies by PLIST */
void twid_plist_free(PLIST list){
	PLIST pnext;
	
	while(list){
		pnext = list->next;
		free(list);
		list = pnext;
	}
}

PLIST
twid_plist_init(int num, ...){
	va_list ap;
	int n = num, len;
	char *path;
	
	if (n < 1){
		return NULL;
	}
	
	va_start(ap, num);	/*  start the variable argument */
	
	PLIST phead = (PLIST)malloc(sizeof(struct listNode));
	PLIST ptr = phead;
	
	while(n-- > 0){
		path = va_arg(ap, char *);
		len = strlen(path);
		
		ptr->name = (char *)malloc(len + 1);
		memset(ptr->name, 0, len + 1);
		strncpy(ptr->name, path, len);
		
		if (n > 0){
			PLIST pnext = (PLIST)malloc(sizeof(struct listNode));
			ptr->next = pnext;
			ptr = pnext;
		}
		else{
			ptr->next = NULL;
		}
	}
	
	va_end(ap);
	
	return phead;
}
