#ifndef CJSON_USER_DEFINE_H
#define CJSON_USER_DEFINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cJSON.h"

#define JSON_CTRL_NULL 		0
#define JSON_CTRL_BREAK		1
#define JSON_CTRL_CONTINUE	2

#define JSON_CTRL(c) \
	{ \
		if (JSON_CTRL_NULL == c); \
		else if (JSON_CTRL_BREAK == c) break; \
		else if (JSON_CTRL_CONTINUE) continue; \
	}

#define JSON_DESERIALIZE_CREATE_OBJECT_START(json_obj) \
	cJSON *json_obj = cJSON_CreateObject();
#define JSON_DESERIALIZE_ADD_ARRAY_TO_OBJECT(json_obj, key, value) \
	cJSON_AddItemToObject(json_obj, key, value);
#define JSON_DESERIALIZE_ADD_OBJECT_TO_OBJECT(json_obj, key, value) \
	cJSON_AddItemToObject(json_obj, key, value);
#define JSON_DESERIALIZE_ADD_STRING_TO_OBJECT(json_obj, key, value) \
	cJSON_AddItemToObject(json_obj, key, cJSON_CreateString(value));
#define JSON_DESERIALIZE_ADD_INT_TO_OBJECT(json_obj, key, value) \
	cJSON_AddItemToObject(json_obj, key, cJSON_CreateNumber(value));
#define JSON_DESERIALIZE_CREATE_ARRAY_START(json_array) \
	cJSON *json_array = cJSON_CreateArray();
#define JSON_DESERIALIZE_ADD_ARRAY_TO_ARRAY(json_array, sub_json_array) \
	cJSON_AddItemToArray(json_array, sub_json_array);
#define JSON_DESERIALIZE_ADD_OBJECT_TO_ARRAY(json_array, json_obj) \
	cJSON_AddItemToArray(json_array, json_obj);
#define JSON_DESERIALIZE_CREATE_END(json_obj) \
	if (json_obj) cJSON_Delete(json_obj);
#define JSON_DESERIALIZE_STRING(json_doc, str, len) \
	{ \
		char *s = cJSON_PrintUnformatted(json_doc); \
		if (s) { \
			snprintf(str, len, "%s", s); \
			free(s); \
			s = NULL; \
		} \
	}

#define JSON_SERIALIZE_START(json_root, json_string, ret) \
	{ \
		cJSON *json_root = NULL; \
		do { \
			if (NULL == json_string) { \
				ret = -1; \
				break; \
			} \
			json_root = cJSON_Parse(json_string); \
			if (NULL == json_root) { \
				ret = -2; \
				break; \
			}
#define JSON_SERIALIZE_GET_INT(json_doc, key, value, ret, jump) \
	if (NULL == json_doc) { \
		ret = -1; \
		printf("%s error\n", key); \
		break; \
	} \
	if (cJSON_HasObjectItem(json_doc, key)) { \
		value = cJSON_GetObjectItem(json_doc, key)->valueint; \
	} \
	else { \
		ret = -3; \
		JSON_CTRL(jump); \
	}
#define JSON_SERIALIZE_GET_DOUBLE(json_doc, key, value, ret, jump) \
	if (NULL == json_doc) { \
		ret = -1; \
		printf("%s error\n", key); \
		break; \
	} \
	if (cJSON_HasObjectItem(json_doc, key)) { \
		value = cJSON_GetObjectItem(json_doc, key)->valuedouble; \
	} \
	else { \
		ret = -3; \
		JSON_CTRL(jump); \
	}
#define JSON_SERIALIZE_GET_STRING(json_doc, key, value, ret, jump) \
	if (NULL == json_doc) { \
		printf("%s error\n", key); \
		ret = -1; \
		break; \
	} \
	if (cJSON_HasObjectItem(json_doc, key)) { \
		value = cJSON_GetObjectItem(json_doc, key)->valuestring; \
	} \
	else { \
		ret = -4; \
		JSON_CTRL(jump); \
	}
#define JSON_SERIALIZE_GET_STRING_COPY(json_doc, key, value, len, ret, jump) \
	if (NULL == json_doc) { \
		printf("%s error\n", key); \
		ret = -1; \
		break; \
	} \
	if (cJSON_HasObjectItem(json_doc, key)) { \
		snprintf(value, len, "%s", cJSON_GetObjectItem(json_doc, key)->valuestring); \
	} \
	else { \
		ret = -5; \
		JSON_CTRL(jump); \
	}
#define JSON_SERIALIZE_GET_ARRAY(json_doc, key, value, ret, jump) \
	cJSON *value = NULL; \
	if (NULL == json_doc) { \
		ret = -1; \
		break; \
	} \
	if (cJSON_HasObjectItem(json_doc, key)) { \
		value = cJSON_GetObjectItem(json_doc, key); \
	} \
	else { \
		ret = -6; \
		JSON_CTRL(jump); \
	}
#define JSON_SERIALIZE_ARRAY_FOR_EACH_START(json_doc, sub_item, pos, total) \
	int pos, total; \
	total = cJSON_GetArraySize(json_doc); \
	for (pos = 0; pos < total; pos++) { \
		cJSON *sub_item = cJSON_GetArrayItem(json_doc, pos);
#define JSON_SERIALIZE_ARRAY_FOR_EACH_END() \
	}
#define JSON_SERIALIZE_GET_OBJECT(json_doc, key, value, ret, jump) \
	cJSON *value = NULL; \
	if (NULL == json_doc) { \
		ret = -1; \
		break; \
	} \
	if (cJSON_HasObjectItem(json_doc, key)) { \
		value = cJSON_GetObjectItem(json_doc, key); \
	} \
	else { \
		ret = -7; \
		JSON_CTRL(jump); \
	}
#define JSON_SERIALIZE_END(json_root, ret) \
			ret = 0; \
		} while (0); \
		if (NULL != json_root) { \
			cJSON_Delete(json_root); \
			json_root = NULL; \
		} \
	}

#ifdef __cplusplus
}
#endif

#endif // CJSON_USER_DEFINE_H