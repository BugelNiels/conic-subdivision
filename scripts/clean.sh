#!/bin/bash

cd ..
rm build -rf
rm *.AppImage
cd appdir || exit
rm AppRun
rm *.desktop
cd usr || exit
rm bin/ -rf
rm lib/ -rf
rm plugins/ -rf
rm translations/ -rf
cd ..
cd ..
cd scripts || exit