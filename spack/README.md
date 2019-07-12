# Spack Environments
The purpose of these environments are to generate a build environment for flecsi

## To Use

1. Set up spack environment on desired platform
2. Copy desired environment file from this directory to your build directory as `spack.yaml`
3. Call `spack concretize -f` to generate a concretization
4. Use `spack install` to generate any necessary spackages (Delete `.spack.env` if re-installing)
5. Use `spack find -p boost` to determine what to pass as BOOST_ROOT
6. Configure and build as before. In the case of the MPI backend the configuration command is `cmake -DBOOST_ROOT=${PATH_TO_BOOST} -DFLECSI_RUNTIME_MODEL=mpi ..`

