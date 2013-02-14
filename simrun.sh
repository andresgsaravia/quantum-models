#!/bin/bash

printf "\nThis script will run a set of calculations using the \"eig\" program with a variable parameter.
For now I've just implemented variable electron-infrared phonons coupling.\n\n"

printf "Please provide the following parameters for the simulations:\n\n"

printf "Band energy: "
read BAND

printf "Nearest neighbor hopping: "
read NNH

printf "On site Coulomb repulsion: "
read ONSITE

printf "Infrared phonon's energy: "
read IRE

printf "Raman phonon's energy: "
read RAME

printf "Electron - raman phonons coupling: "
read ERAM

printf "Raman shift: " 
read RAMSH

printf "Number of infrared phonons: "
read NIR

printf "Number of raman phonons: "
read NRAM

printf "Lowest electron - infrared phonons coupling value: "
read LOWEIR
EIR=`echo "scale=5; $LOWEIR/1.0" | bc`

printf "Greatest electron - infrared phonons coupling value: "
read GREATEIR

printf "How many points should I calculate between %s and %s? " "$LOWEIR" "$GREATEIR"
read POINTS

#printf "How many simultaneous calculations should I run? "
#read SIMCAL

STEP=`echo "scale=5; ($GREATEIR-$LOWEIR)/$POINTS" | bc`
COUNT=0

if [ ! -d "calculations" ]; then
    echo "I created a directory named \"calculations\" to store the results. "
    `mkdir calculations`
else
    echo "I will store the results in the \"calculations\" directory. "
fi

# Create all the directories
while (( COUNT <= POINTS ))
do
    DIRNAME="$BAND-$NNH-$ONSITE-$IRE-$EIR-$RAME-$ERAM-$RAMSH-$NIR-$NRAM"
    if [ -d "calculations/$DIRNAME" ]; then
	printf "It seems that there's already a calculation with the parameters you provided and a coupling of %s. " "$EIR"
	printf "I will skip this calculation.\n"
    else
	printf "Making directory %s\n" "$DIRNAME"
	`mkdir calculations/$DIRNAME`
	printf "Saving \"parameters.inp\" into %s\n" "$DIRNAME"
	printf "%s, Band energy for site 1\n" "$BAND" >> "calculations/$DIRNAME/parameters.inp"
	printf "-%s, Band energy for site 2\n" "$BAND" >> "calculations/$DIRNAME/parameters.inp"
	printf "%s, Band energy for site 3\n" "$BAND" >> "calculations/$DIRNAME/parameters.inp"
	printf "%s, Nearest neighbor hopping\n" "$NNH" >> "calculations/$DIRNAME/parameters.inp"
	printf "%s, On site Coulomb repulsion\n" "$ONSITE" >> "calculations/$DIRNAME/parameters.inp"
	printf "%s, Infrared phonon's energy\n" "$IRE" >> "calculations/$DIRNAME/parameters.inp"
	printf "%s, Electron - infrared phonons coupling\n" "$EIR" >> "calculations/$DIRNAME/parameters.inp"
	printf "%s, Raman phonon's energy\n" "$RAME" >> "calculations/$DIRNAME/parameters.inp"
	printf "%s, Electron - raman phonons coupling\n" "$ERAM" >> "calculations/$DIRNAME/parameters.inp"
	printf "%s, Raman shift\n" "$RAMSH" >> "calculations/$DIRNAME/parameters.inp"
	printf "%s, Number of infrared phonons\n" "$NIR" >> "calculations/$DIRNAME/parameters.inp"
	printf "%s, Number of raman phonons\n" "$NRAM" >> "calculations/$DIRNAME/parameters.inp"
	printf "I will call the \"eig\" routine to make the calculation.\n"
	cd calculations/$DIRNAME
	../../eig
	cd ../../
	printf "\n"
    fi
    EIR=`echo "scale=5; $EIR+$STEP" | bc`
    ((COUNT += 1))
done
