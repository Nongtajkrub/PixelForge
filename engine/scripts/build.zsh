#!/usr/bin/env zsh

echo "Searching for project root directory."

for i in {1..5};
do
	if [[ "$PWD" == "/" ]]; then
		echo "Reached filesystem root."
		exit 1
	fi

	if [[ -f "./CMakeLists.txt" ]];  then
		echo "Project root directory found."
		break
	fi

	cd ..
done

if ! [[ -f "./CMakeLists.txt" ]]; then
	echo "Project root directory not found!"
	exit 1
fi

if ! [[ -d "./build" ]]; then
	echo "Build directory not present in project root directory."
	exit 1
fi

cd build
cmake ..

cmake --build . 
