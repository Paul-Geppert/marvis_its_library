#!/bin/bash

#################### Argument parsing ####################

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    # Will do a cleanup first
    -c|--clean-start)
    CLEAN=True
    shift # past argument
    ;;
    # Will trigger compilation in debug mode
    -d|--debug)
    DEBUG=True
    shift # past argument
    ;;
esac
done

# restore positional parameters
set -- "${POSITIONAL[@]}"

######################## Building ########################

# Prepare emvironment
if [ -z ${CLEAN+x} ]; then
    echo "Reusing existing build environment";
else
    echo "Preparing new build environment";
    make clean
    make prepare_env
fi

# Compile asn files
mkdir -p generated-v1
mkdir -p generated-v2
cd asn_files
asn1c -fno-include-deps -fcompound-names -D ../generated-v1 $(find ./v1 -type f -name "*.asn")
asn1c -fno-include-deps -fcompound-names -D ../generated-v2 $(find ./v2 -type f -name "*.asn")
cd ..

# Compile converter
cp Makefile.generated.template generated-v1/Makefile
cp Makefile.generated.template generated-v2/Makefile
make all DEBUG=$DEBUG

# Zip all important parts
make zip
