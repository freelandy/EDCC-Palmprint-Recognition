#pragma once

#include <gtest/gtest.h>
#include <EDCC.h>

#ifdef _WINDOWS

#define CORRECT_CONFIG_PATH "..\\test\\example\\config\\config_correct.json"
#define NOT_EXISTS_CONFIG_PATH "..\\test\\example\\config\\config.json"
#define PARAMS_ERROR_CONFIG_PATH "..\\test\\example\\config\\config_params_error.json"
#define PARAMS_MISS_CONFIG_PATH "..\\test\\example\\config\\config_params_miss.json"
#define PARAMS_OVER_CONFIG_PATH "..\\test\\example\\config\\config_params_over.json"
#define PARAMS_NO_DEFAULT_CONFIG_PATH "..\\test\\example\\config\\config_params_no_default.json"

#define CORRECT_GROUP_PATH "..\\test\\example\\group\\trainingSetGroup_Windows.json"
#define NOT_EXISTS_GROUP_PATH "..\\test\\example\\group\\trainingSetGroupNOT_Windows.json"
#define WRONG_FORMAT_GROUP_PATH "..\\test\\example\\group\\trainingSetGroup_WrongFormat_Windows.json"

#define FEATURES_OUTPUT_PATH "..\\test\\example\\features\\features.json"

#else

#define CORRECT_CONFIG_PATH "../test/example/config/config_correct.json"
#define NOT_EXISTS_CONFIG_PATH "../test/example/config/config.json"
#define PARAMS_ERROR_CONFIG_PATH "../test/example/config/config_params_error.json"
#define PARAMS_MISS_CONFIG_PATH "../test/example/config/config_params_miss.json"
#define PARAMS_OVER_CONFIG_PATH "../test/example/config/config_params_over.json"
#define PARAMS_NO_DEFAULT_CONFIG_PATH "../test/example/config/config_params_no_default.json"

#define CORRECT_GROUP_PATH "../test/example/group/trainingSetGroup_Linux.json"
#define NOT_EXISTS_GROUP_PATH "../test/example/group/trainingSetGroupNOT_Linux.json"
#define WRONG_FORMAT_GROUP_PATH "../test/example/group/trainingSetGroup_WrongFormat_Linux.json"

#define FEATURES_OUTPUT_PATH "../test/example/features/features.json"

#endif

class ft_edcc_base : public testing::Test {
public:

};