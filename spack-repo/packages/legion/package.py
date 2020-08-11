# Copyright 2013-2020 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *
from spack.pkg.builtin.legion import Legion

class Legion(Legion):
    version('ctrl-rep-7', commit='7041e61e70a4a041bfee983f2aab9e061c5a6e61', preferred=True)
    version('ctrl-rep-6', commit='095be5c6e8d36a6ddb235fd079bc6e9b8d37baeb')
    version('ctrl-rep-5', commit='a204dced578258246ea0933293f4017058bc4bf5')
    version('ctrl-rep-4', commit='b66083076016c63ea8398fdb89c237880fcb0173')
    version('ctrl-rep-3', commit='572576b312509e666f2d72fafdbe9d968b1a6ac3')
    version('ctrl-rep-2', commit='96682fd8aae071ecd30a3ed5f481a9d84457a4b6')
    version('ctrl-rep-1', commit='a03671b21851d5f0d3f63210343cb61a630f4405')
    version('ctrl-rep-0', commit='177584e77036c9913d8a62e33b55fa784748759c')

