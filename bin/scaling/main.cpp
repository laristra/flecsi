#include <iostream>
#include <string>
#include <fstream>
#include <cassert>
#include <streambuf>
#include <sstream>

#define np(X)                                                             \
 std::cout << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__  \
           << ": " << #X << " = " << (X) << std::endl

using namespace std;

const size_t num_domains = 10;
const size_t num_dimensions = 2;

void find_replace(string& source, const string& find, const string& replace){
  size_t pos = 0;

  for(;;){
    pos = source.find(find, pos);

    if(pos == string::npos){
      break;
    }

    source.replace(pos, find.length(), replace);
    pos += replace.length();
  }
}

void find_replace_indent(string& source, const string& find,
  const string& replace){
  size_t pos = 0;

  for(;;){
    pos = source.find(find, pos);

    if(pos == string::npos){
      break;
    }

    string indent;
    int i = int(pos) - 1;
    while(i >= 0){
      if(source[i] != ' '){
        break;
      }
      indent += ' ';
      --i;
    }

    string r = replace;
    find_replace(r, "\n", "\n" + indent);

    source.replace(pos, find.length(), r);
    pos += replace.length();
  }
}

string file_to_string(const string& path){
  ifstream t(path);
  return (string((istreambuf_iterator<char>(t)),
         (istreambuf_iterator<char>())));
}

template<class T>
string to_string(const T& value){
  stringstream sstr;
  sstr << value;
  return sstr.str(); 
}

template<class T>
void set(string& source, const string& find, const T& replace){
  find_replace_indent(source, find, to_string(replace));
}

string open(const string& path){
  string str = file_to_string(path);
  set(str, "$NUM_DOMAINS", num_domains);
  set(str, "$NUM_DIMENSIONS", num_dimensions);
  return str;
}

string use_domain(const string& str, size_t domain){
  string ret = str;
  set(ret, "$DOMAIN", domain);
  return ret;
}

int main(int argc, char** argv){
  string code = open("main.cc");
  
  string vertex = open("vertex.cc");
  string edge = open("edge.cc");
  string cell = open("cell.cc");
  string entity_types = open("entity_types.cc");
  string connectivities = open("connectivities.cc");
  string create_entity = open("create_entity.cc");
  string init = open("init.cc");
  string traversal = open("traversal.cc");

  {
    stringstream sstr;

    for(size_t d = 0; d < num_domains; ++d){

      string v = use_domain(vertex, d);
      string e = use_domain(edge, d);
      string c = use_domain(cell, d);

      sstr << v << e << c;    
    }

    set(code, "$ENTITIES", sstr.str());
  }

  {
    stringstream sstr;

    for(size_t d = 0; d < num_domains; ++d){
      sstr << use_domain(entity_types, d);

      if(d < num_domains - 1){
        sstr << ",\n";
      }
    }

    set(code, "$ENTITY_TYPES", sstr.str());
  }

  {
    stringstream sstr;

    for(size_t d = 0; d < num_domains; ++d){
      sstr << use_domain(connectivities, d);

      if(d < num_domains - 1){
        sstr << ",\n";
      }
    }

    set(code, "$CONNECTIVITIES", sstr.str());
  }

  {
    stringstream sstr;

    for(size_t d = 0; d < num_domains; ++d){
      sstr << use_domain(create_entity, d);
    }

    set(code, "$CREATE_ENTITY", sstr.str());
  }

  {
    stringstream sstr;

    for(size_t d = 0; d < num_domains; ++d){
      sstr << use_domain(init, d);
    }

    set(code, "$INIT", sstr.str());
  }

  {
    stringstream sstr;

    for(size_t d = 0; d < num_domains; ++d){
      sstr << use_domain(traversal, d);
    }

    set(code, "$TRAVERSAL", sstr.str());
  }

  cout << code;

  return 0;
}
