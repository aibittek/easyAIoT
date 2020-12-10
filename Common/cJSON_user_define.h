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

// 创建Json对象，序列化对象名：json_obj
#define JSON_SERIALIZE_CREATE_OBJECT_START(json_obj) \
	cJSON *json_obj = cJSON_CreateObject();

// 创建序列化数组（key, value）到json_obj对象中
#define JSON_SERIALIZE_ADD_ARRAY_TO_OBJECT(json_obj, key, value) \
	cJSON_AddItemToObject(json_obj, key, value);

// 创建序列化对象（key, value）到json_obj对象中
#define JSON_SERIALIZE_ADD_OBJECT_TO_OBJECT(json_obj, key, value) \
	cJSON_AddItemToObject(json_obj, key, value);
	
// 增加一个字符串键值对（key, value）到json_obj对象中
#define JSON_SERIALIZE_ADD_STRING_TO_OBJECT(json_obj, key, value) \
	cJSON_AddItemToObject(json_obj, key, cJSON_CreateString(value));

// 增加一个整型键值对（key, value）到json_obj对象中
#define JSON_SERIALIZE_ADD_INT_TO_OBJECT(json_obj, key, value) \
	cJSON_AddItemToObject(json_obj, key, cJSON_CreateNumber(value));

// 创建一个数组Json对象
#define JSON_SERIALIZE_CREATE_ARRAY_START(json_array) \
	cJSON *json_array = cJSON_CreateArray();

// 增加一个数组Json对象（key, value）到json_array对象中
#define JSON_SERIALIZE_ADD_ARRAY_TO_ARRAY(json_array, sub_json_array) \
	cJSON_AddItemToArray(json_array, sub_json_array);

// 增加一Json对象（key, value）到数组对象json_array中
#define JSON_SERIALIZE_ADD_OBJECT_TO_ARRAY(json_array, json_obj) \
	cJSON_AddItemToArray(json_array, json_obj);

// 创建Json结束符
#define JSON_SERIALIZE_CREATE_END(json_obj) \
	if (json_obj) cJSON_Delete(json_obj);

// JSON序列化为字符串
#define JSON_SERIALIZE_STRING(json_doc, str, len) \
	{ \
		char *s = cJSON_PrintUnformatted(json_doc); \
		if (s) { \
			snprintf(str, len, "%s", s); \
			free(s); \
			s = NULL; \
		} \
	}

// 根据json字符串json_string创建json对象json_root
#define JSON_DESERIALIZE_START(json_root, json_string, ret) \
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
// 根据json对象json_doc，获取键值为key的整型变量放到value中，返回值放入ret中，jump为程序跳出方式
#define JSON_DESERIALIZE_GET_INT(json_doc, key, value, ret, jump) \
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

// 根据json对象json_doc，获取键值为key的浮点型变量放到value中，返回值放入ret中，jump为程序跳出方式
#define JSON_DESERIALIZE_GET_DOUBLE(json_doc, key, value, ret, jump) \
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

// 根据json对象json_doc，获取键值为key的字符串指针赋值给value，返回值放入ret中，jump为程序跳出方式
#define JSON_DESERIALIZE_GET_STRING(json_doc, key, value, ret, jump) \
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
// 根据json对象json_doc，获取键值为key的字符串变量放到value中，返回值放入ret中，jump为程序跳出方式
#define JSON_DESERIALIZE_GET_STRING_COPY(json_doc, key, value, len, ret, jump) \
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

// 根据json对象json_doc，获取键值为key的Json数组变量放到value中，返回值放入ret中，jump为程序跳出方式
#define JSON_DESERIALIZE_GET_ARRAY(json_doc, key, value, ret, jump) \
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

// 根据json对象json_doc生成sub_item迭代器
#define JSON_DESERIALIZE_ARRAY_FOR_EACH_START(json_doc, sub_item, pos, total) \
	int pos, total; \
	total = cJSON_GetArraySize(json_doc); \
	for (pos = 0; pos < total; pos++) { \
		cJSON *sub_item = cJSON_GetArrayItem(json_doc, pos);
// 数组迭代结束标识
#define JSON_DESERIALIZE_ARRAY_FOR_EACH_END() \
	}
// 获取json_doc对象中的Json对象，键值为key，值放到value中，正确ret值为0，否则为负值，jump为失败后的跳转方式
#define JSON_DESERIALIZE_GET_OBJECT(json_doc, key, value, ret, jump) \
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

// Json反序列结束标识
#define JSON_DESERIALIZE_END(json_root, ret) \
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