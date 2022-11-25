#!/usr/bin/env bash

# pouziti:   is_it_ok.sh xlogin01.zip testdir
#  
#   - POZOR: obsah adresare zadaneho druhym parametrem bude VYMAZAN (po dotazu)!
#   - rozbali archiv studenta xlogin01.zip do adresare testdir a overi formalni pozadavky pro odevzdani projektu IFJ
#   - nasledne vyzkousi kompilaci
#   - detaily prubehu jsou logovany do souboru is_it_ok.log v adresari testdir

# Autor: Zbynek Krivka
# Verze: 1.4.4 (2022-09-23)
#  2014-12-06  Pridano kontrola binarek v archivu a formatu souboru rozdeleni
#  2019-10-01  Zrusena kontrola malych pismen v nazvech souboru, pridana kontrola unikatnosti nazvu souboru (case-insensitive), barevne vypisy
#  2019-12-11  Pridan identifikator studentskeho rozsireni TABUNARY
#  2022-09-23  Upraveny identifikatory rozsireni pro IFJ22

LOG="is_it_ok.log"
MAX_ARCHIVE_SIZE=1500000

# Konstanty barev
REDCOLOR='\033[1;31m'
GREENCOLOR='\033[1;32m'
BLUECOLOR='\033[1;34m'
NOCOLOR='\033[0m' # No Color

# Funkce: vypis barevny text
function echo_color () { # $1=color $2=text [$3=-n]
  COLOR=$NOCOLOR
  if [[ $1 == "red" ]]; then
    COLOR=$REDCOLOR
  elif [[ $1 == "blue" ]]; then
    COLOR=$BLUECOLOR
  elif [[ $1 == "green" ]]; then
    COLOR=$GREENCOLOR
  fi
  echo -e $3 "$COLOR$2$NOCOLOR"
}

