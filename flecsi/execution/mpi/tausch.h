#ifndef TAUSCH_H
#define TAUSCH_H

#include <mpi.h>
#include <vector>
#include <array>
#include <map>
#include <cstring>
#include <memory>

#ifdef TAUSCH_OPENCL
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#endif

#ifdef TAUSCH_CUDA
#include <cuda_runtime.h>
#endif

enum TauschOptimizationHint {
    NoHints = 1,
    UseMpiDerivedDatatype = 2,
    CudaStaysOnDevice = 4,
    CudaDoesNotStayOnDevice = 8,
    SenderUsesMpiDerivedDatatype = 16,
    ReceiverUsesMpiDerivedDatatype = 32,
};

template <class buf_t>
class Tausch {

public:
#ifdef TAUSCH_OPENCL
    Tausch(cl::Device device, cl::Context context, cl::CommandQueue queue,
           const MPI_Datatype mpiDataType, const MPI_Comm comm = MPI_COMM_WORLD, const bool useDuplicateOfCommunicator = true) {
#else
    Tausch(const MPI_Datatype mpiDataType, const MPI_Comm comm = MPI_COMM_WORLD, const bool useDuplicateOfCommunicator = true) {
#endif

        if(useDuplicateOfCommunicator)
            MPI_Comm_dup(comm, &TAUSCH_COMM);
        else
            TAUSCH_COMM = comm;

        this->mpiDataType = mpiDataType;

#ifdef TAUSCH_OPENCL
        this->device = device;
        this->context = context;
        this->queue = queue;
#endif

    }

    inline int addLocalHaloInfo(std::vector<int> haloIndices, const size_t numBuffers = 1, const int remoteMpiRank = -1, TauschOptimizationHint hints = TauschOptimizationHint::NoHints) {
        std::vector<std::array<int, 4> > tuple = extractHaloIndicesWithStride(haloIndices);
        std::vector<std::vector<std::array<int, 4> > > haloIndices_;
        for(size_t i = 0; i < numBuffers; ++i)
            haloIndices_.push_back(tuple);
        return addLocalHaloInfo(haloIndices_, numBuffers, remoteMpiRank, hints);
    }
    inline int addLocalHaloInfo(std::vector<std::vector<int> > haloIndices, const size_t numBuffers = 1, const int remoteMpiRank = -1, TauschOptimizationHint hints = TauschOptimizationHint::NoHints) {
        std::vector<std::vector<std::array<int, 4> > > ret;
        for(auto const & eachset : haloIndices)
            ret.push_back(extractHaloIndicesWithStride(eachset));
        return addLocalHaloInfo(ret, numBuffers, remoteMpiRank, hints);
    }

    inline int addLocalHaloInfo(std::vector<size_t> haloIndices, const size_t numBuffers = 1, const int remoteMpiRank = -1, TauschOptimizationHint hints = TauschOptimizationHint::NoHints) {
        std::vector<std::array<int, 4> > tuple = extractHaloIndicesWithStride(std::vector<int>(haloIndices.begin(), haloIndices.end()));
        std::vector<std::vector<std::array<int, 4> > > haloIndices_;
        for(size_t i = 0; i < numBuffers; ++i)
            haloIndices_.push_back(tuple);
        return addLocalHaloInfo(haloIndices_, numBuffers, remoteMpiRank, hints);
    }
    inline int addLocalHaloInfo(std::vector<std::vector<size_t> > haloIndices, const size_t numBuffers = 1, const int remoteMpiRank = -1, TauschOptimizationHint hints = TauschOptimizationHint::NoHints) {
        std::vector<std::vector<std::array<int, 4> > > ret;
        for(auto const & eachset : haloIndices)
            ret.push_back(extractHaloIndicesWithStride(std::vector<int>(eachset.begin(), eachset.end())));
        return addLocalHaloInfo(ret, numBuffers, remoteMpiRank, hints);
    }

    inline int addLocalHaloInfo(std::vector<std::array<int, 4> > haloIndices, const size_t numBuffers = 1, const int remoteMpiRank = -1, TauschOptimizationHint hints = TauschOptimizationHint::NoHints) {
        std::vector<std::vector<std::array<int, 4> > > haloIndices_;
        for(size_t i = 0; i < numBuffers; ++i)
            haloIndices_.push_back(haloIndices);
        return addLocalHaloInfo(haloIndices_, numBuffers, remoteMpiRank, hints);
    }

    inline int addLocalHaloInfo(std::vector<std::vector<std::array<int, 4> > > haloIndices, const size_t numBuffers = 1, const int remoteMpiRank = -1, TauschOptimizationHint hints = TauschOptimizationHint::NoHints) {


        int totalHaloSize = 0;
        std::vector<size_t> haloSizePerBuffer;
        int numHaloRegions = 0;
        for(auto perbuf : haloIndices) {
            size_t s = 0;
            for(auto tuple : perbuf) {
                totalHaloSize += tuple[1]*tuple[2];
                s += tuple[1]*tuple[2];
                ++numHaloRegions;
            }
            haloSizePerBuffer.push_back(s);

        }

        localHaloIndices.push_back(haloIndices);
        localHaloIndicesSize.push_back(haloSizePerBuffer);
        localHaloIndicesSizeTotal.push_back(totalHaloSize);
        localHaloNumBuffers.push_back(numBuffers);
        localHaloRemoteMpiRank.push_back(remoteMpiRank);

        localOptHints.push_back(hints);

        if(hints & UseMpiDerivedDatatype) {

            std::vector<MPI_Datatype> typePerBuffer;

            for(auto const & perbuf : haloIndices) {

                std::vector<MPI_Datatype> vectorDataTypes;
                std::vector<MPI_Aint> displacement;
                std::vector<int> blocklength;

                vectorDataTypes.reserve(perbuf.size());
                displacement.reserve(perbuf.size());
                blocklength.reserve(perbuf.size());

                for(auto const & item : perbuf) {

                    MPI_Datatype vec;
                    MPI_Type_vector(item[2], item[1], item[3], mpiDataType, &vec);
                    MPI_Type_commit(&vec);

                    vectorDataTypes.push_back(vec);
                    displacement.push_back(item[0]*sizeof(buf_t));
                    blocklength.push_back(1);

                }

                MPI_Datatype newtype;
                MPI_Type_create_struct(perbuf.size(), blocklength.data(), displacement.data(), vectorDataTypes.data(), &newtype);
                MPI_Type_commit(&newtype);
                typePerBuffer.push_back(newtype);

            }

            sendDatatype.push_back(typePerBuffer);

            sendBuffer.push_back(std::unique_ptr<buf_t[]>(new buf_t[1]));


        } else {

            void *newbuf = NULL;
            posix_memalign(&newbuf, 64, numBuffers*totalHaloSize*sizeof(buf_t));
            buf_t *newbuf_buft = reinterpret_cast<buf_t*>(newbuf);
            double zero = 0;
            std::fill_n(newbuf_buft, numBuffers*totalHaloSize, zero);
            sendBuffer.push_back(std::unique_ptr<buf_t[]>(std::move(newbuf_buft)));

        }

        mpiSendRequests.push_back(std::unique_ptr<MPI_Request>(new MPI_Request));

        setupMpiSend.push_back(false);

        return sendBuffer.size()-1;

    }


    inline int addRemoteHaloInfo(std::vector<int> haloIndices, const size_t numBuffers = 1, const int remoteMpiRank = -1, TauschOptimizationHint hints = TauschOptimizationHint::NoHints) {
        std::vector<std::array<int, 4> > tuple = extractHaloIndicesWithStride(haloIndices);
        std::vector<std::vector<std::array<int, 4> > > haloIndices_;
        for(size_t i = 0; i < numBuffers; ++i)
            haloIndices_.push_back(tuple);
        return addRemoteHaloInfo(haloIndices_, numBuffers, remoteMpiRank, hints);
    }
    inline int addRemoteHaloInfo(std::vector<std::vector<int> > haloIndices, const size_t numBuffers = 1, const int remoteMpiRank = -1, TauschOptimizationHint hints = TauschOptimizationHint::NoHints) {
        std::vector<std::vector<std::array<int, 4> > > ret;
        for(auto const & eachset : haloIndices)
            ret.push_back(extractHaloIndicesWithStride(eachset));
        return addRemoteHaloInfo(ret, numBuffers, remoteMpiRank, hints);
    }

    inline int addRemoteHaloInfo(std::vector<size_t> haloIndices, const size_t numBuffers = 1, const int remoteMpiRank = -1, TauschOptimizationHint hints = TauschOptimizationHint::NoHints) {
        std::vector<std::array<int, 4> > tuple = extractHaloIndicesWithStride(std::vector<int>(haloIndices.begin(), haloIndices.end()));
        std::vector<std::vector<std::array<int, 4> > > haloIndices_;
        for(size_t i = 0; i < numBuffers; ++i)
            haloIndices_.push_back(tuple);
        return addRemoteHaloInfo(haloIndices_, numBuffers, remoteMpiRank, hints);
    }
    inline int addRemoteHaloInfo(std::vector<std::vector<size_t> > haloIndices, const size_t numBuffers = 1, const int remoteMpiRank = -1, TauschOptimizationHint hints = TauschOptimizationHint::NoHints) {
        std::vector<std::vector<std::array<int, 4> > > ret;
        for(auto const & eachset : haloIndices)
            ret.push_back(extractHaloIndicesWithStride(std::vector<int>(eachset.begin(), eachset.end())));
        return addRemoteHaloInfo(ret, numBuffers, remoteMpiRank, hints);
    }

    inline int addRemoteHaloInfo(std::vector<std::array<int, 4> > haloIndices, const size_t numBuffers = 1, const int remoteMpiRank = -1, TauschOptimizationHint hints = TauschOptimizationHint::NoHints) {
        std::vector<std::vector<std::array<int, 4> > > haloIndices_;
        for(size_t i = 0; i < numBuffers; ++i)
            haloIndices_.push_back(haloIndices);
        return addRemoteHaloInfo(haloIndices_, numBuffers, remoteMpiRank, hints);
    }

    inline int addRemoteHaloInfo(std::vector<std::vector<std::array<int, 4> > > haloIndices, const size_t numBuffers = 1, const int remoteMpiRank = -1, TauschOptimizationHint hints = TauschOptimizationHint::NoHints) {

        int totalHaloSize = 0;
        std::vector<size_t> haloSizePerBuffer;
        int numHaloRegions = 0;
        for(auto perbuf : haloIndices) {
            size_t s = 0;
            for(auto tuple : perbuf) {
                totalHaloSize += tuple[1]*tuple[2];
                s += tuple[1]*tuple[2];
                ++numHaloRegions;
            }
            haloSizePerBuffer.push_back(s);
        }

        remoteHaloIndices.push_back(haloIndices);
        remoteHaloIndicesSize.push_back(haloSizePerBuffer);
        remoteHaloIndicesSizeTotal.push_back(totalHaloSize);
        remoteHaloNumBuffers.push_back(numBuffers);
        remoteHaloRemoteMpiRank.push_back(remoteMpiRank);

        remoteOptHints.push_back(hints);

        if(hints & UseMpiDerivedDatatype) {

            std::vector<MPI_Datatype> typePerBuffer;

            for(auto const & perbuf : haloIndices) {

                std::vector<MPI_Datatype> vectorDataTypes;
                std::vector<MPI_Aint> displacement;
                std::vector<int> blocklength;

                vectorDataTypes.reserve(perbuf.size());
                displacement.reserve(perbuf.size());
                blocklength.reserve(perbuf.size());

                for(auto const & item : perbuf) {

                    MPI_Datatype vec;
                    MPI_Type_vector(item[2], item[1], item[3], mpiDataType, &vec);
                    MPI_Type_commit(&vec);

                    vectorDataTypes.push_back(vec);
                    displacement.push_back(item[0]*sizeof(buf_t));
                    blocklength.push_back(1);

                }

                MPI_Datatype newtype;
                MPI_Type_create_struct(perbuf.size(), blocklength.data(), displacement.data(), vectorDataTypes.data(), &newtype);
                MPI_Type_commit(&newtype);
                typePerBuffer.push_back(newtype);

            }

            recvDatatype.push_back(typePerBuffer);

            recvBuffer.push_back(std::unique_ptr<buf_t[]>(new buf_t[1]));

        } else {

            void *newbuf = NULL;
            posix_memalign(&newbuf, 64, numBuffers*totalHaloSize*sizeof(buf_t));
            buf_t *newbuf_buft = reinterpret_cast<buf_t*>(newbuf);
            double zero = 0;
            std::fill_n(newbuf_buft, numBuffers*totalHaloSize, zero);
            recvBuffer.push_back(std::unique_ptr<buf_t[]>(std::move(newbuf_buft)));

        }

        mpiRecvRequests.push_back(std::unique_ptr<MPI_Request>(new MPI_Request));

        setupMpiRecv.push_back(false);

        return recvBuffer.size()-1;

    }

    inline void packSendBuffer(const size_t haloId, const size_t bufferId, const buf_t *buf) const {

        size_t bufferOffset = 0;
        for(size_t i = 0; i < bufferId; ++i)
            bufferOffset += localHaloIndicesSize[haloId][i];

        size_t mpiSendBufferIndex = 0;
        for(auto const & region : localHaloIndices[haloId][bufferId]) {

            const int &region_start = region[0];
            const int &region_howmanycols = region[1];
            const int &region_howmanyrows = region[2];
            const int &region_stridecol = region[3];

            if(region_howmanycols == 1) {

                for(int rows = 0; rows < region_howmanyrows; ++rows) {
                    sendBuffer[haloId][bufferOffset + mpiSendBufferIndex] = buf[region_start + rows*region_stridecol];
                    ++mpiSendBufferIndex;
                }

            } else if(region_howmanycols == 2) {

                for(int rows = 0; rows < region_howmanyrows; ++rows) {
                    sendBuffer[haloId][bufferOffset + mpiSendBufferIndex  ] = buf[region_start + rows*region_stridecol   ];
                    sendBuffer[haloId][bufferOffset + mpiSendBufferIndex+1] = buf[region_start + rows*region_stridecol +1];
                    mpiSendBufferIndex += 2;
                }

            } else {

                for(int rows = 0; rows < region_howmanyrows; ++rows) {

                    memcpy(&sendBuffer[haloId][bufferOffset + mpiSendBufferIndex], &buf[region_start + rows*region_stridecol], region_howmanycols*sizeof(buf_t));
                    mpiSendBufferIndex += region_howmanycols;

                }

            }

        }

    }

    inline void packSendBuffer(const size_t haloId, const size_t bufferId, const buf_t *buf,
                               std::vector<size_t> overwriteHaloSendIndices, std::vector<size_t> overwriteHaloSourceIndices) const {

        size_t bufferOffset = 0;
        for(size_t i = 0; i < bufferId; ++i)
            bufferOffset += localHaloIndicesSize[haloId][i];

        for(auto index = 0; index < overwriteHaloSendIndices.size(); ++index)
            sendBuffer[haloId][bufferOffset + overwriteHaloSendIndices[index]] = buf[overwriteHaloSourceIndices[index]];

    }

    inline MPI_Request *send(size_t haloId, const int msgtag, int remoteMpiRank = -1, const size_t bufferId = 0, const buf_t *buf = nullptr, const bool blocking = false, MPI_Comm overwriteComm = MPI_COMM_NULL) {

        if(localHaloIndicesSizeTotal[haloId] == 0)
            return nullptr;

        if(overwriteComm == MPI_COMM_NULL)
            overwriteComm = TAUSCH_COMM;

        if(localOptHints[haloId] & UseMpiDerivedDatatype) {

            if(remoteMpiRank == -1)
                remoteMpiRank = localHaloRemoteMpiRank[haloId];

            MPI_Isend(buf, 1, sendDatatype[haloId][bufferId], remoteMpiRank, msgtag, overwriteComm, mpiSendRequests[haloId].get());
            if(blocking)
                MPI_Wait(mpiSendRequests[haloId].get(), MPI_STATUS_IGNORE);

            return mpiSendRequests[haloId].get();

        } else {

            if(!setupMpiSend[haloId]) {

                setupMpiSend[haloId] = true;

                // if we stay on the same rank, we don't need to use MPI
                int myRank;
                MPI_Comm_rank(overwriteComm, &myRank);
                if(remoteMpiRank == myRank && !(localOptHints[haloId] & TauschOptimizationHint::ReceiverUsesMpiDerivedDatatype)) {
                    msgtagToHaloId[myRank*1000000 + msgtag] = haloId;
                    return nullptr;
                }
                MPI_Send_init(sendBuffer[haloId].get(), localHaloIndicesSizeTotal[haloId], mpiDataType, remoteMpiRank,
                          msgtag, overwriteComm, mpiSendRequests[haloId].get());

            } else
                MPI_Wait(mpiSendRequests[haloId].get(), MPI_STATUS_IGNORE);

            MPI_Start(mpiSendRequests[haloId].get());
            if(blocking)
                MPI_Wait(mpiSendRequests[haloId].get(), MPI_STATUS_IGNORE);

            return mpiSendRequests[haloId].get();

        }

    }

    inline MPI_Request *recv(size_t haloId, const int msgtag, int remoteMpiRank = -1, const size_t bufferId = 0, buf_t *buf = nullptr, const bool blocking = true, MPI_Comm overwriteComm = MPI_COMM_NULL) {

        if(remoteHaloIndicesSizeTotal[haloId] == 0)
            return nullptr;

        if(overwriteComm == MPI_COMM_NULL)
            overwriteComm = TAUSCH_COMM;

        if(remoteOptHints[haloId] & UseMpiDerivedDatatype) {

            if(remoteMpiRank == -1)
                remoteMpiRank = remoteHaloRemoteMpiRank[haloId];

            MPI_Irecv(buf, 1, recvDatatype[haloId][bufferId], remoteMpiRank, msgtag, overwriteComm, mpiRecvRequests[haloId].get());
            if(blocking)
                MPI_Wait(mpiRecvRequests[haloId].get(), MPI_STATUS_IGNORE);

            return mpiRecvRequests[haloId].get();

        } else {

            if(!setupMpiRecv[haloId]) {

                if(remoteMpiRank == -1)
                    remoteMpiRank = remoteHaloRemoteMpiRank[haloId];

                // if we stay on the same rank, we don't need to use MPI
                int myRank;
                MPI_Comm_rank(overwriteComm, &myRank);

#ifdef TAUSCH_CUDA
                if(remoteMpiRank == myRank && remoteOptHints[haloId] == TauschOptimizationHint::CudaStaysOnDevice) {

                    const int remoteHaloId = msgtagToHaloId[myRank*1000000 + msgtag];

                    buf_t *cudabuf;
                    cudaMalloc(&cudabuf, remoteHaloNumBuffers[haloId]*remoteHaloIndicesSizeTotal[haloId]*sizeof(buf_t));
                    cudaMemcpy(cudabuf, sendCommunicationBufferKeptOnCuda[remoteHaloId], remoteHaloNumBuffers[haloId]*remoteHaloIndicesSizeTotal[haloId]*sizeof(buf_t), cudaMemcpyDeviceToDevice);
                    recvCommunicationBufferKeptOnCuda[haloId] = cudabuf;

                } else
#endif
                if(remoteMpiRank == myRank && !(remoteOptHints[haloId] & TauschOptimizationHint::SenderUsesMpiDerivedDatatype)) {

                    const int remoteHaloId = msgtagToHaloId[myRank*1000000 + msgtag];

                    memcpy(recvBuffer[haloId].get(), sendBuffer[remoteHaloId].get(), remoteHaloIndicesSizeTotal[haloId]*sizeof(buf_t));

                } else {

                    setupMpiRecv[haloId] = true;

                    MPI_Recv_init(recvBuffer[haloId].get(), remoteHaloIndicesSizeTotal[haloId], mpiDataType,
                                  remoteMpiRank, msgtag, overwriteComm, mpiRecvRequests[haloId].get());
                }

            }

            // this will remain false if we remained on the same mpi rank
            if(setupMpiRecv[haloId]) {

                MPI_Start(mpiRecvRequests[haloId].get());
                if(blocking)
                    MPI_Wait(mpiRecvRequests[haloId].get(), MPI_STATUS_IGNORE);

                return mpiRecvRequests[haloId].get();

            } else

                return nullptr;

        }

    }

    inline void unpackRecvBuffer(const size_t haloId, const size_t bufferId, buf_t *buf) const {

        size_t bufferOffset = 0;
        for(size_t i = 0; i < bufferId; ++i)
            bufferOffset += remoteHaloIndicesSize[haloId][i];

        size_t mpiRecvBufferIndex = 0;

        for(auto const & region : remoteHaloIndices[haloId][bufferId]) {


            const auto &region_start = region[0];
            const auto &region_howmanycols = region[1];
            const auto &region_howmanyrows = region[2];
            const auto &region_stridecol = region[3];

            if(region_howmanycols == 1) {

                for(int rows = 0; rows < region_howmanyrows; ++rows) {
                    buf[region_start + rows*region_stridecol] = recvBuffer[haloId][bufferOffset + mpiRecvBufferIndex];
                    ++mpiRecvBufferIndex;
                }

            } else if(region_howmanycols == 2) {

                for(int rows = 0; rows < region_howmanyrows; ++rows) {
                    buf[region_start + rows*region_stridecol   ] = recvBuffer[haloId][bufferOffset + mpiRecvBufferIndex   ];
                    buf[region_start + rows*region_stridecol +1] = recvBuffer[haloId][bufferOffset + mpiRecvBufferIndex +1];
                    mpiRecvBufferIndex += 2;
                }

            } else {

                for(int rows = 0; rows < region_howmanyrows; ++rows) {

                    memcpy(&buf[region_start + rows*region_stridecol], &recvBuffer[haloId][bufferOffset + mpiRecvBufferIndex], region_howmanycols*sizeof(buf_t));
                    mpiRecvBufferIndex += region_howmanycols;

                }

            }

        }

    }

    inline void unpackRecvBuffer(const size_t haloId, const size_t bufferId, buf_t *buf,
                                 std::vector<size_t> overwriteHaloRecvIndices, std::vector<size_t> overwriteHaloTargetIndices) const {

        size_t bufferOffset = 0;
        for(size_t i = 0; i < bufferId; ++i)
            bufferOffset += remoteHaloIndicesSize[haloId][i];

        for(size_t index = 0; index < overwriteHaloRecvIndices.size(); ++index)
            buf[overwriteHaloTargetIndices[index]] = recvBuffer[haloId][bufferOffset + overwriteHaloRecvIndices[index]];

    }

    inline MPI_Request *packAndSend(const size_t haloId, const buf_t *buf, const int msgtag, const int remoteMpiRank = -1) const {
        packSendBuffer(haloId, 0, buf);
        return send(haloId, msgtag, remoteMpiRank);
    }

    inline void recvAndUnpack(const size_t haloId, buf_t *buf, const int msgtag, const int remoteMpiRank = -1) const {
        recv(haloId, msgtag, remoteMpiRank, true);
        unpackRecvBuffer(haloId, 0, buf);
    }


#ifdef TAUSCH_OPENCL

    void packSendBuffer(const int haloId, int bufferId, cl::Buffer buf) {

        try {

            size_t bufferOffset = 0;
            for(size_t i = 0; i < bufferId; ++i)
                bufferOffset += localHaloIndicesSize[haloId][i];

            size_t mpiSendBufferIndex = 0;
            for(size_t iRegion = 0; iRegion < localHaloIndices[haloId][bufferId].size(); ++iRegion) {
                const std::array<int, 4> vals = localHaloIndices[haloId][bufferId][iRegion];

                const int val_start = vals[0];
                const int val_howmanycols = vals[1];
                const int val_howmanyrows = vals[2];
                const int val_striderows = vals[3];

                for(int rows = 0; rows < val_howmanyrows; ++rows) {

                    cl::size_t<3> buffer_offset;
                    buffer_offset[0] = (val_start+rows*val_striderows)*sizeof(buf_t); buffer_offset[1] = 0; buffer_offset[2] = 0;
                    cl::size_t<3> host_offset;
                    host_offset[0] = (bufferOffset + mpiSendBufferIndex)*sizeof(buf_t); host_offset[1] = 0; host_offset[2] = 0;

                    cl::size_t<3> region;
                    region[0] = sizeof(buf_t); region[1] = val_howmanycols; region[2] = 1;

                    queue.enqueueReadBufferRect(buf,
                                                CL_TRUE,
                                                buffer_offset,
                                                host_offset,
                                                region,
                                                sizeof(buf_t),
                                                0,
                                                sizeof(buf_t),
                                                0,
                                                sendBuffer[haloId].get());

                    mpiSendBufferIndex += val_howmanycols;
                }

            }

        } catch(cl::Error &e) {
            std::cerr << "Tausch::packSendBufferOCL(): OpenCL exception caught: " << e.what() << " (" << e.err() << ")" << std::endl;
        }

    }

    void unpackRecvBuffer(const int haloId, const int bufferId, cl::Buffer buf) {

        try {

            size_t bufferOffset = 0;
            for(size_t i = 0; i < bufferId; ++i)
                bufferOffset += remoteHaloIndicesSize[haloId][i];

            size_t mpiRecvBufferIndex = 0;
            for(size_t iRegion = 0; iRegion < remoteHaloIndices[haloId][bufferId].size(); ++iRegion) {
                const std::array<int, 4> vals = remoteHaloIndices[haloId][bufferId][iRegion];

                const int val_start = vals[0];
                const int val_howmanycols = vals[1];
                const int val_howmanyrows = vals[2];
                const int val_striderows = vals[3];

                for(int rows = 0; rows < val_howmanyrows; ++rows) {

                    cl::size_t<3> buffer_offset;
                    buffer_offset[0] = (val_start+rows*val_striderows)*sizeof(buf_t); buffer_offset[1] = 0; buffer_offset[2] = 0;
                    cl::size_t<3> host_offset;
                    host_offset[0] = (bufferOffset + mpiRecvBufferIndex)*sizeof(buf_t); host_offset[1] = 0; host_offset[2] = 0;

                    cl::size_t<3> region;
                    region[0] = sizeof(buf_t); region[1] = val_howmanycols; region[2] = 1;

                    queue.enqueueWriteBufferRect(buf,
                                                 CL_TRUE,
                                                 buffer_offset,
                                                 host_offset,
                                                 region,
                                                 sizeof(buf_t),
                                                 0,
                                                 sizeof(buf_t),
                                                 0,
                                                 recvBuffer[haloId].get());

                    mpiRecvBufferIndex += val_howmanycols;

                }

            }

        } catch(cl::Error &e) {
            std::cerr << "Tausch::unpackRecvBufferOCL() :: OpenCL exception caught: " << e.what() << " (" << e.err() << ")" << std::endl;
        }

    }

#endif

#ifdef TAUSCH_CUDA

    void packSendBufferCUDA(const int haloId, int bufferId, buf_t *buf_d) {

        size_t bufferOffset = 0;
        for(size_t i = 0; i < bufferId; ++i)
            bufferOffset += localHaloIndicesSize[haloId][i];

        if(localOptHints[haloId] == TauschOptimizationHint::CudaStaysOnDevice) {

            if(sendCommunicationBufferKeptOnCuda.find(haloId) == sendCommunicationBufferKeptOnCuda.end()) {

                buf_t *cudabuf;
                cudaMalloc(&cudabuf, localHaloNumBuffers[haloId]*localHaloIndicesSizeTotal[haloId]*sizeof(buf_t));
                sendCommunicationBufferKeptOnCuda[haloId] = cudabuf;

            }

            size_t mpiSendBufferIndex = 0;
            for(size_t region = 0; region < localHaloIndices[haloId][bufferId].size(); ++region) {
                const std::array<int, 4> vals = localHaloIndices[haloId][bufferId][region];

                const int val_start = vals[0];
                const int val_howmanycols = vals[1];
                const int val_howmanyrows = vals[2];
                const int val_striderows = vals[3];

                for(int rows = 0; rows < val_howmanyrows; ++rows) {

                    cudaError_t err = cudaMemcpy2D(&sendCommunicationBufferKeptOnCuda[haloId][bufferOffset + mpiSendBufferIndex], sizeof(buf_t),
                                                   &buf_d[val_start+rows*val_striderows], sizeof(buf_t),
                                                   sizeof(buf_t), val_howmanycols, cudaMemcpyDeviceToDevice);
                    if(err != cudaSuccess)
                        std::cout << "Tausch::packSendBufferCUDA() 1: CUDA error detected: " << err << std::endl;

                    mpiSendBufferIndex += val_howmanycols;

                }

            }

            return;

        }

        size_t mpiSendBufferIndex = 0;
        for(size_t region = 0; region < localHaloIndices[haloId][bufferId].size(); ++region) {
            const std::array<int, 4> vals = localHaloIndices[haloId][bufferId][region];

            const int val_start = vals[0];
            const int val_howmanycols = vals[1];
            const int val_howmanyrows = vals[2];
            const int val_striderows = vals[3];

            for(int rows = 0; rows < val_howmanyrows; ++rows) {

                cudaError_t err = cudaMemcpy2D(&sendBuffer[haloId][bufferOffset + mpiSendBufferIndex], sizeof(buf_t),
                                               &buf_d[val_start+rows*val_striderows], sizeof(buf_t),
                                               sizeof(buf_t), val_howmanycols, cudaMemcpyDeviceToHost);
                if(err != cudaSuccess)
                    std::cout << "Tausch::packSendBufferCUDA() 2: CUDA error detected: " << err << std::endl;

                mpiSendBufferIndex += val_howmanycols;

            }

        }

    }

    inline void unpackRecvBufferCUDA(const size_t haloId, const size_t bufferId, buf_t *buf_d) {

        size_t bufferOffset = 0;
        for(size_t i = 0; i < bufferId; ++i)
            bufferOffset += remoteHaloIndicesSize[haloId][i];

        if(remoteOptHints[haloId] == TauschOptimizationHint::CudaStaysOnDevice) {

            size_t mpiRecvBufferIndex = 0;
            for(size_t region = 0; region < remoteHaloIndices[haloId][bufferId].size(); ++region) {
                const std::array<int, 4> vals = remoteHaloIndices[haloId][bufferId][region];

                const int val_start = vals[0];
                const int val_howmanycols = vals[1];
                const int val_howmanyrows = vals[2];
                const int val_striderows = vals[3];

                for(int rows = 0; rows < val_howmanyrows; ++rows) {

                    cudaError_t err = cudaMemcpy2D(&buf_d[val_start+rows*val_striderows], sizeof(buf_t),
                                                   &recvCommunicationBufferKeptOnCuda[haloId][bufferOffset + mpiRecvBufferIndex], sizeof(buf_t),
                                                   sizeof(buf_t), val_howmanycols, cudaMemcpyDeviceToDevice);
                    if(err != cudaSuccess)
                        std::cout << "Tausch::unpackRecvBufferCUDA(): CUDA error detected: " << err << std::endl;

                    mpiRecvBufferIndex += val_howmanycols;

                }

            }

        } else {

            size_t mpiRecvBufferIndex = 0;
            for(size_t region = 0; region < remoteHaloIndices[haloId][bufferId].size(); ++region) {
                const std::array<int, 4> vals = remoteHaloIndices[haloId][bufferId][region];

                const int val_start = vals[0];
                const int val_howmanycols = vals[1];
                const int val_howmanyrows = vals[2];
                const int val_striderows = vals[3];

                for(int rows = 0; rows < val_howmanyrows; ++rows) {

                    cudaError_t err = cudaMemcpy2D(&buf_d[val_start+rows*val_striderows], sizeof(buf_t),
                                                   &recvBuffer[haloId][bufferOffset + mpiRecvBufferIndex], sizeof(buf_t),
                                                   sizeof(buf_t), val_howmanycols, cudaMemcpyHostToDevice);
                    if(err != cudaSuccess)
                        std::cout << "Tausch::unpackRecvBufferCUDA(): CUDA error detected: " << err << std::endl;

                    mpiRecvBufferIndex += val_howmanycols;

                }

            }

        }

    }

#endif

    inline std::vector<std::array<int, 4> > extractHaloIndicesWithStride(std::vector<int> indices) {

        // nothing to do
        if(indices.size() == 0)
            return std::vector<std::array<int, 4> >();

        // first we build a collection of all consecutive rows
        std::vector<std::array<int, 2> > rows;

        int curIndex = 1;
        int start = indices[0];
        int howmany = 1;
        while(curIndex < indices.size()) {

            if(indices[curIndex]-indices[curIndex-1] == 1)
                ++howmany;
            else {

                rows.push_back({start, howmany});

                start = indices[curIndex];
                howmany = 1;

            }

            ++curIndex;

        }

        rows.push_back({start, howmany});

        // second we look for simple patterns within these rows
        std::vector<std::array<int, 4> > ret;

        ret.push_back({rows[0][0], rows[0][1], 1, 0});

        for(int currow = 1; currow < rows.size(); ++currow) {

            if(rows[currow][1] == ret.back()[1] && (ret.back()[3] == 0 || rows[currow][0]-(ret.back()[0]+(ret.back()[2]-1)*ret.back()[3]) == ret.back()[3])) {

                if(ret.back()[3] == 0) {
                    ++ret.back()[2];
                    ret.back()[3] = rows[currow][0]-ret.back()[0];
                } else
                    ++ret.back()[2];

            } else {

                ret.push_back({rows[currow][0], rows[currow][1], 1, 0});
            }

        }

        return ret;

    }

    int getLocalHaloTotalSize(int haloId) {
        return localHaloIndicesSizeTotal[haloId];
    }
    int getRemoteHaloTotalSize(int haloId) {
        return remoteHaloIndicesSizeTotal[haloId];
    }

    MPI_Comm TAUSCH_COMM;
    MPI_Datatype mpiDataType;

    std::vector<std::vector<std::vector<std::array<int, 4> > > > localHaloIndices;
    std::vector<std::vector<std::vector<std::array<int, 4> > > > remoteHaloIndices;

    std::vector<std::vector<size_t> > localHaloIndicesSize;
    std::vector<size_t> localHaloIndicesSizeTotal;
    std::vector<std::vector<size_t> > remoteHaloIndicesSize;
    std::vector<size_t> remoteHaloIndicesSizeTotal;

    std::vector<int> localHaloRemoteMpiRank;
    std::vector<int> remoteHaloRemoteMpiRank;

    std::vector<size_t> localHaloNumBuffers;
    std::vector<size_t> remoteHaloNumBuffers;

    std::vector<std::unique_ptr<buf_t[]> > sendBuffer;
    std::vector<std::unique_ptr<buf_t[]> > recvBuffer;

    std::vector<std::vector<MPI_Datatype> > sendDatatype;
    std::vector<std::vector<MPI_Datatype> > recvDatatype;

    std::vector<std::unique_ptr<MPI_Request> > mpiSendRequests;
    std::vector<std::unique_ptr<MPI_Request> > mpiRecvRequests;

    std::vector<bool> setupMpiSend;
    std::vector<bool> setupMpiRecv;

    std::vector<TauschOptimizationHint> localOptHints;
    std::vector<TauschOptimizationHint> remoteOptHints;

    std::vector<bool> useMpiDerivedDatatype;


    // this is used for exchanges on same mpi rank
    std::map<int, int> msgtagToHaloId;

#ifdef TAUSCH_OPENCL

    cl::Device device;
    cl::Context context;
    cl::CommandQueue queue;
    cl::Program programs;

#endif

#ifdef TAUSCH_CUDA
    std::map<int, buf_t*> sendCommunicationBufferKeptOnCuda;
    std::map<int, buf_t*> recvCommunicationBufferKeptOnCuda;
#endif

};


#endif
