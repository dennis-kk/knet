#include "helper.h"
#include "knet.h"

CASE(Test_Rpc_Number) {
    krpc_object_t* number = krpc_object_create();

    krpc_number_set_i8(number, 16);
    EXPECT_TRUE(krpc_object_check_type(number, krpc_type_i8));
    EXPECT_TRUE(krpc_object_check_type(number, krpc_type_number));
    EXPECT_TRUE(16 == krpc_number_get_i8(number));

    krpc_number_set_i16(number, 16);
    EXPECT_TRUE(krpc_object_check_type(number, krpc_type_i16));
    EXPECT_TRUE(krpc_object_check_type(number, krpc_type_number));
    EXPECT_TRUE(16 == krpc_number_get_i16(number));

    krpc_number_set_i32(number, 16);
    EXPECT_TRUE(krpc_object_check_type(number, krpc_type_i32));
    EXPECT_TRUE(krpc_object_check_type(number, krpc_type_number));
    EXPECT_TRUE(16 == krpc_number_get_i32(number));

    krpc_number_set_i64(number, 16);
    EXPECT_TRUE(krpc_object_check_type(number, krpc_type_i64));
    EXPECT_TRUE(krpc_object_check_type(number, krpc_type_number));
    EXPECT_TRUE(16 == krpc_number_get_i64(number));

    krpc_number_set_ui8(number, 16);
    EXPECT_TRUE(krpc_object_check_type(number, krpc_type_ui8));
    EXPECT_TRUE(krpc_object_check_type(number, krpc_type_number));
    EXPECT_TRUE(16 == krpc_number_get_ui8(number));

    krpc_number_set_ui16(number, 16);
    EXPECT_TRUE(krpc_object_check_type(number, krpc_type_ui16));
    EXPECT_TRUE(krpc_object_check_type(number, krpc_type_number));
    EXPECT_TRUE(16 == krpc_number_get_ui16(number));

    krpc_number_set_ui32(number, 16);
    EXPECT_TRUE(krpc_object_check_type(number, krpc_type_ui32));
    EXPECT_TRUE(krpc_object_check_type(number, krpc_type_number));
    EXPECT_TRUE(16 == krpc_number_get_ui32(number));

    krpc_number_set_ui64(number, 16);
    EXPECT_TRUE(krpc_object_check_type(number, krpc_type_ui64));
    EXPECT_TRUE(krpc_object_check_type(number, krpc_type_number));
    EXPECT_TRUE(16 == krpc_number_get_ui64(number));

    krpc_number_set_f32(number, 16.0f);
    EXPECT_TRUE(krpc_object_check_type(number, krpc_type_f32));
    EXPECT_TRUE(krpc_object_check_type(number, krpc_type_number));
    EXPECT_TRUE(16.0f == krpc_number_get_f32(number));

    krpc_number_set_f64(number, 16.0f);
    EXPECT_TRUE(krpc_object_check_type(number, krpc_type_f64));
    EXPECT_TRUE(krpc_object_check_type(number, krpc_type_number));
    EXPECT_TRUE(16.0f == krpc_number_get_f64(number));

    krpc_object_destroy(number);
}

CASE(Test_Rpc_String) {
    krpc_object_t* string = krpc_object_create();

    // 第一次设置决定长度及类型，后面设置不能改变长度
    krpc_string_set(string, "12345");
    EXPECT_TRUE(krpc_object_check_type(string, krpc_type_string));
    EXPECT_TRUE(std::string("12345") == krpc_string_get(string));
    EXPECT_TRUE(krpc_string_get_size(string) == 6);

    krpc_string_set(string, "123456");
    EXPECT_TRUE(krpc_object_check_type(string, krpc_type_string));
    // 长度不可改变
    EXPECT_TRUE(std::string("12345") == krpc_string_get(string));
    EXPECT_TRUE(krpc_string_get_size(string) == 6);

    krpc_string_set_s(string, "12345", 6);
    EXPECT_TRUE(krpc_object_check_type(string, krpc_type_string));
    EXPECT_TRUE(std::string("12345") == krpc_string_get(string));
    EXPECT_TRUE(krpc_string_get_size(string) == 6);

    krpc_string_set_s(string, "123456", 7);
    EXPECT_TRUE(krpc_object_check_type(string, krpc_type_string));
    // 长度不可改变
    EXPECT_TRUE(std::string("12345") == krpc_string_get(string));
    EXPECT_TRUE(krpc_string_get_size(string) == 6);

    krpc_object_destroy(string);
}
