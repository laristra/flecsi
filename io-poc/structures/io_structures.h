#pragma once

#include <string>
#include <sstream>
#include <iostream>


/*!
 Baseclass to allow us to template types
 */
class Base_Record
{
  public:
  virtual ~Base_Record(){};
  virtual void print(){};
};



/*!
 Basic unit of storage information
 */
template <typename T>
class Record : public Base_Record
{
  public:
  std::string name;
  std::string datatype; // int, float, ...
  size_t num_elements;
  std::string desc;     // brief description of the record
  T *data;              // zero copy data storage

  Record() {}

  Record(std::string _name, std::string _datatype, size_t _num_elements, std::string _desc, T *_data)
  {
    init(_name, _datatype, _num_elements, _desc, _data);
  }

  void init(std::string _name, std::string _datatype, size_t _num_elements, std::string _desc, T *_data)
  {
    name = _name;
    datatype = _datatype;
    num_elements = _num_elements;
    desc = _desc;
    data = _data;
  }

  std::string serialize_header()
  {
    std::stringstream ss;
    ss << name << "," << datatype << "," << num_elements << "," << desc;

    return ss.str();
  }

  std::string serialize()
  {
    std::stringstream ss;
    ss << name << " " << datatype << " " << num_elements << " " << desc << " ";

    for (int i = 0; i < num_elements; i++)
      ss << data[i] << " ";

    return ss.str();
  }

  void deserialize(std::string serialized)
  {
    std::stringstream ss(serialized);

    ss >> name >> datatype >> num_elements >> desc;

    data = new T[num_elements];
    for (int i = 0; i < num_elements; i++)
      ss >> data[i];
  }

  void print()
  {
    std::cout << name << " " << datatype << " " << num_elements << " " << desc << " ";

    for (int i = 0; i < num_elements; i++)
      std::cout << data[i] << " ";

    std::cout << "\n";
  }
};