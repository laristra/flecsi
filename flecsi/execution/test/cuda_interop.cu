/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2019 Triad National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

//#include <cinchtest.h>
#include <assert.h>
#include <iostream>
//#include <flecsi/execution/context.h>
//#include <flecsi/execution/execution.h>

//clog_register_tag(devel_handle);

namespace flecsi {
namespace execution {

//cuda function to be tested.  calculates y=a*x where:
// a is a floating point scalar,
// x and y are device pointers to floating point arrays of equal size
__global__ void axpy(float a, float* x, float* y) {
  y[threadIdx.x] = a * x[threadIdx.x];
}

//Just launch the kernel.  Assumes x & y are existing device arrays of length len
void simpleKernelLaunch(float a, float* x, float* y, int len){
	axpy<<<1,len>>>(a, x, y);
}

// driver for test.  Prints out some information about the cuda environment,
// generates input data for the test, calls the test, then transfers data 
// back to host.  
void runCuda() {
  int runtime_ver;
  cudaRuntimeGetVersion(&runtime_ver);
  std::cout << "CUDA Runtime: " << runtime_ver << std::endl;

  int driver_ver;
  cudaDriverGetVersion(&driver_ver);
  std::cout << "CUDA Driver: " << driver_ver << std::endl;

  const int kDataLen = 4;

  float a = 2.0f;
  float host_x[kDataLen] = {1.0f, 2.0f, 3.0f, 4.0f};
  float host_y[kDataLen];

  // Copy input data to device.
  float* device_x;
  float* device_y;
  cudaMalloc(&device_x, kDataLen * sizeof(float));
  cudaMalloc(&device_y, kDataLen * sizeof(float));
  cudaMemcpy(device_x, host_x, kDataLen * sizeof(float), cudaMemcpyHostToDevice);

  // Launch the kernel.
  simpleKernelLaunch(a, device_x, device_y, kDataLen);
  //axpy<<<1, kDataLen>>>(a, device_x, device_y);

  // Copy output data to host.
  cudaDeviceSynchronize();
  cudaMemcpy(host_y, device_y, kDataLen * sizeof(float), cudaMemcpyDeviceToHost);

  // Print the results.
  for (int i = 0; i < kDataLen; ++i) {
    std::cout << "y[" << i << "] = " << host_y[i] << std::endl;
    assert(host_y[i] == a * host_x[i]);
  }
  return;
}

}} //end namespace

main(){
	flecsi::execution::runCuda();
}
