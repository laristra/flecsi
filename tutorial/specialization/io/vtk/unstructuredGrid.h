#if defined(ENABLE_VTK)

#ifndef _UNSTRUCTURED_GRID_H_
#define _UNSTRUCTURED_GRID_H_

#include <iostream>
#include <string>

#include <mpi.h>

#include <vtkCellArray.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkGenericDataArray.h>
#include <vtkIntArray.h>
#include <vtkMPIController.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSOADataArrayTemplate.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLPUnstructuredGridWriter.h>

namespace vtkOutput {

class UnstructuredGrid
{
  vtkSmartPointer<vtkXMLPUnstructuredGridWriter> writer;
  vtkSmartPointer<vtkUnstructuredGrid> uGrid;

  vtkSmartPointer<vtkPoints> pnts;
  vtkSmartPointer<vtkCellArray> cells;
  vtkIdType idx;

public:
  UnstructuredGrid();
  ~UnstructuredGrid(){};

  vtkSmartPointer<vtkUnstructuredGrid> getUGrid() {
    return uGrid;
  }

  // Topology
  template<typename T>
  void addPoint(T * pointData);
  void pushPointsToGrid(int cellType);
  void setPoints(vtkSmartPointer<vtkPoints> _pnts,
    vtkSmartPointer<vtkCellArray> _cells,
    int cellType);

  // Data
  template<typename T>
  void addScalarData(std::string scalarName, int numPoints, T * data);
  template<typename T>
  void addVectorData(std::string scalarName,
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
};

inline UnstructuredGrid::UnstructuredGrid() {
  writer = vtkSmartPointer<vtkXMLPUnstructuredGridWriter>::New();
  uGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();

  pnts = vtkSmartPointer<vtkPoints>::New();
  cells = vtkSmartPointer<vtkCellArray>::New();

  idx = 0;
}

//
// Topology
template<typename T>
inline void
UnstructuredGrid::addPoint(T * pointData) {
  pnts->InsertPoint(idx, pointData);
  cells->InsertNextCell(1, &idx);
  idx++;
}

inline void
UnstructuredGrid::pushPointsToGrid(int cellType) {
  uGrid->SetPoints(pnts);
  uGrid->SetCells(cellType, cells);
}

inline void
UnstructuredGrid::setPoints(vtkSmartPointer<vtkPoints> _pnts,
  vtkSmartPointer<vtkCellArray> _cells,
  int cellType) {
  uGrid->SetPoints(_pnts);
  uGrid->SetCells(cellType, _cells);
}

// Attributes
template<typename T>
inline void
UnstructuredGrid::addFieldScalar(std::string fieldName, T * data) {
  vtkAOSDataArrayTemplate<T> * temp = vtkAOSDataArrayTemplate<T>::New();

  temp->SetNumberOfTuples(1);
  temp->SetNumberOfComponents(1);
  temp->SetName(fieldName.c_str());
  temp->SetArray(data, 1, false, true);

  uGrid->GetFieldData()->AddArray(temp);
}

//
// Data
template<typename T>
inline void
UnstructuredGrid::addScalarData(std::string varName, int numPoints, T * data) {
  vtkSOADataArrayTemplate<T> * temp = vtkSOADataArrayTemplate<T>::New();

  temp->SetNumberOfTuples(numPoints);
  temp->SetNumberOfComponents(1);
  temp->SetName(varName.c_str());
  temp->SetArray(0, data, numPoints, false, true);
  uGrid->GetPointData()->AddArray(temp);

  temp->Delete();
}

template<typename T>
inline void
UnstructuredGrid::addVectorData(std::string varName,
  int numPoints,
  int numComponents,
  T * data) {
  vtkAOSDataArrayTemplate<T> * temp = vtkAOSDataArrayTemplate<T>::New();

  temp->SetNumberOfTuples(numPoints);
  temp->SetNumberOfComponents(numComponents);
  temp->SetName(varName.c_str());
  temp->SetArray(data, numPoints * numComponents, false, true);
  uGrid->GetPointData()->AddArray(temp);

  temp->Delete();
}

//
// Writing
inline void
UnstructuredGrid::writeParts(int numPieces,
  int startPiece,
  int endPiece,
  std::string fileName) {
  writer->SetNumberOfPieces(numPieces);
  writer->SetStartPiece(startPiece);
  writer->SetEndPiece(endPiece);

  write(fileName, 1);
}

inline void
UnstructuredGrid::write(std::string fileName, int parallel) {
  std::string outputFilename;

  if(parallel == 1)
    outputFilename = fileName + ".pvtu";
  else
    outputFilename = fileName + ".vtu";

  writer->SetDataModeToBinary();
  writer->SetCompressor(nullptr);
  writer->SetFileName(outputFilename.c_str());

#if VTK_MAJOR_VERSION <= 5
  writer->SetInput(uGrid);
#else
  writer->SetInputData(uGrid);
#endif

  writer->Write();
}

} // namespace vtkOutput

#endif //_UNSTRUCTURED_GRID_H_
#endif // ENABLE_VTK
