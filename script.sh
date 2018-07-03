echo "DISTRO=$1 
RUNTIME=$2
DOCKERHUB=$3

cd ../ && cp -vr flecsi/docker . && cp -vr flecsi docker/.
sed -i "1s/fedora/${DISTRO}/" docker/Dockerfile

if [[ ${CC} != gcc ]]; then TAG="_${CC}"; fi
if [[ ${CI_COMMIT_REF_NAME} != stable ]]; then TAG="${TAG}_${CI_COMMIT_REF_NAME//\//_}"; fi

docker pull $(sed -n '1s/FROM //p' docker/Dockerfile)
docker build -t laristra/flecsi-third-party:ubuntu /builds/onurcaylak/docker/ --build-arg LEGION=${LEGION} --build-arg RUNTIME=${RUNTIME} --build-arg CXXFLAGS="${WERROR:+-Werror} -Wno-deprecated-declarations" --build-arg COVERAGE=${COVERAGE} --build-arg MINIMAL=${MINIMAL} --build-arg CC=${CC} --build-arg CXX=${CXX} --build-arg CI=${CI} 

CON=$(docker run -d laristra/flecsi-third-party:ubuntu /bin/bash) 
docker cp ${CON}:/home/flecsi /builds/onurcaylak/flecsi/artifacts/."