# implicit task indetifier from config.sh
if [[ $# -ne 2 ]]; then
  echo_color red "ERROR: Missing arguments or too much arguments!"
  echo "Usage: $0  ARCHIVE  TESTDIR"
  echo "       This script checks formal requirements for archive with solution of IFJ project."
  echo "         ARCHIVE - the filename of archive to check"
  echo "         TESTDIR - temporary directory that can be deleted/removed during testing!"
  exit 2
fi

declare -i ERRORS=0

# extrakce archivu
function unpack_archive () {
  local ext=`echo $1 | cut -d . -f 2,3`
  echo -n "Archive extraction: "
  RETCODE=100  
  if [[ "$ext" = "zip" ]]; then
    unzip -o $1 >> $LOG 2>&1
    RETCODE=$?
  elif [[ "$ext" = "gz" || "$ext" = "tgz" || "$ext" = "tar.gz" ]]; then
    tar xfz $1 >> $LOG 2>&1
    RETCODE=$? 
  elif [[ "$ext" = "tbz2" || "$ext" = "tar.bz2" ]]; then
    tar xfj $1 >> $LOG 2>&1
    RETCODE=$?   
  fi
  if [[ $RETCODE -eq 0 ]]; then
    echo_color green "OK"
  elif [[ $RETCODE -eq 100 ]]; then
    echo_color red "ERROR (unsupported extension)"
    let ERROR=ERROR+1
    exit 1
  else
    echo_color red "ERROR (code $RETCODE)"
    let ERROR=ERROR+1
    exit 1
  fi
} 

# flattening aktualniho adresare
function flattening () {
  local FILE=""
  local NFILE=""
  local FILES=`find . -name '*' -type f`
  local err=""
  for FILE in $FILES; do
    NFILE=./${FILE##*/}  # get the filename behind the last / in the path (or use basename) and add prefix "./"          
    if [ "$FILE" != "$NFILE" ]; then
      mv "$FILE" ${NFILE} 2>/dev/null
      echo_color red "ERROR ($FILE -> $NFILE)"
      let ERROR=ERROR+1
      err="yes"
    fi
    # 2019: zrusena podminka, ze musi byt jmena souboru jen malymi pismeny
    #F=`basename $FILE`
    #if [ "$F" != "Makefile" ]; then
    #  to_small ${NFILE}
    #fi
  done
  # Checking file collisions (when the name is case-insensitive as on Windows)
  DUPLICATED_FILENAMES=`ls | sort | uniq -d -i`
  if [ -n "$DUPLICATED_FILENAMES" ]; then
    echo_color red "ERROR - duplicated filesnames (case-insensitive):"
    echo $DUPLICATED_FILENAMES
    let ERROR=ERROR+1
    err="yes"
  fi
  if [ -z "$err" ]; then
    echo_color green "OK"
  fi
}

# stare odstraneni DOSovskych radku (nyni mozno pouzit i utilitu dos2unix)
function remove_CR () {
  FILES=`ls $* 2>/dev/null`
  for FILE in $FILES; do
    mv -f "$FILE" "$FILE.tmp"
    tr -d "\015" < "$FILE.tmp" > "$FILE"
    rm -f "$FILE.tmp"
	done
}

#   0) Priprava testdir a overeni serveru
if [[ -d $2 ]]; then
  read -p "Do you want to delete $2 directory? (y/n)" RESP
  if [[ $RESP = "y" ]]; then
    rm -rf $2 2>/dev/null
  else
    echo_color red "ERROR:" -n
    echo "User cancelled rewriting of existing directory."
    exit 1
  fi
fi
mkdir $2 2>/dev/null
cp $1 $2 2>/dev/null

echo -n "Testing on Merlin: "
HN=`hostname`
if [[ $HN = "merlin.fit.vutbr.cz" ]]; then
  echo_color green "Yes"
else
  echo_color blue "No"
fi


#   1) Extrahovat do testdir (kontrola jmena a velikosti archivu, dekomprimace)


cd $2
touch $LOG
ARCHIVE=`basename $1`
NAME=`echo $ARCHIVE | cut -d . -f 1 | egrep "^x[a-z]{5}[0-9][0-9a-z]$"`
echo -n "Archive name ($ARCHIVE): "
if [[ -n $NAME ]]; then
  echo_color green "OK"
else
  echo_color red "ERROR (the name $NAME does not correspond to a login)"
  let ERROR=ERROR+1
fi

#     Kontrola velikosti archivu
echo -n "Checking size of $ARCHIVE: "
ARCHIVE_SIZE=`du --bytes $ARCHIVE | cut -f 1`
if [[ ${ARCHIVE_SIZE} -ge ${MAX_ARCHIVE_SIZE} ]]; then 
  echo_color red "ERROR (Too big (${ARCHIVE_SIZE} bytes > ${MAX_ARCHIVE_SIZE} bytes)"
  let ERROR=ERROR+1
else 
  echo_color green "OK" 
fi

unpack_archive ${ARCHIVE}

#   2) Normalizace (vybaleni z nepovolenych adresaru), kontrola binarek v archivu
echo -n "Checking binary files: "
ELFS=`find . -type f -exec file '{}' \; | grep "ELF" | perl -nle 'split /:/;print $_[0]'`
if [[ -n $ELFS ]]; then
  echo_color red "ERROR (archive contains binary file(s): $ELFS)"
  let ERROR=ERROR+1
else
  echo_color green "OK" 
fi 

echo -n "Normalization of filenames: "
flattening

#   3) Dokumentace
echo -n "Searching for dokumentace.pdf: "
if [[ -f "dokumentace.pdf" ]]; then
  echo_color green "OK"
else
  echo_color red "ERROR (not found)"
  let ERROR=ERROR+1
fi

#   4) Priprava kompilace
remove_CR *.mak *.c *.cpp *.cc *.h *.c++ *.hpp
chmod 644 *

echo -n "Project compilation: "
#   5) Kompilace
if [[ -f Makefile ]]; then
  ( make ) >> $LOG 2>&1
  RETCODE=$?
  if [[ -z $RETCODE ]]; then
    echo_color red "ERROR (returns code $RETCODE)"
    let ERROR=ERROR+1
  else
    echo_color green "OK"
  fi
else
  echo_color red "ERROR (missing Makefile)"
  let ERROR=ERROR+1 
fi

#    6) Najdi prelozeny binarni soubor
echo -n "Searching for created binary file: "
EXE=`ls -F | grep "*" | tr -d "*" | grep "" -m 1`   # A najít binárku...
if [[ -f $EXE ]]; then
  echo_color green "OK ($EXE)"
else
  echo_color red "ERROR (not found)"
  let ERROR=ERROR+1
fi  

#    7) Kontrola, ze nebyl vytvoren podadresar
echo -n "Searching for new subdirectories: "
DIR_COUNT=`find -type d | grep -v "^\.$" | wc -l`
if [[ $DIR_COUNT -eq 0 ]]; then
  echo_color green "OK (None)"
else
  echo_color red "ERROR (found $DIR_COUNT subdirectory/ies)"
  let ERROR=ERROR+1
fi

#    8) Kontrola rozdeleni
echo -n "Presence of file rozdeleni: "
IFS="$IFS:"
if [[ -f rozdeleni ]]; then

  # zpracovani souboru rozdeleni
  unset LOGINS
  unset PERCENTS
  unset ARCHNAME
  declare -a LOGINS
  {
    i=0
    while read -a RADEK; do
      if [[ "${RADEK[0]}" != "" ]]; then
        LOGINS[$i]=${RADEK[0]}
        PERCENTS[$i]=`echo ${RADEK[1]} | tr -cd [:digit:]`
        BADLF=`echo ${RADEK[1]} | egrep "<LF>$"`
        if [[ -n $BADLF ]]; then
          echo_color red "ERROR (bad format: entry $i should be finished by Unix line feed, not string \"<LF>\")"
          let ERROR=ERROR+1
        fi
        ((TMP_PROC+=${PERCENTS[$i]:-0}))
        ((i++))
        if [[ "$NAME" = "${RADEK[0]}" ]]; then
          ARCHNAME=$NAME
        fi
        LOGINCHECK=`echo ${RADEK[0]} | egrep "x[a-z]{5}[0-9][0-9a-z]$"`
        if [[ -z $LOGINCHECK ]]; then
          echo_color red "ERROR (bad login format of ${RADEK[0]})"
          let ERROR=ERROR+1
        fi
      else
        echo_color red "ERROR (empty line occured)"
        let ERROR=ERROR+1
      fi
    done
  } < rozdeleni
  
  # kontrola formatu rozdeleni a souctu procent
  if [[ -n $RADEK ]]; then
    echo_color red "ERROR (the last line is not ended properly)"
    let ERROR=ERROR+1
  elif [[ $TMP_PROC -ne 100 ]]; then
    echo_color red "ERROR (sum != 100%)"
    let ERROR=ERROR+1
  elif [[ -z $ARCHNAME ]]; then
    echo_color red "ERROR (rozdeleni does not contain the leader's login $NAME)"
    let ERROR=ERROR+1
  else
    echo_color green "OK"
  fi

else
  echo_color red "ERROR (file not found)"
  let ERROR=ERROR+1
fi

#   9) Kontrola rozsireni
echo -n "Presence of file rozsireni (optional): "
if [[ -f rozsireni ]]; then
  echo_color green "Yes"
  echo -n "Unix end of lines in rozsireni: "
  if command -v dos2unix >/dev/null 2>&1; then
      dos2unix -n rozsireni rozsireni.lf >> $LOG 2>&1
  else
      tr -d '\r' < rozsireni > rozsireni.lf 2>&1
  fi
  diff rozsireni rozsireni.lf >> $LOG 2>&1
  RETCODE=$?
  if [[ $RETCODE = "0" ]]; then
    UNKNOWN=`cat rozsireni | grep -v -E -e "^(STRNUM|BOOLTHEN|CYCLES|FUNEXP|GLOBAL)$" | wc -l`
    if [[ $UNKNOWN = "0" ]]; then
      echo_color green "OK" 
    else
      echo_color red "ERROR (Unknown bonus identifier or redundant empty line)"
      let ERROR=ERROR+1
    fi
  else
    echo_color red "ERROR (CRLFs)"
    let ERROR=ERROR+1
  fi
else
  echo_color blue "No"
fi 

#   Kontrola adresare __MACOSX
if [[ -d __MACOSX ]]; then
  echo_color blue "Archive ($ARCHIVE) should not contain __MACOSX directory!"
  let ERROR=ERROR+1
fi

echo -n "ALL CHECKS COMPLETED"
if [[ $ERROR -eq 0 ]]; then
  echo_color green " WITHOUT ERRORS!"
elif [[ $ERROR -eq 1 ]]; then
  echo_color red " WITH $ERROR ERROR!"    
else
  echo_color red " WITH $ERROR ERRORS!"
fi
