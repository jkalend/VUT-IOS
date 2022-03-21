#!/bin/bash

POSIXLY_CORRECT=yes

file=""
args=("$@")
COMMAND=0
OUT=0
COUNT=1212112212121212121212
declare -a AGE=(
        [0]=0
        [1]=0
        [2]=0
        [3]=0
        [4]=0
        [5]=0
        [6]=0
        [7]=0
        [8]=0
        [9]=0
        [10]=0
        [11]=0
    )


declare -A COMMANDS=(
    [infected]=1
    [merge]=2
    [gender]=3
    [age]=4
    [daily]=5
    [monthly]=6
    [yearly]=7
    [countries]=8
    [districts]=9
    [regions]=10
)

declare -A FILTERS=(
    [after]=""
    [before]=""
    [gender]=""
    [width]=-1
)

cross () { 
    case $COMMAND in
    0) none
    ;;
    1)
    infected
    OUT=$(($COUNT + OUT))
    if [ $PRINT == true ]; then
    echo $OUT
    fi
    
    ;;
    2) 
    exit 1
    ;;
    3) 
    gender
    if [ $PRINT == true ]; then
    echo "M: $MUZI"
    echo "Z: $ZENY"
    fi

    ;;
    4) 
    age
    if [ $PRINT == true ]; then
    echo "0-5   : ${AGE[0]}" 
    echo "6-15  : ${AGE[1]}"
    echo "16-25 : ${AGE[2]}"
    echo "26-35 : ${AGE[3]}"
    echo "36-45 : ${AGE[4]}"
    echo "46-55 : ${AGE[5]}"
    echo "56-65 : ${AGE[6]}"
    echo "66-75 : ${AGE[7]}"
    echo "76-85 : ${AGE[8]}"
    echo "86-95 : ${AGE[9]}"
    echo "96-105: ${AGE[10]}"
    echo ">105  : ${AGE[11]}" 
    fi
    
    ;;
    5) exit 1;;
    6) exit 1;;
    7) exit 1;;
    8) exit 1;;
    9) exit 1;;
    10) exit 1;;
    *) exit 1 ;;

    esac 

}

none () { 
    if [[ $file == "" ]]; 
    then
   
        while read line; do  
        echo $line
        done

    else 

        while read line; do  
        echo $line
        done <$file

    fi
}

infected () {
    if [[ $file == "" ]]; 
    then
        COUNT=-1 
    
        while read line; do  
    
        COUNT=$((COUNT+1))
        done

    else
        COUNT=-1 
    
        while read line; do  
    
        COUNT=$((COUNT+1))
        done <$file

    fi

}

gender () { 
    IFS=','
    MUZI=0
    ZENY=0

    if [[ $file == "" ]];
    then
        while read line; do  
        
            read -a string <<< $line
            if [ ${string[3]} == "pohlavi" ]; then
            continue
            fi

            if [ ${string[3]} == "Z" ]; then
            ZENY=$((ZENY+1))
            else
            MUZI=$((MUZI+1))
            fi
        done
    else
        while read line; do  
        
            read -a string <<< $line
            if [ ${string[3]} == "pohlavi" ]; then
            continue
            fi

            if [ ${string[3]} == "Z" ]; then
            ZENY=$((ZENY+1))
            else
            MUZI=$((MUZI+1))
            fi
        done <$file
    fi
}

age () {
    

    

    IFS=','
    local LOW=0
    local HIGH=5
    local ORD=0

    if [[ $file == "" ]];
    then
        while read line; do  
        
            read -a string <<< $line
            if [ ${string[2]} == "vek" ]; then
            continue
            fi

            if [ ${string[2]} -gt 105 ]; then
                AGE[11]=$((AGE[11]+1))
                break
            fi

            while ((HIGH <= 105)); do
                if ((string[2] >= LOW && string[2] <= HIGH)); then
                    AGE[$ORD]=$((AGE[$ORD]+1))
                    break
                else
                    if [ $LOW -eq 0 ]; then
                        LOW=$((LOW+6))
                    else
                        LOW=$((LOW+10))
                    fi
                    HIGH=$((HIGH+10))
                    ORD=$((ORD+1))
                fi

            done

            LOW=0
            HIGH=5
            ORD=0

        done
    else
        while read line; do  
        
            read -a string <<< $line
            if [ ${string[2]} == "vek" ]; then
            continue
            fi

            if [ ${string[2]} -gt 105 ]; then
                AGE[11]=$((AGE[11]+1))
                break
            fi

            while ((HIGH <= 105)); do
                if ((string[2] >= LOW && string[2] <= HIGH)); then
                    AGE[$ORD]=$((AGE[$ORD]+1))
                    break
                else
                    if [ $LOW -eq 0 ]; then
                        LOW=$((LOW+6))
                    else
                        LOW=$((LOW+10))
                    fi
                    HIGH=$((HIGH+10))
                    ORD=$((ORD+1))
                fi

            done

            LOW=0
            HIGH=5
            ORD=0

        done <$file
    fi
}




#look for help 
while getopts :ha:b:g:s: options; do
        case $options in
                h) 
                echo "HELP"
                exit 1
                ;;

                a)
                if [ "${FILTERS[after]}" != "" ];
                then
                echo "Error: Too many flags of the same type" >&2
                exit 1
                fi

                FILTERS[after]=$OPTARG
                ;;

                b) 
                if [ "${FILTERS[before]}" != "" ];
                then
                echo "Error: Too many flags of the same type" >&2
                exit 1
                fi
                
                FILTERS[before]=$OPTARG
                ;;

                g) 
                if [ "${FILTERS[before]}" != "" ];
                then
                echo "Error: Too many flags of the same type" >&2
                exit 1
                fi
                
                FILTERS[after]=$OPTARG
                ;;

                s) 
                if [ "${FILTERS[before]}" != "" ];
                then
                echo "Error: Too many flags of the same type" >&2
                exit 1
                fi

                FILTERS[after]=$OPTARG
                ;;

                ?) 
                echo "Unknown option: $OPTARG"
                exit 1
                ;;
        esac
done

for i in "${args[@]}"
do
    PRINT=false

    if [[ ${args[$#-1]} == $i ]]; 
    then
    PRINT=true
    fi

    if ! [ -f $i ];
    then
    for j in "${!COMMANDS[@]}"
    do
        if [[ $j == $i  &&  $COMMAND -eq 0 ]];
        then
            #echo $j
            COMMAND=${COMMANDS[$j]}
            #echo $COMMAND
            break
        fi

        if ! [ $COMMAND -eq 0 ];
        then
            echo "Too many COMMANDS, maximum 1 allowed" >&2
            exit 1
        fi
    done
    fi

    if [ -f $i ];
    then
        file=$i
        cross
    fi
done

if [[ $file == "" ]]; 
then
cross
fi



#infected $file
#OUT=$COUNT
#echo $OUT




#while read line; do  
  
#Readind each line in sequence  
#echo $line  
#done <$file
