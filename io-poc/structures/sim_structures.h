#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <unordered_map>

#include "io_structures.h"


/*!
 Information pertinent to the whole of the simulation that doesn't change per timestep
 */
class simulation_data
{

  std::unordered_map<std::string, Base_Record *> control;
public:
  template <class T>
  int add_attribute(std::string name, std::string datatype, size_t num_elements, std::string desc, T *data)
  {
    control.insert({name, new Record<T>(name, datatype, num_elements, desc, data)});
    return control.size();
  }

  void print()
  {
    for (auto it = control.begin(); it != control.end(); ++it)
      it->second->print();

    std::cout << std::endl;
  }
};


/*!
 Information pertinent to the whole of the simulation that chnages per timestep
 */
class timestep_data
{
  int timestep;

  std::unordered_map<std::string, Base_Record *> attribute; // metadata for timestep
  std::unordered_map<std::string, Base_Record *> datafield;

public:
  timestep_data() { timestep = 0; }
  timestep_data(int ts) { set_timestep(ts); }

  void set_timestep(int ts)
  {
    timestep = ts;
  }

  template <class T>
  int add_attribute(std::string name, std::string datatype, size_t num_elements, std::string desc, void *data)
  {
    attribute.insert({name, new Record<T>(name, datatype, num_elements, desc, data)});
    return attribute.size();
  }

  template <class T>
  int add_datafield(std::string name, std::string datatype, size_t num_elements, std::string desc, void *data)
  {
    datafield.insert({name, new Record<T>(name, datatype, num_elements, desc, data)});
    return datafield.size();
  }
};
