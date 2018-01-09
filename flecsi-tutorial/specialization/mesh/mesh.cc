#include <flecsi/data/data.h>
#include <specialization/mesh/mesh.h>

namespace flecsi {
namespace tutorial {

flecsi_register_data_client(mesh_t, clients, default_mesh);

} // namespace tutorial
} // namespace flecsi
