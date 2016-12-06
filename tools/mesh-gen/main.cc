/*----------------------------------------------------------------------------*
 *----------------------------------------------------------------------------*/

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

template<typename T>
void write(std::ostream & stream, const T & variable) {
	stream.write(reinterpret_cast<const char *>(&variable), sizeof(T));
} // write

int main(int argc, char ** argv) {

	if(argc < 3) {
		std::cout << "Usage: " << argv[0] << " M N" << std::endl;
		std::exit(1);
	} // if

	// Switch on output type.
	size_t arg(1);
	std::string flag("-a");
	bool ascii = (flag.compare(argv[1]) == 0) && ++arg;

	const size_t M = atoi(argv[arg]);
	const size_t N = atoi(argv[arg]);

	size_t vertices = (M+1)*(N+1);
	size_t cells = M*N;

	//-------------------------------------------------------------------------//
	// Write mesh output.
	//-------------------------------------------------------------------------//

	std::stringstream meshname;
	meshname << "simple2d-" << M << "x" << N << ".msh";

	std::ios_base::openmode mode = ascii ?
		std::ofstream::out | std::ofstream::binary :
		std::ofstream::out;
	std::ofstream mesh(meshname.str(), mode);

	if(ascii) {
		mesh << vertices << " " << cells << std::endl;
	}
	else {
		write(mesh, vertices);
		write(mesh, cells);
	} // if

	double yinc = 1.0/M;
	double xinc = 1.0/N;

	// write vertices
	double yoff(0.0);
	for(size_t j(0); j<(M+1); ++j) {
		double xoff(0.0);
		for(size_t i(0); i<(N+1); ++i) {
			const double x = xoff+i*xinc;
			const double y = yoff+j*yinc;

			if(ascii) {
				mesh << xoff+i*xinc << " " << yoff + j*yinc << std::endl;
			}
			else {
				write(mesh, x);
				write(mesh, y);
			} // if
		} // for
	} // for

	// write cells
	for(size_t j(0); j<M; ++j) {
		for(size_t i(0); i<N; ++i) {
			size_t v0 = i + j*(M+1);
			size_t v1 = v0 + 1;
			size_t v2 = v0 + M+1;
			size_t v3 = v2 + 1;

			if(ascii) {
				mesh << v0 << " " << v1 << " " << v2 << " " << v3 << std::endl;
			}
			else {
				write(mesh, v0);
				write(mesh, v1);
				write(mesh, v2);
				write(mesh, v3);
			} // if
		} // for
	} // for

	//-------------------------------------------------------------------------//
	// Write tikz image.
	//-------------------------------------------------------------------------//

	std::stringstream texname;
	texname << "simple2d-" << M << "x" << N << ".tex";
	std::ofstream tex(texname.str(), std::ofstream::out);

	tex << "% Mesh visualization" << std::endl;
	tex << "\\documentclass[tikz,border=7mm]{standalone}" << std::endl;
	tex << std::endl;

	tex << "\\begin{document}" << std::endl;
	tex << std::endl;

	tex << "\\begin{tikzpicture}" << std::endl;
	tex << std::endl;

	tex << "\\draw[step=1cm,black] (0, 0) grid (" <<
		M << ", " << N << ");" << std::endl;

	size_t cell(0);
	for(size_t j(0); j<M; ++j) {
		double yoff(0.5+j);
		for(size_t i(0); i<M; ++i) {
			double xoff(0.5+i);
			tex << "\\node at (" << xoff << ", " << yoff <<
				") {" << cell++ << "};" << std::endl;
		} // for
	} // for

	tex << "\\end{tikzpicture}" << std::endl;
	tex << std::endl;

	tex << "\\end{document}" << std::endl;

	return 0;
} // main
