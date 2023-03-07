#!/bin/bash

if [ $# -ge 1 ]
then
  echo "additional args forwarded to all builders >>" $@
fi

echo ""
echo ">######## libSocketCom ########"
./DoBuild.sh $@ 

echo ""
echo ">######## AppTestLocalClient ########"
cd AppTestLocalClient
./DoBuild.sh $@

echo ""
echo ">######## AppTestLocalServer ########"
cd ../AppTestLocalServer
./DoBuild.sh $@

echo ""
echo ">######## AppTestInetClient ########"
cd ../AppTestInetClient
./DoBuild.sh $@

echo ""
echo ">######## AppTestInetServer ########"
cd ../AppTestInetServer
./DoBuild.sh $@


cd ..
