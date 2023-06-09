#! /bin/bash

#APL N. 1
#SCRIPT 6
#INTEGRANTES:
      #Carballo, Facundo Nicolas (DNI: 42774931)
      #Garcia Burgio, Matias Nicolas (DNI: 42649117)
      #Mottura, Agostina Micaela (DNI: 41898101)
      #Povoli Olivera, Victor (DNI: 43103780)
      #Szust, Ángel Elías (DNI: 43098495)



#crear mi propia papelera de reciclaje


ayuda(){
    echo "Sintaxis : $0 ACCION [archivo/dir]"
    echo "Acciones : "
    echo "--listar: Muestra el contenido de la papelera de reciclaje"
    echo "--eliminar [archivo/dir]: Envia a la papelera el archivo/directorio pasado por parametro"
    echo "--borrar [archivo/dir]: Elimina de la papelera (definitivamente) al archivo o directorio pasado por parametro. El mismo debe estar en la papelera"
    echo "--vaciar: Vacia la papelera"
    echo "Ejemplo: $0 --eliminar ../archivo"
}

if [[ $1 = "-h" || $1 = "--help" || $1 = "-?" ]]; then
    ayuda
    exit 0
fi

if [[ $# -eq 0 ]]; then
    echo "Error: No se ingreso ningun parametro"
    echo "Ingrese -h para ver la ayuda"
    exit 1
fi
> script.log
papeleraZip="/home/`whoami`/papeleraAMAVF.zip"
papeleraPath="/home/`whoami`/papeleraAMAVF"
logPath="/home/`whoami`/papeleraAMAVF/logPapeleraAMAVF.txt" #iniciales del grupo, para chequear que sea mi papelera
#funciones para cada opcion


listar(){ #listo
    var=`cat $logPath | awk 'BEGIN{FS="  ";OFS=" "} {printf  "%-14s|  %s\n" ,$1,$2}'`
    if [[ $var = "" ]]; then
        echo "La papelera esta vacia"
    else
        echo "Contenido de la papelera: "
        echo "Nombre        |  Direccion de Origen"
        echo "-----------------------------------------"
        echo "$var"
        echo "-----------------------------------------"
        #cat $logPath | awk 'BEGIN{FS="  ";OFS=" "} {print $1,$2}'
    fi

}


getNewName(){
    #if i do have a second argument
    nom="$1"
    num="$2"
    if [[ $2 != "" ]]; then
        awk -v var="$nom" 'BEGIN{FS="  ";OFS="  "}{if(var == $1) print}' $logPath | awk -v var=$num 'BEGIN{FS="  ";OFS="  "}{if(NR == var) printf "%s|%d" , $1, $3}'
    else
        awk -v var="$nom" 'BEGIN{FS="  ";OFS="  "}{if(var == $1) print}' $logPath | awk 'BEGIN{FS="  ";OFS="  "} {printf "%s|%d" , $1, $3}'
    fi
}

getID(){
    nom="$1"
    num="$2"
    if [[ $2 != "" ]]; then
        awk -v var="$nom" 'BEGIN{FS="  ";OFS="  "}{if(var == $1) print}' $logPath | awk -v var=$num 'BEGIN{FS="  ";OFS=" "}{if(NR == var) printf "%d" , $3}'
    else
        awk -v var="$nom" 'BEGIN{FS="  ";OFS="  "}{if(var == $1) print}' $logPath | awk 'BEGIN{FS="  ";OFS=" "} {printf "%d" , $3}'
    fi
}

getAppearances(){
    awk 'BEGIN{FS="  "; OFS="  "}{print $1}' $logPath | awk -v var="$1" 'BEGIN{FS="  ";OFS="  "}{if(var == $1) print var}' | awk 'END{print NR}'
}

getPath(){
    if [[ $2 != "" ]]; then
        awk -v var="$1" 'BEGIN{FS="  ";OFS="  "}{if(var == $1) print}' $logPath | awk -v var=$2 'BEGIN{FS="  ";OFS="  "}{if(NR == var) printf "%s" , $2}'
    else
        awk -v var="$1" 'BEGIN{FS="  ";OFS="  "}{if(var == $1) print}' $logPath | awk 'BEGIN{FS="  ";OFS="  "} {printf "%s" , $2}'
    fi
    
}

showOptions(){
    awk -v var="$1" 'BEGIN{FS="  ";OFS="  "}{if(var == $1) print}' $logPath | awk 'BEGIN{FS="  ";OFS="  "} {printf "%d - %s %s\n" , NR, $1, $2}'
}

recuperar(){ #listo

    app=$(getAppearances "$1")
    

    if [[ $app -gt 1 ]]; then
        showOptions "$1"
        echo "Cual queres recuperar?"
        read archNum
        nuevoNombre=$(getNewName "$1" $archNum)
        IDarch=$(getID "$1" $archNum)
        mv $papeleraPath/"$nuevoNombre" $papeleraPath/"$1" 

        #si es un file
        if [[ -f $papeleraPath/"$1" ]]; then
            cp $papeleraPath/"$1" "$(getPath "$1" $archNum)"
            rm $papeleraPath/"$1" 
        #si es un dir"
        elif [[ -d $papeleraPath/"$1" ]]; then 
            cp -r $papeleraPath/"$1" "$(getPath "$1" $archNum)"
            rm -r $papeleraPath/"$1" 
        fi

        sed -i "/$IDarch/d" $logPath


    elif [[ $app -eq 1 ]]; then
        nuevoNombre=$(getNewName "$1") #consigo el nuevo nombre del archivo dentro de la papelera
        mv $papeleraPath/"$nuevoNombre" $papeleraPath/"$1" #le cambio el nombre al original
        IDarch=$(getID "$1") 

        #si es un file
        if [[ -f $papeleraPath/"$1" ]]; then
            cp $papeleraPath/"$1" "$(getPath "$1")" 
            rm $papeleraPath/"$1" 
        #si es un dir
        elif [[ -d $papeleraPath/"$1" ]]; then 
            cp -r $papeleraPath/"$1" "$(getPath "$1")" 
            rm -r $papeleraPath/"$1"
        fi

        sed -i "/$IDarch/d" $logPath    #borro la linea del log
    else
        echo "El archivo $1 no se encuentra en la papelera de reciclaje"
        return 1
    fi
}

eliminar(){ #listo
    if [[ -d "$1" ]]; then
        cp -r "$1" $papeleraPath
    else
        cp "$1" $papeleraPath
    fi
    pathArchivo="$(realpath "$1")"
    nombreArchivo="$(basename "$1")"
    dateRand=$(date +%s)$RANDOM
    mv $papeleraPath/"$nombreArchivo" "$papeleraPath/$nombreArchivo|$dateRand" #le cambio el nombre al archivo para que no se repita
    echo ""$nombreArchivo"  "$(dirname "$pathArchivo")"  $dateRand" >> $logPath
    if [[ -d "$1" ]]; then
        rm -r "$1"
    else
        rm "$1"
    fi
}

borrar(){
    app=$(getAppearances "$1")

    if [[ $app -gt 1 ]]; then
        showOptions "$1"
        echo "Cual queres borrar?"
        read archNum
        nuevoNombre=$(getNewName "$1" $archNum)
        IDarch=$(getID "$1" $archNum)
        mv $papeleraPath/"$nuevoNombre" $papeleraPath/"$1" 

        #si es un file
        if [[ -f $papeleraPath/"$1" ]]; then
            rm $papeleraPath/"$1" 
        #si es un dir
        elif [[ -d $papeleraPath/"$1" ]]; then 
            rm -r $papeleraPath/"$1" 
        fi

        sed -i "/$IDarch/d" $logPath


    elif [[ $app -eq 1 ]]; then
        nuevoNombre=$(getNewName "$1") 
        mv $papeleraPath/"$nuevoNombre" $papeleraPath/"$1" 
        IDarch=$(getID "$1")

        #si es un file
        if [[ -f $papeleraPath/"$1" ]]; then
            rm $papeleraPath/"$1" 
        #si es un dir
        elif [[ -d $papeleraPath/"$1" ]]; then 
            rm -r $papeleraPath/"$1" 
        fi

        sed -i "/$IDarch/d" $logPath    #borro la linea del log
    else
        echo "El archivo $1 no se encuentra en la papelera de reciclaje"
        return 1
    fi
}
vaciar(){ #listo
    if [[ `cat $logPath | cut -d " " -f -2` = "" ]]; then
        echo "La papelera ya esta vacia"
        return;
    fi
    rm -r $papeleraPath
    mkdir $papeleraPath
    > $logPath
    echo "Papelera vaciada"
}

zipHandler(){
    if [[ -f $papeleraZip ]]; then
        rm $papeleraZip
    fi
    #sleep 5
    cd $papeleraPath

    zip -r -0 $papeleraZip * >> /dev/null

    rm -r $papeleraPath

}

if [[ -d $papeleraZip ]]; then
    rm -r $papeleraZip
fi

if [[ -d $papeleraPath ]]; then
    rm -r $papeleraPath
fi

#si no existe la papelera, la creo
if [[ -f $papeleraZip ]]
then
    unzip -q $papeleraZip -d $papeleraPath >> /dev/null
    if [[ -f $logPath ]]; then
        echo "La papelera ya existe" >> script.log
    else
        echo "La papelera existente no fue creada con este script"
        rm $papeleraZip
        if [[ -d $papeleraPath ]]; then
            rm -r $papeleraPath
        fi
        mkdir $papeleraPath
        touch $logPath
        zipHandler
        exit 1
    fi
else
    mkdir $papeleraPath
    echo "La papelera se ha creado" >> script.log
fi

#si no existe el log, lo creo
if [[ -f $logPath ]]
then
    echo "El log ya existe" >> script.log
else
    touch $logPath
    echo "El log se ha creado" >> script.log
fi

#las opciones son --listar, --vaciar, --recuperar, --eliminar, --borrar

if ! [[ $1 = "--listar" || $1 = "--vaciar" || $1 = "--recuperar" || $1 = "--eliminar" || $1 = "--borrar" ]]; then
    echo "Opcion invalida" 
    zipHandler
    exit 1
fi

if [[ $2 != "" ]]; then
    ruta="$2"
fi


#si la opcion es listar, vaciar, recuperar, eliminar o borrar, ejecuto la funcion correspondiente
if [[ $1 = "--listar" ]]; then #sin params
    listar
elif [[ $1 = "--vaciar" ]]; then #sin params
    vaciar
elif [[ $1 = "--recuperar" ]]; then #param: archivo
    if [[ $ruta != "" ]]; then
        recuperar "$ruta"
        if [[ $? -eq 0 ]]; then
            echo "El archivo "$ruta" se ha recuperado" 
        else
            zipHandler
            exit 1
        fi
        
    else
        echo "Debe proporcionar un archivo/directorio a recuperar"
        echo "Para mas informacion, ejecute el script con la opcion -h"
        zipHandler
        exit 1;
    fi
elif [[ $1 = "--eliminar" ]]; then #param: archivo
    if [[ -f $ruta || -d $ruta ]]; then
        eliminar "$ruta"
        echo "El archivo $ruta se ha eliminado" 
    elif [[ $ruta = "" ]]; then
        echo "Debe proporcionar un archivo/directorio a eliminar"
        echo "Para mas informacion, ejecute el script con la opcion -h"
        zipHandler
        exit 1;
    else
        echo "$ruta no es un parametro valido"
        echo "Para mas informacion, ejecute el script con la opcion -h"
        zipHandler
        exit 1;
    fi
elif [[ $1 = "--borrar" ]]; then #param: archivo
    if [[ $ruta != "" ]]; then
        borrar "$ruta"
        if [[ $? -eq 0 ]]; then
            echo "El archivo "$ruta" se ha borrado de la papelera de reciclaje" 
        else
            zipHandler
            exit 1;
        fi

    else
        echo "Debe proporcionar un archivo/directorio a borrar"
        echo "Para mas informacion, ejecute el script con la opcion -h"
        zipHandler
        exit 1;
    fi
fi

zipHandler

