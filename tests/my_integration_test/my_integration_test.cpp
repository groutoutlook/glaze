#include <glaze/glaze.hpp>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "StringUtils.hpp"
// Define the struct with Glaze metadata
struct TestObject
{
   double x;
   double y;
};

// template <>
// struct glz::meta<TestObject>
//{
//    static constexpr auto value = glz::object("x", &TestObject::x, "y", &TestObject::y);
// };
// INFO: Registry map: string name to TestObject pointer
using RegistryType = std::map<std::string, TestObject*>;
void parse_and_set(RegistryType* registry, const std::string& str)
{
   size_t&& end_pos = str.find('=');
   auto obj_full_path = str.substr(0, end_pos);
   auto assign_value = str.substr(end_pos + 1, str.size() - end_pos + 1);
   std::vector<std::string> obj_delim_path = choc::text::splitString(obj_full_path, '.', false);
   auto obj_name = obj_delim_path[0];
   // Look up the object in the registry
   auto it = registry->find(obj_name);
   if (it == registry->end()) {
      std::cerr << "Object not found: " << obj_name << std::endl;
      return;
   }
   auto& obj_ref = *it->second;
   if (obj_delim_path.size() > 1) {
      auto&& path_to_json_pointer = "/" + choc::text::joinStrings(obj_delim_path | std::views::drop(1), "/");
      glz::read_as_json(obj_ref, path_to_json_pointer, assign_value);
   }
   else
      auto error_code = glz::read_json(obj_ref, assign_value);
}

auto parse_and_get(RegistryType* registry, const std::string& str) -> std::string
{
   size_t&& end_pos = str.find('=');
   auto obj_full_path = str.substr(0, end_pos);
   auto assign_value = str.substr(end_pos + 1, str.size() - end_pos + 1);
   std::vector<std::string> obj_delim_path = choc::text::splitString(obj_full_path, '.', false);
   auto obj_name = obj_delim_path[0];
   // Look up the object in the registry
   auto it = registry->find(obj_name);
   if (it == registry->end()) {
      std::cerr << "Object not found: " << obj_name << std::endl;
      return "Null string";
   }
   auto& obj_ref = *it->second;


   if (obj_delim_path.size() > 1) {
      auto&& path_to_json_pointer = "/" + choc::text::joinStrings(obj_delim_path | std::views::drop(1), "/");
      //glz::write_as_json(obj_ref, "/" + path_to_json_pointer, assign_value);
      std::string out{};
      glz::seek([&](auto& value) { glz::write_json(value, out); }, obj_ref, path_to_json_pointer);
      // TODO: trim quote and escape.
      return out;
   }
   else {
      std::string out{};
      auto error_code = glz::write_json(obj_ref, out);
      // TODO: trim quote and escape.
      return out;
   }
}

#include "ut/ut.hpp"

using namespace ut;

suite set_get_reflection = [] {
   "Reflect simpleinput string to object"_test = [] {
      // Create objects and register them
      TestObject obj1{0.0, 0.0};
      TestObject obj2{1.0, 1.0};
      RegistryType registry{{"obj1", &obj1}, {"obj2", &obj2}};

      // Test setting obj1.x
      parse_and_set(&registry, "obj1.x=90");
      expect(obj1.x == 90.0) << "obj1.x should be set to 90";
      expect(obj1.y == 0.0) << "obj1.y should be 0";

      // Test setting obj2
      parse_and_set(&registry, "obj2={\"x\":45,\"y\":67.8}");
      expect(obj2.x == 45.0) << "obj2.x should be set to 45";
      expect(obj2.y == 67.8) << "obj2.y should be set to 67.8";
      // return 0;
   };

   "Query simple string"_test = [] {
      // Create objects and register them
      TestObject obj1{100.0, 0.0};
      TestObject obj2{1.0, 5};
      RegistryType registry{{"obj1", &obj1}, {"obj2", &obj2}};

      // Test setting obj1.x
      auto res = parse_and_get(&registry, "obj1.x");
      expect( res == "100") << "obj1.x should be set to 100";
      // NOTE: if you want float, have to be somewhat smarter on casting.
      // TODO: also have to work on conversion to value as well, cant just pass strings around

      // Test setting obj2
      res = parse_and_get(&registry, "obj2");
      expect( res == "{\"x\":1,\"y\":5}") << "obj2 is truncated into int.";
      // INFO: either you have to specify glz::meta or you will have big headaches.
      // return 0;
   };
};


struct KeyframeData
{
    std::vector<int> start_ind;
    std::vector<int> duration;
    std::vector<int> delay;
    std::vector<std::string> renderHandle;
    std::vector<int> renderArgument_1;
    std::vector<int> renderArgument_2;

    void clear()
    {
       start_ind.clear();
       duration.clear();
       delay.clear();
       renderHandle.clear();
       renderArgument_1.clear();
       renderArgument_2.clear();
    }
    // using namespace std::string_view_literals;
    // static constexpr auto start_frame{"start_ind"sv};
    auto search_nearest_keyframe(int keyframe_index) -> int
    {
       auto result{0};
       for (auto index{0}; auto& _ind : start_ind) {
          if (keyframe_index < _ind) return (index - 1);
          index++;
       }
       return 0;
    }
    // INFO: only search for nearest keyframe. Havent accounted for
    auto get_start_ind(int keyframe_index) -> int { return start_ind[keyframe_index]; }
 };

 suite odd_csv_test = [] {
    "odd_string"_test = [] {
       KeyframeData obj{};
       std::string buffer{};

       std::string csv_data = R"(start_ind,duration,delay,renderHandle,renderArgument_1,renderArgument_2
0,400,30,gray::SimplePushPull,0,0
400,250,40,gray::SimplePushPull,1,0
650,300,80,gray::SimplePushPull,2,0)";
       auto ec = glz::read<glz::opts_csv{.layout = glz::colwise}>(obj, csv_data);
       expect(not ec) << glz::format_error(ec, buffer) << '\n';
       // this passed.
    };
    "odd_files"_test = [] {
       KeyframeData obj{};
       std::string buffer{};
       auto ec = glz::read_file_csv<glz::colwise>(obj, GLZ_TEST_DIRECTORY "/kf-data.csv", buffer);
       expect(not ec) << glz::format_error(ec, buffer) << '\n';
    };
 };

int main() {}
