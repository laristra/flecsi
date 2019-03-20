#if defined(ENABLE_VTK)

#ifndef _STRUCTURED_GRID_H_
#define _STRUCTURED_GRID_H_

#include <iostream>
#include <string>

#include <vtkAOSDataArrayTemplate.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDebugLeaksManager.h>
#include <vtkDoubleArray.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSOADataArrayTemplate.h>
#include <vtkSmartPointer.h>
#include <vtkStructuredGrid.h>
#include <vtkTrivialProducer.h>
#include <vtkXMLPStructuredGridWriter.h>

namespace vtkOutput {

class StructuredGrid
{
  vtkSmartPointer<vtkXMLPStructuredGridWriter> writer;
  vtkSmartPointer<vtkStructuredGrid> strucGrid;

  vtkSmartPointer<vtkPoints> pnts;

  int dims[3];
  int extents[6];
  int wholeExtents[6];

public:
  StructuredGrid();
  StructuredGrid(int x, int y, int z);
  ~StructuredGrid(){};

  vtkSmartPointer<vtkStructuredGrid> getGrid() {
    return strucGrid;
  }

  // Topology
  template<typename T>
  void addPoint(T * pointData, int _dims = 3);
  void setPoints(vtkSmartPointer<vtkPoints> _pnts) {
    strucGrid->SetPoints(_pnts);
  }
  void pushPointsToGrid() {
    strucGrid->SetPoints(pnts);
  }

  void setDims(int x, int y, int z);
  void setExtents(int minX, int maxX, int minY, int maxY, int minZ, int maxZ);
  void
  setWholeExtents(int minX, int maxX, int minY, int maxY, int minZ, int maxZ);

  // Data
  template<typename T>
  void addScalarPointData(std::string varName, int numPoints, T * data);
  template<typename T>
  void addVectorPointData(std::string varName,
    int numPoints,
    int numComponents,
    T * data);
  template<typename T>
  void addScalarCellData(std::string varName, int numPoints, T * data);
  template<typename T>
  void addVectorCellData(std::string varName,
    int numPoints,
    int numComponents,
    T * data);
  template<typename T>
  void addFieldScalar(std::string fieldName, T * data);

  // Writing
  void writeParts(int numPieces,
    int startPiece,
    int SetEndPiece,
    std::string fileName);
  void write(std::string fileName, int parallel = 0);

