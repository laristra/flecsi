#ifndef flexi_io_h
#define flexi_io_h

#include <exodusII.h>

template<typename MT>
class meshIO {
 public:
  void read(const std::string filename, const MT & m);
  void write(const std::string filename, const MT & m);

 private:
};

template<typename MT>
void meshIO<MT>::read(const std::string filename, const MT & m) {
  std::cout << "Reading mesh from file: " << filename << std::endl;
}

template<typename MT>
void meshIO<MT>::write(const std::string filename, const MT & m) {

  std::cout << "Writing mesh to file: " << filename << std::endl;

  int CPU_word_size = 0, IO_word_size = 0;
  int exoid = ex_create(filename.c_str(), EX_CLOBBER,
    &CPU_word_size, &IO_word_size);

  auto d = m.dimension();
  auto num_nodes = m.numVertices();
  auto num_elem = m.numCells();
  //int num_elem_blk =
  //int num_node_sets =
  //int num_side_sets = 
  //
  //int error = ex_put_init(exoid, "test", d, num_nodes, num_elem, num_elem_blk, num_node_sets, num_side_sets);

}


#endif // flexi_io_h
