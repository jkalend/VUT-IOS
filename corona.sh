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

declare -A COUNTRIES=()

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
    5) 
    daily
    ;;
    6)
    monthly
    ;;
    7)
    yearly
    ;;
    8)
    countries
    for c in "${!COUNTRIES[@]}"; do
        echo "$c: ${COUNTRIES[$c]}"
    done
    ;;
    9)
    districts
    for c in "${!COUNTRIES[@]}"; do
        echo "$c: ${COUNTRIES[$c]}"
    done
    ;;
    10) 
    regions
    for c in "${!COUNTRIES[@]}"; do
        echo "$c: ${COUNTRIES[$c]}"
    done
    ;;
    *) 
    echo "ERROR: Unknown command"
    exit 1;;

    esac 

}

none () {
    if [[ $file != "" && $COMMAND == 0 ]];
    then
    echo "ERROR" # TODO
    exit 1
    fi
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
        COUNT=$(grep -c ^)
        COUNT=$((COUNT-1))

    else 
        COUNT=$(grep -c ^ $file)
        COUNT=$((COUNT-1))
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
            if [ "${string[3]}" == "pohlavi" ]; then
            continue
            fi

            if [ "${string[3]}" == "Z" ]; then
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
                    AGE[$ORD]=$((AGE[ORD]+1))
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
                    AGE[$ORD]=$((AGE[ORD]+1))
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

daily () { 
    # supports only 1 file
    DAY=0
    MONTH=0
    YEAR=0

    if [[ $file == "" ]];
    then
        while read line; do  

            IFS=','
            read -a string <<< $line
            if [ "${string[1]}" == "datum" ]; then
            continue
            fi

            IFS='-'
            read -a date <<< "${string[1]}"
            if [ $YEAR == 0 ]; then
                YEAR=${date[0]}
                MONTH=${date[1]}
                DAY=${date[2]}
            fi

            if [ $YEAR == ${date[0]} ] && [ $MONTH == ${date[1]} ] && [ $DAY == ${date[2]} ]; then
                OUT=$((OUT + 1))
            else
                echo "$YEAR-$MONTH-$DAY: $OUT"
                OUT=1
                YEAR=${date[0]}
                MONTH=${date[1]}
                DAY=${date[2]}
            fi

        done
    else
        while read line; do  
        
            read -a string <<< $line
            if [ "${string[1]}" == "datum" ]; then
            continue
            fi

            read -a date <<< "${string[1]}"
            if [ $YEAR == 0 ]; then
                YEAR=${date[0]}
                MONTH=${date[1]}
                DAY=${date[2]}
            fi

            if [ $YEAR == ${date[0]} ] && [ $MONTH == ${date[1]} ] && [ $DAY == ${date[2]} ]; then
                OUT=$((OUT + 1))
            else
                echo "$YEAR-$MONTH-$DAY: $OUT"
                OUT=1
                YEAR=${date[0]}
                MONTH=${date[1]}
                DAY=${date[2]}
            fi

        done <$file
    fi
    echo "$YEAR-$MONTH-$DAY: $OUT"
}

monthly () { 
    MONTH=0
    YEAR=0

    if [[ $file == "" ]];
    then
        while read line; do  

            IFS=','
            read -a string <<< $line
            if [ "${string[1]}" == "datum" ]; then
            continue
            fi

            IFS='-'
            read -a date <<< "${string[1]}"
            if [ $YEAR == 0 ]; then
                YEAR=${date[0]}
                MONTH=${date[1]}
            fi

            if [ $YEAR == ${date[0]} ] && [ $MONTH == ${date[1]} ]; then
                OUT=$((OUT + 1))
            else
                echo "$YEAR-$MONTH: $OUT"
                OUT=1
                YEAR=${date[0]}
                MONTH=${date[1]}
            fi

        done
    else
        while read line; do  
        
            IFS=','
            read -a string <<< $line
            if [ "${string[1]}" == "datum" ]; then
            continue
            fi

            IFS='-'
            read -a date <<< "${string[1]}"
            if [ $YEAR == 0 ]; then
                YEAR=${date[0]}
                MONTH=${date[1]}
            fi

            if [ $YEAR == ${date[0]} ] && [ $MONTH == ${date[1]} ]; then
                OUT=$((OUT + 1))
            else
                echo "$YEAR-$MONTH: $OUT"
                OUT=1
                YEAR=${date[0]}
                MONTH=${date[1]}
            fi

        done <$file
    fi
    echo "$YEAR-$MONTH: $OUT"
}

yearly() {
    YEAR=0

    if [[ $file == "" ]];
    then
        while read line; do  

            IFS=','
            read -a string <<< $line
            if [ "${string[1]}" == "datum" ]; then
            continue
            fi

            IFS='-'
            read -a date <<< "${string[1]}"
            if [ $YEAR == 0 ]; then
                YEAR=${date[0]}
            fi

            if [ $YEAR == ${date[0]} ]; then
                OUT=$((OUT + 1))
            else
                echo "$YEAR: $OUT"
                OUT=1
                YEAR=${date[0]}
            fi

        done
    else
        while read line; do  
        
            IFS=','
            read -a string <<< $line
            if [ "${string[1]}" == "datum" ]; then
            continue
            fi

            IFS='-'
            read -a date <<< "${string[1]}"
            if [ $YEAR == 0 ]; then
                YEAR=${date[0]}
            fi

            if [ $YEAR == ${date[0]} ]; then
                OUT=$((OUT + 1))
            else
                echo "$YEAR: $OUT"
                OUT=1
                YEAR=${date[0]}
            fi

        done <$file
    fi
    echo "$YEAR: $OUT"
}

countries() {
    CHECKED=""
    CHECK=0
    #COUNTRY=""
    #DATA=$(cat -)

    if [[ $file == "" ]];
    then
        while read line; do 

            #echo "${COUNTRIES[@]}"
            IFS=','
            read -a string <<< $line
            if [ "${string[1]}" == "datum" ]; then
            continue
            fi

            read -a CNT <<< $CHECKED
            if [ "${string[6]}" != 1 ]; then
                continue
            else
                CHECK=0
                for c in "${CNT[@]}"
                do
                    if [ $c == ${string[7]} ]; then
                        CHECK=1
                    fi
                done

                if [ $CHECK -eq 0 ]; then
                    CHECKED+="${string[7]}"
                    CHECKED+=","
                fi

                ADDED=false
                for d in "${!COUNTRIES[@]}"
                do
                    if [ $d == ${string[7]} ]; then
                        COUNTRIES[$d]=$((COUNTRIES[$d] + 1))
                        ADDED=true
                        break
                    fi
                done

                if [ $ADDED == false ]; then
                    COUNTRIES[${string[7]}]=1
                fi

            fi

        done
    else
        while read line; do  
        
            IFS=','
            read -a string <<< $line
            if [ "${string[1]}" == "datum" ]; then
            continue
            fi

            read -a CNT <<< $CHECKED
            if [ "${string[6]}" != 1 ]; then
                continue
            else
                CHECK=0
                for c in "${CNT[@]}"
                do
                    if [ $c == ${string[7]} ]; then
                        CHECK=1
                    fi
                done

                if [ $CHECK -eq 0 ]; then
                    CHECKED+="${string[7]}"
                    CHECKED+=","
                fi

                ADDED=false
                for d in "${!COUNTRIES[@]}"
                do
                    if [ $d == ${string[7]} ]; then
                        COUNTRIES[$d]=$((COUNTRIES[$d] + 1))
                        ADDED=true
                        break
                    fi
                done

                if [ $ADDED == false ]; then
                    COUNTRIES[${string[7]}]=1
                fi

            fi
        done <$file
    fi
}

districts () {
    CHECKED=""
    CHECK=0
    #COUNTRY=""
    #DATA=$(cat -)

    if [[ $file == "" ]];
    then
        while read line; do 

            #echo "${COUNTRIES[@]}"
            IFS=','
            read -a string <<< $line
            if [ "${string[1]}" == "datum" ]; then
            continue
            fi

            read -a CNT <<< $CHECKED
            if [ "${string[5]}" == "" ]; then
                continue
            else
                CHECK=0
                for c in "${CNT[@]}"
                do
                    if [ $c == ${string[5]} ]; then
                        CHECK=1
                    fi
                done

                if [ $CHECK -eq 0 ]; then
                    CHECKED+="${string[5]}"
                    CHECKED+=","
                fi

                ADDED=false
                for d in "${!COUNTRIES[@]}"
                do
                    if [ $d == ${string[5]} ]; then
                        COUNTRIES[$d]=$((COUNTRIES[$d] + 1))
                        ADDED=true
                        break
                    fi
                done

                if [ $ADDED == false ]; then
                    COUNTRIES[${string[5]}]=1
                fi

            fi

        done
    else
        while read line; do  
        
            IFS=','
            read -a string <<< $line
            if [ "${string[1]}" == "datum" ]; then
            continue
            fi

            read -a CNT <<< $CHECKED
            if [ "${string[5]}" == "" ]; then
                continue
            else
                CHECK=0
                for c in "${CNT[@]}"
                do
                    if [ $c == ${string[5]} ]; then
                        CHECK=1
                    fi
                done

                if [ $CHECK -eq 0 ]; then
                    CHECKED+="${string[5]}"
                    CHECKED+=","
                fi

                ADDED=false
                for d in "${!COUNTRIES[@]}"
                do
                    if [ $d == ${string[5]} ]; then
                        COUNTRIES[$d]=$((COUNTRIES[$d] + 1))
                        ADDED=true
                        break
                    fi
                done

                if [ $ADDED == false ]; then
                    COUNTRIES[${string[5]}]=1
                fi

            fi
        done <$file
    fi
}

regions () {
    CHECKED=""
    CHECK=0
    #COUNTRY=""
    #DATA=$(cat -)

    if [[ $file == "" ]];
    then
        while read line; do 

            #echo "${COUNTRIES[@]}"
            IFS=','
            read -a string <<< $line
            if [ "${string[1]}" == "datum" ]; then
            continue
            fi

            read -a CNT <<< $CHECKED
            if [ "${string[4]}" == "" ]; then
                continue
            else
                CHECK=0
                for c in "${CNT[@]}"
                do
                    if [ $c == ${string[4]} ]; then
                        CHECK=1
                    fi
                done

                if [ $CHECK -eq 0 ]; then
                    CHECKED+="${string[4]}"
                    CHECKED+=","
                fi

                ADDED=false
                for d in "${!COUNTRIES[@]}"
                do
                    if [ $d == ${string[4]} ]; then
                        COUNTRIES[$d]=$((COUNTRIES[$d] + 1))
                        ADDED=true
                        break
                    fi
                done

                if [ $ADDED == false ]; then
                    COUNTRIES[${string[4]}]=1
                fi

            fi

        done
    else
        while read line; do  
        
            IFS=','
            read -a string <<< $line
            if [ "${string[1]}" == "datum" ]; then
            continue
            fi

            read -a CNT <<< $CHECKED
            if [ "${string[4]}" == "" ]; then
                continue
            else
                CHECK=0
                for c in "${CNT[@]}"
                do
                    if [ $c == ${string[4]} ]; then
                        CHECK=1
                    fi
                done

                if [ $CHECK -eq 0 ]; then
                    CHECKED+="${string[4]}"
                    CHECKED+=","
                fi

                ADDED=false
                for d in "${!COUNTRIES[@]}"
                do
                    if [ $d == ${string[4]} ]; then
                        COUNTRIES[$d]=$((COUNTRIES[$d] + 1))
                        ADDED=true
                        break
                    fi
                done

                if [ $ADDED == false ]; then
                    COUNTRIES[${string[4]}]=1
                fi

            fi
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