  // Set n Get
  int getNumVertices() {
    return strucGrid->GetNumberOfPoints();
  }
  int getNumCells() {
    return strucGrid->GetNumberOfCells();
  }
};

inline StructuredGrid::StructuredGrid() {
  writer = vtkSmartPointer<vtkXMLPStructuredGridWriter>::New();
  strucGrid = vtkSmartPointer<vtkStructuredGrid>::New();

  pnts = vtkSmartPointer<vtkPoints>::New();
}

inline StructuredGrid::StructuredGrid(int x, int y, int z) {
  writer = vtkSmartPointer<vtkXMLPStructuredGridWriter>::New();
  strucGrid = vtkSmartPointer<vtkStructuredGrid>::New();

  pnts = vtkSmartPointer<vtkPoints>::New();
  setDims(x, y, z);
}

inline void
StructuredGrid::setDims(int x, int y, int z) {
  dims[0] = x;
  dims[1] = y;
  dims[2] = z;
  strucGrid->SetDimensions(dims);
}

inline void
StructuredGrid::setExtents(int minX,
  int maxX,
  int minY,
  int maxY,
  int minZ,
  int maxZ) {
  extents[0] = minX;
  extents[1] = maxX;
  extents[2] = minY;
  extents[3] = maxY;
  extents[4] = minZ;
  extents[5] = maxZ;

  strucGrid->SetExtent(extents);
}

inline void
StructuredGrid::setWholeExtents(int minX,
  int maxX,
  int minY,
  int maxY,
  int minZ,
  int maxZ) {
  wholeExtents[0] = minX;
  wholeExtents[1] = maxX;
  wholeExtents[2] = minY;
  wholeExtents[3] = maxY;
  wholeExtents[4] = minZ;
  wholeExtents[5] = maxZ;
}

template<typename T>
inline void
StructuredGrid::addPoint(T * pointData, int _dims) {
  if(_dims == 1)
    pnts->InsertNextPoint(pointData[0], 0, 0);
  else if(_dims == 2)
    pnts->InsertNextPoint(pointData[0], pointData[1], 0);
  else
    pnts->InsertNextPoint(pointData[0], pointData[1], pointData[2]);
}

// Attributes
template<typename T>
inline void
StructuredGrid::addFieldScalar(std::string fieldName, T * data) {
  vtkAOSDataArrayTemplate<T> * temp = vtkAOSDataArrayTemplate<T>::New();

  temp->SetNumberOfTuples(1);
  temp->SetNumberOfComponents(1);
  temp->SetName(fieldName.c_str());
  temp->SetArray(data, 1, false, true);

  strucGrid->GetFieldData()->AddArray(temp);
}

//
// Data
template<typename T>
inline void
StructuredGrid::addScalarPointData(std::string varName,
  int numPoints,
  T * data) {
  vtkSOADataArrayTemplate<T> * temp = vtkSOADataArrayTemplate<T>::New();

  temp->SetNumberOfTuples(numPoints);
  temp->SetNumberOfComponents(1);
  temp->SetName(varName.c_str());
  temp->SetArray(0, data, numPoints, false, true);
  strucGrid->GetPointData()->AddArray(temp);

  temp->Delete();
}

template<typename T>
inline void
StructuredGrid::addVectorPointData(std::string varName,
  int numPoints,
  int numComponents,
  T * data) {
  vtkAOSDataArrayTemplate<T> * temp = vtkAOSDataArrayTemplate<T>::New();

  temp->SetNumberOfTuples(numPoints);
  temp->SetNumberOfComponents(numComponents);
  temp->SetName(varName.c_str());
  temp->SetArray(data, numPoints * numComponents, false, true);
  strucGrid->GetPointData()->AddArray(temp);

  temp->Delete();
}

template<typename T>
inline void
StructuredGrid::addScalarCellData(std::string varName,
  int numPoints,
  T * data) {
  vtkSOADataArrayTemplate<T> * temp = vtkSOADataArrayTemplate<T>::New();

  temp->SetNumberOfComponents(1);
  temp->SetNumberOfTuples(numPoints);
  temp->SetName(varName.c_str());
  temp->SetArray(0, data, numPoints, false, true);
  strucGrid->GetCellData()->AddArray(temp);

  temp->Delete();
}

template<typename T>
inline void
StructuredGrid::addVectorCellData(std::string varName,
  int numPoints,
  int numComponents,
  T * data) {
  vtkAOSDataArrayTemplate<T> * temp = vtkAOSDataArrayTemplate<T>::New();

  temp->SetNumberOfComponents(numComponents);
  temp->SetNumberOfTuples(numPoints);
  temp->SetName(varName.c_str());
  temp->SetArray(data, numPoints * numComponents, false, true);
  strucGrid->GetCellData()->AddArray(temp);

  temp->Delete();
}

//
// Writing
inline void
StructuredGrid::writeParts(int numPieces,
  int startPiece,
  int endPiece,
  std::string fileName) {
  writer->SetNumberOfPieces(numPieces);
  writer->SetStartPiece(startPiece);
  writer->SetEndPiece(endPiece);

  write(fileName, 1);
}

inline void
StructuredGrid::write(std::string fileName, int parallel) {
  std::string outputFilename;
  if(parallel == 1) {
    outputFilename = fileName + ".pvts";

    vtkNew<vtkTrivialProducer> tp;
    tp->SetOutput(strucGrid);
    tp->SetWholeExtent(wholeExtents[0], wholeExtents[1], wholeExtents[2],
      wholeExtents[3], wholeExtents[4], wholeExtents[5]);

    writer->SetInputConnection(tp->GetOutputPort());
  }
  else
    outputFilename = fileName + ".vts";

  writer->SetFileName(outputFilename.c_str());

#if VTK_MAJOR_VERSION <= 5
  writer->SetInput(strucGrid);
#else
  writer->SetInputData(strucGrid);
#endif

  writer->Write();
}

} // namespace vtkOutput

#endif //_STRUCTURED_GRID_H_
#endif // ENABLE_VTK
