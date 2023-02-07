#! /bin/sh
#First argument is full path to file (including filename), aka "filesdir"
#second arg is text to be found within the file, aka "searchstr"
#exits with return value 1 if any parameters not specified
#exits with return value 1 if filesdir does not represent a directory on the filesystem
#print message "The number of files are X and the number of matching lines are Y"

#Initialize variables
filesdir=$1
searchstr=$2

#Check number of arguments entered
if [ $# -lt 2 ];
then

echo "$0: Missing arguments, please enter two arguments; file location and text to be searched. You entered $# argument(s)."
        exit 1
else
   true
fi

#Check for a directory, if it exists, search directory for text. If the dir doesn't exist, print & exit. Return # of files & # of lines.
DIR=$filesdir
if [ -d "$DIR" ]; then
   #Find text in files, return file # & line #s
   grep -r "$searchstr" $filesdir
   numfiles=$(grep -r -l "$searchstr" $filesdir | wc -l)
   numlines=$(grep -r "$searchstr" $filesdir | wc -l)
   echo "The number of files are $numfiles and the number of matching lines are $numlines"
else
   echo "The directory entered does not exist. Please provide a valid directory."
   exit 1
fi
