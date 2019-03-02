#! /bin/bash

usage="Usage: ${0##*/} [OPTIONS] path/to/artifacts.zip"
tag="fedora"
dockername="flecsi_debug"

die () {
  echo -e "$*"
  exit 1
}

show_help() {
  cat << eof
This script will start an environment to debug CI builds
$usage
OPTIONS:
    --help          Show this help
-t, --tag TAG       Specify the docker tag of votca/buildenv to use
                    Default: $tag
-n, --name NAME     Specify the name of docker build
                    Default: $dockername
-c, --clean         Clean up temp dir and docker images 

Examples:  ${0##*/} --tag ubuntu ./storage/old_build.zip

Report bugs and comments at https://github.com/laristra/flecsi/issues
eof
}

clean_up() {
  set -x
  rm -rf /tmp/${dockername}.*
  docker ps -a | awk "(\$2==\"${dockername}\"){print \$1}" | xargs docker rm
  docker rmi "${dockername}"
}

shopt -s extglob
while [[ $# -gt 0 ]]; do
  if [[ ${1} = --*=* ]]; then # case --xx=yy
    set -- "${1%%=*}" "${1#*=}" "${@:2}" # --xx=yy to --xx yy
  elif [[ ${1} = -[^-]?* ]]; then # case -xy split
    if [[ ${1} = -[nt]* ]]; then #short opts with arguments
       set -- "${1:0:2}" "${1:2}" "${@:2}" # -xy to -x y
    else #short opts without arguments
       set -- "${1:0:2}" "-${1:2}" "${@:2}" # -xy to -x -y
    fi
 fi
 case $1 in
   -n|--name)
     dockername="$2"
     shift 2;;
   -t|--tag)
     tag="$2"
     shift 2;;
   -c|--clean)
     clean_up
     exit $?;;
   --help)
     show_help
     exit $?;;
   -*)
     die "Unknown options $1";;
   --)
     break;;
   *)
     break;;
 esac
done

for prog in unzip docker mktemp; do
  [[ -n $(type -p $prog) ]] || die "Could not find program '$prog'"
done

[[ -z $1 ]] && die "Missing argument - need artifact.zip!\nTry ${0##*/} --help"
zip="$1"
[[ -f $zip ]] || die "Cannot read '$zip'"
[[ $zip != /* ]] && zip="$PWD/$zip"

[[ $tag = @(ubuntu|ubuntu_mpich|fedora|fedora_mpich) ]] || die "Unknown tag"

set -e
tmpdir=$(mktemp -d /tmp/${dockername}.XXXXXX)
cd ${tmpdir}
echo "Unzipping $zip to $tmpdir"
unzip -q -d flecsi "$zip"
docker pull laristra/flecsi-third-party:${tag}
basedir=/builds/next-generation-codes/laristra/flecsi
cat > Dockerfile <<EOF
FROM laristra/flecsi-third-party:${tag}
WORKDIR ${basedir}/build
EOF
docker build -t ${dockername} .
echo "Use 'docker run -it -v ${tmpdir}/flecsi:${basedir} ${dockername} /bin/bash' to re-run this"
docker run -it -v ${tmpdir}/flecsi:${basedir} ${dockername} /bin/bash
