#include "./simdjson.h"
#include <iostream>
using namespace simdjson;
extern "C" {

  ondemand::parser *parser_init() {
    return new ondemand::parser();
  }

  void parser_destroy(ondemand::parser *parser) {
    delete parser;
  }

  ondemand::document *make_document() {
    return new ondemand::document();
  }

  void delete_document(ondemand::document *doc) {
    delete doc;
  }

  padded_string *make_input(char *bytes, size_t len) {
    return new padded_string(bytes, len);
  }

  void delete_input(padded_string *str) {
    delete str;
  }

  void get_iterator(
      ondemand::parser &parser, 
      padded_string &input, 
      ondemand::document &out, 
      error_code &error) {
    parser.iterate(input).tie(out, error);
  }

  const char *get_error_message(error_code &error) {
    return error_message(error);
  }

  void get_raw_json_str(
      ondemand::document &doc, 
      std::string_view &out, 
      error_code &error) {
    doc.raw_json().tie(out, error);
  }

  void get_document_value(
      ondemand::document &doc, 
      ondemand::value &out, 
      error_code &error) {
    doc.get_value().tie(out, error);
  }

  void get_object_from_value(
      ondemand::value &val, 
      ondemand::object &out, 
      error_code &error) {
      val.get_object().tie(out, error);
  }

  void get_array_from_value(
      ondemand::value &val, 
      ondemand::array &out, 
      size_t *len, 
      error_code &error) {
    val.get_array().tie(out, error);
    *len = out.count_elements();
  }

  bool get_array_iter(
      ondemand::value &val, 
      ondemand::array_iterator &iterOut, 
      error_code &error) {
    ondemand::array arr;
    auto err = val.get_array().get(arr);
    if (err != SUCCESS) { error = err; return false; } 
    arr.begin().tie(iterOut, error);
    return true;
  }

  bool arr_iter_is_done(ondemand::array_iterator &arr) {
    return arr.operator==(arr);
  }

  ondemand::value *arr_iter_get_current(
      ondemand::array_iterator &arr, 
      ondemand::value &out, 
      error_code &error) {
    arr.operator*().tie(out, error);
    return &out;
  }

  void arr_iter_move_next(
      ondemand::array_iterator &arr, 
      ondemand::value &out, 
      error_code &error) {
    ++arr;
  }

  void find_field(
      ondemand::object &obj, 
      char *key, 
      ondemand::value &out, 
      error_code &error) {
    obj.find_field(key).tie(out, error);
  }

  void find_field_unordered(
      ondemand::object &obj, 
      char *key, 
      ondemand::value &out, 
      error_code &error) {
    obj.find_field_unordered(key).tie(out, error);
  }

  void get_int(ondemand::value &val, int64_t &out, error_code &error) {
    val.get_int64().tie(out, error);
  }

  void get_double(ondemand::value &val, double &out, error_code &error) {
    val.get_double().tie(out, error);
  }

  void get_bool(ondemand::value &val, bool &out, error_code &error) {
    val.get_bool().tie(out, error);
  }

  void get_string(
      ondemand::value &val, 
      const char **out, 
      int64_t &len, 
      error_code &error) {
    std::string_view buf;
    val.get_string().tie(buf, error);
    *out = buf.data();
    len = buf.length();
  }

  ondemand::json_type get_json_type(ondemand::value &val) {
    return val.type();
  }

  void reset_array(ondemand::array &arr) {
    arr.reset();
  }

  void reset_object(ondemand::object &obj) {
    obj.reset();
  }

  bool is_null(ondemand::value &val) {
    return val.is_null();
  }

}
